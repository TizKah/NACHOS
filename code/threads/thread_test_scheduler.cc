

#include <stdio.h>
#include "system.hh"
#include "thread_test_scheduler.hh"

const int LOW_PRIO = 1;
const int MED_PRIO = 5; 
const int HIGH_PRIO = 8;



void SimplePriorityLoop(void *arg) {
    int priority = currentThread->GetPriority(); 

    for (int i = 0; i < 5; i++) {
        printf("Hilo '%s' (Prioridad %d) ejecutando paso %d\n",
               currentThread->GetName(), priority, i);

        
        for(int j=0; j<1000; j++) ; 
        currentThread->Yield();
    }
    printf("**** Hilo '%s' (Prioridad %d) TERMINADO ****\n", currentThread->GetName(), priority);
}

void PrioritySchedulerTest() {
    printf("Iniciando Prueba del Planificador por Prioridad...\n");

    
    
    Thread *t_low = new Thread("BajaPrio", LOW_PRIO, true);
    Thread *t_med = new Thread("MediaPrio", MED_PRIO, true);
    Thread *t_high = new Thread("AltaPrio", HIGH_PRIO, true);
    Thread *t_med2 = new Thread("MediaPrio2", MED_PRIO, true); 
    Thread *t_high2 = new Thread("AltaPrio2", HIGH_PRIO, true); 

    
    
    t_low->Fork(SimplePriorityLoop, (void *)1);
    t_med->Fork(SimplePriorityLoop, (void *)2);
    t_high->Fork(SimplePriorityLoop, (void *)3);
    t_med2->Fork(SimplePriorityLoop, (void *)4);
    t_high2->Fork(SimplePriorityLoop, (void *)5);


    printf("Hilos creados y listos para ejecutar.\n");
    
    
    printf("Hilo Principal cediendo la CPU...\n");
    for(int k=0; k < 20; k++) { 
         currentThread->Yield();
    }


    
    printf("Hilo principal esperando con Join...\n");
    t_high->Join();
    t_high2->Join();
    t_med->Join();
    t_med2->Join();
    t_low->Join();

    printf("Prueba del Planificador por Prioridad Terminada (o el hilo principal sigue).\n");
    
    
}