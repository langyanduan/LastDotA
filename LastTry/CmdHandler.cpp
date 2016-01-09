#include "Common.h"
#include <tchar.h>

#include <Windows.h>
#include <stdio.h>
int (WINAPIV * __snprintf)(char *, size_t, const char*...) = _snprintf;
int (WINAPIV * _sprintf)(char *, const char*...) = sprintf;
int (WINAPIV * _sscanf)(const char *, const char*...) = sscanf;

CCmdHandler::CCmdHandler(CUnitManager * pUnitManager, TCHAR * mailslotName)
    : m_pUnitManager(pUnitManager),
      m_bExitThread(FALSE),
      m_bRunService(FALSE),
      m_bHeroDetect(FALSE),
      m_bBombDetect(FALSE),
      m_bWardDetect(FALSE),
      m_bRuneDetect(FALSE),
      m_factionType(FactionNone),
      m_bConnected(FALSE),
      m_hListenThread(NULL)
{
	ZeroMemory(m_wcsSrvDesc, SRV_DESC_LENGTH);
    ZeroMemory(&m_localGameOpt, sizeof(GAME_OPTION));

    m_hMailslot = CreateMailslot(mailslotName, 0, 300, NULL);

	GAME_OPTION gameOpt;

	gameOpt.on = TRUE;
	gameOpt.factionType = FactionAutoMatch;
	gameOpt.hero = TRUE;
	gameOpt.ward = TRUE;
	gameOpt.bomb = FALSE;
	gameOpt.rune = TRUE;
	gameOpt.fx   = 1366 / 1366;
	gameOpt.fy   = 768 / 768;

	UnitConfig(&gameOpt);
}

CCmdHandler::~CCmdHandler()
{
    m_bExitThread = TRUE;

    DWORD dwRetn = WaitForSingleObject(m_hListenThread, 1000);

    if (WAIT_TIMEOUT == dwRetn)
    {
		FkDbgPrint((TEXT("Terminate Thread")));
        TerminateThread(m_hListenThread, 1);
    }

	FkDbgPrint((TEXT("Thread is exit")));

    CloseHandle(m_hMailslot);
}

BOOL CCmdHandler::Listen()
{
    if (NULL == m_hMailslot)
	{
		FkDbgPrint((TEXT("油槽未创建成功")));
        return FALSE;
    }

    m_hListenThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ListenThread, this, 0, NULL);

    if (NULL == m_hListenThread)
	{
		FkDbgPrint((TEXT("启动监听线程失败")));
        return FALSE;
    }

    CloseHandle(m_hListenThread);

    return TRUE;
}

void CCmdHandler::OnConnect(UCHAR * buffer, DWORD dwLength)
{
    if (dwLength == sizeof(GAME_OPTION))
    {
        UnitConfig((PGAME_OPTION)buffer);
		m_bConnected = TRUE;

		FkDbgPrint((TEXT("有客户端连入")));
    }
}

void CCmdHandler::OnReceive(UCHAR * buffer, DWORD dwLength)
{
    if (dwLength == sizeof(GAME_OPTION))
    {
        UnitConfig((PGAME_OPTION)buffer);
    }
}

void CCmdHandler::OnClose()
{
    // UnitCleanup();
	m_bConnected = FALSE;

	FkDbgPrint((TEXT("连接关闭")));
}

DWORD CCmdHandler::ListenThread(LPVOID lpThreadParameter)
{
    static DWORD nTimeOutCount = 0; // times of timeout

    CCmdHandler * pCmdHandler = (CCmdHandler *)lpThreadParameter;

    DWORD  dwLastError;
    BOOL   bReadFile;
    DWORD  dwReadByte;
    GAME_OPTION gameOpt;

    while (TRUE)    // read command length
    {
        bReadFile = ReadFile(pCmdHandler->m_hMailslot, &gameOpt, sizeof(gameOpt), &dwReadByte, NULL);

        if (pCmdHandler->m_bExitThread)
		{
			FkDbgPrint((TEXT("退出监听循环")));
            break;
        }
        
        if (!bReadFile)
        {
            dwLastError = GetLastError();

            // error
            if (ERROR_SEM_TIMEOUT != dwLastError)
			{
				FkDbgPrint((TEXT("未知异常 %d"), dwLastError));
                break;
            }

            // time out 
            if (nTimeOutCount != 6)
            {
                nTimeOutCount++;
            }
        }
        else
		{
			if (nTimeOutCount == 5 || nTimeOutCount == 6)
			{
				pCmdHandler->OnConnect((PUCHAR)&gameOpt, dwReadByte);
			}
			else
			{
				pCmdHandler->OnReceive((PUCHAR)&gameOpt, dwReadByte);
			}

			nTimeOutCount = 0;
        }

        // when count is 5, client is off
        // when count is 6, no client connect
        if (nTimeOutCount == 5)
        {
            // Close 
            pCmdHandler->OnClose();
        }
    }

    return 0;
}

void CCmdHandler::UnitConfig(PGAME_OPTION pGameOpt)
{
    if (memcmp(&m_localGameOpt, pGameOpt, sizeof(GAME_OPTION)) != 0)
    {
		FkDbgPrint((TEXT("用户配置发生改变")));
		
        CopyMemory(&m_localGameOpt, pGameOpt, sizeof(GAME_OPTION));

        m_bRunService = m_localGameOpt.on;
        m_bHeroDetect = m_localGameOpt.hero;
        m_bBombDetect = m_localGameOpt.bomb;
        m_bWardDetect = m_localGameOpt.ward;
        m_bRuneDetect = m_localGameOpt.rune;
        m_factionType = m_localGameOpt.factionType;

		m_pUnitManager->SetScreenPixel(m_localGameOpt.fx, m_localGameOpt.fy);

		// 设置描述
		swprintf_s(
			m_wcsSrvDesc, 
			SRV_DESC_LENGTH,
			TEXT("英雄(%s) 守卫(%s) 炸弹(%s) 神符(%s) 敌对("), 
			m_bHeroDetect ? TEXT("开启") : TEXT("关闭"),
			m_bWardDetect ? TEXT("开启") : TEXT("关闭"),
			m_bBombDetect ? TEXT("开启") : TEXT("关闭"),
			m_bRuneDetect ? TEXT("开启") : TEXT("关闭"));

		switch (m_factionType)
		{
		case FactionNone:
			wcscat_s(m_wcsSrvDesc, SRV_DESC_LENGTH, TEXT("无)"));
			break;
		case FactionBoth:
			wcscat_s(m_wcsSrvDesc, SRV_DESC_LENGTH, TEXT("双方)"));
			break;
		case FactionAutoMatch:
			wcscat_s(m_wcsSrvDesc, SRV_DESC_LENGTH, TEXT("自动)"));
			break;
		case FactionSentinel:
			wcscat_s(m_wcsSrvDesc, SRV_DESC_LENGTH, TEXT("近卫)"));
			break;
		case FactionScourge:
			wcscat_s(m_wcsSrvDesc, SRV_DESC_LENGTH, TEXT("天灾)"));
			break;
		default:
			break;
		}
    }
}

void CCmdHandler::UnitCleanup()
{
    m_bConnected  = FALSE;

    m_bRunService = FALSE;
    m_bHeroDetect = FALSE;
    m_bBombDetect = FALSE;
    m_bWardDetect = FALSE;
    m_bRuneDetect = FALSE;
    m_factionType = FactionNone;

    ZeroMemory(&m_localGameOpt, sizeof(GAME_OPTION));
}


//=================================================================
//  功  能: 描述开启的功能
//  备  注: 在游戏中显示
//  作  者: Flicker317
//  更  新: 1.0 2012/11/21
//=================================================================
void CCmdHandler::ServiceDesc(OUT wchar_t wcsSrvDesc[40])
{
	wcscpy_s(wcsSrvDesc, 40, m_wcsSrvDesc);
}