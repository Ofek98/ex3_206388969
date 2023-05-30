#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include "message_slot.h"

void exit_and_print_errno(){
    fprintf(stderr,"%s\n", strerror(errno));
    exit(1);
}

int main (int argc, char **argv){
    int fd;
    size_t msg_len;
    long channel_id;
    char buf[BUF_LEN];

    if (argc != 3){
        fprintf(stderr,"%s\n", strerror(EINVAL));
        return 1;
    }

    fd = open(argv[1],O_RDWR);
    if (fd <0){
        exit_and_print_errno();
    }

    channel_id = strtol(argv[2],NULL,10);
    if (channel_id <= 0){
        fprintf(stderr,"%s\n", strerror(EINVAL));
        return 1;
    }
    
    if (ioctl(fd,MSG_SLOT_CHANNEL,channel_id)<0){
        exit_and_print_errno();
    }

    msg_len = read(fd, buf, BUF_LEN);
    if (msg_len < 0){
       exit_and_print_errno(); 
    }
    if (close(fd) < 0){
        exit_and_print_errno();
    }
    if (write(STDOUT_FILENO, buf, msg_len) != msg_len){
        exit_and_print_errno();
    }
    return 0;
}



