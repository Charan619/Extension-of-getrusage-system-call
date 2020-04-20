
#include <stdio.h>
#include "bathroom.h"

//update stats functions
void updateOverallmin(Thebathroom *CommonBathroom, long qtime) {
    if (CommonBathroom->Overall_min_q_time > qtime) {
        CommonBathroom->Overall_min_q_time = qtime;

    }

}
//sumofq
void sumQtime(Thebathroom *CommonBathroom, int qtime) {

    CommonBathroom->Overall_total_q_time += qtime;
}
//updatemaxtime
void updateOverallmax(Thebathroom *CommonBathroom, int qtime) {
    if (CommonBathroom->Overall_max_q_time < qtime) {
        CommonBathroom->Overall_max_q_time = qtime;

    }


}


//initialize bathroom
void Initialize(Thebathroom *CommonBathroom) {
    // initialize all variables in the bathroom

    pthread_mutex_init(&(CommonBathroom->mutex), NULL);
    pthread_cond_init(&(CommonBathroom->condition), NULL);
    CommonBathroom->state = 0; //state unoccupied = 0; occupied = 1
    CommonBathroom->current_INTHEBATHROOM = 0; //Ya know, "IN THE BATHROOM" ~ NCS/Weird AL. https://www.youtube.com/watch?v=JsCLnMrWbsw
    CommonBathroom->current_waiting = 0; //current num people in q
    CommonBathroom->Overall_total_inQ = 0; //number of people who were in the q through the sim
    CommonBathroom->Overall_total_q_time = 0; //overall total q time (sum of all people's q time)
    CommonBathroom->Overall_min_q_time = 0; //overall min q time of all people
    CommonBathroom->Overall_max_q_time = 0; //overall max q time of all people
    CommonBathroom->Overall_avg_q_time = 0; //Overall_total_q_time/Overall_total_inQ = overall avg q time
    CommonBathroom->total_uses = 0; //total uses the bathroom has gone through
    CommonBathroom->Overall_avg_users = 0;

    //get time for the start time
    gettimeofday(&(CommonBathroom->start), NULL);

}


void Enter(Thebathroom *CommonBathroom, gender g) {

//printf("IN THE BATHROOM ~ NCS\n\n");
    pthread_mutex_lock(&(CommonBathroom->mutex));
    if (CommonBathroom->state == 0) {    //if unoccupied

//change state of bathroom
        if (g == male) {
            CommonBathroom->state = 1;
        } else {
            CommonBathroom->state = 2;
        }
        CommonBathroom->current_INTHEBATHROOM++; //increment num users in bathroom
    } else if ((CommonBathroom->state == 1 && g == male) ||
               (CommonBathroom->state == 2 && g == female)) { //bathroom gender is same as user gender

        CommonBathroom->current_INTHEBATHROOM++; //increment num users in bathroom
    } else {
        CommonBathroom->current_waiting++; //increment wait queue;

        CommonBathroom->Overall_total_inQ++; //increment total waitq



        pthread_cond_wait(&(CommonBathroom->condition), &(CommonBathroom->mutex)); //wait for condition
        //swap gender
        if (CommonBathroom->state == 1) {
            CommonBathroom->state = 2;
        } else {
            CommonBathroom->state = 1;
        }

        CommonBathroom->current_INTHEBATHROOM++; //increment num users in bathroom

    }
//printf("oh k!\n\n");
    pthread_mutex_unlock(&(CommonBathroom->mutex));
//printf("oh k!\n\n");
}

//on leave
void Leave(Thebathroom *CommonBathroom) {
    //lock
    pthread_mutex_lock(&(CommonBathroom->mutex));

    //remove person from bathroom
    CommonBathroom->current_INTHEBATHROOM--;

    if (CommonBathroom->current_INTHEBATHROOM <= 0) {

        CommonBathroom->state = 0; //unoccupied

        pthread_cond_broadcast(&(CommonBathroom->condition)); //bathroom is open

        CommonBathroom->current_waiting = 0; //reset wait current queue
    }
    CommonBathroom->total_uses++;
    //unlock
    pthread_mutex_unlock(&(CommonBathroom->mutex));
}

//FINALIZE
void Finalize(Thebathroom *CommonBathroom) {
    //find time at end of program
    gettimeofday(&(CommonBathroom->end), NULL); // get the Finalize time

    //find differece of start and end time of program
    CommonBathroom->timediff = getDiffTime(&(CommonBathroom->start), &(CommonBathroom->end));

    //calc avg q time
    CommonBathroom->Overall_avg_q_time =
            (float) CommonBathroom->Overall_total_q_time / (float) CommonBathroom->Overall_total_inQ;
    //calc avg users per milisecond
    CommonBathroom->Overall_avg_users = (float) CommonBathroom->total_uses / (float) CommonBathroom->timediff;

    //remove cancer
    pthread_mutex_destroy(&(CommonBathroom->mutex));
    pthread_cond_destroy(&(CommonBathroom->condition));

    //print stats
    printf("Bathroom Statistics:\n");
    printf("Total Time of Simulation: %d ms\n", CommonBathroom->timediff);
    printf("Total uses: %d\n", CommonBathroom->total_uses);
    printf("Total times people were in the Queue:: %d ms\n", CommonBathroom->Overall_total_inQ);
    printf("Total Time spent in Queue:: %d ms\n", CommonBathroom->Overall_total_q_time);
    printf("Minimum Time spent in Queue: %d ms\n", CommonBathroom->Overall_min_q_time);
    printf("Maximum Time spent in Queue: %d ms\n", CommonBathroom->Overall_max_q_time);
    printf("Avg Time spent in Queue: %f ms\n", CommonBathroom->Overall_avg_q_time);
    printf("Avg Users in the Bathroom at any time: %f \n\n", CommonBathroom->Overall_avg_users);

}

//get difference of two time structs
long getDiffTime(struct timeval *start, struct timeval *end) {
    unsigned long long diff = 0;
    unsigned long long sec = 0;
    unsigned long long ms = 0;

    sec = end->tv_sec - start->tv_sec; //get number of seconds
    diff += sec * 1000; //convert to milliseconds

    if (start->tv_usec > end->tv_usec) {
        ms = start->tv_usec - end->tv_usec; //microseconds
        diff += ms / 1000; //convert to milliseconds
    } else {
        ms = end->tv_usec - start->tv_usec;
        diff += ms / 1000; //conver to milliseconds
    }

    return diff;
}


