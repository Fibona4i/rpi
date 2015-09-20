#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#define BUFFER 65536

int main(int argc, char *argv[])
{
    int readfd, writefd, tmp;
    char *fifonam;
    char *fifonam2;
    char buffer[BUFFER + 1];
    ssize_t r_bytes, w_bytes;
    int iter = 0;

    signal(SIGPIPE, SIG_IGN);

    if(3!=argc)
    {
        printf("Usage:\n nameprog fifo1 fifo2\n");
        exit(EXIT_FAILURE);
    }
    fifonam = argv[1];
    fifonam2 = argv[2];

    readfd = open(fifonam, O_RDONLY);
    if(-1==readfd)
    {
        perror("ftee: readfd: open()");
        //printf("ftee: readfd: open(%s)\n", fifonam);
        exit(EXIT_FAILURE);
    }

    tmp = open(fifonam2, O_RDONLY | O_NONBLOCK);
    if(-1==tmp)
    {
        perror("ftee: tmp: open()");
        //printf("ftee: tmp: open(%s)\n", fifonam2);
	close(readfd);
        exit(EXIT_FAILURE);
    }

    writefd = open(fifonam2, O_WRONLY | O_NONBLOCK);
    if(-1==writefd)
    {
        perror("ftee: writefd: open()");
        //printf("ftee: writefd: open(%s)\n", fifonam2);
	close(readfd);
	close(tmp);
        exit(EXIT_FAILURE);
    }
#if 0
    struct stat status;
    if(-1==fstat(readfd, &status))
    {
        perror("ftee: fstat");
        close(readfd);
        exit(EXIT_FAILURE);
    }

    if(!S_ISFIFO(status.st_mode))
    {
        printf("ftee: %s in not a fifo!\n", fifonam);
        close(readfd);
        exit(EXIT_FAILURE);
    }

    writefd = open(fifonam, O_WRONLY | O_NONBLOCK);
    if(-1==writefd)
    {
        perror("ftee: writefd: open()");
        close(readfd);
        exit(EXIT_FAILURE);
    }

    close(readfd);
#endif

    while(1)
    {
	r_bytes = read(readfd, buffer, BUFFER);
        if (r_bytes < 0 && errno == EINTR)
            continue;
        if (r_bytes <= 0)
            break;

	w_bytes = write(writefd, buffer, r_bytes);
	if (w_bytes == -1 && iter++)
	{
	    read(tmp, buffer, BUFFER);
	    //bytes = read(tmp, buffer, BUFFER);
	    //printf("droped = %d\n", bytes);
	    iter = 0;
	}
	//if (r_bytes != w_bytes)
	    //printf("rbytes = %d, w_bytes = %d\n", r_bytes, w_bytes);
    }

    printf("Exit\n");
    close(writefd);
    close(readfd);
    close(tmp);
    return(0);
}
