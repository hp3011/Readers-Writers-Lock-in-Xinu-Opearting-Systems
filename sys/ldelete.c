#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include <q.h>

int ldelete(int ldes){
    //Delete the lock
    STATWORD ps;
    disable(ps);
    ltab[ldes].lavail = DELETED;

    int pid;
    for (pid = 0; pid<NPROC; pid ++) {
        if (proctab[pid].acqlck[ldes] == 1){
            //When has lock acquired and other process deletes the lock
            // proctab[pid].acqlck[ldes] = DELETED;
        }
    }

    int rdr_tail = q[ltab[ldes].rdr_qtail].qprev;
    // Check Readers queue is empty
    while (q[rdr_tail].qkey > 0) {
        //Queue is non empty
        proctab[rdr_tail].pwaitret = DELETED;
        dequeue(rdr_tail);
        rdr_tail = q[rdr_tail].qprev;
    }

    int wrt_tail = q[ltab[ldes].wrt_qtail].qprev;
    while (q[wrt_tail].qkey > 0) {
        //Queue is non empty
        proctab[wrt_tail].pwaitret = DELETED;
        dequeue(wrt_tail);
        wrt_tail = q[wrt_tail].qprev;
    }
    restore(ps);
    return OK;
}