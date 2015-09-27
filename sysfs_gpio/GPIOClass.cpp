#include "GPIOClass.h"

using namespace std;

#define DEFAULT_PORT "21"

GPIOClass::GPIOClass():valuefd(-1),directionfd(-1),exportfd(-1),unexportfd(-1),gpionum(DEFAULT_PORT)
{
        //GPIO4 is default
	this->unexport_gpio(); //close if it is already opened
	this->export_gpio();
}

GPIOClass::GPIOClass(string gnum):valuefd(-1),directionfd(-1),exportfd(-1),unexportfd(-1),gpionum(gnum)
{
	//Instatiate GPIOClass object for GPIO pin number "gnum"
	this->unexport_gpio(); //close if it is already opened
	this->export_gpio();
}

GPIOClass::~GPIOClass()
{
	this->unexport_gpio();
}


int GPIOClass::export_gpio()
{
	
	string numStr;
	stringstream ss;
	int statusVal = -1;
	string exportStr = "/sys/class/gpio/export";

	this->exportfd = statusVal = open(exportStr.c_str(),  O_WRONLY|O_SYNC);
	if (statusVal < 0){
		perror("could not open SYSFS GPIO export device");
        	exit(1);
	}

	ss << this->gpionum;
	numStr = ss.str();
	cout << numStr << "Text" << endl;

	statusVal = write(this->exportfd, numStr.c_str(), numStr.length());
	if (statusVal < 0){
		perror("could not write to SYSFS GPIO export device");
        	exit(1);
	}
	
	statusVal = close(this->exportfd);
	if (statusVal < 0){
		perror("could not close SYSFS GPIO export device");
        	exit(1);
	}

	return statusVal;
}

int GPIOClass::unexport_gpio()
{
	string numStr;
	stringstream ss;
	int statusVal = -1;
	string unexportStr = "/sys/class/gpio/unexport";

	this->unexportfd = statusVal = open(unexportStr.c_str(),  O_WRONLY|O_SYNC);
	if (statusVal < 0){
		perror("could not open SYSFS GPIO unexport device");
        	//exit(1);
	}

	ss << this->gpionum;
	numStr = ss.str();

	statusVal = write(this->unexportfd, numStr.c_str(), numStr.length());
	if (statusVal < 0){
		perror("could not write to SYSFS GPIO unexport device");
        	//exit(1);
	}
	
	statusVal = close(this->unexportfd);
	if (statusVal < 0){
		perror("could not close SYSFS GPIO unexport device");
        	//exit(1);
	}
	
	return statusVal;
}

int GPIOClass::setdir_gpio(string dir)
{
	int statusVal = -1;
	string setdirStr ="/sys/class/gpio/gpio" + this->gpionum + "/direction";
	
	
	this->directionfd = statusVal = open(setdirStr.c_str(), O_WRONLY|O_SYNC); // open direction file for gpio
	if (statusVal < 0){
		perror("could not open SYSFS GPIO direction device");
       		exit(1);
	}
		
	if (dir.compare("in") != 0 && dir.compare("out") != 0 ) {
		perror("Invalid direction value. Should be \"in\" or \"out\".");
		exit(1);
	}
		
	statusVal = write(this->directionfd, dir.c_str(), dir.length());
	if (statusVal < 0){
		perror("could not write to SYSFS GPIO direction device");
        	exit(1);
	}
	
	statusVal = close(this->directionfd);
	if (statusVal < 0){
		perror("could not close SYSFS GPIO direction device");
        	exit(1);
	}

	return statusVal;
}


int GPIOClass::setval_gpio(string val)
{
	int statusVal = -1;
	string setValStr = "/sys/class/gpio/gpio" + this->gpionum + "/value";
	
	this->valuefd = statusVal = open(setValStr.c_str(), O_WRONLY|O_SYNC);
	if (statusVal < 0){
		perror("could not open SYSFS GPIO value device");
        	exit(1);
	}
		
	if (val.compare("1") != 0 && val.compare("0") != 0 ) {
		perror("Invalid  value. Should be \"1\" or \"0\".");
		exit(1);
	}
		
	statusVal = write(this->valuefd, val.c_str(), val.length());
	if (statusVal < 0){
		perror("could not write to SYSFS GPIO value device");
        	exit(1);
	}
	
	statusVal = close(this->valuefd);
	if (statusVal < 0){
		perror("could not close SYSFS GPIO value device");
        	exit(1);
	}

	return statusVal;
}


int GPIOClass::getval_gpio(string& val){

	string getValStr = "/sys/class/gpio/gpio" + this->gpionum + "/value";
	char buff[10];
	int statusVal = -1;

	this->valuefd = statusVal = open(getValStr.c_str(), O_RDONLY|O_SYNC);
	if (statusVal < 0){
		perror("could not open SYSFS GPIO value device");
        	exit(1);
	}

	statusVal = read(this->valuefd, &buff, 1);
	if (statusVal < 0){
		perror("could not read SYSFS GPIO value device");
        	exit(1);
	}
	
	buff[1]='\0';
	
	val = string(buff);
	
	if (val.compare("1") != 0 && val.compare("0") != 0 ) {
		perror("Invalid  value read. Should be \"1\" or \"0\".");
		exit(1);
	}
	
	statusVal = close(this->valuefd);
	if (statusVal < 0){
		perror("could not close SYSFS GPIO value device");
        	exit(1);
	}

	return statusVal;
}

string GPIOClass::get_gpionum(){
	return this->gpionum;
}

int GPIOClass::get_filefd(){
	return this->valuefd;
}
