#include <stdio.h>
#include <string.h>

#include <3ds.h>
#include <jansson.h>

#include "util.h"
#include "http.h"
#include "json.h"

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

weather_t* get_weather(loc_t loc, loc_type type){
	weather_t* weather = malloc(sizeof(weather_t));
	char* appid = "4121cc6770416a4535826cab7e35fb29";
	int zipcode;
	char* baseZipUrl = "http://api.openweathermap.org/data/2.5/weather?q=%05d&appid=%s";
	double lat;
	double lon;
	char* baseCoordsUrl = "http://api.openweathermap.org/data/2.5/weather?lat=%lf&lon=%lf&appid=%s";
	json_t* root;
	json_t* child;
	char* url = calloc(256, 1);

	if(type == AUTODETECT){
		double* coords = get_geocoords();
		lat = coords[0];
		lon = coords[1];
		sprintf(url, baseCoordsUrl, lat, lon, appid);
	}
	else if(type == COORDS){
		lat = loc.coords[0];
		lon = loc.coords[1];
		sprintf(url, baseCoordsUrl, lat, lon, appid);
	}
	else{
		zipcode = loc.zipcode;
		sprintf(url, baseZipUrl, zipcode, appid);
	}

	char* text = http_download(url);
	printf("Data downloaded!\n");

	root = json_loads(text, 0, NULL);
	if(!root){
		printf("Json failed to load.\n");
		return NULL;
	}
	free(text);

	//find id
	child = json_object_get(root, "weather");
	child = json_array_get(child, 0);
	child = json_object_get(child, "id");
	if(!json_is_number(child)){
		fprintf(stderr, "id is not a number!\n");
		exit(1);
	}
	weather->id = (int)json_number_value(child);

	//find desc
	child = json_object_get(root, "weather");
	child = json_array_get(child, 0);
	child = json_object_get(child, "description");
	if(!json_is_string(child)){
		fprintf(stderr, "description is not a string!\n");
		exit(1);
	}
	weather->desc = (char*)json_string_value(child);

	//find temp
	child = json_object_get(root, "main");
	child = json_object_get(child, "temp");
	if(!json_is_number(child)){
		fprintf(stderr, "temp is not a number!\n");
		exit(1);
	}
	weather->temp = json_number_value(child);

	//find humidity
	child = json_object_get(root, "main");
	child = json_object_get(child, "humidity");
	if(!json_is_number(child)){
		fprintf(stderr, "humidity is not number!\n");
		exit(1);
	}
	weather->humidity = (int)json_number_value(child);

	//find wind speed
	child = json_object_get(root, "wind");
	child = json_object_get(child, "speed");
	if(!json_is_number(child)){
		fprintf(stderr, "speed is not number!\n");
		exit(1);
	}
	weather->wind_speed = json_number_value(child);

	//find name
	child = json_object_get(root, "name");
	if(!json_is_string(child)){
		fprintf(stderr, "name is not a string!\n");
		exit(1);
	}
	weather->name = (char*)json_string_value(child);

	//find country
	child = json_object_get(root, "sys");
	child = json_object_get(child, "country");
	if(!json_is_string(child)){
		fprintf(stderr, "country is not a string!\n");
		exit(1);
	}
	weather->country = (char*)json_string_value(child);

	return weather;
}
