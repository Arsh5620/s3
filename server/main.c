#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>
#include <signal.h>
#include "./protocol/protocol.h"

struct argp_option argument_options[] = {
  {"disable_debug_logs",
   'd',
   NULL_ZERO,
   NULL_ZERO,
   "This option takes no value. If the argument is present then the program will not print debug "
   "logs"},
  {"disable_stack_logs",
   'l',
   NULL_ZERO,
   NULL_ZERO,
   "This option takes no value. If the argument is present then the program will not print stack "
   "frame information to the logs"},
  {0},
};

error_t
argument_parser (int key, char *arg, struct argp_state *state)
{
    dbp_log_settings_s *settings = (dbp_log_settings_s *) state->input;

    switch (key)
    {
    case 'd':
        settings->print_debug_logs = FALSE;
        break;
    case 'l':
        settings->print_stack_frames = FALSE;
        break;
    case ARGP_KEY_ARG:
        return 0;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

struct argp argp
  = {argument_options, argument_parser, "server : " __SERVER_VERSION__, NULL, NULL, NULL};

int
main (int argc, char *argv[])
{
    dbp_log_settings_s settings = {.print_debug_logs = TRUE, .print_stack_frames = TRUE};
    argp_parse (&argp, argc, argv, NULL_ZERO, NULL_ZERO, &settings);

    sigaction (SIGPIPE, &(struct sigaction){{SIG_IGN}}, NULL);

    dbp_protocol_s protocol = dbp_connection_initialize_sync (NETWORK_PORT, settings);
    if (protocol.init_complete)
        dbp_connection_accept_loop (&protocol);
    dbp_close (protocol);
}