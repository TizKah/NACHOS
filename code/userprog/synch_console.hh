#ifndef NACHOS_USERPROG_SYNCH_CONSOLE__H
#define NACHOS_USERPROG_SYNCH_CONSOLE__H

#include "../threads/semaphore.hh"
#include "../threads/lock.hh"
#include "console.hh"

class SynchConsole
{
public:
    SynchConsole(const char *readFile, const char *writeFile);
    ~SynchConsole();

    void PutChar(char ch);
    char GetChar();
    static void readAvail(void *data);
    static void writeDone(void *data);

private:
    Lock *write_lock;
    Semaphore *write_completed;

    Lock *read_lock;
    Semaphore *read_completed;

    Console *console;
};

#endif