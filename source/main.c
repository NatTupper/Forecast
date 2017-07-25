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
	weather_t* weather =  NULL;
	conf_t conf;
	bool menuExpanded = false;

	screen_begin_frame();
	screen_select(GFX_TOP);
		draw_top_frame();
	screen_select(GFX_BOTTOM);
		draw_bottom_frame();

	conf = get_config();

	weather = get_weather(conf, 0);
	if(weather == NULL){
		log_output("weather was NULL\n");
		cleanup();
		return 0;
	}
	screen_end_frame();

	// Main loop
	while (aptMainLoop()){
		u32 kDown;

		screen_begin_frame();

		screen_select(GFX_TOP);
		draw_weather_top(weather);
		screen_select(GFX_BOTTOM);
		draw_weather_bottom(weather, conf, menuExpanded);

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
			if(menuExpanded == false &&
					pos.px >= 0 && pos.px < 39 && pos.py >= 0 && pos.py < 29){
				menuExpanded = true;
			}
			else if(menuExpanded == true){
				if(pos.px >= 0 && pos.px < 39 && pos.py >= 0 && pos.py < 29){
					menuExpanded = false;
				}
				else if(pos.px >= 0 && pos.px < TOP_SCREEN_WIDTH/2){
					if(pos.py >= 34+17 && pos.py < 34+17*2){
						if(strcmp(conf.places[0].name, "(New place)") == 0){
							conf.places[0] = set_place(conf.places[0]);
						}
						free(weather);
						weather = get_weather(conf, 0);
					}
					else if(pos.py >= 34+17*2 && pos.py < 34+17*3){
						if(strcmp(conf.places[1].name, "(New place)") == 0){
							conf.places[1] = set_place(conf.places[1]);
						}
						free(weather);
						weather = get_weather(conf, 1);
					}
					else if(pos.py >= 34+17*3 && pos.py < 34+17*4){
						if(strcmp(conf.places[2].name, "(New place)") == 0){
							conf.places[2] = set_place(conf.places[2]);
						}
						free(weather);
						weather = get_weather(conf, 2);
					}
					else if(pos.py >= BOTTOM_SCREEN_HEIGHT/2+17 &&
							pos.py < BOTTOM_SCREEN_HEIGHT/2+17*2){
						weather->units = 0;
						conf.units = "imperial";
					}
					else if(pos.py >= BOTTOM_SCREEN_HEIGHT/2+17*2 &&
							pos.py < BOTTOM_SCREEN_HEIGHT/2+17*3){
						weather->units = 1;
						conf.units = "metric";
					}
				}
			}
		}

		screen_end_frame();
	}

	set_config(conf);
	// Exit services
	cleanup();
	return 0;
}
