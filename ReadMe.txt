# OS_Package

How to compile:
gcc -g -c bathroom.c -Wall -pthread
gcc -g -c Sim_queue.c -Wall -pthread
gcc Sim_queue.o bathroom.o -o Sim_queue -Wall -pthread -lm

How to run:
	In this directory:
		make
		./Sim_queue nUsers meanLoopCount meanArrival meanStay

		where no argument is negative or over 1000 or not a number

