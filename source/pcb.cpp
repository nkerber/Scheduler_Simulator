#ifndef pcb
#define pcb

#define TIMERMAX 100

#include <stack>
#include <cstdlib>
#include <algorithm>
#include <iostream>

#define localstack stack<int>
#define decr 0
#define incr 1

class PCB{
    public:
        int pid; // 300-PTSIZE
        int ppid; // 0-299
        int uacct; // 0 - Admin, 1 - User, 2 - Guest
        int priority; // 1-5, higher number means higher priority
        int state; // 0 - New, 1 - Ready, 2 - Running, 3 - Blocked, 4 - Exit
        int burst; // 1-15 clock cycles (cc)
        int accum; // Total cc this process has been worked on (Transfer, wait, proccess, etc.)
        int pc; // Arbitrary, likely won't use but for completion's sake
        int cSwitch; // Int to act as 32/64 flags. Likely won't be used, but here for completion's sake.
        int wait; // Used to determine waiting queues and how long that would add to the accum. variable.
        int* vars; // Assume ints, for simplicity.
        bool incdec = incr; // Inc means indefinitely running, so count how many time quantums the program runs for. Dec means we have a set number of operations to run, so go until it's finished. This is taken care of in accum().
        
        int arrival; // Int to determine when to initially place a process into the ready queue.

        std::stack<int>* sys_stack = nullptr; // Likely won't be used. Here for completion's sake.

        PCB() : PCB(rand()%9700+300){}
        PCB(int id) : PCB(id, rand()%300){}
        PCB(int id, int ppid) : PCB(id, ppid, rand()%3){}
        PCB(int id, int ppid, int uacct) : PCB(id, ppid, uacct, rand()%5 + 1){}
        PCB(int id, int ppid, int uacct, int priority) : PCB(id, ppid, uacct, priority, 0){}
        PCB(int id, int ppid, int uacct, int priority, int state) : PCB(id, ppid, uacct, priority, state, rand()%15+1){}
        PCB(int id, int ppid, int uacct, int priority, int state, int burst) : PCB(id, ppid, uacct, priority, state, burst, 0){}
        PCB(int id, int ppid, int uacct, int priority, int state, int burst, int accum) : PCB(id, ppid, uacct, priority, state, burst, accum, 0){}
        PCB(int id, int ppid, int uacct, int priority, int state, int burst, int accum, int pc) : PCB(id, ppid, uacct, priority, state, burst, accum, pc, 0){}
        PCB(int id, int ppid, int uacct, int priority, int state, int burst, int accum, int pc, int cSwitch) : PCB(id, ppid, uacct, priority, state, burst, accum, pc, cSwitch, 0){}
        PCB(int id, int ppid, int uacct, int priority, int state, int burst, int accum, int pc, int cSwitch, int wait) : PCB(id, ppid, uacct, priority, state, burst, accum, pc, cSwitch, wait, nullptr){}
        PCB(int id, int ppid, int uacct, int priority, int state, int burst, int accum, int pc, int cSwitch, int wait, int* vars) : PCB(id, ppid, uacct, priority, state, burst, accum, pc, cSwitch, wait, vars, rand()%TIMERMAX){}
        // PCB(int id, int ppid, int uacct, int priority, int state, int burst, int accum, int pc, int cCode, int wait, int* vars, int arrival) : PCB(id, ppid, uacct, priority, state, burst, accum, pc, cCode, wait, vars, arrival, nullptr){}
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
            // sys_stack = sysstackIn;
        }

        void trans(){
            accum += 1; // Transfer time quantum
            cSwitch += 1; // Transfer time quantum
        }

        void accu(int value = 0){
            if(incdec == decr){
                if(accum <= 0){
                    // Remove this process from the table by signalling an exit state
                    state = 4;
                }else{
                    if(value != 0)
                        accum -= value;
                    else
                        accum -= burst * priority;
                }
            }else{
                if(value != 0)
                    accum += value;
                else
                    accum += burst * priority;
            }
        }
};

#endif

// Hunter's PID distribution system.

// #define MINPID 300
// #define MAXPID 10000

// #include <stack>
// #include <exception>
// #include <stdlib.h>
// #include <time.h>
// #include <unistd.h>
// #include <iostream>
// #include <thread>
// #include <mutex>
// #include <vector>
// #include <cstdlib>

// using namespace std;

// // Part 1

// int threadNum = 1000;
// mutex m;
// stack<int> pids;

// int initialize_map(void){
// 	try{	// Assuming we want to utilize the lowest pids first, we push to the stack in reverse.
// 		for(int i = MAXPID; i >= MINPID; i--){
// 			pids.push(i);
// 		}

// 		return 1;
// 	}catch(exception e){
// 		return -1;
// 	}
// }

// // Remove the pid from the stack so nobody else gets to use it
// int allocate_pid(void){
// 	int t = pids.top(); pids.pop();
// 	return t;
// }

// // To release, simply add it back to the stack
// void release_pid(int pid, int time){
// 	pids.push(pid);
// 	cout << "Releasing PID (" << time << "): " << pid << endl;
// }

// // Part 2

// // Code for threads to execute
// void execute(){
// 	int pid;
// 	{	// Grab the lock, and allocate the process ID
// 		lock_guard<mutex> g(m);
// 		pid = allocate_pid();
// 	}	// Unlock

// 	// Simulate thread operation by sleeping for a random time
// 	int t = rand()%10;
// 	sleep(t);

// 	{	// Grab the lock again and release the pid
// 		lock_guard<mutex> g(m);
// 		release_pid(pid, t);
// 	}	// Unlock
// }

// int main(){
// 	srand(time(NULL));
// 	vector<thread> threads;
// 	int pid = 1;

// 	// Initialize our pids
// 	if(initialize_map() == -1){
// 		cout << "Error occured in data structure initialization. Exiting." << endl;
// 		return -1;
// 	}

// 	// Spawn threadNum threads
// 	for(int i = 0; i < threadNum; i++){
// 		threads.push_back(thread(execute));
// 	}

// 	// Join the threads
// 	for(int i = 0; i < threadNum; i++){
// 		threads[i].join();
// 	}
// }
