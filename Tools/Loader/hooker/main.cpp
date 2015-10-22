// main.cpp

#include <Windows.h>
#include <d3d8.h>
#include <d3dx8.h>

// globals
DWORD* pVtable;

//EndScene (offset : 35)
BOOL _stdcall DrawText(LPDIRECT3DDEVICE8 pDxdevice,TCHAR* strText ,int nbuf);
typedef HRESULT (APIENTRY *EndScene_t)(IDirect3DDevice8* );
HRESULT APIENTRY hook_EndScene(IDirect3DDevice8* pInterface);
EndScene_t orig_EndScene;

DWORD WINAPI Patch_StealD3d8Device(LPVOID param);
DWORD WINAPI HookAPI(LPVOID param);
DWORD* GetVtableAddress(void* pObject);
void HookFunction(DWORD* pVtable, void* pHookProc, void* pOldProc, int iIndex);


BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls(hModule);

			HANDLE hpThread;
			hpThread = ::CreateThread(NULL, 0, Patch_StealD3d8Device, NULL, 0, NULL);
			CloseHandle(hpThread);

		}
		break;
	case DLL_PROCESS_DETACH:
		{
			DWORD hook_addr;
			HookFunction(pVtable, (void *)orig_EndScene, (void*)&hook_addr, 35);
		}
		break;
	}
	return TRUE;
}


DWORD WINAPI Patch_StealD3d8Device(LPVOID param)
{
	// Aquire base address of d3d8.dll
	int base_d3d8 = (int)GetModuleHandle(TEXT("d3d8"));

	// add offsets to get addresses
	const int addr_jmp =   base_d3d8 + 0x00076E33;
	const int addr_cave =  base_d3d8 + 0x000F3FD0;
	const int addr_value = base_d3d8 + 0x000F3FC7;

	// The bytecode we got
	byte jmp[] =  "\xE9\x98\xD1\x07"; // last ACTUAL byte is \x00 which works out with a null-terminated string
	byte cave[] = "\x8B\x06\x8B\x48\x08\x89\x35\xC5\x3F\x0B\x6D\xE9\x58\x2E\xF8\xFF"; // This null-terminated
	// buddy is ok because its floating in a sea of zeroes

	// virtualprotect the addresses for writing
	DWORD lpflOldProtect;

	// write jmps
	VirtualProtect((void*)addr_jmp, sizeof(jmp), PAGE_EXECUTE_READWRITE, &lpflOldProtect);
	memcpy((void*)addr_jmp, (void*)jmp, sizeof(jmp));
	VirtualProtect((void*)addr_jmp, sizeof(jmp), lpflOldProtect, &lpflOldProtect);

	// write caves
	VirtualProtect((void*)addr_cave, sizeof(cave), PAGE_EXECUTE_READWRITE, &lpflOldProtect);
	memcpy((void*)addr_cave, (void*)cave, sizeof(cave));
	// modify code to make sure that we store in the right place
	*(int*)(addr_cave + 7) = addr_value;
	VirtualProtect((void*)addr_cave, sizeof(cave), lpflOldProtect, &lpflOldProtect);

	// protect value addr for writing
	VirtualProtect((void*)addr_value, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &lpflOldProtect);

	// Wait for the value of the vtable and hook stuff
	HANDLE hThread = CreateThread(0, 0, HookAPI, 0, 0, 0);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	// restore jmp
	byte orig[] = {0x8B, 0x06, 0x8B, 0x48, 0x08};
	VirtualProtect((void*)addr_jmp, sizeof(orig), PAGE_EXECUTE_READWRITE, &lpflOldProtect);
	memcpy((void*)addr_jmp, (void*)orig, sizeof(orig));
	VirtualProtect((void*)addr_jmp, sizeof(orig), lpflOldProtect, &lpflOldProtect);

	return 0;
}

DWORD WINAPI HookAPI(LPVOID param)
{
	// Aquire base address of d3d8.dll
	int base_d3d8 = (int)GetModuleHandle(TEXT("d3d8")); //(int)GetModuleBaseAddress(GetCurrentProcessId(), "d3d8.dll");

	const int addr_value = base_d3d8 + 0x000F3FC7;

	Sleep(100); // wait for address to get written

	// protect value addr for reading / writing
	DWORD lpflOldProtect;
	VirtualProtect((void*)addr_value, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &lpflOldProtect);

	// poll the value until it gets written by our cave
	DWORD result = 0;
	while (!result) {
		result = *(DWORD*)addr_value;
		Sleep(10);
	}

	// find the vtable
	pVtable = GetVtableAddress((void*)result);

	// APPLY THE HOOK, FINALLY!!!
	HookFunction(pVtable, (void*)&hook_EndScene, (void*)&orig_EndScene, 35);
//	HookFunction(pVtable, (void*)&hook_DrawIndexedPrimitive, (void*)&orig_DrawIndexedPrimitive, 71);
//	HookFunction(pVtable, (void*)&hook_Present, (void*)&orig_Present, 15);
//	HookFunction(pVtable, (void*)&hook_SetStreamSource, (void*)&orig_SetStreamSource, 83);


	return 0;
}

DWORD* GetVtableAddress(void* pObject)
{
	// The first 4 bytes of the object is a pointer to the vtable:
	return (DWORD*)*((DWORD*)pObject);
}

void HookFunction(DWORD* pVtable, void* pHookProc, void* pOldProc, int iIndex)
{
	// Enable writing to the vtable at address we aquired
	DWORD lpflOldProtect;
	VirtualProtect((void*)&pVtable[iIndex], sizeof(DWORD), PAGE_READWRITE, &lpflOldProtect);

	// Store old address
	if (pOldProc) {
		*(DWORD*)pOldProc = pVtable[iIndex];
	}

	// Overwrite original address
	pVtable[iIndex] = (DWORD)pHookProc;

	// Restore protection
	VirtualProtect(pVtable, sizeof(DWORD), lpflOldProtect, &lpflOldProtect);
}


//Endscene
HRESULT APIENTRY hook_EndScene(IDirect3DDevice8* pInterface)
{
 	__asm pushad

	DrawText((LPDIRECT3DDEVICE8)pInterface, TEXT("by£º ÀÇÑÌ¶Ï  2012-03-09"), 20);

 	__asm popad

	return orig_EndScene(pInterface);
}

BOOL _stdcall DrawText(LPDIRECT3DDEVICE8 pDxdevice,TCHAR* strText ,int nbuf)
{
	if (pDxdevice) {
		RECT myrect;
		myrect.top		= 10;
		myrect.left		= 50;
		myrect.right		= 1000 + 50;
 		myrect.bottom 	= 100 + 10;

		HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		LOGFONT lf = {0};
		GetObject(hFont, sizeof(lf), &lf);
		DeleteObject(hFont);
		hFont = NULL;

		ID3DXFont* g_font = NULL;
		if(D3D_OK != D3DXCreateFontIndirect(pDxdevice, &lf, &g_font))
			return FALSE;

		g_font->DrawTextW(
			strText,
			nbuf, 
			&myrect, 
			DT_TOP | DT_LEFT,
			0xFFFFFF00); 

		g_font->Release();
	}
	return true;
}

