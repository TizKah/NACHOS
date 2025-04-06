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


#include "condition.hh"
#include "semaphore.hh"

/// Dummy functions -- so we can compile our later assignments.
///

Condition::Condition(const char *debugName, Lock *conditionLock)
{
    name = debugName;
    lock = conditionLock;
    waitQueue = new List<Semaphore *>();
}

Condition::~Condition()
{
    delete waitQueue;
}

const char *
Condition::GetName() const
{
    return name;
}

void
Condition::Wait()
{
    ASSERT(lock->IsHeldByCurrentThread());

    Semaphore *waiter = new Semaphore("wait cond", 0);
    waitQueue->Append(waiter);
    lock->Release();
    waiter->P();
    lock->Acquire();
    delete waiter;
}

void
Condition::Signal()
{
    ASSERT(lock->IsHeldByCurrentThread());

    if(!waitQueue->IsEmpty()) {
        Semaphore* WakingUpWaiter = waitQueue->Pop();
        WakingUpWaiter->V();
    }
}

void
Condition::Broadcast()
{
    ASSERT(lock->IsHeldByCurrentThread());

    while (!waitQueue->IsEmpty()) {
        Semaphore *s = waitQueue->Pop();
        s->V();
    }
}
