#ifndef _STRUCT_H_
#define _STRUCT_H_

// Íæ¼ÒÑÕÉ«
const DWORD cg_gColor[16] = {
    0xFFFF0202,
    0xFF0041FF,
    0xFF1BE6D8,
    0xFF530080,
    0xFFFFFC00,
    0xFFFE890D,
    0xFF1FBF00,
    0xFFE55AAF,
    0xFF949596,
    0xFF7DBEF1,
    0xFF0F6145,
    0xFF4D2903,
    0xFF272727,
    0xFF272727,
    0xFF272727,
    0xFF272727
};

#define GetMemory(x)    (*(DWORD *)(x))
#define GetRWC3(x)      (((x) & 0x00FF0000) >> 16)
#define GetGWC3(x)      (((x) & 0x0000FF00) >> 8)
#define GetBWC3(x)      (((x) & 0x000000FF))

#define COLOR_AREA			0xFF00FF00
#define COLOR_TEXT			0x40B0E020
#define COLOR_RUNE_TEXT		0xFFFFFF00
#define COLOR_HERO			0xFFFFFF00
#define COLOR_BOMB			0xFFFF0000
#define COLOR_SENTRY_WARD	0xFFFFFF00
#define COLOR_OBSERVER_WARD	0xFF00FFD0

#define HERO_COUNT_MAX	20
#define WARD_COUNT_MAX	40
#define BOMB_COUNT_MAX	100
#define UNIT_TABLE_MAX	100

typedef enum _FactionType
{
    FactionNone = 0,
    FactionBoth,
    FactionAutoMatch,
    FactionSentinel,
    FactionScourge
} FactionTypeId;

typedef enum _RUNE_TYPE_ENUM
{
    RUNE_TYPE_NONE = 0,
    RUNE_TYPE_SPEEDUP,
    RUNE_TYPE_PHANTOM,
    RUNE_TYPE_RECOVER,
    RUNE_TYPE_INVISIBLE,
    RUNE_TYPE_DOUBLE,
} RUNE_TYPE_ENUM;

typedef enum _RUNE_ENUM
{
    RUNE_NONE = 0,
    RUNE_UPPER_SPEEDUP,
    RUNE_UPPER_PHANTOM,
    RUNE_UPPER_RECOVER,
    RUNE_UPPER_INVISIBLE,
    RUNE_UPPER_DOUBLE,
    RUNE_LOWER_SPEEDUP,
    RUNE_LOWER_PHANTOM,
    RUNE_LOWER_RECOVER,
    RUNE_LOWER_INVISIBLE,
    RUNE_LOWER_DOUBLE,
} RUNE_ENUM;

#define SetRuneUpper(x)     
#define SetRuneLower(x)     ((x) += 5)

typedef struct _WC3_UNIT
{
    DWORD id;
    DWORD handle;
    DWORD index;
    DWORD type;
} WC3_UNIT, *LPWC3_UNIT;

typedef struct _MINI_MAP_POINT
{
    float x;
    float y;
} MINI_MAP_POINT, *LPMINI_MAP_POINT;

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

    _GAME_OPTION()
    {
        on = FALSE;
    }

} GAME_OPTION, *PGAME_OPTION;

#endif