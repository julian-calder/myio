/*
 * myio.h
 */

#ifndef __MYIO_H
#define __MYIO_H

#include <stdlib.h>
#include <stdbool.h>

#define BUF_SIZE 1024

struct file_info {
    int fd;
    int flags;
    int bytes_available;
    
    bool prev_read;
    bool prev_write;

    off_t file_offset;
    off_t hbuf_offset;

    char hbuf[BUF_SIZE];
};

struct file_info * myopen(char* pathname, int flags);

int myclose(struct file_info *fi);

int myread(struct file_info *fi, char *ubuf, size_t num_bytes);

int mywrite(struct file_info *fi, char *ubuf, size_t num_bytes);

int myseek(struct file_info *fi, off_t offset, int whence);

int myflush(struct file_info *fi);

#endif
