#ifndef UTILS_H
#define UTILS_H

#include <windows.h>

DWORD get_time_ms(void);

void log_info(const char* fmt, ...);
void log_error(const char* fmt, ...);

int wchar_to_utf8(const wchar_t* wstr, char* out, int out_size);
int utf8_to_wchar(const char* str, wchar_t* out, int out_size);

#endif