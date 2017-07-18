#pragma once

#define TOP_SCREEN_WIDTH 400
#define TOP_SCREEN_HEIGHT 240

#define BOTTOM_SCREEN_WIDTH 320
#define BOTTOM_SCREEN_HEIGHT 240

#define MAX_TEXTURES 1024

#define TEXTURE_TOP_SCREEN_BG 1
#define TEXTURE_BOTTOM_SCREEN_BG 2
#define TEXTURE_CLOUD 3
#define TEXTURE_CLOUD_WITH_LIGHTNING 4
#define TEXTURE_CLOUD_WITH_LOTS_OF_RAIN 5
#define TEXTURE_CLOUD_WITH_RAIN 6
#define TEXTURE_CLOUD_WITH_SNOW 7
#define TEXTURE_CLOUD_WITH_SOME_RAIN 8
#define TEXTURE_SUN 9
#define TEXTURE_SUN_WITH_CLOUD 10
#define TEXTURE_SUN_WITH_RAIN 11
#define TEXTURE_QUESTION 12
#define TEXTURE_TOGGLED_BOX 13
#define TEXTURE_UNTOGGLED_BOX 14

#define TEXTURE_AUTO_START 32

#define NUM_COLORS 11

#define COLOR_TEXT 0
#define COLOR_NAND 1
#define COLOR_SD 2
#define COLOR_GAME_CARD 3
#define COLOR_DS_TITLE 4
#define COLOR_FILE 5
#define COLOR_DIRECTORY 6
#define COLOR_ENABLED 7
#define COLOR_DISABLED 8
#define COLOR_INSTALLED 9
#define COLOR_NOT_INSTALLED 10

void screen_init();
void screen_exit();
void screen_load_texture(u32 id, void* data, u32 size, u32 width, u32 height, GPU_TEXCOLOR format, bool linearFilter);
u32 screen_load_texture_auto(void* data, u32 size, u32 width, u32 height, GPU_TEXCOLOR format, bool linearFilter);
void screen_load_texture_file(u32 id, const char* path, bool linearFilter);
u32 screen_load_texture_file_auto(const char* path, bool linearFilter);
void screen_load_texture_tiled(u32 id, void* tiledData, u32 size, u32 width, u32 height, GPU_TEXCOLOR format, bool linearFilter);
u32 screen_load_texture_tiled_auto(void* tiledData, u32 size, u32 width, u32 height, GPU_TEXCOLOR format, bool linearFilter);
void screen_unload_texture(u32 id);
void screen_get_texture_size(u32* width, u32* height, u32 id);
void screen_begin_frame();
void screen_end_frame();
void screen_select(gfxScreen_t screen);
void screen_set_scissor(bool enabled, u32 x, u32 y, u32 width, u32 height);
void screen_draw_texture(u32 id, float x, float y, float width, float height);
void screen_draw_texture_crop(u32 id, float x, float y, float width, float height);
void screen_get_string_size(float* width, float* height, const char* text, float scaleX, float scaleY);
void screen_get_string_size_wrap(float* width, float* height, const char* text, float scaleX, float scaleY, float wrapX);
void screen_draw_string(const char* text, float x, float y, float scaleX, float scaleY, u32 colorId, bool centerLines);
void screen_draw_string_wrap(const char* text, float x, float y, float scaleX, float scaleY, u32 colorId, bool centerLines, float wrapX);
