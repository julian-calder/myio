#include "myio.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BYTES_PER_ITERATION 10

int
main(int argc, char *argv[])
{
    struct file_info *fd;
    char *filename;
    int num_read;
    char buf[BYTES_PER_ITERATION];

    if(argc != 2) {
        printf("usage: %s filename\n", argv[0]);
        exit(1);
    }
    filename = argv[1];

    fd = myopen(filename, O_RDWR);

    num_read = myread(fd, buf, BYTES_PER_ITERATION);
    mywrite(fd, buf, num_read);

    num_read = myread(fd, buf, BYTES_PER_ITERATION);
    mywrite(fd, buf, num_read);

    myclose(fd);
}
