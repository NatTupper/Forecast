#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <3ds.h>

#include "common.h"
#include "screen.h"
#include "draw.h"

void draw_top_frame(){
	screen_draw_texture(TEXTURE_TOP_SCREEN_BG, 0, 0, 400, 240);
}

void draw_bottom_frame(){
	screen_draw_texture(TEXTURE_BOTTOM_SCREEN_BG, 0, 0, 320, 240);
}

void draw_weather_top(weather_t* weather, int units){
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
	screen_draw_string(tempString, 230, 85, 1.0f, 1.0f, COLOR_WHITE, false);
	screen_draw_string(desc, 230, 111, 0.75f, 0.75f, COLOR_WHITE, false);
	screen_draw_string(locationString, locationXCoord, 177, 1.0f, 1.0f, COLOR_WHITE, false);
}

void draw_weather_bottom(weather_t* weather, int units){
	int humidity = weather->humidity;
	double wind_speed = (units == 0)?(weather->wind_speed / 0.44704):(weather->wind_speed);

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
	screen_draw_string(humidityString, 22, 16, 0.75f, 0.75f, COLOR_WHITE, false);
	screen_draw_string(windSpeedString, 22, 36, 0.75f, 0.75f, COLOR_WHITE, false);
	if(units == 0){
		screen_draw_texture(TEXTURE_TOGGLED_BOX, 0, 192, 160, 48);
		screen_draw_texture(TEXTURE_UNTOGGLED_BOX, 160, 192, 160, 48);
	}
	else{
		screen_draw_texture(TEXTURE_UNTOGGLED_BOX, 0, 192, 160, 48);
		screen_draw_texture(TEXTURE_TOGGLED_BOX, 160, 192, 160, 48);
	}
	screen_draw_string("Imperial", 40, 205, 0.75f, 0.75f, COLOR_WHITE, false);
	screen_draw_string("Metric", 210, 205, 0.75f, 0.75f, COLOR_WHITE, false);
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
