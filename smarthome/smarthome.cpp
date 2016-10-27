using namespace std;
int Debug = 1;

#include "smarthome.h"

class Log_mess log_smart((current_path_get() + "log_smart.txt").c_str());

char *ini_path(char *path)
{
	static char *def_path = NULL;

	if (path)
	{
		free(def_path);
		def_path = strdup(path);
	}

	return def_path;
}

static int init_debug(void)
{
	INIReader ini_file(ini_path(NULL));

	if (ini_file.ParseError() < 0) {
		cout << "Can't load config: " << ini_path(NULL) << endl;
		return -1;
	}

	Debug = ini_file.GetInteger("debug", "enabled", -1);
	if (Debug < 0)
	{
		cerr << LINE_INFO << "Couldn't read Debug value" << endl;
		return -1;
	}
	cerr << "Dubug status: " << (Debug ? "on" : "off") << endl;

	return 0;
}

string current_path_get(void)
{
	ssize_t count;
	char result[PATH_MAX];
	static string path = "";

	if (path.empty())
	{
		count = readlink("/proc/self/exe", result, PATH_MAX);
		path = string(result, (count > 0) ? count : 0);
		path = path.substr(0, path.find_last_of('/')) + "/";
	}

	return path;
}

void setuser(const char *user) {
        struct passwd *pw = getpwnam(user);

        if (!pw || setgid(pw->pw_gid) || setuid(pw->pw_uid))
	{
		cerr << LINE_INFO << "Couldn't change user: " << user << endl;
        	exit(1);
	}
}

static int gpio_thread_crate(pthread_t *thread_gpio_r, struct gpio_t *gpio)
{
	/* Linux Device Tree should be disabled to have access for non-root */
	if (pthread_create(thread_gpio_r, NULL, gpio_read, gpio))
	{
		cerr << LINE_INFO << "Couln't open gpio thread" << endl;
		return -1;
	}

	return 0;
}

static inline int is_gpio_high(struct gpio_t *gpio)
{
	return gpio->stat;
}

static string get_time_str(void)
{
	char buf[80];
	time_t now = time(0);

	strftime(buf, sizeof(buf), "CAM_%Y-%m-%d:%M:%S.ts", localtime(&now));

	return buf;
}

static string get_video_dir_name(void)
{
	static string vdir = "";

	if (vdir.empty())
	{
		INIReader ini_file(ini_path(NULL));

		if (ini_file.ParseError() < 0) {
			cout << LINE_INFO << "Can't load config: " << ini_path(NULL) << endl;
			return NULL;
		}

		vdir = ini_file.Get("path", "vfolder", "");

		if (vdir.empty())
		{
			cerr << LINE_INFO << "Couldn't read vfolder" << endl;
			return NULL;
		}
	}

	return vdir;
}

void parse_cmd(char *line, char **argv)
{
	while (*line != '\0') {         /* if not the end of line ....... */
		while (*line == ' ' || *line == '\t' || *line == '\n')
			*line++ = '\0'; /* replace white spaces with 0    */
		*argv++ = line;         /* save the argument position     */
		while (*line != '\0' && *line != ' ' &&	*line != '\t' && *line != '\n')
			line++;         /* skip the argument until ...    */
	}
	*argv = NULL;                   /* mark the end of argument list  */
}

static int fork_cli_cmd(pid_t *pid, string &cmd)
{
	char  *argv[64], line[1024];

	if (!(*pid = fork()))
	{
		debug("exec " << cmd);
		memcpy(line, cmd.c_str(), cmd.size());
		line[cmd.size()] = '\0';

		parse_cmd(line, argv);
		if (execvp(*argv, argv) < 0)
			perror("ERROR: system failure");
		exit(1);
	}

	if (*pid == -1)
	{
		cerr << LINE_INFO << "Couldn't fork: " << cmd << endl;
		return -1;
	}

	return 0;
}

static int init_video_cam(pid_t *encoder_pid, pid_t *streamer_pid)
{
	static string encoder, streamer;
	INIReader ini_file(ini_path(NULL));

	if (ini_file.ParseError() < 0) {
		cout << LINE_INFO << "Can't load config: " << ini_path(NULL) << endl;
		return -1;
	}

	encoder = ini_file.Get("video", "encoder", "");
	streamer = ini_file.Get("video", "streamer", "");

	//debug("encoder = [" << encoder << "]" << endl << "streamer [" << streamer << "]");
	if (encoder.empty() || streamer.empty())
	{
		cerr << LINE_INFO << "Couldn't read ini_file encoder or streamer" << endl;
		return -1;
	}

	return fork_cli_cmd(encoder_pid, encoder) || fork_cli_cmd(streamer_pid, streamer);
}

static void uninit(struct gpio_t *gpio)
{
	delete gpio->gpio_in;
	delete gpio->gpio_out;
}

static int Smart_status = 1;

void sig_close(int signo)
{
	if (signo == 55)
		Smart_status = 0;

	cerr << "Got signo = " << signo << endl;
}

int main(int argc, char *argv[])
{
	pthread_t thread_gpio_r;
	pid_t encoder_pid = -1, streamer_pid = -1;
	struct gpio_t gpio = {0};
	signal(SIGCHLD, SIG_IGN);
	signal(55, &sig_close);

	log_smart.log_open();
	log_smart.log_write("smarthome start");

	if(argc != 2)
	{
		cerr << "Usage:\n nameprog /path/to/ini_file" << endl;
		return -1;
	}

	if (!ini_path(argv[1]))
		goto Exit;

	if (init_debug())
		goto Exit;

	if (init_video_cam(&encoder_pid, &streamer_pid))
		goto Exit;

	if (gpio_thread_crate(&thread_gpio_r, &gpio))
		goto Exit;

	while(Smart_status)
	{
		usleep(3*1000*1000);
		if (kill(encoder_pid, 0) || kill(streamer_pid, 0))
		{
			log_smart.log_write("encoder or streamer is not runned");
			goto Exit;
		}
	}

Exit:
	log_smart.log_write("Exit");
	cerr << "Exit(" << Smart_status << ")   " << LINE_INFO << endl;
	kill(encoder_pid, SIGKILL);
	kill(streamer_pid, SIGKILL);
	uninit(&gpio);
	return Smart_status;
}
