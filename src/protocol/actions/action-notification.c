#include "files/file.h"
#include "general/common.h"
#include "logging/logs.h"
#include "protocol/protocol.h"
#include <libnotify/notification.h>
#include <libnotify/notify.h>
#include <stdio.h>
#include <string.h>

// definition
void
dbus_notify (const char *title, const char *message);
enum urgency { LOW, NORMAL, CRITICAL };

// implementation
int
s3_preprocess_notification (s3_request_s *request)
{
    if (request->header_info.data_length > 0 && request->header_info.data_length < FILE_BUFFER_LENGTH) {
        return (SUCCESS);
    } else {
        return (S3_RESPONSE_NOTIFY_TOOBIG);
    }
}

int
s3_postprocess_notification (network_s *network)
{
    s3_request_s *request = network->request;
    char *filename = request->file_name.temp_file_name.address;

    FILE *notification_file = fopen (filename, FILE_MODE_READBINARY);
    struct stat file_stats = file_stat (notification_file);

    int memory_size = min (file_stats.st_size, KB (4));
    char *memory = m_malloc (memory_size + 1);
    memory[memory_size] = 0;

    fread (memory, sizeof (char), memory_size, notification_file);
    logs_put (LOG_OUT_LOGS, LOG_LEVEL_INFO, NOTIFICATION_GENERAL, memory);

    char *title = memory;
    char *message = strchr (memory, '\n');

    if (message != NULL) {
        *message = '\0';
        message = message + 1;
    }

    dbus_notify (title, message);

    fclose (notification_file);
    m_free (memory);
    return SUCCESS;
}

void
dbus_notify (const char *title, const char *message)
{
    // https://wiki.archlinux.org/title/Desktop_notifications
    // https://specifications.freedesktop.org/notification-spec/notification-spec-latest.html

    notify_init ("Android Notification Handler");
    NotifyNotification *notify = notify_notification_new (title, message, "dialog-information");
    notify_notification_set_timeout(notify, 15*1000);
    notify_notification_show (notify, NULL);
    g_object_unref (G_OBJECT (notify));
    notify_uninit ();
}