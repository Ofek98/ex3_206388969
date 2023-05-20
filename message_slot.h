#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#include <linux/ioctl.h>


#define MAJOR_NUM 235
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned long)
#define DEVICE_RANGE_NAME "message_slot"
#define DEVICE_FILE_NAME "message_slot"
#define BUF_LEN 128

typedef struct channel{
    unsigned long id;
    int active_msg_len;
    struct channel *next;
    struct channel *prev;
    char *buf;
}CHANNEL;

typedef struct slot{
    int minor;
    struct slot *next;
    struct slot *prev;
    struct channel *head;
}SLOT;

void exit_and_print_errno(void);

#endif
