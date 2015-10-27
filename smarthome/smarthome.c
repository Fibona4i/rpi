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
#define V_DURATION 10
#define TIMEOUT_POLL_MS (V_DURATION*1000)
#define PATH_SIZE 128
#define NAME_SIZE 64
#define SEC_VIDEO_SIZE (1*1024*1024)

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
    ERR_ALLOC = 8,
};

typedef struct {
    char *head;
    char *curr;
    unsigned int full_size;
} Cycle_buf;

struct fifo_free_ctx {
    int fd;
    int need_clean;
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

void *fifo_free(void *data)
{
    char buffer[BUFFER + 1];
    struct fifo_free_ctx *fifo_ctx = (struct fifo_free_ctx *)data;

    while(1)
    {
	if (fifo_ctx->need_clean)
	{
            read(fifo_ctx->fd, buffer, BUFFER);
	    fifo_ctx->need_clean = 0;
	}
	usleep(100*1000);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t thread_gpio_r, clean_fifo;
    ofstream myfile;
    char *fifo_src, *fifo_dst, *video_dir;
    char buffer[BUFFER + 1];
    ssize_t r_bytes;
    int srcfd_read, dstfd_write, v_buf_iter = -1;
    enum ERROR_TYPE ret = NO_ERR;
    char file_name[NAME_SIZE] = {}, path[PATH_SIZE] = {}, is_active = 0;
    Cycle_buf vbuf[V_DURATION] = {};
    time_t t, prev_t = 0;
    struct fifo_free_ctx fifo_ctx = {};

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

    fifo_ctx.fd = open(fifo_dst, O_RDONLY | O_NONBLOCK);
    if(fifo_ctx.fd == -1)
    {
        perror("ftee: fifo_ctx.fd: open()");
	ret = ERR_OPEN_FILE;
	goto Exit_1;
    }

    dstfd_write = open(fifo_dst, O_WRONLY | O_NONBLOCK | O_ASYNC | O_NOATIME);
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

    if(pthread_create(&clean_fifo, NULL, fifo_free, &fifo_ctx))
    {
	perror("can not create thread fifo_free()\n");
	ret = ERR_PTHREAD;
	goto Exit_3;
    }

    /* init video pre-buffer */
    for (int i=0; i < V_DURATION; i++)
    {
	vbuf[i].full_size = SEC_VIDEO_SIZE;
	if ((vbuf[i].head = vbuf[i].curr = (char *)malloc(SEC_VIDEO_SIZE)))
	    continue;

	perror("can not alloc memory for video pre-buffer\n");
	ret = ERR_ALLOC;
	goto Exit_4;
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

	if (!fifo_ctx.need_clean && write(dstfd_write, buffer, r_bytes) == -1)
	    fifo_ctx.need_clean = 1;

	if ((t = time(0)) != prev_t)
	{
	    prev_t = t;
	    if (++v_buf_iter >= V_DURATION)
		    v_buf_iter = 0;
	    vbuf[v_buf_iter].curr = vbuf[v_buf_iter].head;
	}
	
	if ((vbuf[v_buf_iter].curr - vbuf[v_buf_iter].head + r_bytes) < vbuf[v_buf_iter].full_size)
	{
	    memcpy(vbuf[v_buf_iter].curr, buffer, r_bytes);
	    vbuf[v_buf_iter].curr += r_bytes;
	}
	else
	{
	    perror("video bitrade is too high. Can't save pre-buffer stream\n");
	    ret = ERR;
	    goto Exit_4;
	}

	if (GPIO_STAT && !is_active)
	{
	    strftime(file_name, NAME_SIZE, "CAM_%Y-%m-%d_%H:%M:%S.ts", localtime(&t));
	    strcpy(path, video_dir);
	    strcat(path, file_name);

	    myfile.open(path, ios::out | ios::binary);
	    if (!myfile.is_open())
		break;
	    is_active = 1;

	    for (int i=0, j=v_buf_iter; i < V_DURATION; i++)
	    {
		if (++j >= V_DURATION)
		    j = 0;
		myfile.write(vbuf[j].head, vbuf[j].curr - vbuf[j].head);
	    }
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
    }

Exit_4:
    for (int i=0; i < V_DURATION; i++)
	free(vbuf[i].head);
Exit_3:
    close(dstfd_write);
Exit_2:
    close(fifo_ctx.fd);
Exit_1:
    close(srcfd_read);
Exit:
    return ret;
}
