#ifndef _UNITMANAGER_H_
#define _UNITMANAGER_H_

#include <vector>

class CUnitManager
{
public:
    CUnitManager(DWORD dwGameAddr = 0x6F000000);
    ~CUnitManager();

    // game on
    BOOL IsInGame();

    // tracerse the unit table
    void TraversalUnitId();

    // refresh all
    void RefreshUnitId(FactionTypeId factionType, BOOL bHero, BOOL bWard, BOOL bBomb);

    // refresh memory address
    void RefreshHerosId(FactionTypeId factionType);
    void RefreshWardsId(FactionTypeId factionType);
    void RefreshBombsId(FactionTypeId factionType);

    // get Location
    void GetHerosLocation();
    void GetWardsLocation();
    void GetBombsLocation();

    // rune type
    void GetRuneType();

	void SetScreenPixel(float fw, float fh)
	{
		m_fWidth  = fw;
		m_fHeight = fh;
	}

private:
    DWORD GetLocalPlayer();

    inline void AddToHerosUnit(DWORD dwCurrStruct, DWORD dwCurrIndex);
    inline void AddToSentryWardsUnit(DWORD dwCurrStruct, DWORD dwCurrIndex);
    inline void AddToObserverWardsUnit(DWORD dwCurrStruct, DWORD dwCurrIndex);
    inline void AddToBombsUnit(DWORD dwCurrStruct, DWORD dwCurrIndex);

public:
    UINT m_nHerosCount;
    UINT m_nSentryWardsCount;
    UINT m_nObserverWardsCount;
    UINT m_nBombsCount;

    RUNE_ENUM m_runeType;

    MINI_MAP_POINT m_gHerosPoint[HERO_COUNT_MAX];
    MINI_MAP_POINT m_gSentryWardsPoint[WARD_COUNT_MAX];
    MINI_MAP_POINT m_gObserverWardsPoint[WARD_COUNT_MAX];
    MINI_MAP_POINT m_gBombsPoint[BOMB_COUNT_MAX];

private:
	float m_fWidth;
	float m_fHeight;

    DWORD m_dwGameAddr;

    WC3_UNIT m_gHerosUnit[HERO_COUNT_MAX];
    WC3_UNIT m_gSentryWardsUnit[WARD_COUNT_MAX];
    WC3_UNIT m_gObserverWardsUnit[WARD_COUNT_MAX];
    WC3_UNIT m_gBombsUnit[BOMB_COUNT_MAX];

    std::vector<DWORD> m_idAddrList;
};

#endif

