#ifndef RENEIL_GBA_DATA_H
#define RENEIL_GBA_DATA_H

#include <assert.h>
#include <stdint.h>

#define MAX_GBA_SAVE_PAIRS	4
#define GBA_SAVE_PAIR_SIZE	0x284
#define GBA_CHARACTER_SIZE	0x2c
#define GBA_TAUNT_SIZE		0x42
#define GBA_CHECK_SIZE			0x10
#define GBA_UNUSED4_SIZE		0x04
#define GBA_CSS_ORDER_SIZE		0x01
#define GBA_UNUSED1_SIZE		0x01
#define GBA_CLUB_DATA_SIZE		0x6

static_assert(GBA_SAVE_PAIR_SIZE == 2 * GBA_CHARACTER_SIZE
		+ 8 * GBA_TAUNT_SIZE
		+ GBA_CHECK_SIZE
		+ GBA_UNUSED4_SIZE
		+ GBA_CSS_ORDER_SIZE
		+ GBA_UNUSED1_SIZE
		+ GBA_CLUB_DATA_SIZE);

typedef struct {
	int8_t height;
	uint8_t type;
	uint8_t curveIntensity;
	int8_t impact;
	int8_t control;
	int8_t spin;
} ShotAttributes;

typedef struct {
	uint8_t startPad[0x10];
	char name[0xa];
	uint8_t driveDist[2];
	uint8_t icon;
	uint8_t isLefty;
	uint8_t unused;
	ShotAttributes shot;
	uint8_t endPad[0x7];
} GBACharacter;

typedef struct {
	char str[0x40];
	uint8_t type;
	uint8_t index;
} GBATaunt;

typedef struct {
	GBACharacter neil;
	GBACharacter ella;
	GBATaunt taunts[8];
	uint8_t check[0x10];
	uint8_t unused4[GBA_UNUSED4_SIZE];
	uint8_t pairOrder;
	uint8_t unused1;
	uint8_t customWoodsBitmask[2];
	uint8_t customIronsBitmask[2];
	uint8_t customWedgesBitmask[2];
} GBASavePair;

static_assert(sizeof(ShotAttributes) == 0x6);
static_assert(sizeof(GBACharacter) == GBA_CHARACTER_SIZE);
static_assert(sizeof(GBATaunt) == GBA_TAUNT_SIZE);
static_assert(sizeof(GBASavePair) == GBA_SAVE_PAIR_SIZE);



#endif
