using namespace std;

#include "smarthome.h"

static int init_player_cmd(char *line, char **argv)
{
	string cmd;
	INIReader ini_file(ini_path(NULL));

	if (ini_file.ParseError() < 0) {
		cerr << LINE_INFO << "Can't load config: " << ini_path(NULL) << endl;
		return -1;
	}

	cmd = ini_file.Get("script", "player", "");
	memcpy(line, cmd.c_str(), cmd.size());
	line[cmd.size()] = '\0';

	parse_cmd(line, argv);

	return 0;
}

static void play_sound(void)
{
	static char  *argv[64], line[1024] = {};
	static pid_t pid = -1;

	if (!line[0])
	{
		if (init_player_cmd(line, argv))
		{
			cerr << LINE_INFO << "Couldn't read play command" << endl;
			exit(1);
		}
	}

	cerr << pid << endl;
	if (pid >= 0 && !kill(pid, 0))
	    return;//kill(pid, SIGKILL);

	if (!(pid = fork()))
	{
		if (execvp(*argv, argv) < 0)
			perror("play_sound(): failure");
		exit(1);
	}
}

static int read_gpio(struct gpio_t *gpio)
{
	if (poll(&gpio->fds_in, 1, gpio->timeout) == -1)
	{
		cerr << LINE_INFO << "Couldn't read gpio status" << endl;
		return -1;
	}

	gpio->gpio_in->getval_gpio(gpio->in_stat);
	lseek(gpio->fds_in.fd, 0, SEEK_SET);

	return 0;
}

static int init_gpio(struct gpio_t *gpio)
{
	INIReader ini_file(ini_path(NULL));

	if (ini_file.ParseError() < 0) {
		cerr << LINE_INFO << "Can't load config: " << ini_path(NULL) << endl;
		return -1;
	}

	gpio->gpio_in = new GPIOClass(RPI2_GPIO_11);
	gpio->gpio_in->setdir_gpio(GPIO_IN);
	gpio->gpio_in->setedge_gpio(GPIO_EDGE_RISING);

	gpio->gpio_out = new GPIOClass(RPI2_GPIO_7); //added LED for testing only
	gpio->gpio_out->setdir_gpio(GPIO_OUT);

	gpio->fds_in.fd = gpio->gpio_in->get_filefd();
	gpio->fds_in.events = POLLPRI;
	gpio->timeout_def = ini_file.GetInteger("gpio", "high_timeout", -1);
	if (gpio->timeout_def < 0) {
		cerr << LINE_INFO << "Can't read [gpio]:high_timeout" << endl;
		return -1;
	}

	return 0;
}

static int need_set_high(struct gpio_t *gpio)
{
	return gpio->in_stat == HIGH && !gpio->stat;
}

static int need_set_low(struct gpio_t *gpio)
{
	return gpio->in_stat == LOW && gpio->stat;
}

static int set_gpio(struct gpio_t *gpio, int is_high)
{
	gpio->stat = is_high;
	gpio->timeout = is_high ? gpio->timeout_def : -1;

	gpio->gpio_out->setval_gpio(is_high ? HIGH : LOW);

	return 0;
}

static int alarms_high(void *e)
{
	class Log_mess *log = (class Log_mess *)e;

	log->log_write("high gpio");
	debug("gpio UP");

	play_sound();

	return 0;
}

static int alarms_low(void *e)
{
	debug("gpio DOWN");

	return 0;
}

void *gpio_read(void *data)
{
	time_t last_change_time;
	struct gpio_t *gpio = (struct gpio_t *)data;
	class Log_mess log((current_path_get() + "log.txt").c_str());

	log.log_open();

	if (init_gpio(gpio))
		exit(1);

	last_change_time = time(NULL) - gpio->timeout_def;

	while(1)
	{
		if (read_gpio(gpio))
			exit(1);

		if (need_set_high(gpio) && (time(NULL) - last_change_time) > gpio->timeout_def)
		{
			last_change_time = time(NULL);
			set_gpio(gpio, 1);
			alarms_high(&log);
		}
		else if (need_set_low(gpio))
		{
			set_gpio(gpio, 0);
			alarms_low(NULL);
		}
		usleep(100*1000);
	}

	return NULL;
}
