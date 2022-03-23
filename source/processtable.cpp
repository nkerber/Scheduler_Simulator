#ifndef PROCESSTABLE
#define PROCESSTABLE

#define PTSIZE 10000

#include "./pcb.cpp"

class processtable{
    PCB* processes[PTSIZE];

    public: 
        processtable(){
            for(int i = 0; i < PTSIZE; i++){
                processes[i] = nullptr;
            }
        }

        bool add(PCB* in){
            int t = hasSpace();
            if(t >= 0){
                processes[t] = in;
                return true;
            }

            return false;
        }

        bool remove(int i){
            if(processes[i] != nullptr){
                delete processes[i];
                processes[i] = nullptr;
                return true;
            }

            return false;
        }

        PCB* operator[](const int i){
            return processes[i];
        }

    private:
        int hasSpace(){
            for(int i = 0; i < PTSIZE; i++){
                if(processes[i] == nullptr){
                    return i;
                }
            }

            return -1;
        }
};

#endif