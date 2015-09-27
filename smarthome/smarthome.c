#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#define DEBUG 1
#define BUFFER 65536

#define debug_print(fmt, ...) \
	do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
		__LINE__, __func__, __VA_ARGS__); } while (0)

enum ERROR_TYPE
{
    NO_ERR = 0,
    ERR = 1,
    ERR_ARG = 2,
    ERR_OPEN_FILE = 3,
    ERR_NO_FIFO = 4,
    ERR_READ = 5,
    ERR_WRITE = 6,
};

static int is_fifo(int file_fd)
{
    struct stat status;

    if(fstat(file_fd, &status) == -1)
	return 0;

    if(!S_ISFIFO(status.st_mode))
	return 0;

    return 1;
}

int main(int argc, char *argv[])
{
    char *fifo_src, *fifo_dst;
    char buffer[BUFFER + 1];
    ssize_t r_bytes, w_bytes;
    int srcfd_read, dstfd_write, dstfd_read;
    int iter = 0;
    enum ERROR_TYPE ret = NO_ERR;

    if(argc != 3)
    {
        printf("Usage:\n nameprog fifo_src fifo_dst\n");
	ret = ERR_ARG;
	goto Exit;
    }
    fifo_src = argv[1];
    fifo_dst = argv[2];

    signal(SIGPIPE, SIG_IGN);

    srcfd_read = open(fifo_src, O_RDONLY);
    if(srcfd_read == -1)
    {
        perror("ftee: srcfd_read: open()");
        ret = ERR_OPEN_FILE;
	goto Exit;
    }

    dstfd_read = open(fifo_dst, O_RDONLY | O_NONBLOCK);
    if(dstfd_read == -1)
    {
        perror("ftee: dstfd_read: open()");
	ret = ERR_OPEN_FILE;
	goto Exit_1;
    }

    dstfd_write = open(fifo_dst, O_WRONLY | O_NONBLOCK);
    if(dstfd_write == -1)
    {
        perror("ftee: dstfd_write: open()");
	ret = ERR_OPEN_FILE;
	goto Exit_2;
    }

    if (!is_fifo(srcfd_read) || !is_fifo(dstfd_write))
    {
        perror("some of files is not a fifo!\n");
	ret = ERR_NO_FIFO;
	goto Exit_3;
    }

    while(1)
    {
	r_bytes = read(srcfd_read, buffer, BUFFER);
        if (r_bytes < 0 && errno == EINTR)
            continue;
        if (r_bytes <= 0)
	{
	    ret = ERR_READ;
            break;
	}

	w_bytes = write(dstfd_write, buffer, r_bytes);
	if (w_bytes == -1 && iter++)
	{
	    read(dstfd_read, buffer, BUFFER);
	    iter = 0;
	    debug_print("droped = %d\n", read(dstfd_read, buffer, BUFFER));
	}
	if (DEBUG && r_bytes != w_bytes)
	    debug_print("rbytes = %d, w_bytes = %d\n", r_bytes, w_bytes);
    }

Exit_3:
    close(dstfd_write);
Exit_2:
    close(dstfd_read);
Exit_1:
    close(srcfd_read);
Exit:
    return ret;
}
