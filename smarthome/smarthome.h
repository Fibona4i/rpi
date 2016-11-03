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
#include <pwd.h>
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

struct gpio_t {
	int stat; //configured state of gpio_in
	int timeout; //configured timeout
	int timeout_def; //default timeout (config file)
	bool in_stat; //current real grio status
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

string current_path_get(void);
char *ini_path(char *path);
void *gpio_read(void *data);
void parse_cmd(char *line, char **argv);
