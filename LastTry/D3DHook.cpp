#include "Common.h"

#pragma comment(lib, "d3dx8.lib")

#define W1920_H1080		// 1920 * 1080
// #define W1366_H768		// 1366 * 768


EndScene_t  orig_EndScene = NULL;
DWORD		g_d3d8Point   = NULL;

__declspec(naked) void GetValue()
{
	// esp	+ 0x00	  	thisRetnAddr
	//		+ 0x04		lastRetnAddr
	//		+ 0x08		arg1
	//		+ 0x0C		arg2
	__asm
	{
		mov eax, [esp];
		add eax, 2;
		mov [esp], eax;

		mov eax, [esp + 0x8];	// arg1
		mov g_d3d8Point, eax;
		retn;
	}
}

CD3DHook::CD3DHook()
	: m_lpDirectDevice(NULL)
{
	HFONT hFont;

	ZeroMemory(&m_logFont, sizeof(m_logFont));

	hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	GetObject(hFont, sizeof(m_logFont), &m_logFont);
	DeleteObject(hFont);

#ifdef W1920_H1080
	// 1920 * 1080
	m_logFont.lfHeight *= 1.4;
	m_logFont.lfWidth *= 1.4;
	m_logFont.lfWeight = FW_MEDIUM;
#endif
}

CD3DHook::~CD3DHook()
{
	if (NULL != orig_EndScene)
	{
		this->UninstallHook();
	}
}

void CD3DHook::StartPatch()
{
	HANDLE hPatch = CreateThread(
		NULL, 
		0, 
		PatchD3DThreadProc, 
		this, 
		0, 
		NULL);

	CloseHandle(hPatch);
}

DWORD WINAPI CD3DHook::PatchD3DThreadProc(IN LPVOID lpParameter)
{
	FkDbgPrint((TEXT("Patch thread run")));

	((CD3DHook *)lpParameter)->InstallHook();

	return 0;
}

void CD3DHook::InstallHook()
{
	// Win8.1   0x35E50
	// Win7     0x44A50
	// Win8.1   0x39590
	// Win10    0x3A990
	DWORD d3d8Module = (DWORD)GetModuleHandle(TEXT("d3d8.dll"));
	DWORD dwHookAddr = d3d8Module + 0x3A990;

	CD3DHook::PatchHeader(dwHookAddr, (DWORD)GetValue);

	FkDbgPrint((TEXT("wait for write")));

	Sleep(100);	// wait for address to get written

	while (NULL == g_d3d8Point)
	{
		Sleep(10);
	}

	m_lpDirectDevice = (LPDIRECT3DDEVICE8)(/**(DWORD *)*/g_d3d8Point);

	FkDbgPrint((TEXT("get value: %08X"), m_lpDirectDevice));

	CD3DHook::RestoreHeader(dwHookAddr);

	// find the vtable
	DWORD * pVtable = GetVtableAddress(m_lpDirectDevice);

	// APPLY THE HOOK, FINALLY!!!
	CD3DHook::ReplaceVtable(pVtable, (void *)&hook_EndScene, (void *)&orig_EndScene, 35);
}

void CD3DHook::UninstallHook()
{
	DWORD * pVtable = GetVtableAddress(m_lpDirectDevice);
	CD3DHook::ReplaceVtable(pVtable, orig_EndScene, NULL, 35);
}

void CD3DHook::ReplaceVtable(DWORD* pVtable, void * pHookProc, void * pOldProc, int nIndex)
{
	// Enable writing to the vtable at address we aquired
	DWORD lpflOldProtect;
	VirtualProtect((void *)&pVtable[nIndex], sizeof(DWORD), PAGE_READWRITE, &lpflOldProtect);

	// Store old address
	if (pOldProc)
	{
		*(DWORD *)pOldProc = pVtable[nIndex];
	}

	// Overwrite original address
	pVtable[nIndex] = (DWORD)pHookProc;

	// Restore protection
	VirtualProtect(pVtable, sizeof(DWORD), lpflOldProtect, &lpflOldProtect);
}

void CD3DHook::PatchHeader(DWORD hookAddr, DWORD funcAddr)
{
	const BYTE CodeJmpUpper[2] = { 0xEB, 0xF9 };	// eip - 5
	BYTE CodeJmpShadow[5]	   = { 0xE8, 0, 0, 0, 0,};	// 0xE8: call,  0xE9: jmp

	DWORD callAddr = hookAddr - 5;
	DWORD dwOldProtect;
	DWORD dwRelocation = funcAddr - hookAddr; // funcAddr - callAddr + 5;

	memcpy(CodeJmpShadow + 1, &dwRelocation, sizeof(DWORD));

	VirtualProtect((void *)callAddr, sizeof(CodeJmpShadow), PAGE_EXECUTE_READWRITE, &dwOldProtect);
	memcpy((void *)callAddr, (void *)CodeJmpShadow, sizeof(CodeJmpShadow));
	VirtualProtect((void *)callAddr, sizeof(CodeJmpShadow), dwOldProtect, &dwOldProtect);

	VirtualProtect((void *)hookAddr, sizeof(CodeJmpUpper), PAGE_EXECUTE_READWRITE, &dwOldProtect);
	memcpy((void *)hookAddr, (void *)CodeJmpUpper, sizeof(CodeJmpUpper));
	VirtualProtect((void *)hookAddr, sizeof(CodeJmpUpper), dwOldProtect, &dwOldProtect);
}

void CD3DHook::RestoreHeader(DWORD hookAddr)
{
	const BYTE CodeRestore[2]  = { 0x8B, 0xFF };	// mov edi, edi;
	const BYTE CodeDebug[5]    = { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC };

	DWORD callAddr = hookAddr - 5;
	DWORD dwOldProtect;

	VirtualProtect((void *)hookAddr, sizeof(CodeRestore), PAGE_EXECUTE_READWRITE, &dwOldProtect);
	memcpy((void *)hookAddr, (void *)CodeRestore, sizeof(CodeRestore));
	VirtualProtect((void *)hookAddr, sizeof(CodeRestore), dwOldProtect, &dwOldProtect);

	VirtualProtect((void *)callAddr, sizeof(CodeDebug), PAGE_EXECUTE_READWRITE, &dwOldProtect);
	memcpy((void *)callAddr, (void *)CodeDebug, sizeof(CodeDebug));
	VirtualProtect((void *)callAddr, sizeof(CodeDebug), dwOldProtect, &dwOldProtect);
}

// 绘制辅助功能

HRESULT APIENTRY hook_EndScene(IDirect3DDevice8 * pInterface)
{
	DrawMiniMap();

	return orig_EndScene(pInterface);
}

inline void DrawMiniMap()
{
	if (g_pCmdHandler->m_bRunService)
	{
		ID3DXFont * font = NULL;
		HRESULT hResult = D3DXCreateFontIndirect((LPDIRECT3DDEVICE8)g_pD3DHook->m_lpDirectDevice, &g_pD3DHook->m_logFont, &font);

		if(D3D_OK != hResult) //创建字体对象
		{
			return;
		}

		if (g_pUnitManager->IsInGame())
		{
			DrawInfo(font, COLOR_TEXT);

			static BYTE refreshCount = 0;

			// 16 FPS 获取一次Heros、Wards、Bombs、Rune信息（获取新增的或去除消失的）
			if ((refreshCount += 2) == 0)
			{
				// 遍历所有单位
				g_pUnitManager->TraversalUnitId();

				// 从这些单位中筛选并刷新
				g_pUnitManager->RefreshUnitId(
					g_pCmdHandler->m_factionType,
					g_pCmdHandler->m_bHeroDetect,
					g_pCmdHandler->m_bWardDetect,
					g_pCmdHandler->m_bBombDetect);

				// 真、假眼
				if (g_pCmdHandler->m_bWardDetect)
				{
					g_pUnitManager->GetWardsLocation();
				}
				// 炸弹
				if (g_pCmdHandler->m_bBombDetect)
				{
					g_pUnitManager->GetBombsLocation();
				}

				// 遍历 Item，筛选并刷新神符属性
				if (g_pCmdHandler->m_bRuneDetect)
				{
					g_pUnitManager->GetRuneType();
				}
			}

			// 绘制Heros
			if (g_pCmdHandler->m_bHeroDetect)
			{
				g_pUnitManager->GetHerosLocation();

				DrawHero(font);
			}

			// 绘制Wards
			if (g_pCmdHandler->m_bWardDetect)
			{
				DrawWard(font);
			}

			// 绘制Bombs
			if (g_pCmdHandler->m_bBombDetect)
			{
				DrawBomb(font);
			}

			// 神符刷新提示
			if (g_pCmdHandler->m_bRuneDetect)
			{
				DrawRune(font);
			}
		}
		else
		{
			// printf game config 
			DrawInfo(font, COLOR_AREA);
		}
		font->Release();
	}
}

inline void DrawInfo(ID3DXFont * font, DWORD color)
{
	RECT rect;

#ifdef W1920_H1080
	rect.left = 115;
	rect.top = 50;
	rect.right = 605;
	rect.bottom = 90;
#else
	rect.left	= 80;
	rect.top	= 35;
	rect.right	= rect.left + 400;
	rect.bottom	= rect.top + 20;
#endif

	wchar_t wcsSrvDesc[40];
	g_pCmdHandler->ServiceDesc(wcsSrvDesc);

	font->DrawText(wcsSrvDesc, wcslen(wcsSrvDesc), &rect, DT_TOP | DT_LEFT, color);
}

inline void DrawRune(ID3DXFont * font)
{
	RECT rect;

#ifdef W1920_H1080
	rect.left = 115;
	rect.top = 75;
	rect.right = 355;
	rect.bottom = 100;
#else
	rect.left = 80;
	rect.top = 55;
	rect.right = 280;
	rect.bottom	= 80;
#endif

	switch (g_pUnitManager->m_runeType)
	{
	case RUNE_NONE:
		font->DrawText(TEXT("神符尚未刷新！"), 7, &rect, DT_TOP | DT_LEFT, COLOR_TEXT); 
		break;
	case RUNE_UPPER_SPEEDUP:
		font->DrawText(TEXT("上路 极速"), 5, &rect, DT_TOP | DT_LEFT, COLOR_RUNE_TEXT); 
		break;
	case RUNE_UPPER_PHANTOM:
		font->DrawText(TEXT("上路 幻象"), 5, &rect, DT_TOP | DT_LEFT, COLOR_RUNE_TEXT); 
		break;
	case RUNE_UPPER_RECOVER:
		font->DrawText(TEXT("上路 恢复"), 5, &rect, DT_TOP | DT_LEFT, COLOR_RUNE_TEXT); 
		break;
	case RUNE_UPPER_INVISIBLE:
		font->DrawText(TEXT("上路 隐身"), 5, &rect, DT_TOP | DT_LEFT, COLOR_RUNE_TEXT); 
		break;
	case RUNE_UPPER_DOUBLE:
		font->DrawText(TEXT("上路 双倍"), 5, &rect, DT_TOP | DT_LEFT, COLOR_RUNE_TEXT); 
		break;
	case RUNE_LOWER_SPEEDUP:
		font->DrawText(TEXT("下路 极速"), 5, &rect, DT_TOP | DT_LEFT, COLOR_RUNE_TEXT); 
		break;
	case RUNE_LOWER_PHANTOM:
		font->DrawText(TEXT("下路 幻象"), 5, &rect, DT_TOP | DT_LEFT, COLOR_RUNE_TEXT); 
		break;
	case RUNE_LOWER_RECOVER:
		font->DrawText(TEXT("下路 恢复"), 5, &rect, DT_TOP | DT_LEFT, COLOR_RUNE_TEXT); 
		break;
	case RUNE_LOWER_INVISIBLE:
		font->DrawText(TEXT("下路 隐身"), 5, &rect, DT_TOP | DT_LEFT, COLOR_RUNE_TEXT); 
		break;
	case RUNE_LOWER_DOUBLE:
		font->DrawText(TEXT("下路 双倍"), 5, &rect, DT_TOP | DT_LEFT, COLOR_RUNE_TEXT); 
		break;
	default:
		break;
	}
}

inline void DrawHero(ID3DXFont * font)
{
	LPMINI_MAP_POINT points = g_pUnitManager->m_gHerosPoint;

	RECT rect;
	for (unsigned int i = 0; i < g_pUnitManager->m_nHerosCount; i++)
	{
		rect.left	= points[i].x;
		rect.right	= 20 + points[i].x;
		rect.top	= points[i].y;
		rect.bottom	= 20 + points[i].y;

#ifdef W1920_H1080
		rect.left--;
#endif

		// 要绘制的文本
		// 字符居左显示
		font->DrawText(TEXT("⊙"), 1, &rect, DT_TOP | DT_LEFT, COLOR_HERO); 
	}
}

inline void DrawWard(ID3DXFont * font)
{
	LPMINI_MAP_POINT sentryPoints = g_pUnitManager->m_gSentryWardsPoint;
	LPMINI_MAP_POINT observerPoints = g_pUnitManager->m_gObserverWardsPoint;

	RECT rect;
	for (unsigned int i = 0; i < g_pUnitManager->m_nSentryWardsCount; i++)
	{
		rect.left	= sentryPoints[i].x;
		rect.right	= 20 + sentryPoints[i].x;
		rect.top	= sentryPoints[i].y;
		rect.bottom	= 20 + sentryPoints[i].y;

#ifdef W1920_H1080
		rect.top--;
#endif

		font->DrawText(TEXT("。"), 1, &rect, DT_TOP | DT_LEFT, COLOR_SENTRY_WARD); 
	}
	for (unsigned int i = 0; i < g_pUnitManager->m_nObserverWardsCount; i++)
	{
		rect.left	= observerPoints[i].x;
		rect.right	= 20 + observerPoints[i].x;
		rect.top	= observerPoints[i].y;
		rect.bottom	= 20 + observerPoints[i].y;

#ifdef W1920_H1080
		rect.top--;
#endif

		font->DrawText(TEXT("。"), 1, &rect, DT_TOP | DT_LEFT, COLOR_OBSERVER_WARD); 
	}
}

inline void DrawBomb(ID3DXFont * font)
{
	LPMINI_MAP_POINT points = g_pUnitManager->m_gBombsPoint;

	RECT rect;
	for (unsigned int i = 0; i < g_pUnitManager->m_nBombsCount; i++)
	{
		rect.left	= points[i].x;
		rect.right	= 10 + points[i].x;
		rect.top	= points[i].y;
		rect.bottom = 10 + points[i].y;

#ifdef W1920_H1080
		rect.top--;
#endif

		// 要绘制的文本
		// 字符居左显示
		font->DrawText(TEXT("。"), 1, &rect, DT_TOP | DT_LEFT, COLOR_BOMB); 
	}
}