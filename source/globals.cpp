#ifndef globals
#define globals

#define RUNTIME 2
#define RANDPROCESSNUM 1000
#define queueCount 5
#define NUMCORES 5
#define USEDATAFILE true
#define FCFS true
#define TIMERMAX 100

// Access enum using PSTATE::[NEW/READY/RUNNING/BLOCKED/EXIT]
enum PSTATE{NEW, READY, RUNNING, BLOCKED, EXIT};

#endif