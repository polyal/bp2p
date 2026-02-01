#ifndef PWDGETTER_H
#define PWDGETTER_H

#include <termios.h>

class PwdGetter
{
public:

	static bool getPwd(char** const pwd, unsigned int size);
	static bool freePwd(char** const pwd, unsigned int size);

private:
	PwdGetter();

	static bool allocPwd(char** const pwd, unsigned int size);
	static bool disableTerminalEcho();
	static bool restoreTerminal();

	static struct termios oflags, nflags;
};

#endif
