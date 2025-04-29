#include "synch_console.hh"

SynchConsole::SynchConsole(const char *readFile, const char *writeFile)
{

    write_completed = new Semaphore("write sem", 0);
    write_lock = new Lock("write lock");

    read_completed = new Semaphore("read sem", 0);
    read_lock = new Lock("read lock");

    console = new Console(readFile, writeFile, readAvail, writeDone, this);
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
SynchConsole::readAvail(void *data) {
    ((SynchConsole *)data)->read_completed->V();
}

void
SynchConsole::writeDone(void *data) {
    ((SynchConsole *)data)->write_completed->V();
}