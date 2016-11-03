#include "GPIOClass.h"

using namespace std;

bool wait_for_gpio(string file, int msec)
{
	while (msec--)
	{
		if (exist(file))
			break;
		usleep(1000);
	}

	return !exist(file);
}

GPIOClass::GPIOClass():valuefd(-1),directionfd(-1),exportfd(-1),unexportfd(-1),gpionum(-1)
{
}

GPIOClass::GPIOClass(int gpio):valuefd(-1),directionfd(-1),exportfd(-1),unexportfd(-1),gpionum(gpio)
{
	this->unexport_gpio(); //close if it was opened before
	this->export_gpio();
}

GPIOClass::~GPIOClass()
{
	this->unexport_gpio();
}

void GPIOClass::export_gpio()
{
	string numStr, ValStr;

	if (this->gpionum < 0)
		perror_exit("could not export uninited gpio");

	this->exportfd = open("/sys/class/gpio/export",  O_WRONLY|O_SYNC);
	if (this->exportfd < 0)
		perror_exit("could not open SYSFS GPIO export device");

	numStr = number_to_srt(this->gpionum);
	if (write(this->exportfd, numStr.c_str(), numStr.length()) < 0)
		perror_exit("could not write to SYSFS GPIO export device");
	
	if (close(this->exportfd) < 0)
		perror_exit("could not close SYSFS GPIO export device");

	ValStr = "/sys/class/gpio/gpio" + number_to_srt(this->gpionum) + "/value";
	if (wait_for_gpio(ValStr, 1000))
		perror_exit("/sys/class/gpio/gpio/NUMB has not been created");

	this->valuefd = open(ValStr.c_str(), O_RDWR|O_SYNC);
	if (this->valuefd < 0)
		perror_exit("Could not open SYSFS GPIO value device");
}

void GPIOClass::unexport_gpio()
{
	string numStr;

	if (this->valuefd != -1 && close(this->valuefd))
		perror("could not close SYSFS GPIO value device");

	this->unexportfd = open("/sys/class/gpio/unexport", O_WRONLY|O_SYNC);
	if (this->unexportfd < 0) {
		perror("could not open SYSFS GPIO unexport device");
		return;
	}

	numStr = number_to_srt(this->gpionum);
	if (write(this->unexportfd, numStr.c_str(), numStr.length()) < 0)
		perror("could not write to SYSFS GPIO unexport device");

	if (close(this->unexportfd) < 0)
		perror("could not close SYSFS GPIO unexport device");
}

void GPIOClass::set_gpio_direction(string dir)
{
	string setdirStr ="/sys/class/gpio/gpio" + number_to_srt(this->gpionum) + "/direction";

	this->directionfd = open(setdirStr.c_str(), O_WRONLY|O_SYNC); // open direction file for gpio
	if (this->directionfd < 0)
		perror_exit("could not open SYSFS GPIO direction device");

	if (dir.compare(GPIO_IN) != 0 && dir.compare(GPIO_OUT) != 0 )
		perror_exit("Invalid direction value. Should be \"in\" or \"out\".");

	if (write(this->directionfd, dir.c_str(), dir.length()) < 0)
		perror_exit("could not write to SYSFS GPIO direction device");

	if (close(this->directionfd) < 0)
		perror_exit("could not close SYSFS GPIO direction device");
}

void GPIOClass::set_gpio_edge(string edge)
{
	int edge_fd;
	string setedgeStr ="/sys/class/gpio/gpio" + number_to_srt(this->gpionum) + "/edge";

	edge_fd = open(setedgeStr.c_str(), O_WRONLY|O_SYNC); // open edge file for gpio
	if (edge_fd < 0)
		perror_exit("could not open SYSFS GPIO edge device");
		
	if (edge.compare(GPIO_EDGE_NONE) != 0 && edge.compare(GPIO_EDGE_RISING) != 0 &&
		edge.compare(GPIO_EDGE_FALLING) != 0 &&	edge.compare(GPIO_EDGE_BOTH) != 0)
	{
		perror_exit("Invalid edge value. Should be \"none\", \"rising\" or \"falling\".");
	}
		
	if (write(edge_fd, edge.c_str(), edge.length()) < 0)
		perror_exit("could not write to SYSFS GPIO edge device");

	if (close(edge_fd) < 0)
		perror_exit("could not close SYSFS GPIO edge device");
}

void GPIOClass::set_gpio_value(string val)
{
	if (val.compare(GPIO_HIGH) != 0 && val.compare(GPIO_LOW) != 0 )
		perror_exit("Invalid  value. Should be \"1\" or \"0\".");
		
	if (write(this->valuefd, val.c_str(), val.length()) < 0)
		perror_exit("could not write to SYSFS GPIO value device");
}


bool GPIOClass::get_gpio_value(void)
{
	char buff;

	if (read(this->valuefd, &buff, 1) < 0)
		perror_exit("could not read SYSFS GPIO value device");

	if (buff != '1' && buff != '0')
		perror_exit("Invalid  value read. Should be \"1\" or \"0\".");

	return buff == '1';
}

int GPIOClass::get_gpionum(){
	return this->gpionum;
}

int GPIOClass::get_filefd(){
	return this->valuefd;
}
