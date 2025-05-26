#include "../userprog/syscall.h"
#include "lib.hh"

#define BUFFER_SIZE 64

int main(int argc, char **argv) {
    if (argc < 2) {
        puts_("Usage: cat <filename>");
        return 1;
    }

    const char *filename = argv[1];
    OpenFileId file = Open(filename);
    if (file < 0) {
        puts_("Error: could not open file.");
        return 1;
    }

    char buffer[BUFFER_SIZE];
    int bytesRead;

    while ((bytesRead = Read(buffer, BUFFER_SIZE, file)) > 0) {
        Write(buffer, bytesRead, CONSOLE_OUTPUT);
    }

    Close(file);
    return 0;
}
