#include "GPIOClass.h"

using namespace std;

bool wait_for_gpio(string file, int msec)
{
	while (!exist(file) && --msec)
		usleep(1000);

	return !msec;
}

GPIOClass::GPIOClass(int gpio, string in_out)
{
	this->gpio = gpio;
	this->unexport_gpio(); //close if it was opened before
	this->export_gpio();

	this->set_gpio_direction(in_out);
	this->set_gpio_edge(GPIO_EDGE_RISING);
}

GPIOClass::GPIOClass(int gpio, string in_out, string edge)
{
	this->gpio = gpio;
	this->unexport_gpio(); //close if it was opened before
	this->export_gpio();

	this->set_gpio_direction(in_out);
	this->set_gpio_edge(edge);
}

GPIOClass::~GPIOClass()
{
	this->unexport_gpio();
}

void GPIOClass::export_gpio()
{
	string numStr, ValStr;
	int exportfd;

	if (this->gpio < 0)
		perror_exit("could not export uninited gpio");

	exportfd = open("/sys/class/gpio/export", O_WRONLY|O_SYNC);
	if (exportfd < 0)
		perror_exit("could not open SYSFS GPIO export device");

	numStr = number_to_srt(this->gpio);
	if (write(exportfd, numStr.c_str(), numStr.length()) < 0)
		perror_exit("could not write to SYSFS GPIO export device");

	if (close(exportfd) < 0)
		perror_exit("could not close SYSFS GPIO export device");

	ValStr = "/sys/class/gpio/gpio" + number_to_srt(this->gpio) + "/value";
	if (wait_for_gpio(ValStr, 1000))
		perror_exit("/sys/class/gpio/gpio/NUMB has not been created");

	this->gpio_fd = open(ValStr.c_str(), O_RDWR|O_SYNC);
	if (this->gpio_fd < 0)
		perror_exit("Could not open SYSFS GPIO value device");
}

void GPIOClass::unexport_gpio()
{
	string numStr;
	int unexportfd;

	if (this->gpio_fd != -1 && close(this->gpio_fd))
		perror("could not close SYSFS GPIO value device");

	unexportfd = open("/sys/class/gpio/unexport", O_WRONLY|O_SYNC);
	if (unexportfd < 0) {
		perror("could not open SYSFS GPIO unexport device");
		return;
	}

	numStr = number_to_srt(this->gpio);
	if (write(unexportfd, numStr.c_str(), numStr.length()) < 0)
		perror("could not write to SYSFS GPIO unexport device");

	if (close(unexportfd) < 0)
		perror("could not close SYSFS GPIO unexport device");
}

void GPIOClass::set_gpio_direction(string dir)
{
	string setdirStr ="/sys/class/gpio/gpio" + number_to_srt(this->gpio) + "/direction";
	int directionfd = open(setdirStr.c_str(), O_WRONLY|O_SYNC); // open direction file for gpio

	if (directionfd < 0)
		perror_exit("could not open SYSFS GPIO direction device");

	if (dir.compare(GPIO_IN) != 0 && dir.compare(GPIO_OUT) != 0 )
		perror_exit("Invalid direction value. Should be \"in\" or \"out\".");

	if (write(directionfd, dir.c_str(), dir.length()) < 0)
		perror_exit("could not write to SYSFS GPIO direction device");

	if (close(directionfd) < 0)
		perror_exit("could not close SYSFS GPIO direction device");
}

void GPIOClass::set_gpio_edge(string edge)
{
	int edge_fd;
	string setedgeStr ="/sys/class/gpio/gpio" + number_to_srt(this->gpio) + "/edge";

	edge_fd = open(setedgeStr.c_str(), O_WRONLY|O_SYNC); // open edge file for gpio
	if (edge_fd < 0)
		perror_exit("could not open SYSFS GPIO edge device");

	if (edge.compare(GPIO_EDGE_NONE) != 0 && edge.compare(GPIO_EDGE_RISING) != 0 &&
		edge.compare(GPIO_EDGE_FALLING) != 0 &&	edge.compare(GPIO_EDGE_BOTH) != 0) {
		perror_exit("Invalid edge value. Should be \"none\", \"rising\" or \"falling\".");
	}

	if (write(edge_fd, edge.c_str(), edge.length()) < 0)
		perror_exit("could not write to SYSFS GPIO edge device");

	if (close(edge_fd) < 0)
		perror_exit("could not close SYSFS GPIO edge device");
}

void GPIOClass::set_gpio_value(int val)
{
	string val_str = number_to_srt(val);
	if (write(this->gpio_fd, val_str.c_str(), val_str.length()) < 0)
		perror_exit("could not write to SYSFS GPIO value device");
}

bool GPIOClass::get_gpio_value(void)
{
	char buff[8];

	if (read(this->gpio_fd, &buff, 8) < 0)
		perror_exit("could not read SYSFS GPIO value device");

	return atoi(buff);
}

int GPIOClass::get_gpio(){
	return this->gpio;
}

int GPIOClass::get_gpio_fd(){
	return this->gpio_fd;
}
