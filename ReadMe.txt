# OS_Package

How to compile:
gcc -g -c bathroom.c -Wall -pthread
gcc -g -c bathroomSim.c -Wall -pthread
gcc bathroomSim.o bathroom.o -o bathroomSim -Wall -pthread -lm

