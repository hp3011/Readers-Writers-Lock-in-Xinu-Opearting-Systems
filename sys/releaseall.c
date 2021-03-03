#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include <q.h>

extern unsigned long ctr1000;

int releaseall (int numlocks, int ldesN){
    //release the lock
    int ldes,i;
    STATWORD ps;
    disable(ps);
    for (i=0; i<numlocks; i++) {
        ldes = (int)*((&ldesN)+i);
        // kprintf("ldes: %d\n", ldes);
        if (release(currpid, ldes) == SYSERR){
            restore(ps);
            return SYSERR;
        }
    }
    restore(ps);
    return OK;
}

int release(int pid, int ldes) {
    //Valid ldes
    if (ldes <0 || ldes >= NLOCKS) {
        return SYSERR;
    }


    ltab[ldes].lhold[pid] = 0;
    proctab[pid].acqlck[ldes] = 0;
    if (ltab[ldes].ltype == READ){
        ltab[ldes].rdr_count -=1;
        if (ltab[ldes].rdr_count >0){
            return OK;
        }
    }


    int rprio = get_hprior(ltab[ldes].rdr_qtail);
    int wprio = get_hprior(ltab[ldes].wrt_qtail);
    

    //If Writer Priority > Reader Priority 
    if (wprio > 0 && rprio < wprio) {
        int wrt_pid = getlast(ltab[ldes].wrt_qtail);
        ltab[ldes].ltype = WRITE;
        ltab[ldes].lhold[wrt_pid] = proctab[wrt_pid].pprio;
        proctab[wrt_pid].pwaitret = DELETED;
        proctab[wrt_pid].acqlck[ldes] = 1;

        ltab[ldes].lprior = get_lprio(ldes);
        update_pinh(ldes);
        
        ready(wrt_pid, RESCHYES);


    } else if(rprio >0 && rprio > wprio) {
        //Checking if the writer has MININT(Head)priority, then setting it to 0.
        if (wprio < 0) {
            wprio = 0;
        } else {
            wprio = wprio;
        }
        while(get_hprior(ltab[ldes].rdr_qtail) >= wprio){
            int rdr_prior = lastkey(ltab[ldes].rdr_qtail);
            int rdr_pid = getlast(ltab[ldes].rdr_qtail);
            ltab[ldes].ltype = READ;
            ltab[ldes].lhold[rdr_pid] = proctab[rdr_pid].pprio;

            proctab[rdr_pid].pwaitret = DELETED;
            proctab[rdr_pid].acqlck[ldes] = 1;
            ltab[ldes].lprior = get_lprio(ldes);
            update_pinh(ldes);
            
            ltab[ldes].rdr_count += 1;
            ready(rdr_pid, RESCHNO);
        }
        resched();
    } else if (rprio>0 && wprio>0 && rprio == wprio){
        int rdr_pid = q[ltab[ldes].rdr_qtail].qprev;
        int wrt_pid = q[ltab[ldes].wrt_qtail].qprev;

        unsigned long rdr_time = ctr1000 - proctab[rdr_pid].wtime;
        unsigned long wrt_time = ctr1000 - proctab[wrt_pid].wtime;

        if (wrt_time > rdr_time) {
            int wrt_pid = getlast(ltab[ldes].wrt_qtail);
            ltab[ldes].ltype = WRITE;
            ltab[ldes].lhold[wrt_pid] = proctab[wrt_pid].pprio;
            proctab[wrt_pid].pwaitret = DELETED;
            proctab[wrt_pid].acqlck[ldes] = 1;
            ltab[ldes].lprior = get_lprio(ldes);
            update_pinh(ldes);
            
            ready(wrt_pid, RESCHYES);
        } else{
            if ( (rdr_time - wrt_time) < 1000) {
                dequeue(wrt_pid);
                ltab[ldes].ltype = WRITE;
                ltab[ldes].lhold[wrt_pid] = proctab[wrt_pid].pprio;
                proctab[wrt_pid].pwaitret = DELETED;
                proctab[wrt_pid].acqlck[ldes] = 1;
                ltab[ldes].lprior = get_lprio(ldes);
                update_pinh(ldes);
                
                ready(wrt_pid, RESCHYES);

            } else {
                while(get_hprior(ltab[ldes].rdr_qtail) >= wprio){
                    int rdr_prior = get_hprior(ltab[ldes].rdr_qtail);
                    rdr_pid = getlast(ltab[ldes].rdr_qtail);

                    ltab[ldes].ltype = READ;
                    ltab[ldes].lhold[rdr_pid] = proctab[rdr_pid].pprio;
                    proctab[rdr_pid].pwaitret = DELETED;
                    proctab[rdr_pid].acqlck[ldes] = 1;
                    ltab[ldes].lprior = get_lprio(ldes);
                    update_pinh(ldes);
                    
                    ltab[ldes].rdr_count += 1;
                    ready(rdr_pid, RESCHNO);
                }
            }
        }
    } else {
        //When both queue are empty
        ltab[ldes].ltype = LFREE;
        ltab[ldes].lprior = 0;
    }
}

