#include "../userprog/syscall.h"
#include "lib.hh"

#define BUFFER_SIZE 64

int main(int argc, char **argv) {
    if (argc < 3) {
        puts_("Usage: cp <source> <destination>");
        return 1;
    }

    const char *src = argv[1];
    const char *dst = argv[2];

    OpenFileId srcFile = Open(src);
    if (srcFile < 0) {
        puts_("Error: could not open source file.");
        return 1;
    }

    OpenFileId dstFile = Open(dst);
    if (dstFile < 0) {
        if (Create(dst) < 0) {
            puts_("Error: could not create destination file.");
            Close(srcFile);
            return 1;
        }
        dstFile = Open(dst);
        if (dstFile < 0) {
            puts_("Error: could not open destination file.");
            Close(srcFile);
            return 1;
        }
    }



    char buffer[BUFFER_SIZE];
    int bytesRead;
    int totalBytesRead = 0;

    while (bytesRead = Read(buffer, BUFFER_SIZE, srcFile)) {
        Write(buffer, bytesRead, dstFile);
    } 
    

    Close(srcFile);
    Close(dstFile);
    return 0;
}
