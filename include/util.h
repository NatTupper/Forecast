#pragma once

#define APP_NAME "Pocket-NLSE"

typedef struct linked_list_s linked_list;
typedef enum {STREAM_STDOUT, STREAM_FILE} stream_t;

void util_panic(const char* s, ...);

FS_Path* util_make_path_utf8(const char* path);
void util_free_path_utf8(FS_Path* path);

FS_Path util_make_binary_path(const void* data, u32 size);

Result util_open_archive(FS_Archive* archive, FS_ArchiveID id, FS_Path path);
Result util_ref_archive(FS_Archive archive);
Result util_close_archive(FS_Archive archive);

void util_escape_file_name(char* out, const char* in, size_t size);

void log_reset();
void log_set_stream(stream_t stream);
void log_output(char* output, ...);
