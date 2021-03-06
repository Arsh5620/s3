#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>
#include <signal.h>
#include "./protocol/protocol.h"

struct argp_option argument_options[] = {
  {"disable_debug_logs", 'd', NULL_ZERO, NULL_ZERO, "** do not ** print debug information in logs"},
  {"print_stack_logs",
   's',
   NULL_ZERO,
   NULL_ZERO,
   "** do ** print stack frame information to the logs"},
  {0},
};

error_t
argument_parser (int key, char *arg, struct argp_state *state)
{
    s3_log_settings_s *settings = (s3_log_settings_s *) state->input;

    switch (key)
    {
    case 'd':
        settings->print_debug_logs = FALSE;
        break;
    case 's':
        settings->print_stack_frames = TRUE;
        break;
    case ARGP_KEY_ARG:
        return 0;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

struct argp argp = {
  argument_options,
  argument_parser,
  APPLICATION_NAME "server : " __SERVER_VERSION__,
  NULL,
  NULL,
  NULL};

int
main (int argc, char *argv[])
{
    s3_log_settings_s settings = {.print_debug_logs = TRUE, .print_stack_frames = FALSE};
    argp_parse (&argp, argc, argv, NULL_ZERO, NULL_ZERO, &settings);

    sigaction (SIGPIPE, &(struct sigaction){{SIG_IGN}}, NULL);

    s3_protocol_s protocol = s3_connection_initialize_sync (NETWORK_PORT, settings);
    if (protocol.init_complete)
        s3_connection_accept_loop_async (&protocol);
    s3_close (protocol);
}