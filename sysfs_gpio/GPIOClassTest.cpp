#include "GPIOClass.h"
 
using namespace std;

#define GPIO_IN "in"
#define GPIO_OUT "out"
#define SEC_US 1000000
#define HIGH "1"
#define LOW "0"

int main (void)
{
 
    string inputstate;
    int i = 0;
    GPIOClass* gpio_out = new GPIOClass("4"); //create new GPIO object to be attached to  gpio_out
    GPIOClass* gpio_in = new GPIOClass("17"); //create new GPIO object to be attached to  gpio_in
 
    cout << " GPIO pins exported" << endl;
 
    gpio_out->setdir_gpio(GPIO_OUT); // gpio_out set to input
    gpio_in->setdir_gpio(GPIO_IN); //gpio_in set to output
 
    cout << " Set GPIO pin directions" << endl;
 
    while(i++ < 20)
    {
        usleep(0.5 * SEC_US);  // wait for 0.5 seconds
        gpio_in->getval_gpio(inputstate); //read state of gpio_out input pin
        cout << "Current input pin state is " << inputstate  <<endl;

        if(inputstate == HIGH) // if input pin is at state "0" i.e. button pressed
        {
                cout << "input pin state is definitely \"Pressed\". Turning LED ON" <<endl;
                gpio_out->setval_gpio(HIGH); // turn LED ON
        }
	else
	{
        	gpio_out->setval_gpio(LOW);
	}
    }
    
    cout << "Releasing heap memory and exiting....." << endl;
    delete gpio_in;
    delete gpio_out;
    gpio_in = NULL;
    gpio_out = NULL;
    
    return 0;
}
