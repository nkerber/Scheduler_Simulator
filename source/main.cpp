#define PROCESSNUM 1000
#define RUNTIME 30

#include "./pcb.cpp"
#include "./processtable.cpp"
#include <queue>
#include <thread>
#include <unistd.h>
#include <mutex>

using namespace std;

// Access enum using PSTATE::[NEW/READY/RUNNING/BLOCKED/EXIT]
enum PSTATE{NEW, READY, RUNNING, BLOCKED, EXIT};

int transferIn = 29; // Taken from PCB data structure, counting relevant register/memory that would need to be swapped into the processor for seamless running
int IO1 = 500; //
int IO2 = 400; //
int IO3 = 300; // All assumed to have differrent wait times based on priority, with highest priority having the lowest wait time.
int IO4 = 200; //
int IO5 = 100; //

queue<PCB*> ready;      // All ready processes will be placed in this queue
mutex rm;
queue<PCB*> pready[6];  // This will waste memory space for pqueue[0] as we will only be using 1-5
mutex prm[6];

processtable PT;

void FCFS_Exit(PCB *myProc){
    int exitVar = rand() % 2;
    if(exitVar == 0){
        // I/O
    }else{
        //push it back to the queue
    
}

// Unicore process algorithm 1
void run_sim_unicore_FCFS() {
    PCB* x;
    while(!ready.empty()){
        rm.lock();
        x = ready.front();
        ready.pop();
        
        rm.unlock();
    }
}

// Push a process onto the queue and set its state to ready.
void r(PCB* item){
    rm.lock();
    ready.push(item);
    item->state = PSTATE::READY;
    rm.unlock();
}

// Unicore processing algorithm 2
void run_sim_unicore_RR() {
    // while all queues are not empty
    // grab a process from the highest priority queue
    // 
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
    thread endThread(sleep, RUNTIME);

    // Create threads for processing cores and make them do the thing
    thread processors[numCores];

    // Initialize and start thread processing
    for(int i = 0; i < numCores; i++) {
        processors[i] = thread(run_sim_unicore_FCFS);
        // processors[i] = thread(run_sim_unicore_RR);
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
}