#pragma once

#include <jansson.h>

typedef struct{
	int zipcode;
	double lat;
	double lon;
	char* api_key;
	char* units;
} conf_t;

typedef struct{
	int id;
	char* desc;
	double temp;
	int humidity;
	double wind_speed;
	char* name;
	char* country;
	int units;
} weather_t;

double* get_geocoords();
weather_t* get_weather(conf_t conf);
conf_t get_config();
void set_config(conf_t conf);
