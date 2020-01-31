#include <stdio.h>
#include "../networking/network.h"
#include "../dbp.h"
#include "../logs.h"

#define DBP_FILE_TEMP_DIR "./temp/"

int dbp_protocol_notification(dbp_s *protocol);
int dbp_create(dbp_s *protocol);