#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <3ds.h>

#include "common.h"
#include "screen.h"
#include "draw.h"

static char* get_time(){
	time_t unixTime = time(NULL);
	struct tm* timeStruct = gmtime((const time_t*)&unixTime);
	char* buf = calloc(10, 1);

	sprintf(buf, "%02d:%02d", timeStruct->tm_hour, timeStruct->tm_min);

	return buf;
}

void draw_top_frame(){
	screen_draw_texture(TEXTURE_TOP_SCREEN_BG, 0, 0, 400, 240);
}

void draw_bottom_frame(){
	float width;
	char* curTime;

	screen_draw_texture(TEXTURE_BOTTOM_SCREEN_BG, 0, 0, 320, 240);
	screen_draw_texture(TEXTURE_MENUBAR, 0, 0, 320, 29);
	curTime = get_time();
	screen_get_string_size(&width, NULL, curTime, 0.75f, 0.75f);
	screen_draw_string(curTime, BOTTOM_SCREEN_WIDTH-width-4, 4, 0.75f, 0.75f, COLOR_WHITE, false);	
	screen_get_string_size(&width, NULL, "Press START to exit.", 0.75f, 0.75f);
	screen_draw_string("Press START to exit.", BOTTOM_SCREEN_WIDTH/2-width/2, 210, 0.75f, 0.75f, COLOR_WHITE, false);
}

void draw_weather_top(weather_t* weather){
	int units = weather->units;
	int id = weather->id;
	char* desc = weather->desc;
	double temp = (units == 0)?(1.8*(weather->temp - 273) + 32):(weather->temp - 273);
	char* name = weather->name;
	char* country = weather->country;

	char* tempString = calloc(10, 1);
	char* locationString = calloc(strlen(name)+2+strlen(country)+1, 1);
	float locationWidth;
	float locationXCoord;

	if(units == 0){
		snprintf(tempString, 10, "%.02lf°F", temp);
	}
	else{
		snprintf(tempString, 10, "%.02lf°C", temp);
	}
	snprintf(locationString, strlen(name)+2+strlen(country)+1, "%s, %s", name, country);
	screen_get_string_size(&locationWidth, NULL, locationString, 1.0f, 1.0f);
	locationXCoord = TOP_SCREEN_WIDTH/2 - locationWidth/2;

	draw_top_frame();
	draw_icon(id);
	float tempWidth;
	float descWidth;
	screen_get_string_size(&tempWidth, NULL, tempString, 1.0f, 1.0f);
	screen_draw_string(tempString, 230, 85, 1.0f, 1.0f, COLOR_WHITE, false);
	screen_get_string_size(&descWidth, NULL, desc, 0.75, 0.75);
	screen_draw_string(desc, 230+tempWidth/2-descWidth/2, 111, 0.75f, 0.75f, COLOR_WHITE, false);
	screen_draw_string(locationString, locationXCoord, 177, 1.0f, 1.0f, COLOR_WHITE, false);
}

void draw_weather_bottom(weather_t* weather, conf_t conf, bool drawMenu){
	int humidity = weather->humidity;
	int units = weather->units;
	double wind_speed = (units == 0)?(weather->wind_speed / 0.44704):(weather->wind_speed);
	char* places[3] = {
		(char*)conf.places[0].name,
		(char*)conf.places[1].name,
		(char*)conf.places[2].name
	};

	char* humidityString = calloc(20, 1);
	char* windSpeedString = calloc(25, 1);

	if(units == 0){
		snprintf(windSpeedString, 25, "Wind speed: %.02lf mph", wind_speed);
	}
	else{
		snprintf(windSpeedString, 25, "Wind speed: %.02lf m/s", wind_speed);
	}
	snprintf(humidityString, 20, "Humidity: %d%%", humidity);

	draw_bottom_frame();
	float width;
	float height;
	screen_get_string_size(&width, &height, humidityString, 0.75f, 0.75f);
	screen_draw_string(humidityString, BOTTOM_SCREEN_WIDTH/2-width/2, BOTTOM_SCREEN_HEIGHT/2-height-1, 0.75f, 0.75f, COLOR_WHITE, false);
	screen_get_string_size(&width, &height, windSpeedString, 0.75f, 0.75f);
	screen_draw_string(windSpeedString, BOTTOM_SCREEN_WIDTH/2-width/2, BOTTOM_SCREEN_HEIGHT/2+1, 0.75f, 0.75f, COLOR_WHITE, false);

	if(drawMenu == true){
		screen_draw_texture(TEXTURE_MENU_OVERLAY, 0, 29, 320, 211);
		screen_draw_string("Places:", 4, 34, 0.6f, 0.6f, COLOR_WHITE, false);
		int i;
		for(i = 0; i < 3; i++){
			screen_draw_string(places[i], 8, 34+17*(i+1), 0.6f, 0.6f, COLOR_WHITE, false);
		}
		screen_draw_string("Units", 4, BOTTOM_SCREEN_HEIGHT/2, 0.6f, 0.6f, COLOR_WHITE, false);
		screen_draw_string("Imperial", 8, BOTTOM_SCREEN_HEIGHT/2+17, 0.6f, 0.6f, COLOR_WHITE, false);
		screen_draw_string("Metric", 8, BOTTOM_SCREEN_HEIGHT/2+17*2, 0.6f, 0.6f, COLOR_WHITE, false);
	}
}

void draw_icon(int id){
	if(id >= 200 && id <= 232)
		screen_draw_texture(TEXTURE_CLOUD_WITH_LIGHTNING, 30, 35, 148, 140);
	else if(id >= 300 && id <= 321)
		screen_draw_texture(TEXTURE_CLOUD_WITH_SOME_RAIN, 30, 35, 148, 156);
	else if(id >= 500 && id <= 501)
		screen_draw_texture(TEXTURE_CLOUD_WITH_RAIN, 30, 35, 148, 146);
	else if(id >= 502 && id <= 531)
		screen_draw_texture(TEXTURE_CLOUD_WITH_LOTS_OF_RAIN, 30, 35, 148, 154);
	else if(id >= 600 && id <= 622)
		screen_draw_texture(TEXTURE_CLOUD_WITH_SNOW, 30, 35, 148, 165);
	else if(id == 800)
		screen_draw_texture(TEXTURE_SUN, 30, 35, 143, 142);
	else if(id >= 801 && id <= 804)
		screen_draw_texture(TEXTURE_SUN_WITH_CLOUD, 30, 35, 170, 122);
	else
		screen_draw_texture(TEXTURE_QUESTION, 30, 35, 66, 129);
}
