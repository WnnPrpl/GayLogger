#ifndef BUFFER_H
#define BUFFER_H

#include <stdbool.h>
#include <stddef.h>

bool buffer_init(void);
void buffer_cleanup(void);
void buffer_add_key(const char* key);
char* buffer_get_json_and_clear(void);

#endif