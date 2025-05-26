#include "syscall.h"


int
main(void)
{
    SpaceId    newProc;
    OpenFileId input  = CONSOLE_INPUT;
    OpenFileId output = CONSOLE_OUTPUT;
    char       prompt[2] = { '-', '-' };
    char       ch, buffer[60];
    int        i;
    
    for (;;) {
        Write(prompt, 2, output);
        i = 0;
        do {
            Read(&buffer[i], 1, input);
        } while (buffer[i++] != '\n');
        
        buffer[--i] = '\0';
        
        if (i > 0) {
            int background = 0;
            
            if (buffer[0] == '&') {
                background = 1;

                for (int j = 0; j < i; j++) {
                    buffer[j] = buffer[j+1];
                }

                i--;
            }

            newProc = Exec(buffer);

            if (!background) {
                Join(newProc);
            }
        }
    }

    return -1;
}
