#define PROCESSNUM 1000
#define RUNTIME 30

#include "pcb.h"
#include "processtable.h"
#include <queue>
#include <thread>
#include <stdlib.h>

using namespace std;

queue<PCB*> ready;      // All ready processes will be placed in this queue
queue<PCB*> pqueue[6];  // This will waste memory space for pqueue[0] as we will only be using 1-5

// Push a process onto the queue and set its state to ready.
void r(PCB* item){
    ready.push(item);
    item->state = PSTATE::READY;
}
/*
may need to pop or push x at different times, but this is just the general idea we have right now
void run_sim_multicore(thread cores[]) {
    while (queues are not empty) {
        for (i = 0; i < numCores; i++) {
            if (cores[i] is empty) {
                lock core[i]
                x = queue.pop()
                "run" process x on core[i] for x.priority * x.burstTime
                unlock core[i]
                queue.push(x)
            }
        }
    }
}
*/

/*
void run_sim_unicore(thread processor) {

}
*/


int main(){
    srand(12345);
    int timer = 0;  // Variable for adding later-spawned processes to our simulation.
    processtable PT;
    stack<int> pids; // Stack for assigning PIDs

    for(int i = PTSIZE; i >= 300; i--){ // Initialize stack with all potential PIDs. Shouldn't ever reach this, given our PROCESSNUM is lower than 
        pids.push(i);
    }

    // Create our processes, and add them to our process table.
    for(int i = 0; i < PROCESSNUM; i++){
        if(!pids.empty()){
            PT.add(new PCB(pids.top()));
            pids.pop();
        }else{
            printf("Unable to create new process. Insufficient memory.\n");
            break;
        }
    }

    // Push all arrival 0 items into the ready queue
    for(int i = 0; i < PROCESSNUM; i++){
        if(!PT[i]->state && PT[i]->arrival <= timer){
            r(PT[i]);
        }
    }

    // Create thread for ending the simulation. *1000 for seconds --> milliseconds
    thread endThread(_sleep, RUNTIME*1000);

    // Create threads for processing cores and make them do the thing
    int numCores = 5;
    thread processors[numCores];

    for (int i = 0; i < numCores; i++) {
        //initialize processors here
    }

    for (int i = 0; i < numCores; i++) {
        // start processor
        //processors[i] = thread(function,id);
    }

    for (int i = 0; i < numCores; i++) {
        // join to main process
        processors[i].join();
    }

    // "Slowly" iterate timer to simulate time passing, and have those later processes added to the queue
    while(timer++ < TIMERMAX){
        for(int i = 0; i < PROCESSNUM; i++){
            if(!PT[i]->state && PT[i]->arrival <= timer){
                r(PT[i]);
            }
        }
    }
}