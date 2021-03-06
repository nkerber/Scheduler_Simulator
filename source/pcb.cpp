#ifndef pcb
#define pcb

#include <stack>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <vector>
#include <mutex>
#include "./globals.cpp"

#define decr 1
#define incr 0

const int IOsims[5]= {10,8,6,4,2}; // All assumed to have differrent wait times based on to simulate slower IO devices.

class PCB{
    public:
        int pid; // 300-PTSIZE
        int ppid; // 0-299
        int uacct; // 0 - Admin, 1 - User, 2 - Guest
        int priority; // 1-5, higher number means higher priority
        int state; // 0 - New, 1 - Ready, 2 - Running, 3 - Blocked, 4 - Exit
        int burst; // 1-15 clock cycles (cc)
        int pc; // Arbitrary, likely won't use but for completion's sake
        int* vars; // Assume ints, for simplicity.
        bool incdec = incr; // Inc means indefinitely running, so count how many time quantums the program runs for. Dec means we have a set number of operations to run, so go until it's finished. This is taken care of in accum().
        
        // Analysis variables
        int accum; // Total cc this process has been worked on (Transfer, wait, proccess, etc.)
        int arrival; // Int to determine when to initially place a process into the ready queue.
        int closure; // Int to track at what time the process completes.
        int cSwitch; // Used to keep track of how many time quantums each process is being swapped in/out.
        int wait; // Used to determine waiting queues and how long that would add to the accum. variable.
        int rt = 0;

        std::stack<int>* sys_stack = nullptr; // Likely won't be used. Here for completion's sake.

        // Constructor stacks -- Ignore these for the most part. Shouldn't ever instantiate a PCB, using PCBFile instead.
        PCB() : PCB(rand()%9700+300){}
        PCB(int id) : PCB(id, rand()%300){}
        PCB(int id, int ppid) : PCB(id, ppid, rand()%3){}
        PCB(int id, int ppid, int uacct) : PCB(id, ppid, uacct, rand()%queueCount + 1){}
        PCB(int id, int ppid, int uacct, int priority) : PCB(id, ppid, uacct, priority, PSTATE::NEW){}
        PCB(int id, int ppid, int uacct, int priority, int state) : PCB(id, ppid, uacct, priority, state, rand()%15+1){}
        PCB(int id, int ppid, int uacct, int priority, int state, int burst) : PCB(id, ppid, uacct, priority, state, burst, 0){}
        PCB(int id, int ppid, int uacct, int priority, int state, int burst, int accum) : PCB(id, ppid, uacct, priority, state, burst, accum, 0){}
        PCB(int id, int ppid, int uacct, int priority, int state, int burst, int accum, int pc) : PCB(id, ppid, uacct, priority, state, burst, accum, pc, 0){}
        PCB(int id, int ppid, int uacct, int priority, int state, int burst, int accum, int pc, int cSwitch) : PCB(id, ppid, uacct, priority, state, burst, accum, pc, cSwitch, 0){}
        PCB(int id, int ppid, int uacct, int priority, int state, int burst, int accum, int pc, int cSwitch, int wait) : PCB(id, ppid, uacct, priority, state, burst, accum, pc, cSwitch, wait, nullptr){}
        PCB(int id, int ppid, int uacct, int priority, int state, int burst, int accum, int pc, int cSwitch, int wait, int* vars) : PCB(id, ppid, uacct, priority, state, burst, accum, pc, cSwitch, wait, vars, rand()%TIMERMAX){}
        PCB(int idIn, int ppidIn, int uacctIn, int priorityIn, int stateIn, int burstIn, int accumIn, int pcIn, int cSwitchIn, int waitIn, int* varsIn, int arrivalIn){//, localstack* sysstackIn){
            pid = idIn;
            ppid = ppidIn;
            uacct = uacctIn;
            priority = priorityIn;
            state = stateIn;
            burst = burstIn;
            if(accumIn > 0)
                incdec = decr;
            accum = accumIn;
            pc = pcIn;
            cSwitch = cSwitchIn;
            wait = waitIn;
            vars = varsIn;
            arrival = arrivalIn;
            closure = 0;
        }
};

class PCBFile : public PCB {
    private:
        std::mutex m;
    public:
        std::vector<int>* CPUt;
        std::vector<int>* IOt;

        PCBFile(int pid, int arr, std::vector<int> *cputimes, std::vector<int> *iotimes) : PCB(pid) {
            this->pid = pid;
            if(arr != -1)
                this->arrival = arr;
            this->CPUt = cputimes;
            this->IOt = iotimes;

            if(CPUt != nullptr && !CPUt->empty()){
                incdec = decr;
            }
        }
        
        void trans(){
            if(incdec == decr && CPUt != nullptr && CPUt->empty() && IOt != nullptr && IOt->empty()){  // If we have no more to do
                // Remove this process from the table by signalling an exit state
                state = PSTATE::EXIT;
                cSwitch += TIMEQUANTUM; // Transfer time quantum
            }else{
                // std::cout << (CPUt->empty()) << std::endl;
                accum += TIMEQUANTUM; // Transfer time quantum
                cSwitch += TIMEQUANTUM; // Transfer time quantum
            }
        }

        void accu(int value = 0){
            if(incdec == decr){ // If we are using file IO
                if(CPUt != nullptr && CPUt->empty() && IOt != nullptr && IOt->empty()){  // If we have no more to do
                    // Remove this process from the table by signalling an exit state
                    state = 4;
                    cSwitch += 1; // Transfer time quantum
                }else{
                    // Values to decrement by
                    int t[2] = {0,0};

                    // FCFS value
                    t[0] = burst * priority;
                    
                    // RR value
                    // Protect against divide by 0 because 1/2 = 0 is dumb
                    if(priority/2 != 0)
                        t[1] = (burst * priority) - rand() % (burst * (priority/2)); 
                    else
                        t[1] = (burst * priority) - rand() % (burst*priority);

                    // m.lock();
                    // If we have CPU stuff to decrement
                    if(CPUt != nullptr && !CPUt->empty()){
                        // Decrement it, using the correct algorithm
                        (FCFS) ? CPUt->at(0) -= t[0] : CPUt->at(0) -= t[1];
                        //                      ^FCFS                  ^RR
                    }
                    // m.unlock();
                }
            }else{  // If we are not using file IO and this is an indefinite process
                if(value != 0)
                    accum += value;
                else
                    accum += burst * priority;
            }
        }

        void io(){
            if(incdec == incr){
                // If process is sent to an I/O wait
                if(rand() % 2){
                    int io = rand()%5;
                    state = PSTATE::BLOCKED;
                    wait += IOsims[io]; // 5 = Number of IO "queues" to simulate IO wait times
                    accum += IOsims[io];
                    trans();
                }
            }else{
                // Calculate where the next IO location would be based on however we would store our I/O locations
                if(CPUt != nullptr && !CPUt->empty() && CPUt->front() <= 0){  // If we have hit an IO location
                    CPUt->erase(CPUt->begin());
                    if(IOt != nullptr && !IOt->empty()){
                        state = PSTATE::BLOCKED;
                        wait += IOt->front();
                        IOt->erase(IOt->begin());
                        trans();
                    }
                }
            }
        }
};

#endif