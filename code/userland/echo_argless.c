/// Outputs arguments entered on the command line.

#include "syscall.h"

unsigned
StringLength(const char *s)
{
    // What if `s` is null?

    unsigned i;
    for (i = 0; s[i] != '\0'; i++) {}
    return i;
}

int
PrintString(const char *s)
{
    // What if `s` is null?
    unsigned len = StringLength(s);
    return Write(s, len, CONSOLE_OUTPUT);
}

int
PrintChar(char c)
{
    return Write(&c, 1, CONSOLE_OUTPUT);
}

int
main()
{
    char* str[] = {"hola, ", "rezando ", "que ", "esto ", "funcione."};
    unsigned len = sizeof str / sizeof str[0];

    for (unsigned i = 0; i < len; i++) {
        if (i != 0) {
            PrintChar(' ');
        }
        PrintString(str[i]);
    }
    PrintChar('\n');
}
