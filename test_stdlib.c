#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BYTES_PER_ITERATION 10
#define SMALL_R 500
#define BIG_R 5000

int
main(int argc, char *argv[])
{
    int src_fd, dst_fd;
    char *src_filename, *dst_filename;
    int n1, n2, n3, num_written;
    char buf1[SMALL_R], buf2[SMALL_R], buf3[BIG_R];

    /* check command-line args */
    if(argc != 3) {
        printf("usage: %s src_filename dst_filename\n", argv[0]);
        exit(1);
    }
    src_filename = argv[1];
    dst_filename = argv[2];

    src_fd = open(src_filename, O_RDWR);
    printf("Source file descriptor is: %d\n", src_fd);

    dst_fd = open(dst_filename, O_WRONLY);
    printf("Destination file descriptor is: %d\n", dst_fd);
    /* do the copy */

    n1 = read(src_fd, buf1, SMALL_R);
    printf("Value returned from first read: %d\n", n1);
    printf("buffer 1 contents: %s\n", buf1);

    n2 = read(src_fd, buf2, SMALL_R);
    printf("Value returned from second read: %d\n", n1);
    printf("buffer 2 contents: %s\n", buf1);

    n3 = read(src_fd, buf3, BIG_R);
    printf("Value returned from third read: %d\n", n2);
    printf("buffer 3 contents: %s\n", buf2);
    
    num_written = write(dst_fd, buf1, n1);
    printf("Value returned from first write: %d\n", num_written);
    printf("buffer 1 contents: %s\n", buf1);

    num_written = write(dst_fd, buf2, n2);
    printf("Value returned from second write: %d\n", num_written);
    printf("buffer 2 contents: %s\n", buf2);
    
    num_written = write(dst_fd, buf3, n3);
    printf("Value returned from second write: %d\n", num_written);
    printf("buffer 3 contents: %s\n", buf3);

    /* clean up */
    close(src_fd);
    close(dst_fd);
}


	    

