#ifndef LOG_MESS_CLASS_H
#define LOG_MESS_CLASS_H

#include <iostream>
#include <string>
#include <fstream>
#include <string.h>
#include <time.h>

class Log_mess {
private:
	string file_path;
	ofstream file;
	string get_time(void);
public:
	Log_mess(string file_name);
	~Log_mess();
	int log_open(void);
	int log_close(void);
	int log_write(string str);
};

#endif
