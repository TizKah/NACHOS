#include "synch_console.hh"



static void ReadAvailStub(void *arg) {
    ((SynchConsole *)arg)->readAvail();
}

static void WriteDoneStub(void *arg) {
    ((SynchConsole *)arg)->writeDone();
}

SynchConsole::SynchConsole(const char *readFile, const char *writeFile, void *callArg)
{

    write_completed = new Semaphore("write sem", 0);
    write_lock = new Lock("write lock");

    read_completed = new Semaphore("read sem", 0);
    read_lock = new Lock("read lock");

    console = new Console(readFile, writeFile, ReadAvailStub, WriteDoneStub, callArg);
}


SynchConsole::~SynchConsole() {
    delete console;
    delete read_completed;
    delete read_lock;
    delete write_completed;
    delete write_lock;
}

void 
SynchConsole::PutChar(char ch)
{
    write_lock->Acquire();
    console->PutChar(ch);
    write_completed->P();
    write_lock->Release();
}

char 
SynchConsole::GetChar()
{
    char temp;
    read_lock->Acquire();
    read_completed->P();
    temp = console->GetChar();
    read_lock->Release();
    
    return temp;
}

void
SynchConsole::readAvail() {
    read_completed->V();
}

void
SynchConsole::writeDone() {
    write_completed->V();
}