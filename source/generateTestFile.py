import random

numToGenerate = random.randrange(0,500)

with open("random_pids.csv",'w+') as f:
    arrival = 0 # arrival time of the process being generated, incremented after each process
    for proc in range(numToGenerate):
        f.write("PID "+str((proc+300))+", Arrival "+str(arrival)+", ") #write the process ID & arrival time to the process's line
        numBursts = random.randrange(1,10)
        for currentBurst in range(numBursts): #generate a random number of 
            if not (currentBurst % 2): # if the current burst is an odd number, it is a CPU burst
                f.write("CPU ")
            elif (currentBurst % 2): # if the current burst is an even number, it is an IO burst
                f.write("IO ") 
            f.write(str(random.randrange(1,15)))
            if currentBurst != numBursts-1:
                 f.write(", ")
        arrival+= random.randrange(2,5) #increment arrival time randomly by 2-5 milliseconds
        f.write("\n")

print("Generated new data file.")