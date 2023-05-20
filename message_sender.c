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
    if (argc != 4){
        fprintf(stderr,"%s\n", strerror(EINVAL));
        return 1;
    }
    fd = open(argv[1],O_RDWR);
    if (fd <0){
        exit_and_print_errno();
    }
    if (ioctl(fd,MSG_SLOT_CHANNEL, atoi(argv[2]))<0){
        exit_and_print_errno();
    } 

    if (write(fd,argv[3],strlen(argv[3]))<0){
        exit_and_print_errno();
    }
    if (close(fd) < 0){
        exit_and_print_errno();
    }
    return 0;
}




