/**
 * \file    	EnableWindowsConsoleColor.hpp
 * \author  	XY0797
 * \date    	2024.8.6
 * \brief		让Windows的默认终端启用彩色显示
 */
#ifndef _XY0797_ENABLEWINDOWSCONSOLECOLOR
#define _XY0797_ENABLEWINDOWSCONSOLECOLOR 1

#include <windows.h>

bool EnableWindowsConsoleColor() {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE) {
		return false;
	}

	DWORD dwMode = 0;
	if (!GetConsoleMode(hOut, &dwMode)) {
		return false;
	}

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if (!SetConsoleMode(hOut, dwMode)) {
		return false;
	}

	return true;
}

#endif /* _XY0797_ENABLEWINDOWSCONSOLECOLOR */
