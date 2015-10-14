#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <poll.h>
#include <fstream>
#include "../sysfs_gpio/GPIOClass.h"
#include "../sysfs_gpio/GPIORpi2.h"

#define DEBUG 0
#define BUFFER 65536
#define TIMEOUT_POLL_MS 5000
#define PATH_SIZE 120

#define debug_print(fmt, ...) \
	do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
		__LINE__, __func__, __VA_ARGS__); } while (0)

static unsigned int GPIO_STAT = 0;

enum ERROR_TYPE
{
    NO_ERR = 0,
    ERR = 1,
    ERR_ARG = 2,
    ERR_OPEN_FILE = 3,
    ERR_NO_FIFO = 4,
    ERR_READ = 5,
    ERR_WRITE = 6,
    ERR_PTHREAD = 7,
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

void *gpio_read(void *data)
{
    struct pollfd fds[1];
    string inputstate;
    int timeout = -1;
    GPIOClass* gpio_in = new GPIOClass(RPI2_GPIO_11);
    GPIOClass* gpio_out = new GPIOClass(RPI2_GPIO_7); //added LED for testing only

    gpio_in->setdir_gpio(GPIO_IN);
    gpio_in->setedge_gpio(GPIO_EDGE_RISING);
    gpio_out->setdir_gpio(GPIO_OUT);

    fds[0].fd = gpio_in->get_filefd();
    fds[0].events = POLLPRI;

    while(1)
    {
	if (poll(fds, 1, timeout) == -1)
	    exit(1);

	gpio_in->getval_gpio(inputstate);
	lseek(fds[0].fd, 0, SEEK_SET);

	if (inputstate == HIGH && !GPIO_STAT)
        {
	    GPIO_STAT = 1;
	    timeout = TIMEOUT_POLL_MS;

	    debug_print("gpio UP (%d)\n", 1);
            gpio_out->setval_gpio(HIGH);
        }
	else if (inputstate == LOW && GPIO_STAT)
	{
	    GPIO_STAT = 0;
	    timeout = -1;

	    debug_print("gpio DOWN (%d)\n", 0);
            gpio_out->setval_gpio(LOW);
	}
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t thread_gpio_r;
    ofstream myfile;
    char *fifo_src, *fifo_dst, *video_dir;
    char buffer[BUFFER + 1];
    ssize_t r_bytes, w_bytes;
    int srcfd_read, dstfd_write, dstfd_read;
    int iter = 0;
    enum ERROR_TYPE ret = NO_ERR;
    char file_name[PATH_SIZE], is_active = 0;

    if(argc != 4)
    {
        printf("Usage:\n nameprog fifo_src fifo_dst /path/to/video/dir/\n");
	ret = ERR_ARG;
	goto Exit;
    }
    video_dir = argv[1];
    fifo_src = argv[2];
    fifo_dst = argv[3];

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

    if(!is_fifo(srcfd_read) || !is_fifo(dstfd_write))
    {
        perror("some of files is not a fifo!\n");
	ret = ERR_NO_FIFO;
	goto Exit_3;
    }

    if(pthread_create(&thread_gpio_r, NULL, gpio_read, NULL))
    {
	perror("can not create thread gpio_read()\n");
	ret = ERR_PTHREAD;
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

	if (GPIO_STAT && !is_active)
	{
	    time_t t = time(0);
	    strftime(file_name, PATH_SIZE, "/CAM_%Y-%m-%d_%H:%M:%S.ts", localtime(&t));
	    strcat(file_name, video_dir);

	    myfile.open(file_name);
	    if (myfile.is_open())
		is_active = 1;
	}
	else if (!GPIO_STAT && is_active)
	{
	    if (myfile.is_open())
		myfile.close();
	    is_active = 0;
	}
	else if (GPIO_STAT && is_active)
	{
	    myfile.write(buffer, r_bytes);
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
