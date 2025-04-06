#include "channel.hh"
#include "condition.hh"
#include "lock.hh"
#include <stdio.h>
#include "system.hh"


Channel::Channel() {
    condition_lock = new Lock("channel_lock");
    sender_condition = new Condition("sender_condition", condition_lock);
    receiver_condition = new Condition("receiver_condition", condition_lock);
    buffer_busy = false;
}

Channel::~Channel() {
    delete condition_lock;
    delete sender_condition;
    delete receiver_condition;
}


void Channel::Send(int message) {
    condition_lock->Acquire();
    
    while(buffer_busy) {
        printf("Thread %s esperando receptor...\n", currentThread->GetName());
        sender_condition->Wait();
    }
    
    // Seteo el mensaje
    buffer = message;
    buffer_busy = true;
    printf("Thread %s mandó el mensaje: %d\n", currentThread->GetName(), buffer);
    
    // Le aviso al receiver que ya puede recibir el mensaje
    receiver_condition->Signal();

    condition_lock->Release();

}


void Channel::Receive(int *message) {
    condition_lock->Acquire();
    
    while(!buffer_busy) {
        printf("Thread %s esperando emisor...\n", currentThread->GetName());
        receiver_condition->Wait();
    }
    
    // Recibo el mensaje
    *message = buffer;
    buffer_busy = false;
    printf("Thread %s recibió el mensaje: %d\n", currentThread->GetName(), *message);
    
    // Le aviso al sender que ya leí el mensaje
    // Es decir, ya puede usar el buffer nuevamente
    sender_condition->Signal();

    condition_lock->Release();

}