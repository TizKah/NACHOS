#include "../userprog/syscall.h"
#include "lib.hh"
#include <stdio.h>

unsigned strlen(const char *s ) {
    if (!s) return 0;

    unsigned len = 0;
    for(; *s != '\0'; s++, len++);
    return len;
}

void puts_(const char *s ) {
    Write(s, strlen(s), CONSOLE_OUTPUT);
    char final_line = '\n';
    Write(&final_line, sizeof final_line, CONSOLE_OUTPUT);
}

void itoa (int n , char *str ) {
    if(!str) return;
    sprintf(str, "%d", n);
}