#pragma once

typedef enum{AUTODETECT, COORDS, ZIP} loc_type;

typedef union{
	double* coords;
	int zipcode;
} loc_t;

typedef struct{
	int id;
	char* desc;
	double temp;
	int humidity;
	double wind_speed;
	char* name;
	char* country;
} weather_t;

double* get_geocoords();
weather_t* get_weather(loc_t loc, loc_type type);
