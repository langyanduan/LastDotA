#include "Common.h"

CUnitManager * g_pUnitManager = NULL;
CCmdHandler  * g_pCmdHandler  = NULL;
CD3DHook	 * g_pD3DHook	  = NULL;

HMODULE	g_hModule = NULL;


#ifdef FKDBG
void DbgOutput(const wchar_t * strOutputString, ...)
{
	wchar_t strBuffer[4096] = {0};
	va_list vlArgs;
	va_start(vlArgs, strOutputString);
	_vsnwprintf_s(strBuffer, sizeof(strBuffer) - 1, strOutputString, vlArgs);
	//vsprintf(strBuffer,strOutputString,vlArgs);
	va_end(vlArgs);
	OutputDebugString(strBuffer);
}
#endif 