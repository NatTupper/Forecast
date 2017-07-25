#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <3ds.h>
#include <jansson.h>

#include "common.h"
#include "util.h"
#include "http.h"
#include "json.h"
#include "draw.h"
#include "screen.h"

double* get_geocoords(){
	json_t* root;
	json_t* child;
	char* url = "http://ip-api.com/json";
	char* data;
	double* coords = malloc(2*sizeof(double));

	data = http_download(url);

	root = json_loads(data, 0, NULL);
	if(!root){
		log_output("Failed to load json\n");
		log_output("data: %s\n", data);
		return NULL;
	}

	child = json_object_get(root, "lat");
	if(!child){
		json_decref(root);
		log_output("Failed to get object \"lat\"\n");
		return NULL;
	}
	coords[0] = json_number_value(child);

	child = json_object_get(root, "lon");
	if(!child){
		json_decref(root);
		log_output("Failed to get object \"lon\"\n");
		return NULL;
	}
	coords[1] = json_number_value(child);

	if(coords[0] == 0.0f || coords[1] == 0.0f){
		json_decref(root);
		json_decref(child);
		log_output("json_number_value_failed\n");
		return NULL;
	}

	json_decref(root);
	json_decref(child);
	return coords;
}

weather_t* get_weather(conf_t conf, int index){
	weather_t* weather = malloc(sizeof(weather_t));
	char* baseZipUrl = "http://api.openweathermap.org/data/2.5/weather?zip=%05d&appid=%s";
	char* baseCoordsUrl = "http://api.openweathermap.org/data/2.5/weather?lat=%lf&lon=%lf&appid=%s";
	double lat;
	double lon;
	json_t* root;
	json_t* child;
	char* url = calloc(256, 1);

	if(conf.places[index].zipcode == -1 && conf.places[index].lat == -1.0){
		double* coords = get_geocoords();
		if(coords == NULL){
			log_output("coords are NULL\n");
			return NULL;
		}
		lat = coords[0];
		lon = coords[1];
		sprintf(url, baseCoordsUrl, lat, lon, conf.api_key);
	}
	else if(conf.places[index].zipcode == -1){
		sprintf(url, baseCoordsUrl, conf.places[index].lat, conf.places[index].lon, conf.api_key);
	}
	else{
		sprintf(url, baseZipUrl, conf.places[index].zipcode, conf.api_key);
	}

	if(strcmp("imperial", conf.units) == 0)
		weather->units = 0;
	else
		weather->units = 1;

	float width;
	float height;
	screen_get_string_size(&width, &height, "Loading...", 1.0f, 1.0f);
	screen_draw_string("Loading...", BOTTOM_SCREEN_WIDTH/2-width/2, BOTTOM_SCREEN_HEIGHT/2-height/2, 1.0f, 1.0f, COLOR_WHITE, false);
	screen_end_frame();
	char* text = http_download(url);
	screen_begin_frame();
	screen_select(GFX_BOTTOM);
	screen_draw_string("Loading...", BOTTOM_SCREEN_WIDTH/2-width/2, BOTTOM_SCREEN_HEIGHT/2-height/2, 1.0f, 1.0f, COLOR_WHITE, false);

	root = json_loads(text, 0, NULL);
	if(!root){
		log_output("Json failed to load.\n");
		log_output("text: %s\n", text);
		return NULL;
	}
	free(text);

	//find id
	child = json_object_get(root, "weather");
	child = json_array_get(child, 0);
	child = json_object_get(child, "id");
	if(!json_is_number(child)){
		log_output("id is not a number!\n");
		return NULL;
	}
	weather->id = (int)json_number_value(child);

	//find desc
	child = json_object_get(root, "weather");
	child = json_array_get(child, 0);
	child = json_object_get(child, "description");
	if(!json_is_string(child)){
		log_output("description is not a string!\n");
		return NULL;
	}
	weather->desc = (char*)json_string_value(child);

	//find temp
	child = json_object_get(root, "main");
	child = json_object_get(child, "temp");
	if(!json_is_number(child)){
		log_output("temp is not a number!\n");
		return NULL;
	}
	weather->temp = json_number_value(child);

	//find humidity
	child = json_object_get(root, "main");
	child = json_object_get(child, "humidity");
	if(!json_is_number(child)){
		log_output("humidity is not number!\n");
		return NULL;
	}
	weather->humidity = (int)json_number_value(child);

	//find wind speed
	child = json_object_get(root, "wind");
	child = json_object_get(child, "speed");
	if(!json_is_number(child)){
		log_output("speed is not number!\n");
		return NULL;
	}
	weather->wind_speed = json_number_value(child);

	//find name
	child = json_object_get(root, "name");
	if(!json_is_string(child)){
		log_output("name is not a string!\n");
		return NULL;
	}
	weather->name = (char*)json_string_value(child);

	//find country
	child = json_object_get(root, "sys");
	child = json_object_get(child, "country");
	if(!json_is_string(child)){
		log_output("country is not a string!\n");
		return NULL;
	}
	weather->country = (char*)json_string_value(child);

	return weather;
}

conf_t get_config(){
	const char* romfsPath = "romfs:/config.json";
	const char* sdmcPath = "/weather/config.json";
	json_t* root;
	json_t* child;
	json_t* subchild;
	conf_t conf;

	if(access(sdmcPath, F_OK) != -1)
		root = json_load_file(sdmcPath, 0, NULL);
	else
		root = json_load_file(romfsPath, 0, NULL);
	if(!root){
		log_output("root is NULL\n");
		return conf;
	}

	conf.places = malloc(3*sizeof(place_t));

	child = json_object_get(root, "api_key");
	if(!child){
		log_output("api_key is NULL\n");
		return conf;
	}
	conf.api_key = (char*)json_string_value(child);
	child = json_object_get(root, "units");
	if(!child){
		log_output("units is NULL\n");
		return conf;
	}
	conf.units = (char*)json_string_value(child);
	root = json_object_get(root, "places");
	int i;
	for(i = 0; i < 3; i++){
		child = json_array_get(root, i);

		subchild = json_object_get(child, "name");
		conf.places[i].name = json_string_value(subchild);

		subchild = json_object_get(child, "zipcode");
		conf.places[i].zipcode = json_integer_value(subchild);

		subchild = json_object_get(child, "lat");
		conf.places[i].lat = json_real_value(subchild);

		subchild = json_object_get(child, "lon");
		conf.places[i].lon = json_real_value(subchild);
	}

	return conf;
}

void set_config(conf_t conf){
	json_t* root;
	json_t* childA;
	json_t* childB;
	json_t* childC;

	root = json_object();

	childA = json_string(conf.api_key);
	json_object_set(root, "api_key", childA);

	childA = json_string(conf.units);
	json_object_set(root, "units", childA);

	childA = json_array();
	int i;
	for(i = 0; i < 3; i++){
		childB = json_object();
		json_array_append(childA, childB);

		childC = json_string(conf.places[i].name);
		json_object_set(childB, "name", childC);

		childC = json_integer(conf.places[i].zipcode);
		json_object_set(childB, "zipcode", childC);

		childC = json_real(conf.places[i].lat);
		json_object_set(childB, "lat", childC);

		childC = json_real(conf.places[i].lon);
		json_object_set(childB, "lon", childC);
	}

	json_object_set(root, "places", childA);

	json_dump_file(root, "/weather/config.json", 0);
}

place_t set_place(place_t place){
	SwkbdState swkbd;
	char* mybuf;

	mybuf = calloc(10, 1);
	swkbdInit(&swkbd, SWKBD_TYPE_NUMPAD, 2, 5);
	swkbdSetHintText(&swkbd, "Enter zipcode");
	swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT, "Cancel", false);
	swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, "OK", true);
	swkbdInputText(&swkbd, mybuf, 10);
	place.zipcode = atoi(mybuf);
	free(mybuf);

	mybuf = calloc(60, 1);
	swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, -1);
	swkbdSetHintText(&swkbd, "Enter a name for this place");
	swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT, "Cancel", false);
	swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, "OK", true);
	swkbdInputText(&swkbd, mybuf, 60);
	place.name = mybuf;

	return place;
}
