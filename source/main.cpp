#include "./pcb.cpp"
#include "./processtable.cpp"
#include "./globals.cpp"
#include <queue>
#include <thread>
#include <unistd.h>
#include <mutex>
#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;

int transfer = 1; // Taken from PCB data structure, counting relevant register/memory that would need to be swapped into the processor for seamless running

queue<PCBFile*> ready;                  // All ready processes will be placed in these queues
queue<PCBFile*> pready[queueCount+1];   // This will waste memory space for pqueue[0] as we will only be using 1-queueCount+1
mutex rm;
mutex prm[queueCount+1];

processtable PT;
bool done = false;
int timer = 0;

// Lock the correct queue, push the given process onto the queue and set its state to ready.
void r(PCBFile* item){
    if(FCFS){
        rm.lock();
        ready.push(item);
        item->state = PSTATE::READY;
        rm.unlock();
    }else{
        if(item == nullptr){
            cout << "NULLPTR ERROR! DROPPING PROCESS." << endl;
            return;
        }
        prm[item->priority].lock();
        pready[item->priority].push(item);
        item->state = PSTATE::READY;
        prm[item->priority].unlock();
    }
}

// Check to see if we have processes in the priority queues
bool emptyQueues(){
    for(int i = 1; i< queueCount;i++){
        if(!pready[i].empty()){
            return false;
        }
    }
    return true;
}

// Return the newest PCB for whichever algorithm we are using ()
PCBFile* getPCB(bool fcfs = true, int pq = 1){
    PCBFile* x;

    // If we are using FCFS
    if(fcfs){
        if(!ready.empty()){
            // Lock, pop, unlock
            rm.lock();
            x = ready.front();
            ready.pop();
            rm.unlock();
        }else{
            return nullptr;
        }
    }else{
        if(!pready[pq].empty()){
            // Lock, pop, unlock
            prm[pq].lock();
            try{
                x = pready[pq].front();
                pready[pq].pop();
            }catch(exception e){
                return nullptr;
            }
            prm[pq].unlock();
        }else{
            return nullptr;
        }
    }
    
    return x;
}

// Remove the given PCB from the processor
void proc_exit(PCBFile *myProc){
    // Add to transfer times
    myProc->trans();

    // Check for IO location/proc
    myProc->io();
    
    if(myProc->state != PSTATE::EXIT)
        r(myProc);
    else
        myProc->closure = timer;
}

// Unicore process algorithm 1
void run_sim_unicore_FCFS() {
    PCBFile* x;
    while(!done){
        // Get the next available PCB
        x = getPCB();

        if(x != nullptr){
            // Change state
            x->state = PSTATE::RUNNING;

            // Add transfer time to the trackers
            x->trans();

            // "Execute" the process. I.e. run the process for burst time * priority
            x->accu();

            // Remove PCB from processor
            proc_exit(x);
        }
    }
}

// Unicore processing algorithm 2
void run_sim_unicore_RR() {

    // Round-Robin relies on interrupting current processes at designated/random time, so we will do the following to simulate interrupts:
    // PCB->accu(burst*priority - rand()%(burst*priority/2))

    int num_ran = 0; // Keep track of runs of current queue to make sure we aren't stuck on high priority forever
    int cqueue = (rand()%5) + 1; // Set the current queue to random priority
    PCBFile* tempPCB;
    int count = 0;
    while(!done && !emptyQueues()){
        
        // Determine which queue we are pulling from
        if(num_ran == cqueue) {
            num_ran = 0;
            if(cqueue > 1){
                --cqueue;
            }else{
                cqueue = queueCount;
            }
        }

        // If that queue is not empty, pull a new PCB to process
        if(!pready[cqueue].empty()){

            tempPCB = getPCB(false, cqueue);
            if(tempPCB != nullptr){
                tempPCB->state = PSTATE::RUNNING;

                if(tempPCB->burst == 0 || tempPCB->priority == 0){
                    cout << "Burst: " << tempPCB->burst;
                    cout << "Prio: " << tempPCB->priority;
                }

                // Protect against divide by 0 because 1/2 = 0 is dumb
                if(tempPCB->priority/2 != 0)
                    count = (tempPCB->burst * tempPCB->priority) - rand() % (tempPCB->burst * (tempPCB->priority/2)); 
                else{
                    count = (tempPCB->burst * tempPCB->priority) - rand() % (tempPCB->burst*tempPCB->priority);
                }
                // "Run" process by time quantum x burst (increment accumulation) with variable cutoffs using rand().
                tempPCB->accu(count);
                
                // Send process to end of current queue
                proc_exit(tempPCB);
            }
        }

        // Increment numran whether we executed a process or not.
        num_ran++;
    }
}

// Sleep function to wait on setting the done variable to true, which pulls all executing threads back into the main thread
void ourSleep(int time){
    sleep(time);
    done = true;
}

// Take input from random_pids.csv & add the processes to the process table
void importData(){
    fstream inputfile;

    int arrivalTime;
    int pid;
    vector<int> cpuTimes;
    vector<int> ioTimes;

    inputfile.open("random_pids.csv");
        if (inputfile.is_open()) {
            string li;
            while (inputfile >> li) {

                // If we are at the start of a new line (Process)
                if (li == "PID") {

                    // Check if we already have data that we need to add to our PT
                    if (!cpuTimes.empty()) {
                        PT.add(new PCBFile(pid, arrivalTime, new vector<int>(cpuTimes), new vector<int>(ioTimes)));
                        cpuTimes.clear();
                        ioTimes.clear();
                    }

                    // Get the PID from the next word in the file, and store it in pid variable
                    inputfile >> li;
                    pid = stoi(li.substr(0,li.length()-1));

                // If we found the arrival keyword
                } else if (li == "Arrival") {
                    // Get the arrival time from the next word in the file, and store it in arrivalTime variable
                    inputfile >> li;
                    cout << "Arrival is: " << li.substr(0,li.length()-1) << endl;
                    arrivalTime = stoi(li.substr(0,li.length()-1));

                // If we find a CPU burst time quantum header
                } else if (li == "CPU") {
                    // Get the CPU burst time from the next word in the file, and push it back to the CPU burst vector
                    inputfile >> li;
                    if (li.find(',') && li.length() != 1) {
                        cpuTimes.push_back(stoi(li.substr(0,li.length()-1)));
                    } else cpuTimes.push_back(stoi(li));
                    
                // If we find an IO burst time quantum header
                } else if (li == "IO") {
                    // Get the IO burst time from the next word in the file, and push it back to the IO burst vector
                    inputfile >> li;
                    if (li.find(',') && li.length() != 1) {
                        ioTimes.push_back(stoi(li.substr(0,li.length()-1)));
                    } else ioTimes.push_back(stoi(li));
                
                // Otherwise, you screwed up. My generator is perfect - Nate
                } else {
                    throw invalid_argument("Fatal error in file import!");
                }
            }

            // Push the last set of process variables to the process table
            if (!cpuTimes.empty()) {
                PT.add(new PCBFile(pid, arrivalTime, &cpuTimes, &ioTimes));
            }
        }

    inputfile.close();
}

// Analyze the data taken from a data file
void aDataFile(processtable& PT){
    long prioAccum[queueCount+1];
    long prioCount[queueCount+1];
    long avgAccum, avgArriv, avgPrio = 0;
    int processes = PT.oldSize();

    // TODO: FINALIZE ANALYSIS METHODS AND IMPLEMENT
    cout << "TODO" << endl;
}

// Analyze our random, infinite process system
void aNoDataFile(processtable& PT){
    long prioAccum[queueCount+1];
    long prioCount[queueCount+1];
    long avgAccum, avgArriv, avgPrio = 0;
    int processes = PT.size();

    // Initialize counters to 0
    for(int i = 0; i < queueCount+1; i++){
        prioCount[i] = 0;
        prioAccum[i] = 0;
    }

    for(int i = 0; i < processes; i++){
        // cout << "Prio: " << PT[i]->priority << endl;
        // cout << prioAccum[PT[i]->priority] << endl;
        // cout << prioCount[PT[i]->priority] << endl;

        // Add to global average variables
        avgAccum += PT[i]->accum;
        avgArriv += PT[i]->arrival;
        avgPrio  += PT[i]->priority;

        // Add to priority average variables
        // cout << "+= " << PT[i]->accum << endl;
        prioAccum[PT[i]->priority] += PT[i]->accum;
        prioCount[PT[i]->priority]++;
        // cout << prioAccum[PT[i]->priority] << endl;
        // cout << prioCount[PT[i]->priority] << endl;
        // cout << "---------------" << endl;
    }

    cout << "Processes: " << processes << endl;

    cout << "Average Accumulation: " << avgAccum/processes << endl;
    cout << "Average Arrival:      " << avgArriv/processes << endl;
    cout << "Average Priority:     " << avgPrio/processes << endl;

    cout << "Averages based on priorities:" << endl;
    for(int i = 1; i < queueCount+1; i++){
        cout << "Priority " << i << " Accumulation Average: " << prioAccum[i]/prioCount[i] << endl;
    }
}

// Calculate averages and output them to the terminal
void analyze(processtable& PT){
    if(USEDATAFILE){
        aDataFile(PT);
    }else{
        aNoDataFile(PT);
    }
}

int main(){
    srand(12345);
    stack<int> pids; // Stack for assigning PIDs
    int processCount = RANDPROCESSNUM;

    for(int i = PTSIZE; i >= 300; i--){ // Initialize stack with all potential PIDs. Shouldn't ever reach this, given our RANDPROCESSNUM is lower than 
        pids.push(i);
    }

    if (USEDATAFILE) {
        // Use the process data provided by random_pids.csv to add processes to our process table,
        // and quit if something goes wrong with the import.
        try {
            importData();
        } catch (invalid_argument &e) {
            cerr << e.what() << endl;
            return -1;
        }

        processCount = PT.size();
    } else {
        // Randomly generate processes, and add them to our process table.
        for(int i = 0; i < RANDPROCESSNUM; i++){
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
    for(int i = 0; i < processCount; i++){
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
        for(int j = 0; j < processCount; j++){
            if(!PT[j]->state && PT[j]->arrival <= timer)
                r(PT[j]);
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