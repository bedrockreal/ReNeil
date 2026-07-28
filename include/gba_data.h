#ifndef RENEIL_GBA_DATA_H
#define RENEIL_GBA_DATA_H

#include "boutiste.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_GBA_SAVE_PAIRS	4
#define GBA_SAVE_PAIR_SIZE	0x284
#define GBA_CHARACTER_SIZE	0x2c
#define GBA_NUM_CHARACTERS	2
#define GBA_TAUNT_SIZE		0x42
#define GBA_NUM_TAUNTS		8
#define GBA_CHECK_SIZE			0x10
#define GBA_UNUSED4_SIZE		0x04
#define GBA_CSS_ORDER_SIZE		0x01
#define GBA_UNUSED1_SIZE		0x01
#define GBA_NUM_CUSTOM_CLUBS	16
#define GBA_CLUB_DATA_SIZE		0x6

static_assert(GBA_SAVE_PAIR_SIZE
		== GBA_NUM_CHARACTERS * GBA_CHARACTER_SIZE
		+ GBA_NUM_TAUNTS * GBA_TAUNT_SIZE
		+ GBA_CHECK_SIZE
		+ GBA_UNUSED4_SIZE
		+ GBA_CSS_ORDER_SIZE
		+ GBA_UNUSED1_SIZE
		+ GBA_CLUB_DATA_SIZE);

/* typedef enum { */
/* 	GBA_CHARACTER_NEIL, */
/* 	GBA_CHARACTER_ELLA */
/* } GBACharacter; */

#define GBA_CHARACTER_NEIL	0
#define GBA_CHARACTER_ELLA	1

typedef struct {
	int8_t height;
	uint8_t type;	/* 0=fade, 1=draw*/
	uint8_t curve;
	int8_t impact;
	int8_t control;
	int8_t spin;
} ShotAttributes;

typedef struct {
	uint8_t startPad[0x10];
	char name[0xa];
	uint16_be driveDistance;
	uint8_t icon;
	uint8_t isLefty;
	uint8_t unused;
	ShotAttributes shot;
	uint8_t endPad[0x7];
} GBACharacterData;

typedef struct {
	char str[0x40];
	uint8_t type;
	uint8_t index;
} GBATaunt;

typedef struct {
	GBACharacterData neil;
	GBACharacterData ella;
	GBATaunt taunts[8];
	uint8_t check[0x10];
	uint8_t unused4[GBA_UNUSED4_SIZE];
	uint8_t cssPrimaryCharacter;
	uint8_t unused1;
	uint16_be customWoodsBitmask;
	uint16_be customIronsBitmask;
	uint16_be customWedgesBitmask;
} GBASavePair;

/* static_assert(sizeof(GBACharacter) == 1); */
static_assert(sizeof(ShotAttributes) == 0x6);
static_assert(sizeof(GBACharacterData) == GBA_CHARACTER_SIZE);
static_assert(sizeof(GBATaunt) == GBA_TAUNT_SIZE);
static_assert(sizeof(GBASavePair) == GBA_SAVE_PAIR_SIZE);

const char *getControllerString(GBATaunt *taunt);
const char *getCustomClubName(int index);

bool isGBAPairActive(GBASavePair *pair);
void GBAMakeDefaultPair(GBASavePair *pair);
void GBACopyPair(GBASavePair *dest, GBASavePair *src);
void GBASwapPair(GBASavePair *p1, GBASavePair *p2);
void GBADeletePair(GBASavePair *pair);

#endif
