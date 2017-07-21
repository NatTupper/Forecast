#include <stdio.h>
#include <string.h>
#include <unistd.h>

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

weather_t* get_weather(conf_t conf){
	weather_t* weather = malloc(sizeof(weather_t));
	char* baseZipUrl = "http://api.openweathermap.org/data/2.5/weather?zip=%05d&appid=%s";
	char* baseCoordsUrl = "http://api.openweathermap.org/data/2.5/weather?lat=%lf&lon=%lf&appid=%s";
	double lat;
	double lon;
	json_t* root;
	json_t* child;
	char* url = calloc(256, 1);

	if(conf.zipcode == -1 && conf.lat == -1.0){
		double* coords = get_geocoords();
		if(coords == NULL){
			log_output("coords are NULL\n");
			return NULL;
		}
		lat = coords[0];
		lon = coords[1];
		sprintf(url, baseCoordsUrl, lat, lon, conf.api_key);
	}
	else if(conf.zipcode == -1){
		sprintf(url, baseCoordsUrl, conf.lat, conf.lon, conf.api_key);
	}
	else{
		sprintf(url, baseZipUrl, conf.zipcode, conf.api_key);
	}

	if(strcmp("imperial", conf.units) == 0)
		weather->units = 0;
	else
		weather->units = 1;

	char* text = http_download(url);

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
	conf_t conf;

	if(access(sdmcPath, F_OK) != -1)
		root = json_load_file(sdmcPath, 0, NULL);
	else
		root = json_load_file(romfsPath, 0, NULL);
	if(!root){
		log_output("root is NULL\n");
		return conf;
	}

	child = json_object_get(root, "zipcode");
	if(!child){
		log_output("zipcode is NULL\n");
		return conf;
	}
	conf.zipcode = json_integer_value(child);
	child = json_object_get(root, "lat");
	if(!child){
		log_output("lat is NULL\n");
		return conf;
	}
	conf.lat = json_real_value(child);
	child = json_object_get(root, "lon");
	if(!child){
		log_output("lon is NULL\n");
		return conf;
	}
	conf.lon = json_real_value(child);
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

	return conf;
}

void set_config(conf_t conf){
	json_t* root;
	json_t* child;

	root = json_object();

	child = json_integer(conf.zipcode);
	json_object_set(root, "zipcode", child);

	child = json_real(conf.lat);
	json_object_set(root, "lat", child);

	child = json_real(conf.lon);
	json_object_set(root, "lon", child);

	child = json_string(conf.api_key);
	json_object_set(root, "api_key", child);

	child = json_string(conf.units);
	json_object_set(root, "units", child);

	json_dump_file(root, "/weather/config.json", 0);
}
