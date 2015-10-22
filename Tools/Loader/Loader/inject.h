// inject.h

#ifndef __INJECT_H__
#define __INJECT_H__

#include <windows.h>
#include <iostream>

HMODULE InjectDLL(DWORD ProcessID, char* dllName);

#endif