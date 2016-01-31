#ifndef LOG_MESS_CLASS_H
#define LOG_MESS_CLASS_H

#include <iostream>
#include <string>
#include <fstream>
#include <string.h>
#include <time.h>

using namespace std;

class Log_mess {
private:
	string file_path;
	string message;
	ofstream file;
	string get_time(void);
public:
	Log_mess();
	Log_mess(string file);
	~Log_mess();
	int log_open();
	int log_open(string file_path);
	int log_close(void);
	int log_write(string str);
};

#endif
