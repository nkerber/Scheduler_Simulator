#define PROCESSNUM 1000
#define RUNTIME 30

#include "./pcb.cpp"
#include "./processtable.cpp"
#include <queue>
#include <thread>
#include <unistd.h>
#include <mutex>
#include <iostream>

using namespace std;

// Access enum using PSTATE::[NEW/READY/RUNNING/BLOCKED/EXIT]
enum PSTATE{NEW, READY, RUNNING, BLOCKED, EXIT};

int transfer = 60; // Taken from PCB data structure, counting relevant register/memory that would need to be swapped into the processor for seamless running
int IO1 = 1000; //
int IO2 = 800;  //
int IO3 = 600;  // All assumed to have differrent wait times based on to simulate slower IO devices.
int IO4 = 400;  //
int IO5 = 200;  //

queue<PCB*> ready;      // All ready processes will be placed in this queue
queue<PCB*> pready[6];  // This will waste memory space for pqueue[0] as we will only be using 1-5
mutex rm;
mutex prm[6];

processtable PT;
bool done = false;

// Push a process onto the queue and set its state to ready.
void r(PCB* item){
    rm.lock();
    ready.push(item);
    item->state = PSTATE::READY;
    rm.unlock();
}

void FCFS_Exit(PCB *myProc){
    // Account for transfer time to any of our queues
    myProc->accum += transfer;
    
    if(rand() % 2){
        // Process is sent to an I/O wait
        myProc->state = PSTATE::BLOCKED;
        int ioVar = rand() % 5;
        switch(ioVar){
            case 0:{
                myProc->accum += IO1;
                myProc->accum += transfer;
                break;
            }
            case 1:{
                myProc->accum += IO2;
                myProc->accum += transfer;
                break;
            }
            case 2:{
                myProc->accum += IO3;
                myProc->accum += transfer;
                break;
            }
            case 3:{
                myProc->accum += IO4;
                myProc->accum += transfer;
                break;
            }
            case 4:{
                myProc->accum += IO5;
                myProc->accum += transfer;
                break;
            }
            default:{
                break;
            }

        }
    }
    
    // Push it back to the queue after IO or if no IO is necessary
    r(myProc);
}

// Unicore process algorithm 1
void run_sim_unicore_FCFS() {
    PCB* x;
    while(!ready.empty() && !done){
        // Critical to grab the lock
        rm.lock();
        x = ready.front();
        ready.pop();
        rm.unlock();
        // End critical

        // Change state
        x->state = PSTATE::RUNNING;

        // Add transfer-in time to the tracker
        x->accum += transfer;

        // "Execute" the process. I.e. run the process for burst time * priority
        x->accum += x->burst*x->priority;
        
        // Add transfer-out time to the tracker
        FCFS_Exit(x);
    }
}

// Unicore processing algorithm 2
void run_sim_unicore_RR() {

    // Round-Robin relies on interrupting current processes at designated/random time, so we will need to do the following:
    // PCB->accum -= rand()%(burst*priority/2)

    // int num_ran = 0; // keep track of runs of current queue to make sure we aren't stuck on high priority forever
    // *queue cqueue = highest priority queue; // set the current queue to the highest priority
    // while all queues are not empty {
    //   if current queue is not empty && num_ran < cqueue->num {
    //     lock queue
    //     pop next process off the queue
    //     unlock queue
    //     "run" process by time quantum x burst (increment accumulation)
    //     send process to end of current queue
    //     increment numran
    //   else if (currentQueue is empty) or (num_ran = cqueue-> num) {
    //     num_ran = 0
    //     *cqueue = next lowest priority queue
    //  
}

// Sleep function to wait on setting the done variable to true, which pulls all executing threads back into the main thread
void ourSleep(int time){
    sleep(time);
    done = true;
}

int main(){
    srand(12345);
    int timer = 0;  // Variable for adding later-spawned processes to our simulation.
    int numCores = 5;
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

    // Create thread for ending the simulation, making it sleep for a designated time.
    thread endThread(ourSleep, RUNTIME);

    // Create threads for processing cores and make them do the thing
    thread processors[numCores];
    
    // Initialize and start thread processing
    for(int i = 0; i < numCores; i++) {
        processors[i] = thread(run_sim_unicore_FCFS);
        // processors[i] = thread(run_sim_unicore_RR);
        // processors[i] = thread(sleep, 5);
    }

    // "Slowly" iterate timer to simulate time passing, and have those later processes added to the queue later in the simulation
    while(timer++ < TIMERMAX){
        for(int i = 0; i < PROCESSNUM; i++){
            if(!PT[i]->state && PT[i]->arrival <= timer)
                r(PT[i]);
        }
    }

    // Block this main thread until the sleeping thread is finished
    endThread.join();

    // Join our processor threads to the main thread to close things out
    for(int i = 0; i < numCores; i++) {
        processors[i].join();
    }

    // Do analysis stuff via PCB information
    for(int i = 0; i < PROCESSNUM; i++){
        cout << i << "| Accum: " << PT[i]->accum << " Arrival: " << PT[i]->arrival << " Priority: " << PT[i]->priority << endl;
    }

    return 0;
}