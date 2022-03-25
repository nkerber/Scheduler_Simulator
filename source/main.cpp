#define PROCESSNUM 1000
#define RUNTIME 10
#define queueCount 5
#define NUMCORES 5
#define FCFS true
#define USEDATAFILE false

#include "./pcb.cpp"
#include "./processtable.cpp"
#include <queue>
#include <thread>
#include <unistd.h>
#include <mutex>
#include <iostream>
#include <fstream>

using namespace std;

// Access enum using PSTATE::[NEW/READY/RUNNING/BLOCKED/EXIT]
enum PSTATE{NEW, READY, RUNNING, BLOCKED, EXIT};

int transfer = 1; // Taken from PCB data structure, counting relevant register/memory that would need to be swapped into the processor for seamless running

int IOsims[5]= {10,8,6,4,2}; // All assumed to have differrent wait times based on to simulate slower IO devices.

queue<PCBFile*> ready;      // All ready processes will be placed in this queue
queue<PCBFile*> pready[queueCount+1];  // This will waste memory space for pqueue[0] as we will only be using 1-queueCount+1
mutex rm;
mutex prm[queueCount+1];

processtable PT;
bool done = false;
int timer = 0;

// Push a process onto the queue and set its state to ready.
void r(PCBFile* item){
    if(FCFS){
        rm.lock();
        ready.push(item);
        item->state = PSTATE::READY;
        rm.unlock();
    }else{
        prm[item->priority].lock();
        pready[item->priority].push(item);
        item->state = PSTATE::READY;
        prm[item->priority].unlock();
    }
}

PCBFile* getPCB(bool fcfs = true, int pq = 1){
    PCBFile* x;
    if(fcfs == true){
        rm.lock();
        x = ready.front();
        ready.pop();
        rm.unlock();
    }else{
        // lock queue
        prm[pq].lock();

        // pop next process off the queue
        x = pready[pq].front();
        pready[pq].pop();

        // Unlock queue
        prm[pq].unlock();
    }
    
    return x;
}

void FCFS_Exit(PCBFile *myProc){
    // Account for transfer time to any of our queues
    myProc->trans();
    
    // If process is sent to an I/O wait
    if(rand() % 2){
        myProc->state = PSTATE::BLOCKED;
        myProc->wait += IOsims[rand() % 5]; // 5 = Number of IO "queues" to simulate IO wait times
        myProc->trans();
    }
    
    // Push it back to the queue after IO or if no IO is necessary
    if(myProc->state != PSTATE::EXIT)
        r(myProc);
}

// 
void RR_Exit(PCBFile *myProc){
    myProc->trans();

    // Calculate where the next IO location would be based on however we would store our I/O locations
    if(myProc->CPUt->front() <= 0){  // If we have hit an IO location
        myProc->CPUt->erase(myProc->CPUt->begin());
        if(!myProc->IOt->empty()){
            myProc->state = PSTATE::BLOCKED;
            myProc->wait += myProc->IOt->front();
            myProc->trans();
        }
    }
    
    if(myProc->state != PSTATE::EXIT)
        r(myProc);
}

// Unicore process algorithm 1
void run_sim_unicore_FCFS() {
    PCBFile* x;
    while(!ready.empty() && !done){
        // Get the next available PCB
        x = getPCB();

        // Change state
        x->state = PSTATE::RUNNING;

        // Add transfer time to the trackers
        x->trans();

        // "Execute" the process. I.e. run the process for burst time * priority
        x->accu();
        
        // Remove PCB from processor
        FCFS_Exit(x);
    }
}

bool emptyQueues(){
    for(int i = 1; i< queueCount;i++){
        if(!pready[i].empty()){
            return false;
        }
    }
    return true;
}

// Unicore processing algorithm 2
void run_sim_unicore_RR() {

    // Round-Robin relies on interrupting current processes at designated/random time, so we will do the following to simulate interrupts:
    // PCB->accu(burst*priority - rand()%(burst*priority/2))

    int num_ran = 0; // Keep track of runs of current queue to make sure we aren't stuck on high priority forever
    int cqueue = (rand()%5) + 1; // Set the current queue to random priority
    PCBFile* tempPCB;
    while(!emptyQueues()){
        if(num_ran == cqueue) {
            num_ran = 0;
            if(cqueue > 1){
                cqueue--;
            }
            else{
                cqueue = queueCount;
            }
        }

        if(!pready[cqueue].empty()){

            tempPCB = getPCB(false, cqueue);
            tempPCB->state = PSTATE::RUNNING;
            // "run" process by time quantum x burst (increment accumulation)
            tempPCB->accu((tempPCB->burst * tempPCB->priority) - (rand() % (tempPCB->burst * tempPCB->priority/2)));
            
            // Send process to end of current queue
            RR_Exit(tempPCB);

            // Increment numran
            num_ran++;

        }
    }
}

// Sleep function to wait on setting the done variable to true, which pulls all executing threads back into the main thread
void ourSleep(int time){
    sleep(time);
    done = true;
}

// Calculate averages and output them to the terminal
void analyze(processtable PT){
    long avgAccum, prio1Accum, prio2Accum, prio3Accum, prio4Accum, prio5Accum = 0;
    int prio1count, prio2count, prio3count, prio4count, prio5count = 0;
    int avgArriv = 0;
    int avgPrio  = 0;

    for(int i = 0; i < PROCESSNUM; i++){
        avgAccum += PT[i]->accum;
        avgArriv += PT[i]->arrival;
        avgPrio  += PT[i]->priority;

        if(PT[i]->priority == 1){
            prio1Accum += PT[i]->accum;
            prio1count++;
        }else if(PT[i]->priority == 2){
            prio2Accum += PT[i]->accum;
            prio2count++;
        }else if(PT[i]->priority == 3){
            prio3Accum += PT[i]->accum;
            prio3count++;
        }else if(PT[i]->priority == 4){
            prio4Accum += PT[i]->accum;
            prio4count++;
        }else if(PT[i]->priority == 5){
            prio5Accum += PT[i]->accum;
            prio5count++;
        }
    }

    avgAccum /= 1000;
    avgArriv /= 1000;
    avgPrio /= 1000;

    cout << "Average Accumulation: " << avgAccum << endl;
    cout << "Average Arrival:      " << avgArriv << endl;
    cout << "Average Priority:     " << avgPrio << endl << endl;

    cout << "Priority 1 Accumulation Average: " << prio1Accum/prio1count << endl;
    cout << "Priority 2 Accumulation Average: " << prio2Accum/prio2count << endl;
    cout << "Priority 3 Accumulation Average: " << prio3Accum/prio3count << endl;
    cout << "Priority 4 Accumulation Average: " << prio4Accum/prio4count << endl;
    cout << "Priority 5 Accumulation Average: " << prio5Accum/prio5count << endl;
}

int main(){
    srand(12345);
    stack<int> pids; // Stack for assigning PIDs

    for(int i = PTSIZE; i >= 300; i--){ // Initialize stack with all potential PIDs. Shouldn't ever reach this, given our PROCESSNUM is lower than 
        pids.push(i);
    }

    // TODO: Create data structure for storing IO locations? 
    if(USEDATAFILE) {
        int pid;
        int arrivalTime;
        vector<int> cpuTimes;
        vector<int> ioTimes;
        fstream inputfile;
        inputfile.open("random_pids.csv");
        if (inputfile.is_open()) {
            string li;
            while (getline(inputfile, li,' ')) {
               cout << li << endl;
                

                
            }
        }

        PT.add(new PCBFile(pid, arrivalTime, &cpuTimes, &ioTimes));

    } else {
        // Create our processes, and add them to our process table.
        for(int i = 0; i < PROCESSNUM; i++){
            if(!pids.empty()){
                PT.add(new PCBFile(pids.top(), -1, nullptr, nullptr));
                pids.pop();
            }else{
                printf("Unable to create new process. Insufficient memory.\n");
                break;
            }
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
    thread processors[NUMCORES];
    
    // Initialize and start thread processing
    for(int i = 0; i < NUMCORES; i++) {
        if(FCFS){
            processors[i] = thread(run_sim_unicore_FCFS);
        }else{
            processors[i] = thread(run_sim_unicore_RR);
        }
        // processors[i] = thread(sleep, 5);
    }

    // "Slowly" iterate timer to simulate time passing, and have those later processes added to the queue later in the simulation
    while(timer++ < TIMERMAX){
        for(int i = 0; i < PROCESSNUM; i++){
            if(!PT[i]->state && PT[i]->arrival <= timer)
                r(PT[i]);
        }
    }

    // While our timer thread hasn't signalled the done boolean, increment timer to calculate throughput/turnaround times
    while(!done){
        ++timer;
    }
    
    // Join our threads
    endThread.join();
    for(int i = 0; i < NUMCORES; i++) {
        processors[i].join();
    }

    // Do analysis stuff via PCB information
    analyze(PT);

    return 0;
}