#include "Common.h"

CUnitManager::CUnitManager(DWORD dwGameAddr) 
	: m_idAddrList(UNIT_TABLE_MAX), 
      m_dwGameAddr(dwGameAddr), 
	  m_nHerosCount(0), 
      m_nSentryWardsCount(0), 
      m_nObserverWardsCount(0), 
      m_nBombsCount(0), 
      m_runeType(RUNE_NONE)
{
}

CUnitManager::~CUnitManager(void)
{
}

//=================================================================
//  功  能: 判断是否在游戏中
//  备  注: 
//  作  者: Flicker317
//  更  新: 1.0 2012/11/21
//=================================================================
BOOL CUnitManager::IsInGame()
{
	return (*(DWORD *)(m_dwGameAddr + 0xACAA38));
}

//=================================================================
//  功  能: 得到本地玩家索引
//  备  注: 返回值大小 0~15
//  作  者: Flicker317
//  更  新: 1.0 2012/11/21
//=================================================================
DWORD CUnitManager::GetLocalPlayer()
{	
    WORD rt;
    DWORD dwGameAddr = m_dwGameAddr;
    __asm
    {
        mov  eax, dwGameAddr;
        mov  eax, dword ptr ds:[eax + 0xACD44C];
        test eax, eax;
        je   err;
        mov  eax, dword ptr ds:[eax + 0x28];
        mov  rt, ax;
    }
    return (DWORD)rt;
err:
    return 0xF;
}

//=================================================================
//  功  能: 遍历以保存所有Unit数组
//  备  注: Game.dll+0xACB0D8  00000310		结构体大小
//			Game.dll+0xACB0DC  00000100		数组大小
//			Game.dll+0xACB0E0  00000???		单位总数量
//			Game.dll+0xACB0E4  ????????		二维动态数组指针
//			Game.dll+0xACB0E8  ????????		被销毁的单位链表
//			结构体头为 Game.dll+0x943A94		表示未被销毁
//  作  者: Flicker317
//  更  新: 1.0 2012/11/19
//=================================================================
void CUnitManager::TraversalUnitId()
{
	DWORD dwArraySize;			// 数组大小
	DWORD dwStructSize;			// 结构体大小
	DWORD dwStructCount;		// 结构体总数
	DWORD dwCurrStoreAddr;		// 当前遍历数组的地址（存入链表时）

	dwStructSize	= GetMemory(m_dwGameAddr + 0xACB0D8);
	dwStructCount	= GetMemory(m_dwGameAddr + 0xACB0E0);
	dwArraySize		= GetMemory(m_dwGameAddr + 0xACB0DC);

	dwCurrStoreAddr = m_dwGameAddr + 0xACB0E4;

	// 清空
	m_idAddrList.clear();

	while ((dwCurrStoreAddr = GetMemory(dwCurrStoreAddr)))
	{
		m_idAddrList.push_back(dwCurrStoreAddr);
	}
}

//=================================================================
//  功  能: 刷新保存单位信息的数组
//  备  注: 在游戏中实时调用（由hook住的EndScene调用，频率可以降低）
//  作  者: Flicker317
//  更  新: 1.0 2012/11/30
//=================================================================
void CUnitManager::RefreshUnitId(FactionTypeId factionType, BOOL bHero, BOOL bWard, BOOL bBomb)
{
	DWORD dwArraySize;			// 数组大小
	DWORD dwStructSize;			// 结构体大小
	DWORD dwStructCount;		// 结构体总数
	DWORD dwCurrStruct;			// 当前遍历的结构地址，即 unit 或 item 的 id
	DWORD dwLocalIndex;			// 本地玩家索引
	DWORD dwCurrIndex;			// 当前单位索引

	dwStructSize	= GetMemory(m_dwGameAddr + 0xACB0D8);
	dwStructCount	= GetMemory(m_dwGameAddr + 0xACB0E0);
	dwArraySize		= GetMemory(m_dwGameAddr + 0xACB0DC);

	dwLocalIndex	= GetLocalPlayer();

	// 清空
	m_nHerosCount			= 0;
	m_nSentryWardsCount		= 0;
	m_nObserverWardsCount	= 0;
	m_nBombsCount			= 0;

	for (unsigned int i = 0, totalCount = 0; i < m_idAddrList.size(); i++)
	{
		for (unsigned int j = 0; j < dwArraySize && totalCount < dwStructCount; j++)
		{
			dwCurrStruct = m_idAddrList[i] + 4 + j * dwStructSize;

			if (GetMemory(dwCurrStruct) == m_dwGameAddr + 0x943A94)
			{
				totalCount++;

HEROBLOCK:
				if (bHero)
				{
					switch ((GetMemory(dwCurrStruct + 0x30) >> 24))
					{
					case 'O': case 'U': case 'H': case 'N': case 'E':
						break;
					default:
						goto WARDBLOCK;
					}

					dwCurrIndex = GetMemory(dwCurrStruct + 0x24);

					switch (factionType)
					{
					case FactionNone:
						break;
					case FactionBoth:
						{
							if (m_nHerosCount < HERO_COUNT_MAX && 
								(dwCurrIndex > 0 && dwCurrIndex < 6) || (dwCurrIndex > 6 && dwCurrIndex < 12))
							{
								AddToHerosUnit(dwCurrStruct, dwCurrIndex);
							}
						}
						break;
					case FactionAutoMatch:
						{
							if (m_nHerosCount < HERO_COUNT_MAX && 
								((dwCurrIndex > 0 && dwCurrIndex < 6) && (dwLocalIndex > 6 && dwLocalIndex < 12)) || 
								((dwLocalIndex > 0 && dwLocalIndex < 6) && (dwCurrIndex > 6 && dwCurrIndex < 12)))
							{
								AddToHerosUnit(dwCurrStruct, dwCurrIndex);
							}
						}
						break;
					case FactionSentinel:
						{
							if (m_nHerosCount < HERO_COUNT_MAX && dwCurrIndex > 0 && dwCurrIndex < 6)
							{
								AddToHerosUnit(dwCurrStruct, dwCurrIndex);
							}
						}
						break;
					case FactionScourge:
						{
							if (m_nHerosCount < HERO_COUNT_MAX && dwCurrIndex > 6 && dwCurrIndex < 12)
							{
								AddToHerosUnit(dwCurrStruct, dwCurrIndex);
							}
						}
						break;
					default:
						break;
					}
				}
WARDBLOCK:
				if (bWard)
				{
					if (GetMemory(dwCurrStruct + 0x8) == 0)
						goto BOMBBLOCK;

					dwCurrIndex = GetMemory(dwCurrStruct + 0x24);

					switch(GetMemory(dwCurrStruct + 0x30))
					{
					case 0x6F657965:	// 真眼
						{
							switch (factionType)
							{
							case FactionNone:
								break;
							case FactionBoth:
								{
									if (m_nObserverWardsCount < WARD_COUNT_MAX && 
										(dwCurrIndex > 0 && dwCurrIndex < 6) || (dwCurrIndex > 6 && dwCurrIndex < 12))
									{
										AddToObserverWardsUnit(dwCurrStruct, dwCurrIndex);
									}
								}
								break;
							case FactionAutoMatch:
								{
									if (m_nObserverWardsCount < WARD_COUNT_MAX && 
										((dwCurrIndex  > 0 && dwCurrIndex  < 6) && (dwLocalIndex > 6 && dwLocalIndex < 12)) || 
										((dwLocalIndex > 0 && dwLocalIndex < 6) && (dwCurrIndex  > 6 && dwCurrIndex  < 12)))
									{
										AddToObserverWardsUnit(dwCurrStruct, dwCurrIndex);
									}
								}
								break;
							case FactionSentinel:
								{
									if (m_nObserverWardsCount < WARD_COUNT_MAX && dwCurrIndex > 0 && dwCurrIndex < 6)
									{
										AddToObserverWardsUnit(dwCurrStruct, dwCurrIndex);
									}
								}
								break;
							case FactionScourge:
								{
									if (m_nObserverWardsCount < WARD_COUNT_MAX && dwCurrIndex > 6 && dwCurrIndex < 12)
									{
										AddToObserverWardsUnit(dwCurrStruct, dwCurrIndex);
									}
								}
								break;
							}
						}
						break;
					case 0x6F303034:	// 假眼
						{
							switch (factionType)
							{
							case FactionNone:
								break;
							case FactionBoth:
								{
									if (m_nSentryWardsCount < WARD_COUNT_MAX && 
										(dwCurrIndex > 0 && dwCurrIndex < 6) || (dwCurrIndex > 6 && dwCurrIndex < 12))
									{
										AddToSentryWardsUnit(dwCurrStruct, dwCurrIndex);
									}
								}
								break;
							case FactionAutoMatch:
								{
									if (m_nSentryWardsCount < WARD_COUNT_MAX && 
										((dwCurrIndex > 0 && dwCurrIndex < 6) && (dwLocalIndex > 6 && dwLocalIndex < 12)) || 
										((dwLocalIndex > 0 && dwLocalIndex < 6) && (dwCurrIndex > 6 && dwCurrIndex < 12)))
									{
										AddToSentryWardsUnit(dwCurrStruct, dwCurrIndex);
									}
								}
								break;
							case FactionSentinel:
								{
									if (m_nSentryWardsCount < WARD_COUNT_MAX && dwCurrIndex > 0 && dwCurrIndex < 6)
									{
										AddToSentryWardsUnit(dwCurrStruct, dwCurrIndex);
									}
								}
								break;
							case FactionScourge:
								{
									if (m_nSentryWardsCount < WARD_COUNT_MAX && dwCurrIndex > 6 && dwCurrIndex < 12)
									{
										AddToSentryWardsUnit(dwCurrStruct, dwCurrIndex);
									}
								}
								break;
							}
						}
						break;
					}

				}
BOMBBLOCK:
				if (bBomb)
				{
					if (GetMemory(dwCurrStruct + 0x8) == 0)
						continue;

					dwCurrIndex = GetMemory(dwCurrStruct + 0x24);

					switch(GetMemory(dwCurrStruct + 0x30))
					{
					case 0x6E30304F:
					case 0x6E303050:
					case 0x6E303051:
					case 0x6E30304E:
					case 0x6F746F74:
					case 0x6F303138:
					case 0x6F303032:
					case 0x6F303042:
					case 0x6F303142:
						{
							switch (factionType)
							{
							case FactionNone:
								break;
							case FactionBoth:
								{
									if (m_nBombsCount < BOMB_COUNT_MAX && 
										(dwCurrIndex > 0 && dwCurrIndex < 6) || (dwCurrIndex > 6 && dwCurrIndex < 12))
									{
										AddToBombsUnit(dwCurrStruct, dwCurrIndex);
									}
								}
								break;
							case FactionAutoMatch:
								{
									if (m_nBombsCount < BOMB_COUNT_MAX && 
										((dwCurrIndex > 0 && dwCurrIndex < 6) && (dwLocalIndex > 6 && dwLocalIndex < 12)) || 
										((dwLocalIndex > 0 && dwLocalIndex < 6) && (dwCurrIndex > 6 && dwCurrIndex < 12)))
									{
										AddToBombsUnit(dwCurrStruct, dwCurrIndex);
									}
								}
								break;
							case FactionSentinel:
								{
									if (m_nBombsCount < BOMB_COUNT_MAX && dwCurrIndex > 0 && dwCurrIndex < 6)
									{
										AddToBombsUnit(dwCurrStruct, dwCurrIndex);
									}
								}
								break;
							case FactionScourge:
								{
									if (m_nBombsCount < BOMB_COUNT_MAX && dwCurrIndex > 6 && dwCurrIndex < 12)
									{
										AddToBombsUnit(dwCurrStruct, dwCurrIndex);
									}
								}
								break;
							}
						}
						break;
					}
				}
			}
		}
	}
}


//=================================================================
//  功  能: 将英雄信息保存到成员变量
//  备  注: 无
//  作  者: Flicker317
//  更  新: 1.0 2012/11/19
//=================================================================
inline void CUnitManager::AddToHerosUnit(DWORD dwCurrStruct, DWORD dwCurrIndex)
{
	m_gHerosUnit[m_nHerosCount].id		= dwCurrStruct;
	m_gHerosUnit[m_nHerosCount].index	= dwCurrIndex;
	m_nHerosCount++;
}

//=================================================================
//  功  能: 将假眼信息保存到成员变量
//  备  注: 无
//  作  者: Flicker317
//  更  新: 1.0 2012/11/22
//=================================================================
inline void CUnitManager::AddToSentryWardsUnit(DWORD dwCurrStruct, DWORD dwCurrIndex)
{
	m_gSentryWardsUnit[m_nSentryWardsCount].id	  = dwCurrStruct;
	m_gSentryWardsUnit[m_nSentryWardsCount].index = dwCurrIndex;
	m_nSentryWardsCount++;
}
//=================================================================
//  功  能: 将真眼信息保存到成员变量
//  备  注: 无
//  作  者: Flicker317
//  更  新: 1.0 2012/11/22
//=================================================================
inline void CUnitManager::AddToObserverWardsUnit(DWORD dwCurrStruct, DWORD dwCurrIndex)
{
	m_gObserverWardsUnit[m_nObserverWardsCount].id	  = dwCurrStruct;
	m_gObserverWardsUnit[m_nObserverWardsCount].index = dwCurrIndex;
	m_nObserverWardsCount++;
}

//=================================================================
//  功  能: 将真眼炸弹信息保存到成员变量
//  备  注: 无
//  作  者: Flicker317
//  更  新: 1.0 2012/11/22
//=================================================================
inline void CUnitManager::AddToBombsUnit(DWORD dwCurrStruct, DWORD dwCurrIndex)
{
	m_gBombsUnit[m_nBombsCount].id	  = dwCurrStruct;
	m_gBombsUnit[m_nBombsCount].index = dwCurrIndex;
	m_nBombsCount++;
}

//=================================================================
//  功  能: 获取当前英雄坐标
//  备  注: 在游戏中实时调用（由hook住的EndScene调用）
//  作  者: Flicker317
//  更  新: 1.0 2012/11/19
//=================================================================
void CUnitManager::GetHerosLocation()
{
	// 应由玩家主动调用 RefreshHerosId(factionType); 

	for (unsigned int i = 0; i < m_nHerosCount; i++)
	{			
		/*
		// 可以通过单位生命值判断该单位是否死亡  下面的方法是直接通过内存值判断
		if (GetUnitState(dwCurrStruct, UNIT_STATE_LIFE) == 0)
			continue;
		*/
		// RVA    死亡	  存活
		// 064  00000000  00000F12
		// 0A8  00000000  ????????
		// 0B4  00000000  ????????
		// 194  00000004  00000000
		// 1C4  00000001  00000000
		// 25C  FFFFFFFF  000000??
		// 260  FFFFFFFF  000000??
		// 判断该单位是否死亡
		if (GetMemory(m_gHerosUnit[i].id + 0x64) == 0)
		{
			m_gHerosPoint[i].x = -10;
			m_gHerosPoint[i].y = -10;
		}
		else
		{
			m_gHerosPoint[i].x = *(float *)(m_gHerosUnit[i].id + 0x284);
			m_gHerosPoint[i].y = *(float *)(m_gHerosUnit[i].id + 0x288);

			m_gHerosPoint[i].x = ((7870 + m_gHerosPoint[i].x) * 0.0155f + 9) * m_fWidth;
			m_gHerosPoint[i].y = ((7910 - m_gHerosPoint[i].y) * 0.0115f + 572) * m_fHeight;
		}
	}
}

//=================================================================
//  功  能: 获取真假眼坐标
//  备  注: 在游戏中实时调用（由hook住的EndScene调用，频率可以降低）
//  作  者: Flicker317
//  更  新: 1.0 2012/11/22
//=================================================================
void CUnitManager::GetWardsLocation()
{
	for (unsigned int i = 0; i < m_nSentryWardsCount; i++)
	{
		m_gSentryWardsPoint[i].x = *(float *)(m_gSentryWardsUnit[i].id + 0x284);
		m_gSentryWardsPoint[i].y = *(float *)(m_gSentryWardsUnit[i].id + 0x288);

		m_gSentryWardsPoint[i].x = ((7870 + m_gSentryWardsPoint[i].x) * 0.0155f + 8) * m_fWidth;
		m_gSentryWardsPoint[i].y = ((7910 - m_gSentryWardsPoint[i].y) * 0.0115f + 572) * m_fHeight;
	}
	for (unsigned int i = 0; i < m_nObserverWardsCount; i++)
	{
		m_gObserverWardsPoint[i].x = *(float *)(m_gObserverWardsUnit[i].id + 0x284);
		m_gObserverWardsPoint[i].y = *(float *)(m_gObserverWardsUnit[i].id + 0x288);

		m_gObserverWardsPoint[i].x = ((7870 + m_gObserverWardsPoint[i].x) * 0.0155f + 8) * m_fWidth;
		m_gObserverWardsPoint[i].y = ((7910 - m_gObserverWardsPoint[i].y) * 0.0115f + 572) * m_fHeight;
	}

}

//=================================================================
//  功  能: 获取炸弹坐标
//  备  注: 在游戏中实时调用（由hook住的EndScene调用，频率可以降低）
//  作  者: Flicker317
//  更  新: 1.0 2012/11/22
//=================================================================
void CUnitManager::GetBombsLocation()
{
	for (unsigned int i = 0; i < m_nBombsCount; i++)
	{
		m_gBombsPoint[i].x = *(float *)(m_gBombsUnit[i].id + 0x284);
		m_gBombsPoint[i].y = *(float *)(m_gBombsUnit[i].id + 0x288);

		m_gBombsPoint[i].x = ((7870 + m_gBombsPoint[i].x) * 0.0155f + 9) * m_fWidth;
		m_gBombsPoint[i].y = ((7910 - m_gBombsPoint[i].y) * 0.0115f + 572) * m_fHeight;
	}
}

//=================================================================
//  功  能: 获取神符刷新信息
//  备  注: 在游戏中实时调用（由hook住的EndScene调用，频率可以降低）
//			Game.dll+0xACBA10  000000F8		结构体大小
//			Game.dll+0xACBA14  00000040		数组大小
//			Game.dll+0xACBA18  00000???		总数量
//			Game.dll+0xACBA1C  ????????		二维动态数组指针
//			Game.dll+0xACBA20  ????????		被销毁的单位链表
//  		结构体头为 Game.dll+0x944214	表示未被销毁
//  作  者: Flicker317
//  更  新: 1.0 2012/11/22
//=================================================================
void CUnitManager::GetRuneType()
{
	std::vector<DWORD> itemList;

	DWORD dwArraySize;			// 数组大小
	DWORD dwStructSize;			// 结构体大小
	DWORD dwStructCount;		// 结构体总数
	DWORD dwCurrStoreAddr;		// 当前遍历数组的地址（存入链表时）
	DWORD dwCurrStruct;			// 当前遍历的结构地址，即 unit 或 item 的 id

	dwStructSize	= GetMemory(m_dwGameAddr + 0xACBA10);
	dwStructCount	= GetMemory(m_dwGameAddr + 0xACBA18);
	dwArraySize		= GetMemory(m_dwGameAddr + 0xACBA14);

	dwCurrStoreAddr = m_dwGameAddr + 0xACBA1C;

	while ((dwCurrStoreAddr = GetMemory(dwCurrStoreAddr)))
	{
		itemList.push_back(dwCurrStoreAddr);
	}

	for (unsigned int i = 0, totalCount = 0; i < itemList.size(); i++)
	{
		for (unsigned int j = 0; j < dwArraySize && totalCount < dwStructCount; j++)
		{
			dwCurrStruct = itemList[i] + 4 + j * dwStructSize;

			// 判断单位是否被销毁
			if (GetMemory(dwCurrStruct) == m_dwGameAddr + 0x944214)
			{
				totalCount++;

				DWORD runeType = RUNE_TYPE_NONE;
				switch (GetMemory(dwCurrStruct + 0x30))
				{
				case 0x49303036:	// 极速
					runeType = RUNE_TYPE_SPEEDUP;
					break;
				case 0x49303037:	// 幻象
					runeType = RUNE_TYPE_PHANTOM;
					break;
				case 0x49303038:	// 恢复
					runeType = RUNE_TYPE_RECOVER;
					break;
				case 0x4930304A:	// 隐身
					runeType = RUNE_TYPE_INVISIBLE;
					break;
				case 0x4930304B:	// 双倍
					runeType = RUNE_TYPE_DOUBLE;
					break;
				default:
					continue;
				}

				// 0x58 != 0 && => 0x78 != 0xFFFFFFFF	则未吃
				if (GetMemory(dwCurrStruct + 0x58) != 0)
				{
					// itemx = [[itemid + 0x28] + 0x88]
					// itemy = [[itemid + 0x28] + 0x8C]
					if (GetMemory(GetMemory(dwCurrStruct + 0x28) + 0x88) == 0x453C0000)
					{
						// 刷新在下路
                        SetRuneLower(runeType);
					}
                    else
                    {
                        // 刷新在上路
                        SetRuneUpper(runeType);
                    }

					m_runeType = (RUNE_ENUM)runeType;

					return;
				}
			}
		}
	}

	m_runeType = RUNE_NONE;
}

/*——————————————————————————————————————————————————————————————————
    不建议在hook中调用下面的函数，应该由用户主动调用
*/

//=================================================================
//  功  能: 刷新保存英雄信息的数组
//  备  注: 仅当有需要时调用（如用户主动请求）
//  作  者: Flicker317
//  更  新: 1.0 2012/11/19
//=================================================================
void CUnitManager::RefreshHerosId(FactionTypeId factionType)
{
    DWORD dwArraySize;			// 数组大小
    DWORD dwStructSize;			// 结构体大小
    DWORD dwStructCount;		// 结构体总数
    DWORD dwCurrStruct;			// 当前遍历的结构地址，即 unit 或 item 的 id
    DWORD dwLocalIndex;			// 本地玩家索引
    DWORD dwCurrIndex;			// 当前单位索引

    dwStructSize	= GetMemory(m_dwGameAddr + 0xACB0D8);
    dwStructCount	= GetMemory(m_dwGameAddr + 0xACB0E0);
    dwArraySize		= GetMemory(m_dwGameAddr + 0xACB0DC);

    dwLocalIndex	= GetLocalPlayer();

    // 清空
    m_nHerosCount = 0;

    for (unsigned int i = 0, totalCount = 0; i < m_idAddrList.size(); i++)
    {
        for (unsigned int j = 0; j < dwArraySize && totalCount < dwStructCount; j++)
        {
            dwCurrStruct = m_idAddrList[i] + 4 + j * dwStructSize;

            // 判断单位是否被销毁
            if (GetMemory(dwCurrStruct) == m_dwGameAddr + 0x943A94)
            {
                totalCount++;

                // 从单位type中，获取最高字节的字符
                // 由字符判断是否是英雄
                switch ((GetMemory(dwCurrStruct + 0x30) >> 24))
                {
                case 'O': case 'U': case 'H': case 'N': case 'E':
                    break;
                default:
                    continue;
                }

                // 获取英雄的玩家索引
                dwCurrIndex = GetMemory(dwCurrStruct + 0x24);

                switch (factionType)
                {
                case FactionNone:
                    break;
                case FactionBoth:
                    {
                        if (m_nHerosCount < HERO_COUNT_MAX && 
                            (dwCurrIndex > 0 && dwCurrIndex < 6) || (dwCurrIndex > 6 && dwCurrIndex < 12))
                        {
                            AddToHerosUnit(dwCurrStruct, dwCurrIndex);
                        }
                    }
                    break;
                case FactionAutoMatch:
                    {
                        if (m_nHerosCount < HERO_COUNT_MAX && 
                            ((dwCurrIndex > 0 && dwCurrIndex < 6) && (dwLocalIndex > 6 && dwLocalIndex < 12)) || 
                            ((dwLocalIndex > 0 && dwLocalIndex < 6) && (dwCurrIndex > 6 && dwCurrIndex < 12)))
                        {
                            AddToHerosUnit(dwCurrStruct, dwCurrIndex);
                        }
                    }
                    break;
                case FactionSentinel:
                    {
                        if (m_nHerosCount < HERO_COUNT_MAX && dwCurrIndex > 0 && dwCurrIndex < 6)
                        {
                            AddToHerosUnit(dwCurrStruct, dwCurrIndex);
                        }
                    }
                    break;
                case FactionScourge:
                    {
                        if (m_nHerosCount < HERO_COUNT_MAX && dwCurrIndex > 6 && dwCurrIndex < 12)
                        {
                            AddToHerosUnit(dwCurrStruct, dwCurrIndex);
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }
}

//=================================================================
//  功  能: 刷新保存炸弹信息的数组
//  备  注: 在游戏中实时调用（由hook住的EndScene调用，频率可以降低）
//  作  者: Flicker317
//  更  新: 1.0 2012/11/22
//=================================================================
void CUnitManager::RefreshBombsId(FactionTypeId factionType)
{
    DWORD dwArraySize;			// 数组大小
    DWORD dwStructSize;			// 结构体大小
    DWORD dwStructCount;		// 结构体总数
    DWORD dwCurrStruct;			// 当前遍历的结构地址，即 unit 或 item 的 id
    DWORD dwLocalIndex;			// 本地玩家索引
    DWORD dwCurrIndex;			// 当前单位索引

    dwStructSize	= GetMemory(m_dwGameAddr + 0xACB0D8);
    dwStructCount	= GetMemory(m_dwGameAddr + 0xACB0E0);
    dwArraySize		= GetMemory(m_dwGameAddr + 0xACB0DC);

    dwLocalIndex	= GetLocalPlayer();

    // 清空
    m_nBombsCount	= 0;

    for (unsigned int i = 0, totalCount = 0; i < m_idAddrList.size(); i++)
    {
        for (unsigned int j = 0; j < dwArraySize && totalCount < dwStructCount; j++)
        {
            dwCurrStruct = m_idAddrList[i] + 4 + j * dwStructSize;

            // 判断单位是否被销毁
            if (GetMemory(dwCurrStruct) == m_dwGameAddr + 0x943A94)
            {
                totalCount++;

                // 单位消失后
                // +08  00000000
                // +0C  FFFFFFFF
                // +10  FFFFFFFF
                if (GetMemory(dwCurrStruct + 0x8) == 0)
                    continue;

                // 获取英雄的玩家索引
                dwCurrIndex = GetMemory(dwCurrStruct + 0x24);

                switch(GetMemory(dwCurrStruct + 0x30))
                {
                // 炸弹
                // 1技能
                // 6E30304F		1级
                // 6E303050		2级
                // 6E303051		3级
                // 6E30304E		4级
                // 
                // 2技能
                // 6F746F74  	1级-4级
                // 
                // 4技能
                // 6F303138		1级
                // 6F303032		2级
                // 6F303042		3级
                // 6F303142		A帐
                case 0x6E30304F:
                case 0x6E303050:
                case 0x6E303051:
                case 0x6E30304E:
                case 0x6F746F74:
                case 0x6F303138:
                case 0x6F303032:
                case 0x6F303042:
                case 0x6F303142:
                    {
                        switch (factionType)
                        {
                        case FactionNone:
                            break;
                        case FactionBoth:
                            {
                                if (m_nBombsCount < BOMB_COUNT_MAX && 
                                    (dwCurrIndex > 0 && dwCurrIndex < 6) || (dwCurrIndex > 6 && dwCurrIndex < 12))
                                {
                                    AddToBombsUnit(dwCurrStruct, dwCurrIndex);
                                }
                            }
                            break;
                        case FactionAutoMatch:
                            {
                                if (m_nBombsCount < BOMB_COUNT_MAX && 
                                    ((dwCurrIndex > 0 && dwCurrIndex < 6) && (dwLocalIndex > 6 && dwLocalIndex < 12)) || 
                                    ((dwLocalIndex > 0 && dwLocalIndex < 6) && (dwCurrIndex > 6 && dwCurrIndex < 12)))
                                {
                                    AddToBombsUnit(dwCurrStruct, dwCurrIndex);
                                }
                            }
                            break;
                        case FactionSentinel:
                            {
                                if (m_nBombsCount < BOMB_COUNT_MAX && dwCurrIndex > 0 && dwCurrIndex < 6)
                                {
                                    AddToBombsUnit(dwCurrStruct, dwCurrIndex);
                                }
                            }
                            break;
                        case FactionScourge:
                            {
                                if (m_nBombsCount < BOMB_COUNT_MAX && dwCurrIndex > 6 && dwCurrIndex < 12)
                                {
                                    AddToBombsUnit(dwCurrStruct, dwCurrIndex);
                                }
                            }
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }
}

//=================================================================
//  功  能: 刷新保存真假眼信息的数组
//  备  注: 在游戏中实时调用（由hook住的EndScene调用，频率可以降低）
//  作  者: Flicker317
//  更  新: 1.0 2012/11/19
//=================================================================
void CUnitManager::RefreshWardsId(FactionTypeId factionType)
{
    DWORD dwArraySize;			// 数组大小
    DWORD dwStructSize;			// 结构体大小
    DWORD dwStructCount;		// 结构体总数
    DWORD dwCurrStruct;			// 当前遍历的结构地址，即 unit 或 item 的 id
    DWORD dwLocalIndex;			// 本地玩家索引
    DWORD dwCurrIndex;			// 当前单位索引

    dwStructSize	= GetMemory(m_dwGameAddr + 0xACB0D8);
    dwStructCount	= GetMemory(m_dwGameAddr + 0xACB0E0);
    dwArraySize		= GetMemory(m_dwGameAddr + 0xACB0DC);

    dwLocalIndex	= GetLocalPlayer();

    // 清空
    m_nSentryWardsCount	  = 0;
    m_nObserverWardsCount = 0;

    for (unsigned int i = 0, totalCount = 0; i < m_idAddrList.size(); i++)
    {
        for (unsigned int j = 0; j < dwArraySize && totalCount < dwStructCount; j++)
        {
            dwCurrStruct = m_idAddrList[i] + 4 + j * dwStructSize;

            // 判断单位是否被销毁
            if (GetMemory(dwCurrStruct) == m_dwGameAddr + 0x943A94)
            {
                totalCount++;

                // 单位消失后
                // +08  00000000
                // +0C  FFFFFFFF
                // +10  FFFFFFFF
                if (GetMemory(dwCurrStruct + 0x8) == 0)
                    continue;

                // 获取英雄的玩家索引
                dwCurrIndex = GetMemory(dwCurrStruct + 0x24);

                switch(GetMemory(dwCurrStruct + 0x30))
                {
                case 0x6F657965:	// 真眼
                    {
                        switch (factionType)
                        {
                        case FactionNone:
                            break;
                        case FactionBoth:
                            {
                                if (m_nObserverWardsCount < WARD_COUNT_MAX && 
                                    (dwCurrIndex > 0 && dwCurrIndex < 6) || (dwCurrIndex > 6 && dwCurrIndex < 12))
                                {
                                    AddToObserverWardsUnit(dwCurrStruct, dwCurrIndex);
                                }
                            }
                            break;
                        case FactionAutoMatch:
                            {
                                if (m_nObserverWardsCount < WARD_COUNT_MAX && 
                                    ((dwCurrIndex > 0 && dwCurrIndex < 6) && (dwLocalIndex > 6 && dwLocalIndex < 12)) || 
                                    ((dwLocalIndex > 0 && dwLocalIndex < 6) && (dwCurrIndex > 6 && dwCurrIndex < 12)))
                                {
                                    AddToObserverWardsUnit(dwCurrStruct, dwCurrIndex);
                                }
                            }
                            break;
                        case FactionSentinel:
                            {
                                if (m_nObserverWardsCount < WARD_COUNT_MAX && dwCurrIndex > 0 && dwCurrIndex < 6)
                                {
                                    AddToObserverWardsUnit(dwCurrStruct, dwCurrIndex);
                                }
                            }
                            break;
                        case FactionScourge:
                            {
                                if (m_nObserverWardsCount < WARD_COUNT_MAX && dwCurrIndex > 6 && dwCurrIndex < 12)
                                {
                                    AddToObserverWardsUnit(dwCurrStruct, dwCurrIndex);
                                }
                            }
                            break;
                        }
                    }
                    break;
                case 0x6F303034:	// 假眼
                    {
                        switch (factionType)
                        {
                        case FactionNone:
                            break;
                        case FactionBoth:
                            {
                                if (m_nSentryWardsCount < WARD_COUNT_MAX && 
                                    (dwCurrIndex > 0 && dwCurrIndex < 6) || (dwCurrIndex > 6 && dwCurrIndex < 12))
                                {
                                    AddToSentryWardsUnit(dwCurrStruct, dwCurrIndex);
                                }
                            }
                            break;
                        case FactionAutoMatch:
                            {
                                if (m_nSentryWardsCount < WARD_COUNT_MAX && 
                                    ((dwCurrIndex > 0 && dwCurrIndex < 6) && (dwLocalIndex > 6 && dwLocalIndex < 12)) || 
                                    ((dwLocalIndex > 0 && dwLocalIndex < 6) && (dwCurrIndex > 6 && dwCurrIndex < 12)))
                                {
                                    AddToSentryWardsUnit(dwCurrStruct, dwCurrIndex);
                                }
                            }
                            break;
                        case FactionSentinel:
                            {
                                if (m_nSentryWardsCount < WARD_COUNT_MAX && dwCurrIndex > 0 && dwCurrIndex < 6)
                                {
                                    AddToSentryWardsUnit(dwCurrStruct, dwCurrIndex);
                                }
                            }
                            break;
                        case FactionScourge:
                            {
                                if (m_nSentryWardsCount < WARD_COUNT_MAX && dwCurrIndex > 6 && dwCurrIndex < 12)
                                {
                                    AddToSentryWardsUnit(dwCurrStruct, dwCurrIndex);
                                }
                            }
                            break;
                        }
                    }
                    break;
                }

            }
        }
    }
}
