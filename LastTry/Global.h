#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define FKDBG

// global
extern CUnitManager * g_pUnitManager;
extern CCmdHandler  * g_pCmdHandler;
extern CD3DHook		* g_pD3DHook;

extern HMODULE g_hModule;

#ifdef FKDBG
void DbgOutput(const wchar_t * strOutputString, ...);
#endif

#ifdef FKDBG
#define FkDbgPrint(x) DbgOutput##x
#else
#define FkDbgPrint(x)
#endif

#endif