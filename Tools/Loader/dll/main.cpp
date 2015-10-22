#include <windows.h>
#include <d3dx8.h>

#include "MinHook.h"
#include <wchar.h>
#include <stdio.h>

#if defined _M_X64
#pragma comment(lib, "MinHook.x64.lib")
#elif defined _M_IX86
#pragma comment(lib, "MinHook.x86.lib")
#endif

// globals
// Our hook function
IDirect3D8* __stdcall hook_Direct3DCreate8(UINT sdkVers);
HMODULE WINAPI LoadLibrary_Hook ( LPCSTR lpFileName );
void HookAPI();

// CreateDevice
typedef HRESULT (APIENTRY *CreateDevice_t)(
	IDirect3D8*,
	UINT,D3DDEVTYPE,
	HWND,DWORD,
	D3DPRESENT_PARAMETERS*,
	IDirect3DDevice8**);

CreateDevice_t orig_CreateDevice;

HRESULT APIENTRY hook_CreateDevice(
	IDirect3D8* pInterface,
	UINT Adapter,
	D3DDEVTYPE DeviceType,
	HWND hFocusWindow,
	DWORD BehaviorFlags,
	D3DPRESENT_PARAMETERS* pPresentationParameters,
	IDirect3DDevice8** ppReturnedDeviceInterface);

// The original to call
typedef IDirect3D8* (__stdcall *Direct3DCreate8_t)(UINT SDKVersion);
Direct3DCreate8_t orig_Direct3DCreate8;

// Holds address that we get in our LoadLibrary hook (used for detour)
PBYTE pDirect3DCreate8;

typedef HMODULE (WINAPI *LoadLibrary_t)(LPCSTR);
LoadLibrary_t orig_LoadLibrary = LoadLibraryA; // holds address of original non-detoured function

LoadLibrary_t orig_LoadLibrary1 = NULL;

BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls(hModule);

			if (MH_OK != MH_Initialize())
			{
				OutputDebugStringA("Initialize fail");
			}
			else
			{
				OutputDebugStringA("Initialize success");
			}

			CHAR outString[80];

			sprintf(outString, "%08X   %08X   %08X   %08X", LoadLibraryA, &LoadLibraryA, orig_LoadLibrary, &orig_LoadLibrary);
			OutputDebugStringA(outString);

			if (MH_OK != MH_CreateHook(&LoadLibraryA, &LoadLibrary_Hook, reinterpret_cast<void**>(&orig_LoadLibrary)))
			{
				OutputDebugStringA("CreateHook fail");
			}
			else
			{
				OutputDebugStringA("CreateHook success");
			}

			if (MH_EnableHook(&LoadLibraryA) != MH_OK)
			{
				OutputDebugStringA("EnableHook fail");
			}
			else
			{
				OutputDebugStringA("EnableHook success");
			}
			//DetourRestoreAfterWith();
			//DetourTransactionBegin();
			//DetourUpdateThread(GetCurrentThread());

			//DetourAttach(&(PVOID&)orig_LoadLibrary, LoadLibrary_Hook);

			//DetourTransactionCommit();
		}
	}
	return TRUE;
}

// Our hooked LoadLibrary
HMODULE WINAPI LoadLibrary_Hook ( LPCSTR lpFileName ) 
{
	static int hooked = 0;


	HMODULE hM = orig_LoadLibrary( lpFileName );
	if ( strcmp( lpFileName, "d3d8.dll" ) == 0) 
	{
		hooked++;

		OutputDebugStringA("test");
		if (hooked == 3) {
			// get address of function to hook
			pDirect3DCreate8 = (PBYTE)GetProcAddress(hM, "Direct3DCreate8");
			HookAPI();
		}
	}

	return hM;
}

void HookAPI()
{
	if (MH_OK != MH_CreateHook(pDirect3DCreate8, &hook_Direct3DCreate8, reinterpret_cast<void**>(&orig_Direct3DCreate8)))
	{
		OutputDebugStringA("CreateHook fail");
	}
	else
	{
		OutputDebugStringA("CreateHook success");
	}

	if (MH_OK != MH_EnableHook(pDirect3DCreate8))
	{
		OutputDebugStringA("EnableHook fail");
	}
	else
	{
		OutputDebugStringA("EnableHook success");
	}

}

//void HookAPI()
//{
//	// simple detour
//	orig_Direct3DCreate8 = (Direct3DCreate8_t)pDirect3DCreate8;
//
//	DetourRestoreAfterWith();
//	DetourTransactionBegin();
//	DetourUpdateThread(GetCurrentThread());
//
//	DetourAttach(&(PVOID&)orig_Direct3DCreate8, hook_Direct3DCreate8);
//
//	DetourTransactionCommit();
//
//}

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

IDirect3D8* __stdcall hook_Direct3DCreate8(UINT sdkVers)
{
	IDirect3D8* pD3d8 = orig_Direct3DCreate8(sdkVers); // real one

	// Use a vtable hook on CreateDevice to get the device pointer later
	DWORD* pVtable = GetVtableAddress(pD3d8);
	HookFunction(pVtable, (void*)&hook_CreateDevice, (void*)&orig_CreateDevice, 15);

	return pD3d8;
}

HRESULT APIENTRY hook_CreateDevice(
	IDirect3D8* pInterface,
	UINT Adapter,
	D3DDEVTYPE DeviceType,
	HWND hFocusWindow,
	DWORD BehaviorFlags,
	D3DPRESENT_PARAMETERS* pPresentationParameters,
	IDirect3DDevice8** ppReturnedDeviceInterface)
{
	HRESULT ret = orig_CreateDevice(pInterface, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

	
	// Registers MUST be preserved when doing your own stuff!!
	__asm pushad

	// get a pointer to the created device 
	IDirect3DDevice8* d3ddev = *ppReturnedDeviceInterface;

	// lets log it (format in hex mode to make it easier to work with)
	char buf[50] = {0};

	wsprintfA(buf, "pD3ddev: %X", d3ddev);
	OutputDebugStringA(buf);

	__asm popad

	return ret;
}