
// LastAppDlg.h : 头文件
//

#pragma once


typedef enum _FactionType
{
	FactionNone = 0,
	FactionBoth,
	FactionAutoMatch,
	FactionSentinel,
	FactionScourge
} FactionTypeId;

typedef struct _GAME_OPTION
{
	BOOL on;

	FactionTypeId factionType;
	BOOL hero;
	BOOL ward;
	BOOL bomb;
	BOOL rune;

	float fx;
	float fy;

} GAME_OPTION, *PGAME_OPTION;

// CLastAppDlg 对话框
class CLastAppDlg : public CDialogEx
{
// 构造
public:
	CLastAppDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_LASTAPP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    
	// afx_msg void OnBnClickedNotifyChange();
    
	afx_msg void OnBnClickedBtnWinmax();
    afx_msg void OnBnClickedBtnWinnormal();
    afx_msg void OnBnClickedBtnInject();

	void ConfigGameOpt(GAME_OPTION &gameOpt);

    static DWORD ConnectThread(LPARAM lParam);
};

static BOOL ImprovePrivilege();
static BOOL InjectLibrary(HANDLE hRemoteProcess, TCHAR * szLibPath);