#include "protocol/build.h"
#include "protocol/protocol.h"
#include <argp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct argp_option argument_options[] = {
  {"disable_debug_logs", 'd', NULL_ZERO, NULL_ZERO, "DO NOT print debug information in logs"},
  {"print_stack_logs", 's', NULL_ZERO, NULL_ZERO, "print stack frame information to the logs"},
  {"ip", 't', "[4,6]", NULL_ZERO, "Valid ip type values are '4' for ipv4 or '6' for ipv6"},
  {"interface", 'i', "<interface name>", NULL_ZERO, "Interface to be bound to, defaults to 'loopback'. e.g. --interface=wlp2s0"},
  {"port", 'p', "<port number>", NULL_ZERO, "Port number to use, defaults to 5665"},
  {0},
};

#define INVAL_VAL(__x, __arg)                                                                                                    \
    "Given value '%s' is unknown for option '" __x "', use '--help' or '--usage' option for help.\n", __arg

error_t
argument_parser (int key, char *arg, struct argp_state *state)
{
    settings_s *settings = (settings_s *) state->input;

    switch (key) {
        case 'd': {
            settings->log_settings.print_debug_logs = FALSE;
            break;
        }
        case 's': {
            settings->log_settings.print_stack_frames = TRUE;
            break;
        }
        case 't': {
            if (*arg == '4' || *arg == '6') {
                settings->ip_settings.address_type = *arg == '4' ? AF_INET : AF_INET6;
            } else {
                printf (INVAL_VAL ("-i", arg));
                exit (EXIT_FAILURE);
            }
            break;
        }
        case 'i': {
            settings->ip_settings.interface = arg;
            break;
        }
        case 'p': {
            char *endptr;
            long port = strtol (arg, &endptr, 10);
            if ((port == 0 && errno != 0) || arg == endptr) {
                printf (INVAL_VAL ("-p", arg));
                exit (EXIT_FAILURE);
            }

            if (port < 1024 || port > 49151) {
                printf ("Port number is in invalid range, available ports are between [1024, 49151] inclusive\n");
                exit (EXIT_FAILURE);
            }

            settings->ip_settings.port = (ushort) port;
            break;
        }
        case ARGP_KEY_ARG: {
            return 0;
        }
        default: {
            return ARGP_ERR_UNKNOWN;
        }
    }
    return 0;
}

struct argp argp = {argument_options, argument_parser, APPLICATION_NAME " server v" __SERVER_VERSION__, NULL, NULL, NULL};

void
print_banner ()
{
    printf ("=====================================\n");
    printf ("s3 server version %s\n", __SERVER_VERSION__);
    printf ("%s\n", __SERVER_INFO__);
    printf ("=====================================\n");
}

void
sigint_interrupt_handler (int v)
{
    s3_exit_sigint_fired ();
}

void
sigsegv_interrupt_handler (int v)
{
    logs_enable_backtrace ();
    logs_put (LOG_OUT_BOTH, LOG_LEVEL_FATAL, "Program has received SIGSEGV (Segmentation Fault)");
    logs_close ();
}

char *
rle_string (char *string, int length)
{
    char *temp = m_calloc (length * 2);
    short index = 0;

    short last = -1;
    uchar counter = 0;

    for (size_t i = 0; i < length; i++) {
        uchar current = *(uchar *) (string + i);
        if ((current != last && last != -1) || counter >= 25) {
            *(temp + index) = counter + 'a';
            *(temp + index + 1) = (uchar) last;
            index += 2;
            counter = 1;
        } else {
            counter++;
        }

        last = current;
    }

    if (counter > 0) {
        *(temp + index) = counter + 'a';
        *(temp + index + 1) = (uchar) last;
        index += 2;
    }

    index++; // For \0

    temp = m_realloc (temp, index);
    return temp;
}

void
print_connection_banner (s3_protocol_s *protocol, int port)
{
    logs_put (LOG_OUT_BOTH, LOG_LEVEL_INFO, "=====================================");
    char *temp = m_malloc (sizeof (struct sockaddr_in6));
    char *hex;

    if (protocol->sock_family == AF_INET) {
        struct sockaddr_in sockaddr = protocol->server_sockv4;
        *(in_addr_t *) (temp) = sockaddr.sin_addr.s_addr;
        *(in_port_t *) (temp + sizeof (in_addr_t)) = sockaddr.sin_port;

        hex = (char *) hexstring ((uchar *) temp, sizeof (in_addr_t) + sizeof (in_port_t));
    } else {
        struct sockaddr_in6 sockaddr = protocol->server_sockv6;

        int ip6addr_size = sizeof (sockaddr.sin6_addr.__in6_u);
        memcpy (temp, &sockaddr.sin6_addr.__in6_u, ip6addr_size);
        *(in_port_t *) (temp + ip6addr_size) = sockaddr.sin6_port;

        hex = (char *) hexstring ((uchar *) temp, ip6addr_size + sizeof (in_port_t));
    }

    m_free (temp);

    struct sockaddr *sock = protocol->sock_family == AF_INET ? (struct sockaddr *) &protocol->server_sockv4
                                                             : (struct sockaddr *) &protocol->server_sockv6;

    const char *error = NULL;
    char *hostname = network_gethostname (protocol->sock_family, sock, &error);

    logs_put (LOG_OUT_BOTH, LOG_LEVEL_DEBUG, "Bound to address: %s (%d)", hostname, port);
    m_free (hostname);

    logs_put (LOG_OUT_BOTH, LOG_LEVEL_INFO, "Connection Key: 0-%s", hex);

    char *rle_conn = rle_string (hex, strlen (hex));
    logs_put (LOG_OUT_BOTH, LOG_LEVEL_INFO, "Alternate Key: 1-%s", rle_conn);

    m_free (rle_conn);
    m_free (hex);

    logs_put (LOG_OUT_BOTH, LOG_LEVEL_INFO, "=====================================");
}

int
main (int argc, char *argv[])
{
    // Handle signals so when debugging we can show all the memory that was not freed
    signal (SIGINT, sigint_interrupt_handler);
    signal (SIGSEGV, sigsegv_interrupt_handler);

    settings_s settings = {
      .log_settings =
        {
          .print_debug_logs = TRUE,
          .print_stack_frames = FALSE,
        },
      .ip_settings =
        {
          .address_type = 0,
          .interface = NULL,
          .port = NETWORK_PORT,
        },
    };

    argp_parse (&argp, argc, argv, NULL_ZERO, NULL_ZERO, &settings);

    sigaction (SIGPIPE, &(struct sigaction){{SIG_IGN}}, NULL);
    print_banner ();

    boolean success;
    s3_protocol_s protocol = s3_connection_initialize_sync (settings, &success);
    print_connection_banner (&protocol, settings.ip_settings.port);

    if (success) {
        s3_connection_accept_loop_async (&protocol);
    }

    s3_close (protocol);
}