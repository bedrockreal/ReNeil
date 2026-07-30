#include "boutiste.h"
#include "gba_data.h"

/* #include <stdio.h> */
#include <string.h>

#define GBA_DEFAULT_CSS_ORDER	0x00 /* Neil is primary */


const char *GBA_DEFAULT_TAUNTS[2][GBA_NUM_TAUNTS] = {
	// Neil's taunts
	"Whoa...",					/* Control stick up */
	"You gotta hit this!",		/* Control stick down */
	"You better nail it!",		/* Control stick left */
	"Don't freak out!",			/* Control stick right */
	"You can do it!",			/* C stick up */
	"Looking good!",			/* C stick down */
	"Think happy\x01thoughts!",	/* C stick left */
	"Watch the ball!",			/* C stick right */

	// Ella's taunts
	"Keep your\x01head down!",	/* Control stick up */
	"Watch your elbows!",		/* Control stick down */
	"Don't overthink it!",		/* Control stick left */
	"Are you\x01gonna swing?",	/* Control stick right */
	"Good luck!",				/* C stick up */
	"Swing like\x01you always do!",	/* C stick down */
	"Don't worry!\x01You'll do fine!",	/* C stick left */
	"Let's go!"					/* C stick right */
};

const char *GBA_TAUNT_CONTROLLER_STRING[GBA_NUM_TAUNTS] = {
	"Control Stick Up",
	"Control Stick Down",
	"Control Stick Left",
	"Control Stick Right",
	"C Stick Up",
	"C Stick Down",
	"C Stick Left",
	"C Stick Right"
};

const char *GBA_CUSTOM_CLUB_NAME[GBA_NUM_CUSTOM_CLUBS] = {
	"Basic",
	"POW",
	"Super POW",
	"Low-fly",
	"Super Low-fly",
	"Low-fly Spin",
	"Backspin",
	"Super Spin",
	"Straight",
	"Super Straight",
	"Straight 'n' Low",
	"Sweet",
	"Super Sweet",
	"Control",
	"Sweet Control",
	"Risky"
};

void strReplace(char *dest, const char *src, int n, char from, char to) {
	for (int i = 0; i < n; ++i) {
		dest[i] = (src[i] == from ? to : src[i]);
	}
}

const char *getControllerString(GBATaunt *taunt) {
	return GBA_TAUNT_CONTROLLER_STRING[taunt->index];
}

const char *getCustomClubName(int index) {
	return GBA_CUSTOM_CLUB_NAME[index];
}

bool isGBAPairActive(GBASavePair *pair) {
	assert(pair != NULL);
	for (int i = 0; i < GBA_CHECK_SIZE; ++i) {
		if (pair->check[i] != 0) {
			/* printf("a\n"); */
			/* fflush(stdout); */
			return 1;
		}
	}
	return 0;
}

void GBASetDefaultTaunts(GBASavePair *pair) {
	uint8_t p = pair->cssPrimaryCharacter;
	assert(p == 0 || p == 1);
	for (int i = 0; i < GBA_NUM_TAUNTS; ++i) {
		strcpy(pair->taunts[i].str, GBA_DEFAULT_TAUNTS[p][i]);
		pair->taunts[i].type = (i < 4 ? 1 : 0);
		pair->taunts[i].index = i;
	}
}

void GBAMakeDefaultPair(GBASavePair *pair) {
	GBADeletePair(pair);

	// set Neil's default stats
	strcpy(pair->neil.name, "Neil");
	set_be16(&pair->neil.driveDistance, 205);
	pair->neil.icon = GBA_CHARACTER_NEIL;
	pair->neil.isLefty = 0;
	pair->neil.unusedOne = 0x01;
	pair->neil.shot.height = -1;
	pair->neil.shot.type = SHOTTYPE_DRAW;
	pair->neil.shot.curve = 2;
	pair->neil.shot.impact = 4;
	pair->neil.shot.control = 4;
	pair->neil.shot.spin = -3;

	// set Ella's default stats
	strcpy(pair->ella.name, "Ella");
	set_be16(&pair->ella.driveDistance, 200);
	pair->ella.icon = GBA_CHARACTER_ELLA;
	pair->ella.isLefty = 0;
	pair->ella.unusedOne = 0x01;
	pair->ella.shot.height = 3;
	pair->ella.shot.type = SHOTTYPE_DRAW;
	pair->ella.shot.curve = 0;
	pair->ella.shot.impact = 5;
	pair->ella.shot.control = 6;
	pair->ella.shot.spin = 0;

	// make this pair active
	memset(&pair->check, 0xff, GBA_CHECK_SIZE);

	// reset experience
	set_be16(&pair->experience[GBA_CHARACTER_NEIL], 0);
	set_be16(&pair->experience[GBA_CHARACTER_ELLA], 0);

	// CSSOrder = 0 already, no need modify

	// unlock all custom clubs by default
	set_be16(&pair->customWoodsBitmask, 0xffff);
	set_be16(&pair->customIronsBitmask, 0xffff);
	set_be16(&pair->customWedgesBitmask, 0xffff);

	// setup taunts
	GBASetDefaultTaunts(pair);
}

void GBACopyPair(GBASavePair *dest, GBASavePair *src) {
	memcpy(dest, src, GBA_SAVE_PAIR_SIZE);
}

void GBASwapPair(GBASavePair *p1, GBASavePair *p2) {
	GBASavePair tmp;
	GBACopyPair(&tmp, p1);
	GBACopyPair(p1, p2);
	GBACopyPair(p2, &tmp);
}

void GBADeletePair(GBASavePair *pair) {
	memset(pair, 0, GBA_SAVE_PAIR_SIZE);
}
