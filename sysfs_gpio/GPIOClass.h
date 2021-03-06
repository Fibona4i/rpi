
#ifndef GPIO_CLASS_H
#define GPIO_CLASS_H

#include <string>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define GPIO_IN "in"
#define GPIO_OUT "out"
#define GPIO_EDGE_NONE "none"
#define GPIO_EDGE_RISING "rising"
#define GPIO_EDGE_FALLING "falling"
#define GPIO_EDGE_BOTH "both"
#define HIGH "1"
#define LOW "0"

/* GPIO Class
 * Purpose: Each object instatiated from this class will control a GPIO pin
 * The GPIO pin number must be passed to the overloaded class constructor
 */
class GPIOClass
{
public:
    GPIOClass();
    GPIOClass(string gnum);
    ~GPIOClass();
    int setdir_gpio(string dir);
    int setedge_gpio(string dir);
    int setval_gpio(string val);
    int getval_gpio(string& val);
    string get_gpionum();
    int get_filefd();
private:
    int export_gpio();
    int unexport_gpio();

    int valuefd;
    int directionfd;
    int exportfd;
    int unexportfd;
    string gpionum;
};

#endif
