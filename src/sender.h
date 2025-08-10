#ifndef SENDER_H
#define SENDER_H

#include <stdbool.h>
#include <windows.h>

bool send_data_https(const wchar_t* server, const wchar_t* endpoint, const char* json_data);
bool send_buffer_to_server(void);

#endif