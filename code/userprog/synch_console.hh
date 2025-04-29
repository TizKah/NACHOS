#ifndef NACHOS_USERPROG_SYNCH_CONSOLE__H
#define NACHOS_USERPROG_SYNCH_CONSOLE__H

#include "semaphore.hh"
#include "lock.hh"
#include "console.hh"

class SynchConsole
{
public:
    SynchConsole(const char *readFile, const char *writeFile, void *callArg);
    ~SynchConsole();

    void PutChar(char ch);
    char GetChar();
    void readAvail();
    void writeDone();

private:
    Lock *write_lock;
    Semaphore *write_completed;

    Lock *read_lock;
    Semaphore *read_completed;

    Console *console;
};

#endif