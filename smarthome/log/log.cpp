#include "log.h"

Log_mess::Log_mess():file_path(0),message(0){}

Log_mess::Log_mess(string file):file_path(file),message(0){}

Log_mess::~Log_mess() {
	log_close();
}

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
string Log_mess::get_time(void)
{
    char buf[80];
    time_t now = time(0);
    struct tm tstruct = *localtime(&now);

    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

int Log_mess::log_open()
{
	if (!file_path.size())
	{
		cerr << "The filename isn't init'ed" << endl;
		return -1;
	}

	file.open(file_path.c_str(), std::ios::app);
	if (!file.is_open())
	{
		cerr << "Could not open log file:" << file_path << endl;
		return -1;
	}

	return 0;
}

int Log_mess::log_open(string file_path)
{
	if (!file_path.size())
		return -1;

	file.open(file_path.c_str(), std::ios::app);
	if (!file.is_open())
	{
		cerr << "Could not open log file:" << file_path << endl;
		return -1;
	}

	return 0;
}

int Log_mess::log_write(string str)
{
	string date;

	if (!file.is_open())
	{
		cerr << "The file:" << file_path << " is closed" << endl;
		return -1;
	}

	file << get_time() << " " << str << endl;

	return 0;
}

int Log_mess::log_close(void)
{
	file.close();

	return 0;
}
