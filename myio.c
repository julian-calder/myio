/*
 * myio.c
 */
#include "myio.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

struct file_info *
myopen(char* pathname, int flags) 
{ 
    struct file_info *fi;
    if ((fi = malloc(sizeof(struct file_info))) == NULL) {
        return NULL;
    }

    if ((fi->fd = open(pathname, flags)) > 0) {
        fi->file_offset = 0;
        fi->flags = flags;
        fi->hbuf_offset = 0;

        return fi;
    }
    else {
        // if open fails, free the allocated memory and return NULL
        // free has no return value we can check
        free(fi);
        return NULL;
    }
}

int 
myclose(struct file_info *fi) 
{
    int fd = fi->fd;

    if (fi->prev_write == true) {
        // myflush doesn't have to succeed in order for us to close a file
        myflush(fi);
    }
    free(fi);
    // if close fails, this is reflected in return from myclose
    return close(fd);
}

int
myread(struct file_info *fi, char *ubuf, size_t num_bytes)
{
    if (fi->flags == O_WRONLY) {
        return -1;
    }

    if (fi->prev_write == true) {
        if (myflush(fi) < 0) {
            return -1;
        }
        fi->prev_write = false;
        fi->hbuf_offset = 0;
    }

    int num_read, num_to_copy;
    off_t ubuf_offset;

    fi->prev_read = true;

    ubuf_offset = 0;
    num_to_copy = 0;

    // if user wants more than BUF_SIZE with empty buffer
    if (fi->hbuf_offset == 0 && num_bytes >= BUF_SIZE) {
        if ((num_read = read(fi->fd, ubuf, num_bytes)) < 0) {
            return -1;
        }
        fi->file_offset += num_read;
        return num_read;
    }

    // if buffer is empty and user wants less than BUF_SIZE   
    if (fi->hbuf_offset == 0) {
        // fill the buffer
        if ((fi->bytes_available = read(fi->fd, fi->hbuf, BUF_SIZE)) < 0) {
            return -1;
        }
        // if we have more bytes than user wants
        if (fi->bytes_available > num_bytes) {
            // copy to user buffer
            memcpy(ubuf, fi->hbuf, num_bytes);
            // increase file offset and hbuf offset accordingly
            fi->file_offset += num_bytes;
            fi->hbuf_offset += num_bytes;
            // return number of bytes read to user buffer
            return num_bytes;
        }
        // if file has fewer bytes than user wants
        else {
            // copy remainder of file to user buffer
            memcpy(ubuf, fi->hbuf, fi->bytes_available);
            // update file offset and hbuf offset accordingly
            fi->file_offset += fi->bytes_available;
            fi->hbuf_offset += fi->bytes_available;
            // return number of bytes left in file
            return fi->bytes_available;
        }
    }
    
    // if number of bytes requested will exceed our buffer
    if ((num_bytes + fi->hbuf_offset) >= BUF_SIZE) {
        num_to_copy = fi->bytes_available - fi->hbuf_offset;
        // empty our buffer to user buffer
        memcpy(ubuf, fi->hbuf + fi->hbuf_offset, num_to_copy);
        fi->file_offset += num_to_copy;
        // reset bytes available to indicate we have emptied buffer
        fi->bytes_available = 0;
        fi->hbuf_offset = 0;
        // increase temporary user buffer offset by however much we just copied
        ubuf_offset += num_to_copy;
        
        // if user still wants more than BUF_SIZE
        if ((num_bytes - num_to_copy) >= BUF_SIZE) {
            // read directly to user buffer
            if ((num_read = read(fi->fd, ubuf + ubuf_offset, num_bytes - num_to_copy)) < 0) {
                return -1;
            }
            // increase file offset appropriately
            fi->file_offset += num_read;
            return num_read + num_to_copy;
        }
        // if user wants less than BUF_SIZE
        else {
            // refill our buffer
            if ((num_read = read(fi->fd, fi->hbuf, BUF_SIZE)) < 0) {
                return -1;
            }
            // update bytes_available and reset hbuf offset
            fi->bytes_available = num_read;
            fi->hbuf_offset = 0;
    
            // if the user wants more bytes than we have
            if ((num_bytes - num_to_copy) > num_read) {
                // copy whatever we have in the buffer
                memcpy(ubuf + ubuf_offset, fi->hbuf, num_read);
   
                // increase file offset and hidden buf offset appropriately
                fi->file_offset += num_read; 
                fi->hbuf_offset += num_read; 
                return num_to_copy + num_read;
            }
            else {
                // copy amount that user wants into buffer
                memcpy(ubuf + ubuf_offset, fi->hbuf, num_bytes - num_to_copy);

                // increase file offset and hbuf offset to reflect new amount
                // that was processed
                fi->file_offset += num_bytes; 
                fi->hbuf_offset += num_bytes - num_to_copy; 
                return num_bytes;
            }
        }
    }
    // default case, num_bytes will not overflow and is less than BUF_SIZE
    else {
        // if we have space in our buffer
        if (fi->hbuf_offset + num_bytes < fi->bytes_available) {
            // copy amount user wants into buffer
            memcpy(ubuf, fi->hbuf + fi->hbuf_offset, num_bytes);

            // increase file offset and hbuf offset appropriately
            fi->file_offset += num_bytes;
            fi->hbuf_offset += num_bytes;

            return num_bytes;
        }
        // if we have insufficient space in buffer
        else {
            // copy however much we have left
            memcpy(ubuf, fi->hbuf + fi->hbuf_offset, fi->bytes_available - fi->hbuf_offset);

            // increase file offset and hbuf offset accordingly
            fi->file_offset += fi->bytes_available - fi->hbuf_offset;
            fi->hbuf_offset += fi->bytes_available - fi->hbuf_offset;

            return fi->bytes_available - fi->hbuf_offset;
        }
    }
    return EXIT_SUCCESS;
}

int
mywrite(struct file_info *fi, char *ubuf, size_t count) 
{
    if (fi->flags == O_RDONLY) {
        return -1;
    }
    // if file was previously read, we need to seek backwards
    if (fi->prev_read == true) {
        // flush read buffer on next write call
        if ((myseek(fi, fi->file_offset, SEEK_SET)) < 0) {
            return -1;
        }
        // change read flag to false, as we are now writing 
        fi->prev_read = false;
        // initialize hbuf offset to reflect that it is empty
        fi->hbuf_offset = 0;
    }

    int num_written, return_check;
    off_t ubuf_offset;
    
    fi->prev_write = true;

    ubuf_offset = 0;

    // we can memcpy to our buffer without overflowing
    if (fi->hbuf_offset + count <  BUF_SIZE) {
        // memcpy to our buffer
        memcpy(fi->hbuf + fi->hbuf_offset, ubuf, count);
        // update our buffer offset and file offset accordingly
        fi->hbuf_offset += count; 
        fi->file_offset += count;
        return count;
    }
    // if we have exactly the amount of BUF_SIZE
    else if (fi->hbuf_offset + count == BUF_SIZE) {
        // memcpy to our buffer
        memcpy(fi->hbuf + fi->hbuf_offset, ubuf, count);
        // write our (full) buffer to the file
        if ((num_written = write(fi->fd, fi->hbuf, BUF_SIZE)) < 0) {
            return -1;
        }
        // reset our buffer and file offset
        fi->hbuf_offset = 0; 
        fi->file_offset += count;
        return count;
    }
    // if write will overfill our buffer
    else {
        // if our buffer is empty
        if (fi->hbuf_offset == 0) {
            if ((num_written = write(fi->fd, ubuf, count)) < 0) {
                return -1;
            }
            fi->file_offset += num_written;
            return num_written;
        }
        // otherwise, empty write buffer and write directly from user buffer 
        else {
            // fill up remainder of buffer
            memcpy(fi->hbuf + fi->hbuf_offset, ubuf, BUF_SIZE - fi->hbuf_offset);
            // increase file offset by this remainder
            fi->file_offset += BUF_SIZE - fi->hbuf_offset;

            // reflect how many bytes user just "wrote"
            num_written = BUF_SIZE - fi->hbuf_offset;

            // update temporary user buffer offset
            ubuf_offset += num_written;

            // write our buffer to file
            // return check value represents value from full write to file
            // not the amount that user is writing in this call to mywrite
            if ((return_check = write(fi->fd, fi->hbuf, BUF_SIZE)) < 0) {
                return -1;
            }
            // reset our buffer offset
            fi->hbuf_offset = 0;
    
            // if user still wants more than our buffer size
            if (count - ubuf_offset >= BUF_SIZE){
                // write directly to file
                // we don't verify this write because the write on line 239 would have failed first
                num_written += write(fi->fd, ubuf + ubuf_offset, count - ubuf_offset);
                // increase num_written by however many bytes user still wanted
                fi->file_offset += num_written;
                return num_written;
            }
     	    else {
                // fill our buffer with remaining bytes user wants to write
                memcpy(fi->hbuf + fi->hbuf_offset, ubuf + ubuf_offset, count - num_written);
                // update file offset and hbuf offset accordingly
                fi->file_offset += count - num_written;
    	        fi->hbuf_offset = count - num_written;
                return count;
        	}
        }
    }
}


int
myseek(struct file_info *fi, off_t offset, int whence) 
{
    if (whence == SEEK_SET) {
        fi->file_offset = offset;
        // if lseek fails, myseek returns -1
        return lseek(fi->fd, offset, whence);
    }
    else if (whence == SEEK_CUR) {
        fi->file_offset += offset;
        // if lseek fails, myseek returns -1
        return lseek(fi->fd, offset, whence);
    }
    else {
        // indicate failure (or that we are using invalid whence)
        return -1;
    }
}

int myflush(struct file_info *fi) 
{
    // if write fails, myflush returns -1
    return write(fi->fd, fi->hbuf, fi->hbuf_offset);
}

