#include "files/path.h"
#include "general/common.h"
#include "protocol/build.h"
#include "protocol/protocol.h"

#include <argp.h>
#include <curses.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct argp_option argument_options[] = {
  {"disable_debug_logs", 'd', ZERO, ZERO, "DO NOT print debug information in logs"},
  {"print_stack_logs", 's', ZERO, ZERO, "print stack frame information to the logs"},
  {"ip", 't', "[4,6]", ZERO, "Valid ip type values are '4' for ipv4 or '6' for ipv6"},
  {"interface", 'i', "<interface name>", ZERO, "Interface to be bound to, defaults to 'loopback'. e.g. --interface=wlp2s0"},
  {"port", 'p', "<port number>", ZERO, "Port number to use, defaults to 5665"},
  {"initialize",
   1,
   ZERO,
   ZERO,
   "Go through an initial interactive setup of the program to add users and create ssl certificates"},
  {"plain-mode",
   2,
   ZERO,
   ZERO,
   "Run the server in plain mode, **DO NOT** use unless you know what you are doing. Any passphrases are visible on the network "
   "to anyone who can read traffic"},
  {0},
};

#define GENERATE_CERTS                                                                                                           \
    "CERTIFICATE_DIR=~/s3/certificates && mkdir $CERTIFICATE_DIR -p && cd $CERTIFICATE_DIR && openssl req -nodes -x509 "         \
    "-newkey rsa:4096 "                                                                                                          \
    "-keyout CA-key.pem -out CA-cert.pem -days 365"

#define INVAL_VAL(__x, __arg)                                                                                                    \
    "Given value '%s' is unknown for option '" __x "', use '--help' or '--usage' option for help.\n", __arg

void
print_banner ();
void
interactive_setup ();
void
add_user_to_credentials_db (char *username, char *passphrase, int username_length, int passphrase_length);
void
generate_ssl_certs ();
error_t
argument_parser (int key, char *arg, struct argp_state *state);

struct argp argp = {argument_options, argument_parser, APP_NAME " server v" __SERVER_VERSION__, NULL, NULL, NULL};
boolean connection_initialized = FALSE;
void (*original_sigint_handler) (int) = NULL;

error_t
argument_parser (int key, char *arg, struct argp_state *state)
{
    settings_s *settings = (settings_s *) state->input;

    switch (key) {
        case '\1': {
            interactive_setup ();
            exit (EXIT_SUCCESS);
            break;
        }
        case '\2': {
            settings->plain_mode = TRUE;
            break;
        }
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

void
interactive_setup ()
{
    print_banner ();

    char username[64];
    char passphrase[64];
    puts ("Starting intial setup, if you would like to cancel anytime before committing press ctrl+c");
    char *username_value;
    char *passphrase_value;

    do {
        puts ("Enter username: ");
        username_value = fgets (username, sizeof (username), stdin);
    } while (username_value == NULL || username_value[0] == '\0');

    do {
        puts ("Enter passphrase: ");
        passphrase_value = fgets (passphrase, sizeof (passphrase), stdin);
    } while (passphrase_value == NULL || passphrase_value[0] == '\0');

    puts ("Commiting now.");
    // -1 is for the new line terminators
    add_user_to_credentials_db (username, passphrase, strlen (username_value) - 1, strlen (passphrase_value) - 1);

    char response[2];
    puts ("Would you like to also generate SSL certificates? Please note that without SSL certificates you cannot connect to s3d "
          "securely");
    fgets (response, sizeof (response), stdin);

    if (response[0] == 'y') {
        puts ("Generating SSL certs, If you want you can leave all the certificate fields empty");
        generate_ssl_certs ();
    }

    puts ("Initial setup finished, if you have not already enabled the s3d daemon you can do so by following the steps: ");
    puts ("\t* systemctl enable s3d.service");
    puts ("\t* systemctl start s3d.service");
    puts ("");
    puts ("Goodbye!!");
}

void
add_user_to_credentials_db (char *username, char *passphrase, int username_length, int passphrase_length)
{
    if (s3_verify_db ()) {
        if (s3_auth_add_user (username, passphrase, username_length, passphrase_length)) {
            puts ("User successfully added to auth database");
            return;
        } else {
            fprintf (stderr, "Failed adding user to auth database. Exiting.");
        }
    } else {
        fprintf (stderr, "Could not verify the integrity of \'" S3_DATABASE_FILENAME "\'. Exiting.");
    }

    fflush (stderr);
    exit (EXIT_FAILURE);
}

void
generate_ssl_certs ()
{
    system (GENERATE_CERTS);
}

void
print_banner ()
{
    printf ("=====================================\n");
    printf ("s3 server version %s\n", __SERVER_VERSION__);
    printf ("%s\n", __SERVER_INFO__);
    printf ("=====================================\n");
}

void
sigsegv_interrupt_handler (int value)
{
    logs_enable_backtrace ();
    logs_put (LOG_OUT_BOTH, LOG_LEVEL_FATAL, "Program has received SIGSEGV (Segmentation Fault)");
    logs_close ();
}

void
sigint_interrupt_handler (int value)
{
#ifdef DEBUG
    printf ("\n\n*************************************\n"
            "Please wait while we try to close everything...\n"
            "If this is taking too long, open another terminal and use: kill -SIGKILL %d\n"
            "*************************************\n",
            getpid ());
    memory_debug_print_lost ();
#endif

    printf ("\nGoodbye!!\n");
    logs_close ();

    signal (SIGINT, original_sigint_handler);
    raise (SIGINT);
}

char *
rle_string (char *string, int length, int *out_len)
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

    if (out_len != NULL) {
        *out_len = index;
    }

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

        int size = sizeof (in_addr_t) + sizeof (in_port_t);
        hex = m_malloc (size * 2);
        string_hexify ((uchar *) hex, (uchar *) temp, size);
    } else {
        struct sockaddr_in6 sockaddr = protocol->server_sockv6;

        int ip6addr_size = sizeof (sockaddr.sin6_addr.__in6_u);
        memcpy (temp, &sockaddr.sin6_addr.__in6_u, ip6addr_size);
        *(in_port_t *) (temp + ip6addr_size) = sockaddr.sin6_port;

        int size = ip6addr_size + sizeof (in_port_t);
        hex = m_malloc (size * 2);
        string_hexify ((uchar *) hex, (uchar *) temp, size);
    }

    m_free (temp);

    struct sockaddr *sock = protocol->sock_family == AF_INET ? (struct sockaddr *) &protocol->server_sockv4
                                                             : (struct sockaddr *) &protocol->server_sockv6;

    const char *error = NULL;
    char *hostname = network_gethostname (protocol->sock_family, sock, &error);

    logs_put (LOG_OUT_BOTH, LOG_LEVEL_DEBUG, "Bound to address: %s (%d)", hostname, port);
    m_free (hostname);

    logs_put (LOG_OUT_BOTH, LOG_LEVEL_INFO, "Connection Key: 0-%s", hex);

    int hex_length = strlen (hex);
    int length;
    char *rle_conn = rle_string (hex, hex_length, &length);

    if (length + 2 < hex_length) {
        logs_put (LOG_OUT_BOTH, LOG_LEVEL_INFO, "Alternate Key: 1-%s", rle_conn);
    }

    m_free (rle_conn);
    m_free (hex);

    logs_put (LOG_OUT_BOTH, LOG_LEVEL_INFO, "=====================================");
}

int
main (int argc, char *argv[])
{
    // Handle signals so when debugging we can show all the memory that was not freed
    original_sigint_handler = signal (SIGINT, sigint_interrupt_handler);
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

    argp_parse (&argp, argc, argv, ZERO, ZERO, &settings);

    sigaction (SIGPIPE, &(struct sigaction){{SIG_IGN}}, NULL);
    print_banner ();

    boolean success;
    s3_protocol_s protocol = s3_connection_initialize_sync (settings, &success);
    print_connection_banner (&protocol, settings.ip_settings.port);

    connection_initialized = TRUE;
    if (success) {
        s3_connection_accept_loop_async (&protocol);
    }

    s3_close (protocol);
}