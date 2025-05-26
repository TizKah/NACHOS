#include "../userprog/syscall.h"
#include "lib.hh"

int main(int argc, char **argv) {
    if (argc < 2) {
        puts_("Usage: rm <filename>");
        return 1;
    }

    const char *filename = argv[1];

    if (Remove(filename) == 0) {
        puts_("File removed successfully.");
    } else {
        puts_("Error: could not remove file.");
    }

    return 0;
}
