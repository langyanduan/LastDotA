#include "common.h"

// forward
DWORD WINAPI UnloadProc(PVOID param);


BOOL APIENTRY DllMain( 
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
    )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		{
            DisableThreadLibraryCalls(hModule);

			DWORD hGameLib;

			FkDbgPrint((TEXT("LibEntry")));

			g_hModule = hModule;
			hGameLib = (DWORD)GetModuleHandle(TEXT("game.dll"));

			FkDbgPrint((TEXT("game.dll: %08X"), hGameLib));

            g_pUnitManager = new CUnitManager(hGameLib);
            g_pCmdHandler  = new CCmdHandler(g_pUnitManager, TEXT("\\\\.\\Mailslot\\FF68535B-4AF0-72F6-0F5B-6A40141468E3"));
			g_pD3DHook	   = new CD3DHook();

			g_pCmdHandler->Listen();
			g_pD3DHook->StartPatch();
        }
        break;
    case DLL_PROCESS_DETACH:
        {
			if (g_pD3DHook)
			{
				delete g_pD3DHook;
				g_pD3DHook = NULL;
			}

			if (g_pUnitManager)
			{
				delete g_pUnitManager;
				g_pUnitManager = NULL;
			}

			if (g_pCmdHandler)
			{
				delete g_pCmdHandler;
				g_pCmdHandler = NULL;
			}
        }
        break;
    }

    return TRUE;
}

DWORD WINAPI UnloadProc(PVOID param)   
{   
	FreeLibraryAndExitThread((HMODULE)g_hModule, 0);
	return 0;   
}