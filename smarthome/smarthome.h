#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <poll.h>
#include <fstream>
#include <new>
#include "../sysfs_gpio/GPIOClass.h"
#include "../sysfs_gpio/GPIORpi2.h"
#include "../ini_parser/INIReader.h"
#include "log/log.h"

#define LINE_INFO __FUNCTION__ << ":" << __LINE__ << " "
#define debug(str) \
	if (Debug) {\
		cerr << "[" << LINE_INFO << "][" << __FUNCTION__ << "] " << str << endl;}

struct ring_buf {
	char *head;
	char *curr;
	unsigned int full_size;
	struct ring_buf *next;
};

struct fifo_free_ctx {
	int fd;
	int need_clean;
	int pipe_size;
};

struct finf_t {
	int fd;
	char *path;
};

struct video_ctx {
	char *buf;
	int r_bytes;
	int duration;
	int sec_size;
};

struct vfifo_t {
	struct finf_t vsrc;
	struct finf_t vdst;
	struct fifo_free_ctx fifo_ctx;
	struct video_ctx v_ctx;
};

struct gpio_t {
	int stat;
	int in_progr;
	int timeout;
	int timeout_def;
	string in_stat;
	GPIOClass *gpio_in;
	GPIOClass *gpio_out;
	struct pollfd fds_in;
};

struct fsave_t {
	string name;
	string path; //video folder
	string path_tmp;
	ofstream file;
};

extern int Debug;

string path_def_get(void);
char *ini_path(char *path);
void *gpio_read(void *data);
