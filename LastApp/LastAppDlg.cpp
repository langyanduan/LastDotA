
// LastAppDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "LastApp.h"
#include "LastAppDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CLastAppDlg 对话框




CLastAppDlg::CLastAppDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLastAppDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_MAIN);
}

void CLastAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLastAppDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    /*
	ON_BN_CLICKED(IDC_RADIO_AUTO, &CLastAppDlg::OnBnClickedNotifyChange)
	ON_BN_CLICKED(IDC_RADIO_SENTINEL, &CLastAppDlg::OnBnClickedNotifyChange)
	ON_BN_CLICKED(IDC_RADIO_SCOURGE, &CLastAppDlg::OnBnClickedNotifyChange)
	ON_BN_CLICKED(IDC_RADIO_BOTH, &CLastAppDlg::OnBnClickedNotifyChange)
	ON_BN_CLICKED(IDC_CHECK_HERO, &CLastAppDlg::OnBnClickedNotifyChange)
	ON_BN_CLICKED(IDC_CHECK_WARD, &CLastAppDlg::OnBnClickedNotifyChange)
	ON_BN_CLICKED(IDC_CHECK_BOMB, &CLastAppDlg::OnBnClickedNotifyChange)
	ON_BN_CLICKED(IDC_CHECK_RUNE, &CLastAppDlg::OnBnClickedNotifyChange)
    */
	ON_BN_CLICKED(IDC_BTN_WINMAX, &CLastAppDlg::OnBnClickedBtnWinmax)
	ON_BN_CLICKED(IDC_BTN_WINNORMAL, &CLastAppDlg::OnBnClickedBtnWinnormal)
	ON_BN_CLICKED(IDC_BTN_INJECT, &CLastAppDlg::OnBnClickedBtnInject)
END_MESSAGE_MAP()


// CLastAppDlg 消息处理程序

BOOL CLastAppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	((CButton *)GetDlgItem(IDC_RADIO_AUTO))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_CHECK_HERO))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_CHECK_WARD))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_CHECK_RUNE))->SetCheck(TRUE);

    HANDLE hConnectThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CLastAppDlg::ConnectThread, this, 0, NULL);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CLastAppDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CLastAppDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CLastAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/*
void CLastAppDlg::OnBnClickedNotifyChange()
{
	GAME_OPTION gameOpt;
	DWORD bytesWritten;
	HANDLE hMailslot;

	hMailslot = CreateFile(
		TEXT("\\\\.\\Mailslot\\FF68535B-4AF0-72F6-0F5B-6A40141468E3"),
		GENERIC_WRITE,
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (INVALID_HANDLE_VALUE == hMailslot)
	{
		MessageBox(TEXT("未检测到辅助模块！"), TEXT("提示"), MB_OK);
		return;
	}

	ConfigGameOpt(gameOpt);

	if(0 == WriteFile(hMailslot, &gameOpt, sizeof(gameOpt), &bytesWritten, NULL))
	{
		MessageBox(TEXT("设置出错！"), TEXT("提示"), MB_OK);
	}

	CloseHandle(hMailslot);
}
*/

void CLastAppDlg::ConfigGameOpt(GAME_OPTION & gameOpt)
{
    int cx, cy;

	ZeroMemory(&gameOpt, sizeof(gameOpt));

	gameOpt.on = TRUE;
	gameOpt.hero = ((CButton *)GetDlgItem(IDC_CHECK_HERO))->GetCheck() ? TRUE : FALSE;
	gameOpt.ward = ((CButton *)GetDlgItem(IDC_CHECK_WARD))->GetCheck() ? TRUE : FALSE;
	gameOpt.bomb = ((CButton *)GetDlgItem(IDC_CHECK_BOMB))->GetCheck() ? TRUE : FALSE;
	gameOpt.rune = ((CButton *)GetDlgItem(IDC_CHECK_RUNE))->GetCheck() ? TRUE : FALSE;

    cx = GetSystemMetrics(SM_CXSCREEN);
    cy = GetSystemMetrics(SM_CYSCREEN);

	gameOpt.fx = 1.0f * cx / 1366;
	gameOpt.fy = 1.0f * cy / 768;

	if (((CButton *)GetDlgItem(IDC_RADIO_AUTO))->GetCheck())
	{
		gameOpt.factionType = FactionAutoMatch;
	}
	else
	if (((CButton *)GetDlgItem(IDC_RADIO_SENTINEL))->GetCheck())
	{
		gameOpt.factionType = FactionSentinel;
	}
	else
	if (((CButton *)GetDlgItem(IDC_RADIO_SCOURGE))->GetCheck())
	{
		gameOpt.factionType = FactionScourge;
	}
	else
	if (((CButton *)GetDlgItem(IDC_RADIO_BOTH))->GetCheck())
	{
		gameOpt.factionType = FactionBoth;
	}
}

void CLastAppDlg::OnBnClickedBtnWinmax()
{
	int cx, cy, nStyle;
	HWND hGame;
	
	hGame = ::FindWindow(NULL, TEXT("Warcraft III"));
	if (NULL == hGame)
	{
		MessageBox(TEXT("未检测到魔兽窗口！"), TEXT("提示"), MB_OK);
		return;
	}

	cx = GetSystemMetrics(SM_CXSCREEN);
	cy = GetSystemMetrics(SM_CYSCREEN);

	::ShowWindow(hGame, TRUE);
	::SetForegroundWindow(hGame);

	nStyle  = ::GetWindowLong(hGame, GWL_STYLE);
	nStyle &= ~WS_CAPTION;
	nStyle &= ~WS_THICKFRAME;
	::SetWindowLong(hGame, GWL_STYLE, nStyle);
	::SetWindowPos(hGame, CWnd::wndNoTopMost, 0, 0, cx, cy,0);
}

void CLastAppDlg::OnBnClickedBtnWinnormal()
{
	int cx, cy, nStyle;
	HWND hGame;

	hGame = ::FindWindow(NULL, TEXT("Warcraft III"));
	if (NULL == hGame)
	{
		MessageBox(TEXT("未检测到魔兽窗口！"), TEXT("提示"), MB_OK);
		return;
	}

	cx = GetSystemMetrics(SM_CXSCREEN);
	cy = GetSystemMetrics(SM_CYSCREEN);

	::ShowWindow(hGame, TRUE);
	::SetForegroundWindow(hGame);

	nStyle  = ::GetWindowLong(hGame, GWL_STYLE);
	nStyle |= WS_CAPTION;
	nStyle |= WS_THICKFRAME;
	::SetWindowLong(hGame, GWL_STYLE, nStyle);
	::SetWindowPos(hGame, CWnd::wndNoTopMost, cx / 2 - 400, cy / 2 - 300, 800, 600, 0);
}

void CLastAppDlg::OnBnClickedBtnInject()
{
	HWND	hGameWnd;
	HANDLE	hGameHandle;
	DWORD	dwGamePid;
	WCHAR	wcsLibName[256];
	WCHAR * pLibPath;

	ImprovePrivilege();

	hGameWnd = ::FindWindow(NULL, TEXT("Warcraft III"));
	if (NULL == hGameWnd)
	{
		MessageBox(TEXT("未检测到魔兽窗口！"), TEXT("提示"), MB_OK);
		return;
	}

	GetWindowThreadProcessId(hGameWnd, &dwGamePid);

	hGameHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwGamePid);
	if (NULL == hGameHandle)
	{
		MessageBox(TEXT("打开进程出错！"), TEXT("提示"), MB_OK);
		return;
	}

	GetModuleFileName(NULL, wcsLibName, 256);
	pLibPath = wcsrchr(wcsLibName, '\\');
	pLibPath[0] = '\0';	// *pLibPath = '\0';

	wsprintf(wcsLibName, TEXT("%s\\%s"), wcsLibName, TEXT("LastTry.dll"));

	if (!InjectLibrary(hGameHandle, wcsLibName))
	{
		MessageBox(TEXT("注入模块失败！"), TEXT("提示"), MB_OK);
	}

	CloseHandle(hGameHandle);
}

DWORD CLastAppDlg::ConnectThread(LPARAM lParam)
{
    CLastAppDlg *pLastAppDlg = (CLastAppDlg *)lParam;

    GAME_OPTION gameOpt;
    DWORD bytesWritten;
    HANDLE hMailslot;

    CString strLastError = L"";
    while (TRUE)
    {
        Sleep(500);

        hMailslot = CreateFile(
            TEXT("\\\\.\\Mailslot\\FF68535B-4AF0-72F6-0F5B-6A40141468E3"),
            GENERIC_WRITE,
            FILE_SHARE_READ, 
            NULL, 
            OPEN_EXISTING, 
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        if (INVALID_HANDLE_VALUE == hMailslot)
        {
            continue;
        }

        while (TRUE)
        {
            pLastAppDlg->ConfigGameOpt(gameOpt);

            if (0 == WriteFile(hMailslot, &gameOpt, sizeof(gameOpt), &bytesWritten, NULL))
            {
                if (ERROR_HANDLE_EOF != GetLastError())
                {
                    OutputDebugString(L"unknown error");
                }
                
                CloseHandle(hMailslot);
                break;
            }
            Sleep(500);
        }
    }
}


// 进程提权
static BOOL ImprovePrivilege()
{
	HANDLE	hToken	= NULL;										//令牌句柄
	BOOL	bRet	= FALSE;									//返回执行结果
	TOKEN_PRIVILEGES tp = {1, {0, 0, SE_PRIVILEGE_ENABLED}};	//填充权限令牌结构

	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);			//查询是否具有调试权限
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);	//打开进程权限令牌
	AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof tp, 0, 0);					//为进程申请 DEBUG 权限
	bRet = (GetLastError() == ERROR_SUCCESS);									//检测是否执行成功

	return bRet;
}

// 注入dll
static BOOL InjectLibrary(HANDLE hRemoteProcess, TCHAR * szLibPath)
{
	HANDLE hRemoteThread     = NULL;
	HANDLE hRemoteFunc       = NULL;
	PVOID  pRemoteParam      = NULL;
	DWORD  dwWriten          = 0;
	BOOL   bRet              = FALSE;

	TCHAR   szLibPathCopy[256] = {0};
	lstrcpyW(szLibPathCopy, szLibPath);

	if (hRemoteProcess == NULL)
	{
		return FALSE;
	}

	int iSize = (_tcslen(szLibPath) + 1) * sizeof(TCHAR);

	pRemoteParam = VirtualAllocEx(hRemoteProcess, NULL, iSize, MEM_COMMIT, PAGE_READWRITE);
	if (pRemoteParam == NULL)
	{
		return FALSE;
	}

	bRet = WriteProcessMemory(hRemoteProcess, pRemoteParam, (LPVOID)szLibPathCopy, iSize, &dwWriten);
	if (!bRet)
	{
		if (pRemoteParam)
		{
			VirtualFreeEx(hRemoteProcess, pRemoteParam, 0, MEM_RELEASE);
		}

		return FALSE; 
	}
#ifdef UNICODE
	hRemoteFunc = GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
#else
	hRemoteFunc = GetProcAddress(GetModuleHandle("Kernel32"), "LoadLibraryA");
#endif
	hRemoteThread = CreateRemoteThread(hRemoteProcess, 0, 0, (LPTHREAD_START_ROUTINE)hRemoteFunc, pRemoteParam, 0, &dwWriten);

	// 等待线程结束
	if (hRemoteThread)
	{
		WaitForSingleObject(hRemoteThread, INFINITE);
		HMODULE g_hRemoteHandle;
		GetExitCodeThread(hRemoteThread, (DWORD*)&g_hRemoteHandle);
	}

	// 清理工作
	if (pRemoteParam)
	{
		VirtualFreeEx(hRemoteProcess, pRemoteParam, 0, MEM_RELEASE);
	}

	CloseHandle(hRemoteThread);

	return TRUE;
}
