#include "./pcb.cpp"
#include "./processtable.cpp"
#include "./globals.cpp"
#include <queue>
#include <thread>
#include <unistd.h>
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
    if(item == nullptr){
        cout << "NULLPTR ERROR! DROPPING PROCESS." << endl;
        return;
    }

    if(item->state == PSTATE::NEW)
        item->rt = -timer;
    
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
    PCBFile* x = nullptr;

    // If we are using FCFS
    if(fcfs){
        // Lock, pop, unlock
        rm.lock();
        if(!ready.empty()){
            x = ready.front();
            ready.pop();
        }
        rm.unlock();
    }else{
        if(!pready[pq].empty()){
            // Lock, pop, unlock
            prm[pq].lock();
            x = pready[pq].front();
            pready[pq].pop();
            prm[pq].unlock();
        }
    }

    if(x != nullptr && x->rt < 0) x-> rt += timer;

    return x;
}

// Remove the given PCB from the processor
void proc_exit(PCBFile *myProc){
    // Add to transfer times
    myProc->trans();

    if(myProc->state != PSTATE::EXIT){
        // Check for IO location/proc
        myProc->io();
    }

    if(myProc->state != PSTATE::EXIT)
        r(myProc);
    else{
        if(!PT.remove(myProc)){
            cout << "Error" << endl;
        }
        myProc->closure = timer;
    }
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
            
            // Add transfer time to the trackers for entering the processor
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

                // "Run" process by time quantum x burst (increment accumulation) with variable cutoffs using rand().
                tempPCB->accu();
                
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
    vector<int>* cpuTimes = new vector<int>();
    vector<int>* ioTimes = new vector<int>();

    inputfile.open("random_pids.csv");
        if (inputfile.is_open()) {
            string li;
            while (inputfile >> li) {

                // If we are at the start of a new line (Process)
                if (li == "PID") {

                    // Check if we already have data that we need to add to our PT
                    if (!cpuTimes->empty()) {
                        PT.add(new PCBFile(pid, arrivalTime, cpuTimes, ioTimes));
                        cpuTimes->clear();
                        ioTimes->clear();
                    }

                    // Get the PID from the next word in the file, and store it in pid variable
                    inputfile >> li;
                    pid = stoi(li.substr(0,li.length()-1));

                // If we found the arrival keyword
                } else if (li == "Arrival") {
                    // Get the arrival time from the next word in the file, and store it in arrivalTime variable
                    inputfile >> li;
                    arrivalTime = stoi(li.substr(0,li.length()-1));

                // If we find a CPU burst time quantum header
                } else if (li == "CPU") {
                    // Get the CPU burst time from the next word in the file, and push it back to the CPU burst vector
                    inputfile >> li;
                    if (li.find(',') && li.length() != 1) {
                        cpuTimes->push_back(stoi(li.substr(0,li.length()-1)));
                    } else cpuTimes->push_back(stoi(li));
                    
                // If we find an IO burst time quantum header
                } else if (li == "IO") {
                    // Get the IO burst time from the next word in the file, and push it back to the IO burst vector
                    inputfile >> li;
                    if (li.find(',') && li.length() != 1) {
                        ioTimes->push_back(stoi(li.substr(0,li.length()-1)));
                    } else ioTimes->push_back(stoi(li));
                
                // Otherwise, you screwed up. My generator is perfect - Nate
                } else {
                    throw invalid_argument("Fatal error in file import!");
                }
            }

            // Push the last set of process variables to the process table
            if (!cpuTimes->empty()) {
                PT.add(new PCBFile(pid, arrivalTime, cpuTimes, ioTimes));
            }
        }

    inputfile.close();
}

// Analyze the data taken from a data file
// Analyzes the following:
/*
* Average turnaround    --
* Average wait          
* Average responsetime  
* Average throughput    --
*/
void aDataFile(processtable& PT,float& totTP, float& totWait, float& totRT, float& totTA, float& totCS){
    int processes = PT.oldSize() + PT.size(), maxClosure = 0;
    long prioCount[queueCount+1];
    long avgArriv = 0, avgPrio = 0;
    float throughPut = 0;
    long avgWait = 0, avgRT = 0, avgtat = 0, avgTrans = 0;

    //(FCFS) ? printf("First-Come-First-Serve Algorithm Results:\n"): printf("Round-Robin Algorithm Results:\n");
    //printf("Using local file data!\n Completed Processes: %d\n",PT.oldSize());

    // Initialize counters to 0
    for(int i = 0; i < queueCount+1; i++){
        prioCount[i] = 0;
    }

    // For all completed processes
    for(int i = 0; i < PT.oldSize(); i++){
        // Turn around time data collection
        avgtat += (PT.getOld(i)->closure - PT.getOld(i)->arrival);

        // Throughput data collection
        if(PT.getOld(i)->closure > maxClosure){
            maxClosure = PT.getOld(i)->closure;
        }
    }
    throughPut = static_cast<float>(PT.oldSize()) / static_cast<float>(maxClosure);

    // For all processes
    for(int i = 0; i < processes; i++){
        if(PT[i] != nullptr){
            // Add to global average variables
            avgArriv += PT[i]->arrival;
            // Add to priority average variables
            ++prioCount[PT[i]->priority];
            // Add to average weight
            avgWait += PT[i]->wait;
            // Add to response time
            avgRT += PT[i]->rt;
            // Add to transfer time
            avgTrans += PT[i]->cSwitch;

            if(PT[i]->CPUt->empty() && PT[i]->IOt->size()){
                cout << "Error" << endl;
            }
        }
        if(PT.getOld(i) != nullptr){
            // Add to global average variables
            avgArriv += PT.getOld(i)->arrival;
            // Add to priority average variables
            ++prioCount[PT.getOld(i)->priority];
            // Add to average weight
            avgWait += PT.getOld(i)->wait;
            // Add to response time
            avgRT += PT.getOld(i)->rt;
            // Add to transfer time
            avgTrans += PT.getOld(i)->cSwitch;
        }
    }
    
    totTP += throughPut;
    totWait += static_cast<float>(avgWait)/static_cast<float>(processes);
    totRT += static_cast<float>(avgRT)/static_cast<float>(processes);
    totTA += static_cast<float>(avgtat)/static_cast<float>(processes);
    totCS += static_cast<float>(avgTrans)/static_cast<float>(processes);

    
    // cout << "Processes:                 " << processes << endl;
    // cout << "Simulated Throughput:      " << throughPut << endl;
    // cout << "Average Arrival:           " << static_cast<int>(static_cast<float>(avgArriv)/static_cast<float>(processes)) << endl;
    // cout << "Average Response Time:     " << static_cast<float>(avgRT)/static_cast<float>(processes) << endl;
    // cout << "Average Wait Time:         " << static_cast<float>(avgWait)/static_cast<float>(processes) << endl;
    //cout << "Average Turnaround time:   " << static_cast<float>(avgtat)/static_cast<float>(processes) << endl;
    // cout << "Average Context Swap time: " << static_cast<float>(avgTrans)/static_cast<float>(processes) << endl;
    

}

// Analyze our random, infinite process system
void aNoDataFile(processtable& PT){
    long prioAccum[queueCount+1];
    long prioCount[queueCount+1];
    long avgAccum, avgArriv, avgPrio = 0;
    int processes = PT.size() + PT.oldSize();

    // Initialize counters to 0
    for(int i = 0; i < queueCount+1; i++){
        prioCount[i] = 0;
        prioAccum[i] = 0;
    }

    for(int i = 0; i < processes; i++){
        // Add to global average variables
        avgAccum += PT[i]->accum;
        avgArriv += PT[i]->arrival;
        avgPrio  += PT[i]->priority;

        // Add to priority average variables
        prioAccum[PT[i]->priority] += PT[i]->accum;
        prioCount[PT[i]->priority]++;
    }

    cout << "Processes: " << processes << endl;

    cout << "Average Accumulation: " << static_cast<float>(avgAccum)/static_cast<float>(processes) << endl;
    cout << "Average Arrival:      " << static_cast<float>(avgArriv)/static_cast<float>(processes) << endl;
    cout << "Average Priority:     " << static_cast<float>(avgPrio)/static_cast<float>(processes) << endl;

    cout << "Averages based on priorities:" << endl;
    for(int i = 1; i < queueCount+1; i++){
        cout << "Priority " << i << " Accumulation Average: " << prioAccum[i]/prioCount[i] << endl;
    }
}

// Calculate averages and output them to the terminal
void analyze(processtable& PT, float& totTP, float& totWait, float& totRT, float& totTA, float& totCS){
    if(USEDATAFILE){
        aDataFile(PT,totTP,totWait,totRT,totTA,totCS);
    }else{
        aNoDataFile(PT);
    }
}

int main(){
    float totTP = 0, totWait = 0, totRT = 0, totTA = 0, totCS = 0;
    for (int i = 0; i < 10; i++) {
    srand(12345);
    stack<int> pids; // Stack for assigning PIDs
    int processCount = RANDPROCESSNUM;
    vector<PCBFile*> temp;

    for(int i = PTSIZE; i >= 300; i--){ // Initialize stack with all potential PIDs. Shouldn't ever reach this, given our RANDPROCESSNUM is lower than 
        pids.push(i);
    }

    if(USEDATAFILE){
        // Use the process data provided by random_pids.csv to add processes to our process table,
        // and quit if something goes wrong with the import.
        try{
            importData();
        }catch(invalid_argument &e){
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

    for(int i = 0; i < PT.size(); i++){
        temp.push_back(PT[i]);
    }
    
    vector<PCBFile*>::iterator p = temp.begin();
    // Push all arrival 0 items into the ready queue
    for(int i = 0; i < processCount; i++){
        if(!temp[i]->state && temp[i]->arrival <= timer){
            r(temp[i]); 
            swap(temp[i], temp[temp.size()-1]);
            temp.pop_back();
            i--;
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
        for(int j = 0; j < temp.size(); j++){
            if(temp[j] != nullptr && !temp[j]->state && temp[j]->arrival <= timer){
                r(temp[j]);
                swap(temp[j], temp[temp.size()-1]);
                temp.pop_back();
                j--;
            }
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
    analyze(PT,totTP,totWait,totRT,totTA,totCS);
    timer = 0;
    }

    cout << "Simulated Throughput:      " << totTP/10 << endl;
    cout << "Average Response Time:     " << totRT/10 << endl;
    cout << "Average Wait Time:         " << totWait/10 << endl;
    cout << "Average Turnaround time:   " << totTA/10 << endl;
    cout << "Average Context Swap time: " << totCS/10 << endl;
    

    return 0;
}