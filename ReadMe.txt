# OS_Package

How to compile:
gcc -g -c bathroom.c -Wall -pthread
gcc -g -c bathroomSim.c -Wall -pthread
gcc bathroomSim.o bathroom.o -o bathroomSim -Wall -pthread -lm

How to run:
	In this directory:
		make
		./bathroomSim nUsers meanLoopCount meanArrival meanStay

		where no argument is negative or over 1000 or not a number

