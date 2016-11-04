
#ifndef GPIO_CLASS_H
#define GPIO_CLASS_H

#include <string>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "../smarthome/common.h"

using namespace std;

#define GPIO_IN "in"
#define GPIO_OUT "out"
#define GPIO_EDGE_NONE "none"
#define GPIO_EDGE_RISING "rising"
#define GPIO_EDGE_FALLING "falling"
#define GPIO_EDGE_BOTH "both"
#define GPIO_HIGH "1"
#define GPIO_LOW "0"

/* GPIO Class
 * Purpose: Each object instatiated from this class will control a GPIO pin
 * The GPIO pin number must be passed to the overloaded class constructor
 */
class GPIOClass
{
public:
    GPIOClass(int gpio, string in_out);
    GPIOClass(int gpio, string in_out, string edge);
    ~GPIOClass();

    int get_gpio();
    int get_gpio_fd();
    bool get_gpio_value();
    void set_gpio_value(int val);
private:
    int gpio;
    int gpio_fd;

    void set_gpio_direction(string dir);
    void set_gpio_edge(string dir);
    void export_gpio();
    void unexport_gpio();
};

#endif
