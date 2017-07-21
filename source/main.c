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
	weather_t* weather;
	conf_t conf;

	screen_begin_frame();
	screen_select(GFX_TOP);
		draw_top_frame();
	screen_select(GFX_BOTTOM);
		draw_bottom_frame();
	screen_end_frame();

	conf = get_config();

	weather = get_weather(conf);
	if(weather == NULL){
		log_output("weather was NULL\n");
		cleanup();
		return 0;
	}

	// Main loop
	while (aptMainLoop()){
		u32 kDown;

		screen_begin_frame();

		screen_select(GFX_TOP);
		draw_weather_top(weather);
		screen_select(GFX_BOTTOM);
		draw_weather_bottom(weather);

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
				weather->units = (pos.px >= 0 && pos.px < BOTTOM_SCREEN_WIDTH/2)?0:1;
				conf.units = (weather->units == 0)?"imperial":"metric";
			}
		}

		screen_end_frame();
	}

	set_config(conf);
	// Exit services
	cleanup();
	return 0;
}
