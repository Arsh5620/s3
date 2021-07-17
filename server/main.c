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
  {"ip", 'i', "[4,6]", NULL_ZERO, "Valid ip type values are '4' for ipv4 or '6' for ipv6"},
  {"interface", 'f', "<interface name>", NULL_ZERO, "Interface to be bound to, defaults to 'loopback'. e.g. --interface=wlp2s0"},
  {0},
};

error_t
argument_parser (int key, char *arg, struct argp_state *state)
{
    settings_s *settings = (settings_s *) state->input;

    switch (key) {
    case 'd':
        settings->log_settings.print_debug_logs = FALSE;
        break;
    case 's':
        settings->log_settings.print_stack_frames = TRUE;
        break;
    case 'i':
        if (*arg == '4' || *arg == '6') {
            settings->ip_settings.address_type = *arg == '4' ? AF_INET : AF_INET6;
        } else {
            printf ("Given value '%s' is unknown for option '-i', use '--help' or '--usage' option for help.\n", arg);
            exit (EXIT_FAILURE);
        }
        break;
    case 'f':
        settings->ip_settings.interface = arg;
        break;
    case ARGP_KEY_ARG:
        return 0;
    default:
        return ARGP_ERR_UNKNOWN;
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
        if ((current != last && last != -1) || counter >= 9) {
            *(temp + index) = counter + '0';
            *(temp + index + 1) = (uchar) last;
            index += 2;
            counter = 1;
        } else {
            counter++;
        }

        last = current;
    }

    if (counter > 0) {
        *(temp + index) = counter + '0';
        *(temp + index + 1) = (uchar) last;
        index += 2;
    }

    index++; // For \0

    temp = m_realloc (temp, index);
    return temp;
}

void
print_connection_banner (s3_protocol_s *protocol)
{
    logs_put (LOG_OUT_BOTH, LOG_LEVEL_INFO, "=====================================");
    char *hex;

    if (protocol->sock_family == AF_INET) {
        struct sockaddr_in sockaddr = protocol->server_sockv4;
        hex = (char *) hexstring ((uchar *) &sockaddr, sizeof (struct sockaddr_in));
    } else {
        struct sockaddr_in6 sockaddr = protocol->server_sockv6;
        hex = (char *) hexstring ((uchar *) &sockaddr, sizeof (struct sockaddr_in6));
    }

    struct sockaddr *sock = protocol->sock_family == AF_INET ? (struct sockaddr *) &protocol->server_sockv4
                                                             : (struct sockaddr *) &protocol->server_sockv6;

    const char *error = NULL;
    char *hostname = network_gethostname (protocol->sock_family, sock, &error);

    logs_put (LOG_OUT_BOTH, LOG_LEVEL_DEBUG, "Bound to address: %s", hostname);
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

    s3_log_settings_s log_settings = {.print_debug_logs = TRUE, .print_stack_frames = FALSE};
    settings_s settings = {.log_settings = log_settings, .ip_settings.address_type = 0};
    argp_parse (&argp, argc, argv, NULL_ZERO, NULL_ZERO, &settings);

    sigaction (SIGPIPE, &(struct sigaction){{SIG_IGN}}, NULL);
    print_banner ();

    boolean success;
    s3_protocol_s protocol = s3_connection_initialize_sync (NETWORK_PORT, settings, &success);
    print_connection_banner (&protocol);

    if (success) {
        s3_connection_accept_loop_async (&protocol);
    }

    s3_close (protocol);
}