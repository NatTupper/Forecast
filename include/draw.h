#pragma once

#include "json.h"

void draw_top_frame();
void draw_bottom_frame();

void draw_weather_top(weather_t* weather, int units);
void draw_weather_bottom(weather_t* weather, int units);
void draw_icon(int id);
