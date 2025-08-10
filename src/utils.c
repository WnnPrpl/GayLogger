#include "utils.h"
#include <stdio.h>
#include <stdarg.h>

DWORD get_time_ms(void) {
    return GetTickCount();
}

void log_info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
}

void log_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

int wchar_to_utf8(const wchar_t* wstr, char* out, int out_size) {
    if (!wstr || !out || out_size <= 0) return 0;
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, out, out_size, NULL, NULL);
    if (len == 0) {
        out[0] = 0;
        return 0;
    }
    return len;
}

int utf8_to_wchar(const char* str, wchar_t* out, int out_size) {
    if (!str || !out || out_size <= 0) return 0;
    int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, out, out_size);
    if (len == 0) {
        out[0] = 0;
        return 0;
    }
    return len;
}
