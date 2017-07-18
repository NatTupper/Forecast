#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include <3ds.h>
#include <jansson.h>

#include "common.h"
#include "util.h"
#include "screen.h"
#include "draw.h"
#include "json.h"

void cleanup(){
	httpcExit();
	screen_exit();
	romfsExit();
	gfxExit();
}

void init(){
	log_reset();
	//log_set_stream(STREAM_STDOUT);
	log_set_stream(STREAM_FILE);
	gfxInitDefault();
	romfsInit();
	screen_init();
	httpcInit(0); // Buffer size when POST/PUT.
}

int main(){
	init();
	loc_t loc;
	weather_t* weather; 
	int units = 0;

	screen_begin_frame();
	screen_select(GFX_TOP);
		draw_top_frame();
	screen_select(GFX_BOTTOM);
		draw_bottom_frame();
	screen_end_frame();

	weather = get_weather(loc, AUTODETECT);
	if(weather == NULL){
		cleanup();
		return 0;
	}

	// Main loop
	while (aptMainLoop()){
		u32 kDown;

		screen_begin_frame();

		screen_select(GFX_TOP);
		draw_weather_top(weather, units);
		screen_select(GFX_BOTTOM);
		draw_weather_bottom(weather, units);

		hidScanInput();

		kDown = hidKeysDown();
		if(kDown & KEY_START){
			screen_end_frame();
			break;
		}
		if(kDown & KEY_TOUCH){
			touchPosition pos;
			hidTouchRead(&pos);

			if(pos.py >= 192 && pos.py < BOTTOM_SCREEN_HEIGHT){
				units = (pos.px >= 0 && pos.px < BOTTOM_SCREEN_WIDTH/2)?0:1;
			}
		}

		screen_end_frame();
	}

	// Exit services
	cleanup();
	return 0;
}
