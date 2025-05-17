#include "../userprog/syscall.h"

#define BUFFER_TAM 20

int main() {
    char buffer[BUFFER_TAM];
    int readenBytes;

    readenBytes = Read(buffer, BUFFER_TAM, CONSOLE_INPUT);

    if (readenBytes > 0) {
        Write(buffer, readenBytes, CONSOLE_OUTPUT);
    }

    Exit(0);
}