/// Routines for synchronizing threads.
///
/// The implementation for this primitive does not come with base Nachos.
/// It is left to the student.
///
/// When implementing this module, keep in mind that any implementation of a
/// synchronization routine needs some primitive atomic operation.  The
/// semaphore implementation, for example, disables interrupts in order to
/// achieve this; another way could be leveraging an already existing
/// primitive.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "lock.hh"
#include "system.hh"
#include "../lib/debug.hh"

/// Dummy functions -- so we can compile our later assignments.

Lock::Lock(const char *debugName)
{
    name = debugName;
    sem = new Semaphore(debugName, 1);
    owner = NULL;
}

Lock::~Lock()
{
    delete sem;
}

const char *
Lock::GetName() const
{
    return name;
}

void
Lock::Acquire()
{
    ASSERT(!IsHeldByCurrentThread());
    DEBUG('s', "Thread %s trying to acquire lock %s\n", currentThread->GetName(), name);

#ifdef PRIORITY_INVERSION
    if (owner != nullptr && owner->GetPriority() < currentThread->GetPriority()) {
        DEBUG('b', "The owner %s has lower priority than the current thread %s.\n", owner->GetName(), currentThread->GetName());
        currentThread->SetPriority(owner->GetPriority());
    }
#endif

    sem->P();
    owner = currentThread;
    DEBUG('s', "Thread %s acquired lock %s\n", currentThread->GetName(), name);
}

void
Lock::Release()
{
    ASSERT(IsHeldByCurrentThread());
    DEBUG('s', "Thread %s trying to release lock %s\n", currentThread->GetName(), name);

#ifdef PRIORITY_INVERSION
    currentThread->SetPriority(currentThread->GetOriginalPriority());
#endif

    owner = NULL;
    sem->V();
    DEBUG('s', "Thread %s released lock %s\n", currentThread->GetName(), name);
}

bool
Lock::IsHeldByCurrentThread() const
{
    return (owner == currentThread);
}
