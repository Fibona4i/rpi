using namespace std;

#include "smarthome.h"

static string init_sound_cmd(void)
{
	INIReader ini_file(ini_path(NULL));

	if (ini_file.ParseError() < 0) {
		cerr << LINE_INFO << "Can't load config: " << ini_path(NULL) << endl;
		return NULL;
	}

	return ini_file.Get("script", "play_sound", 0);
}

static void play_sound(void)
{
	static string cmd = NULL;

	if (cmd.empty())
	{
		if ((cmd = init_sound_cmd()).empty())
		{
			cerr << LINE_INFO << "Couldn't read play sound command" << endl;
			exit(1);
		}
	}

	if (!fork())
	{
		debug("exec " << cmd);
		execl(cmd.c_str(), "", NULL);
		_exit(0);
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
	gpio->timeout_def = ini_file.GetInteger("video", "duration_min", -1);
	if (gpio->timeout_def < 0) {
		cerr << LINE_INFO << "Can't read [video]:duration_min" << endl;
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

static int set_high(struct gpio_t *gpio)
{
	gpio->stat = 1;
	gpio->timeout = gpio->timeout_def;

	gpio->gpio_out->setval_gpio(HIGH);

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

static int set_low(struct gpio_t *gpio)
{
	gpio->stat = 0;
	gpio->timeout = -1;

	gpio->gpio_out->setval_gpio(LOW);

	return 0;
}

void *gpio_read(void *data)
{
	struct gpio_t *gpio = (struct gpio_t *)data;
	class Log_mess log((path_def_get() + "log.txt").c_str());
	log.log_open();

	init_gpio(gpio);

	while(1)
	{
		if (read_gpio(gpio))
			exit(1);

		if (need_set_high(gpio))
		{
			set_high(gpio);
			alarms_high(&log);
			usleep(100*1000);
		}
		else if (need_set_low(gpio))
		{
			set_low(gpio);
			alarms_low(NULL);
		}
	}

	return NULL;
}
