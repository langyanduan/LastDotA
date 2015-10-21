#ifndef _D3DHOOK_H_
#define _D3DHOOK_H_

#include <d3d8.h>
#include <d3dx8.h>

#define GetVtableAddress(pObject) ((DWORD *)(*((DWORD *)(pObject))))

// 定义 EndScene 原型
typedef 
HRESULT 
(APIENTRY *EndScene_t)(
	IDirect3DDevice8 * pInterface
	);

// 我们的函数 EndScene
HRESULT	
APIENTRY hook_EndScene(
	IDirect3DDevice8 * pInterface
	);

inline void DrawMiniMap();
inline void DrawInfo(ID3DXFont * font, DWORD color);
inline void DrawRune(ID3DXFont * font);
inline void DrawHero(ID3DXFont * font);
inline void DrawWard(ID3DXFont * font);
inline void DrawBomb(ID3DXFont * font);

class CD3DHook
{
public:
	CD3DHook();
	~CD3DHook();

	void StartPatch();

	// do patch in this thread
	static DWORD WINAPI PatchD3DThreadProc(IN LPVOID lpParameter);


	static void PatchHeader(DWORD hookAddr, DWORD funcAddr);
	static void RestoreHeader(DWORD hookAddr);
	static void ReplaceVtable(DWORD* pVtable, void* pHookProc, void* pOldProc, int nIndex);

private:
	void InstallHook();
	void UninstallHook();

public:
	LPDIRECT3DDEVICE8	m_lpDirectDevice;
	LOGFONT				m_logFont;
};

#endif
