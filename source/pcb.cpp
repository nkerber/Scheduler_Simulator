#ifndef pcb
#define pcb

#define TIMERMAX 1000

#include <stack>
#include <cstdlib>
#include <algorithm>
#include "processtable.cpp"

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
        int cCode; // Int to act as 32/64 flags. Likely won't be used, but here for completion's sake.
        int wait; // Used to determine waiting queues and how long that would add to the accum. variable.
        int vars[20]; // Assume ints, for simplicity.
        
        int arrival; // Int to determine when to initially place a process into the ready queue.

        std::stack<int>* sys_stack; // Likely won't be used. Here for completion's sake.

        PCB() : PCB(rand()%9700+300){}

        PCB(int id){
            pid = id;
            ppid = rand()%300;
            uacct = rand()%3;
            priority = rand()%5 + 1;
            state = 0;
            burst = rand()%15 + 1;
            accum = 0;
            pc = 0;
            cCode = 0;
            wait = 0;
            std::fill_n(vars, 20, 0);
            arrival = rand()%TIMERMAX;
            sys_stack = nullptr;
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
