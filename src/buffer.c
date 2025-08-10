#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char* buffer = NULL;
static size_t buffer_len = 0;
static size_t buffer_cap = 0;

bool buffer_init(void) {
    buffer_cap = 1024;
    buffer_len = 0;
    buffer = malloc(buffer_cap);
    if (!buffer) return false;
    buffer[0] = '\0';
    return true;
}

void buffer_cleanup(void) {
    if (buffer) {
        free(buffer);
        buffer = NULL;
        buffer_len = 0;
        buffer_cap = 0;
    }
}

void buffer_add_key(const char* key) {
    size_t key_len = strlen(key);
    if (buffer_len + key_len + 1 > buffer_cap) {
        size_t new_cap = buffer_cap * 2;
        while (new_cap < buffer_len + key_len + 1) new_cap *= 2;
        char* new_buf = realloc(buffer, new_cap);
        if (!new_buf) return;
        buffer = new_buf;
        buffer_cap = new_cap;
    }
    memcpy(buffer + buffer_len, key, key_len);
    buffer_len += key_len;
    buffer[buffer_len] = '\0';
}

char* buffer_get_json_and_clear(void) {
    if (!buffer || buffer_len == 0) {
        return NULL;
    }

    size_t json_cap = buffer_len * 2 + 64;
    char* json = malloc(json_cap);
    if (!json) return NULL;


    snprintf(json, json_cap, "{\"keys\":\"%s\"}", buffer);

    buffer_len = 0;
    if (buffer) buffer[0] = '\0';

    return json;
}
