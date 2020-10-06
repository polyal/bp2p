#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sys/mman.h>
#include "pwdGetter.h"

using namespace std;

struct termios PwdGetter::oflags; 
struct termios PwdGetter::nflags;

PwdGetter::PwdGetter()
{
}

bool PwdGetter::allocPwd(char** const pwd, unsigned int size)
{
	*pwd = new char[size];
	if (mlock(*pwd, size) == -1){
		cout << "PwdGetter: mlock error: " << errno << endl;
		return false;
	}
	else{
		memset(*pwd, 0x00, size);
	}
	return true;
}

bool PwdGetter::freePwd(char** const pwd, unsigned int size)
{
	if (*pwd){
		if (munlock(*pwd, size) == -1){
			cout << "PwdGetter: munlock error: " << errno << endl;
			return false;
		}
		delete *pwd;
		*pwd = nullptr;
	}
	return true;
}

bool PwdGetter::getPwd(char** const pwd, unsigned int size)
{
	if (!allocPwd(pwd, size))
		return false;
	if (!disableTerminalEcho())
		return false;
    fgets(*pwd, size, stdin);
    (*pwd)[strlen(*pwd)-1] = '\0'; // remove \n at the end of the password
    if (!restoreTerminal())
    	return false;
    return true;
}

bool PwdGetter::disableTerminalEcho()
{
	if (tcgetattr(fileno(stdin), &oflags) != 0)
		return false;
    nflags = oflags;
    nflags.c_lflag &= ~ECHO;
    nflags.c_lflag |= ECHONL;

    if (tcsetattr(fileno(stdin), TCSANOW, &nflags) != 0) {
        return false;
    }
    return true;
}

bool PwdGetter::restoreTerminal()
{
    if (tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0) {
        return false;
    }
    return true;
}