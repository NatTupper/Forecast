#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <3ds.h>

#include "util.h"
#include "error.h"
#include "linkedlist.h"

static stream_t stream_out;

extern void cleanup();

static int util_get_line_length(PrintConsole* console, const char* str) {
	int lineLength = 0;
	while(*str != 0) {
		if(*str == '\n') {
			break;
		}

		lineLength++;
		if(lineLength >= console->consoleWidth - 1) {
			break;
		}

		str++;
	}

	return lineLength;
}

static int util_get_lines(PrintConsole* console, const char* str) {
	int lines = 1;
	int lineLength = 0;
	while(*str != 0) {
		if(*str == '\n') {
			lines++;
			lineLength = 0;
		} else {
			lineLength++;
			if(lineLength >= console->consoleWidth - 1) {
				lines++;
				lineLength = 0;
			}
		}

		str++;
	}

	return lines;
}

void util_panic(const char* s, ...) {
	va_list list;
	va_start(list, s);

	char buf[1024];
	vsnprintf(buf, 1024, s, list);

	va_end(list);

	gspWaitForVBlank();

	u16 width;
	u16 height;
	for(int i = 0; i < 2; i++) {
		memset(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &width, &height), 0, (size_t) (width * height * 3));
		memset(gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, &width, &height), 0, (size_t) (width * height * 3));
		memset(gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, &width, &height), 0, (size_t) (width * height * 3));

		gfxSwapBuffers();
	}

	PrintConsole* console = consoleInit(GFX_TOP, NULL);

	const char* header_format = "%s has encountered a fatal error!";
	char* header = calloc(strlen(header_format)-2+strlen(APP_NAME)+1, 1);
	sprintf(header, header_format, APP_NAME);
	const char* footer = "Press any button to exit.";

	printf("\x1b[0;0H");
	for(int i = 0; i < console->consoleWidth; i++) {
		printf("-");
	}

	printf("\x1b[%d;0H", console->consoleHeight - 1);
	for(int i = 0; i < console->consoleWidth; i++) {
		printf("-");
	}

	printf("\x1b[0;%dH%s", (console->consoleWidth - util_get_line_length(console, header)) / 2, header);
	printf("\x1b[%d;%dH%s", console->consoleHeight - 1, (console->consoleWidth - util_get_line_length(console, footer)) / 2, footer);

	int bufRow = (console->consoleHeight - util_get_lines(console, buf)) / 2;
	char* str = buf;
	while(*str != 0) {
		if(*str == '\n') {
			bufRow++;
			str++;
			continue;
		} else {
			int lineLength = util_get_line_length(console, str);

			char old = *(str + lineLength);
			*(str + lineLength) = '\0';
			printf("\x1b[%d;%dH%s", bufRow, (console->consoleWidth - lineLength) / 2, str);
			*(str + lineLength) = old;

			bufRow++;
			str += lineLength;
		}
	}

	gfxFlushBuffers();
	gspWaitForVBlank();

	while(aptMainLoop()) {
		hidScanInput();
		if(hidKeysDown() & ~KEY_TOUCH) {
			break;
		}

		gspWaitForVBlank();
	}

	cleanup();
	exit(1);
}

FS_Path* util_make_path_utf8(const char* path) {
	size_t len = strlen(path);

	u16* utf16 = (u16*) calloc(len + 1, sizeof(u16));
	if(utf16 == NULL) {
		return NULL;
	}

	ssize_t utf16Len = utf8_to_utf16(utf16, (const uint8_t*) path, len);

	FS_Path* fsPath = (FS_Path*) calloc(1, sizeof(FS_Path));
	if(fsPath == NULL) {
		free(utf16);
		return NULL;
	}

	fsPath->type = PATH_UTF16;
	fsPath->size = (utf16Len + 1) * sizeof(u16);
	fsPath->data = utf16;

	return fsPath;
}

void util_free_path_utf8(FS_Path* path) {
	free((void*) path->data);
	free(path);
}

FS_Path util_make_binary_path(const void* data, u32 size) {
	FS_Path path = {PATH_BINARY, size, data};
	return path;
}

typedef struct {
	FS_Archive archive;
	u32 refs;
} archive_ref;

static linked_list opened_archives;

Result util_open_archive(FS_Archive* archive, FS_ArchiveID id, FS_Path path) {
	if(archive == NULL) {
		return R_INVALID_ARGUMENT;
	}

	Result res = 0;

	FS_Archive arch = 0;
	if(R_SUCCEEDED(res = FSUSER_OpenArchive(&arch, id, path))) {
		if(R_SUCCEEDED(res = util_ref_archive(arch))) {
			*archive = arch;
		} else {
			FSUSER_CloseArchive(arch);
		}
	}

	return res;
}

Result util_ref_archive(FS_Archive archive) {
	linked_list_iter iter;
	linked_list_iterate(&opened_archives, &iter);

	while(linked_list_iter_has_next(&iter)) {
		archive_ref* ref = (archive_ref*) linked_list_iter_next(&iter);
		if(ref->archive == archive) {
			ref->refs++;
			return 0;
		}
	}

	Result res = 0;

	archive_ref* ref = (archive_ref*) calloc(1, sizeof(archive_ref));
	if(ref != NULL) {
		ref->archive = archive;
		ref->refs = 1;

		linked_list_add(&opened_archives, ref);
	} else {
		res = R_OUT_OF_MEMORY;
	}

	return res;
}

Result util_close_archive(FS_Archive archive) {
	linked_list_iter iter;
	linked_list_iterate(&opened_archives, &iter);

	while(linked_list_iter_has_next(&iter)) {
		archive_ref* ref = (archive_ref*) linked_list_iter_next(&iter);
		if(ref->archive == archive) {
			ref->refs--;

			if(ref->refs == 0) {
				linked_list_iter_remove(&iter);
				free(ref);
			} else {
				return 0;
			}
		}
	}

	return FSUSER_CloseArchive(archive);
}

void log_reset(){
	FILE* output_file;
	struct stat st = {0};

	if(stat("/weather", &st) == -1){
		mkdir("/weather", 0755);
	}

	output_file = fopen("/weather/weather.log", "w");

	fclose(output_file);
}

void log_set_stream(stream_t stream){
	stream_out = stream;
}

void log_output(char* output, ...){
	FILE* output_file;
	struct stat st = {0};
	va_list args;
	char buffer[2048];

	if(stat("/weather", &st) == -1){
		mkdir("/weather", 0755);
	}

	output_file = fopen("/weather/weather.log", "a");

	memset(buffer, '\0', 2048);
	va_start(args, output);
	vsnprintf(buffer, 2048, output, args);
	if(stream_out == STREAM_FILE)
		fprintf(output_file, buffer);
	else
		printf(buffer);
	va_end(args);

	fclose(output_file);
}
