#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

using namespace std;

int perror_exit(string msg)
{
	perror(msg.c_str());
       	exit(1);
}

string number_to_srt(int number)
{
	ostringstream ss;
	ss << number;
	return ss.str();
}

bool exist(const string &name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0); 
}
