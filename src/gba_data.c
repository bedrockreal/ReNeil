#include "boutiste.h"
#include "gba_data.h"

/* #include <stdio.h> */
#include <string.h>

#define GBA_DEFAULT_CSS_ORDER	0x00 /* Neil is primary */


const char *GBA_DEFAULT_TAUNTS[GBA_NUM_TAUNTS] = {
	"Whoa...",					/* Control stick up */
	"You gotta hit this!",		/* Control stick down */
	"You better nail it!",		/* Control stick left */
	"Don't freak out!",			/* Control stick right */
	"You can do it!",			/* C stick up */
	"Looking good!",			/* C stick down */
	"Think happy\x01thoughts!",	/* C stick left */
	"Watch the ball!"			/* C stick right */
};

const char *GBA_TAUNT_CONTROLLER_STRING[GBA_NUM_TAUNTS] = {
	"Control stick up",
	"Control stick down",
	"Control stick left",
	"Control stick right",
	"C stick up",
	"C stick down",
	"C stick left",
	"C stick right"
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

void GBAMakeDefaultPair(GBASavePair *pair) {
	GBADeletePair(pair);

	// set Neil's default stats
	strcpy(pair->neil.name, "Neil");
	set_be16(&pair->neil.driveDistance, 205);
	pair->neil.shot.height = -1;
	pair->neil.shot.type = 1;
	pair->neil.shot.curve = 2;
	pair->neil.shot.impact = 4;
	pair->neil.shot.control = 4;
	pair->neil.shot.spin = -3;

	// set Ella's default stats
	strcpy(pair->ella.name, "Ella");
	set_be16(&pair->ella.driveDistance, 200);
	pair->ella.shot.height = 3;
	pair->ella.shot.type = 1;
	pair->ella.shot.curve = 0;
	pair->ella.shot.impact = 5;
	pair->ella.shot.control = 6;
	pair->ella.shot.spin = 0;

	// setup taunts
	for (int i = 0; i < GBA_NUM_TAUNTS; ++i) {
		strcpy(pair->taunts[i].str, GBA_DEFAULT_TAUNTS[i]);
		pair->taunts[i].type = (i < 4 ? 1 : 0);
		pair->taunts[i].index = i;
	}
	
	// make this pair active
	memset(&pair->check, 0xff, GBA_CHECK_SIZE);

	// CSSOrder = 0 already, no need modify

	// unlock all custom clubs by default
	set_be16(&pair->customWoodsBitmask, 0xffff);
	set_be16(&pair->customIronsBitmask, 0xffff);
	set_be16(&pair->customWedgesBitmask, 0xffff);
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
