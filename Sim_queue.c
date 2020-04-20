

#include <math.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "bathroom.h"

typedef struct {
    int pid; //person id
    gender mygender; //person gender


int loopct; //number of bathroom trips a person takes
long times_inQueue; //times person was in the queue
long minQtime; //min wait time //long because int overflow is possible
long maxQtime; //max wait time
long totalWaittime; //total wait time
long avgQtime; //totalQueuetime/inQueue = avg wait time
int staytime; //time in loo
int arrivaltime; //time the person gets arrives in the queue

} person_info;


Thebathroom CommonBathroom;
pthread_mutex_t printing; //mutex for several lines of printing

int meanLoopCount;
int meanArrivaltime;
int meanStaytime;

//generate random number from a given mean.
int NormDist(int mean) {
    float a = drand48();
    float b = drand48();
    float z = sqrt(-2 * log(a)) * cos(2 * M_PI * b); //box-muller equation
    float num = ((mean / 2) * z) + mean; //random result within our mean
    if (num < 1) { //check if rand result makes sense
        num = 1;
    }
    if (num > 1000) { //hold on there space cowboy...
        num = 1000;
    }
    return (int) floor(num); //turn float to int and round down
}

void *Person(void *info) {
    person_info *Stranger = (person_info *) info;
    unsigned long minQtime = 999999999; //minimum time spent in queue
    long avgQtime = 0; //average time spent in queue
    long maxQtime = 0; //maximum time spent in queue
    long totalWaittime = 0; //total time spent in queue
    int times_inQueue = 0; //total number of times waited in queue

    struct timeval tryEntercall; //finds the time person trys to enter
    struct timeval Entercall; //finds the time person actually entered


    int loopct = NormDist(meanLoopCount);
    int vcsw=0;
    int ivcsw=0;

    for (int i = 0; i < loopct; i++) {
        //printf("Stranger %d is on trip number %d\n",Stranger->pid,i++);
	struct rusage buf;
	getrusage(RUSAGE_SELF, &buf);

        int arrivaltime = NormDist(meanArrivaltime);
        int staytime = NormDist(meanStaytime);

        /* wait for a random length of time
           based on arrival_time */

        usleep(arrivaltime * 1000);

        gettimeofday(&tryEntercall, NULL); //get time before
        Enter(&CommonBathroom, Stranger->mygender);
        gettimeofday(&Entercall, NULL); //get time after

        /* wait for a random length of time
           based on stay time */
        usleep(staytime * 1000);

        Leave(&CommonBathroom);

        //do some time calculations
        long diff = getDiffTime(&tryEntercall, &Entercall);

	vcsw+=buf.ru_nvcsw;
	ivcsw+=buf.ru_nivcsw;
        if (diff) {
            if (diff > maxQtime) {
                maxQtime = diff; //update person max
                updateOverallmax(&CommonBathroom, diff); //update bathroom max
            }

            if (diff < minQtime) {
                minQtime = diff; //update person min
                updateOverallmin(&CommonBathroom, diff); //update bathroom min
            }

            times_inQueue++;
            totalWaittime += diff; //update person sum
            sumQtime(&CommonBathroom, diff); //update bathroom sum
        }
    }

    // get average from the total / the loop count if they were in a q
    if (times_inQueue) {
        avgQtime = totalWaittime / times_inQueue;
    }

    //change value for printing
    if (minQtime == 999999999) {
        minQtime = 0;
    }

    pthread_mutex_lock(&printing); //lock so only this thread prints

    printf("\nPerson: %d has Finished.\n", Stranger->pid);

    if (Stranger->mygender == 0) {
        printf("Gender: Male\n");
    } else {
        printf("Gender: Female\n");
    }

    printf("No. of voluntary context switches: %d\n", vcsw);
    printf("No. of involuntary context switches: %d\n", ivcsw);
    printf("Bathroom Trips: %d\n", loopct);
    printf("Min queue time: %ld ms\n", minQtime);
    printf("Max queue time: %ld ms\n", maxQtime);
    printf("Avg queue time: %ld ms\n", avgQtime);
    printf("Times in queue: %d\n", times_inQueue);
    printf("Total time in Queue: %ld ms\n\n\n", totalWaittime);


    //Upload stats to bathroom
    //CommonBathroom->total_uses += loopct; need to use a getter



    pthread_mutex_unlock(&printing);
    //unlock

    pthread_exit(NULL); //kill thread
}



int main(int argc, char *argv[]) {

    if (argc == 5) { //check valid # of input
//check for valid input
//check greater that 1000
        if (atoi(argv[1]) > 1000 || atoi(argv[2]) > 1000 || atoi(argv[3]) > 1000 || atoi(argv[4]) > 1000) {
            printf("Please enter values less than 1000.\n\n");
            exit(1);
        }
//check less than 1
        if (atoi(argv[1]) < 1 || atoi(argv[2]) < 1 || atoi(argv[3]) < 1 || atoi(argv[4]) < 1) {
            printf("Please enter values 1 or greater.\n\n");
            exit(1);
        }


        int nUsers = atoi(argv[1]); //# of users
        meanLoopCount = atoi(argv[2]); //mean loop count
        meanArrivaltime = atoi(argv[3]); //mean arrival time
        meanStaytime = atoi(argv[4]); //mean stay


        printf("\nValid Input recognized!\n");
        printf("Number of Users:%d\n", nUsers);
        printf("Mean Loop Count:%d\n", meanLoopCount);
        printf("Mean Arrival:%d\n", meanArrivaltime);
        printf("Mean Stay:%d\n\n", meanStaytime);
//VALID INPUT!


        pthread_mutex_init(&printing, NULL); //init print mutex

        pthread_t People[nUsers]; //produce thread of size users
        Initialize(&CommonBathroom); //init the bathroom

//generate nuser threads
        for (int i = 0; i < nUsers; i++) {
            person_info *person = malloc(sizeof(person_info *));
            person->pid = i; //thread number of the thread

            double determine_gender = drand48(); //determine gender
            if (determine_gender < .5) { //get 50/50 split
                person->mygender = male;
            } else {
                person->mygender = female;
            }


            pthread_create(&People[i], NULL, Person, (void *) person);
            //Person cant pass avgs so they must be globals
        }
//printf("HELLO!\n\n");
        for (int i = 0; i < nUsers; ++i) {

            pthread_join(People[i], NULL);

        }
//printf("HELLO!\n\n");
        Finalize(&CommonBathroom);
//return 0;
    } else { //BAD # of args
        printf("\n Please use the program like so:");
        printf("\n\t./Sim_queue nUsers meanLoopCount meanArrivaltime meanStaytime\n\n");
        exit(1);
    }


    return 0;
}
