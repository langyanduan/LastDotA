#ifndef _CMDHANDLE_H_
#define _CMDHANDLE_H_

#define SRV_DESC_LENGTH	40

class CCmdHandler
{
public:
    CCmdHandler(CUnitManager * pUnitManager, TCHAR * mailslotName);
    ~CCmdHandler();

    BOOL Listen();

    // event
    void OnConnect(UCHAR * buffer, DWORD dwLength);
    void OnReceive(UCHAR * buffer, DWORD dwLength);
    void OnClose();

	void CCmdHandler::ServiceDesc(OUT wchar_t wcsSrvDesc[40]);

    static DWORD WINAPI ListenThread(LPVOID lpThreadParameter);

private:
    void UnitConfig(PGAME_OPTION pGameOpt);
    void UnitCleanup();

public:
    CUnitManager *  m_pUnitManager;
    HANDLE          m_hMailslot;
    BOOL            m_bExitThread;

    BOOL            m_bRunService;

    BOOL            m_bHeroDetect;
    BOOL            m_bBombDetect;
    BOOL            m_bWardDetect;
    BOOL            m_bRuneDetect;
    FactionTypeId   m_factionType;

    /* 
        this value will set to unit manager
    
        float           m_fWidth;    
        float           m_fHeight;
    */
private:
    HANDLE          m_hListenThread;
    GAME_OPTION     m_localGameOpt;
    BOOL            m_bConnected;
	WCHAR			m_wcsSrvDesc[SRV_DESC_LENGTH];
};

#define COMMAND_BUFFER_LENGTH   256


#endif