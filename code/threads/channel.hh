


#ifndef NACHOS_THREADS_CHANNEL__HH
#define NACHOS_THREADS_CHANNEL__HH
#include "lock.hh"
#include "condition.hh"


class Channel {
public:
    Channel();
    ~Channel();

    void Send(int message);
    void Receive(int *message);

private:
    bool buffer_busy;
    int buffer;
    Condition* sender_condition;
    Condition* receiver_condition;
    Lock *condition_lock;
};


#endif