#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <q.h>
#include <lock.h>

extern unsigned long ctr1000;

int lock (int ldes, int type, int priority){
    //Check if lock is created
    struct pentry *pptr;
    STATWORD ps;
    disable(ps);
    if (ltab[ldes].lavail == LFREE || ltab[ldes].lavail == DELETED || ldes < 0 || ldes >= NLOCKS) {
        restore(ps);
        return SYSERR;
    }  

    //Check the status of the lock
    if (ltab[ldes].lstatus == LACQUIRE) {
        ltab[ldes].lprior = get_lprio(ldes);
        update_pinh(ldes);
        
        if (type == READ){
            if (ltab[ldes].ltype == READ) {
                if (get_hprior(ltab[ldes].wrt_qtail) > priority){
                    pptr = &proctab[currpid];
		            pptr->pstate = PRWAIT;
                    pptr->wtime = ctr1000;
                    proctab[currpid].pwaitret = ldes;
                    
                    insert(currpid, ltab[ldes].rdr_qhead,priority);

                    ltab[ldes].lprior = get_lprio(ldes);
                    update_pinh(ldes);
                    resched();
                } else{
                    ltab[ldes].rdr_count += 1;
                    proctab[currpid].acqlck[ldes] = 1; 
                    ltab[ldes].lhold[currpid] = proctab[currpid].pprio;

                }

            }else if (ltab[ldes].ltype == WRITE) {
                pptr = &proctab[currpid];
                pptr->pstate = PRWAIT;
                pptr->wtime = ctr1000;
                proctab[currpid].pwaitret = ldes;

                insert(currpid, ltab[ldes].rdr_qhead,priority);
                ltab[ldes].lprior = get_lprio(ldes);
                update_pinh(ldes);
                resched();
            }  
        } else if(type == WRITE){
            pptr = &proctab[currpid];
            pptr->pstate = PRWAIT;
            pptr->wtime = ctr1000;
            proctab[currpid].pwaitret = ldes;

            insert(currpid, ltab[ldes].wrt_qhead ,priority);

            ltab[ldes].lprior = get_lprio(ldes);
            update_pinh(ldes);
            resched();
        }
    } else if (ltab[ldes].lstatus == LFREE) {
        ltab[ldes].lstatus = LACQUIRE;
        ltab[ldes].ltype = type;
        ltab[ldes].lhold[currpid] = proctab[currpid].pprio;
        if (type == READ){
            ltab[ldes].rdr_count += 1;
        }
        proctab[currpid].acqlck[ldes] = 1; 
    
    }
    restore(ps);
    return OK;
}

int get_hprior(int qtail) {
    int lastnode = q[qtail].qprev;
    return q[lastnode].qkey;
}

int printq(int qhead, int  fl) {

    int curr = qhead;
    if (fl==READ){
        kprintf("Read queue: ");
    } else {
        kprintf("Write Queue: ");
    }
    while (q[qhead].qkey != MAXINT) {
        kprintf("%d\t", q[qhead].qkey);
        qhead = q[qhead].qnext;
    }
    kprintf("\n");
}

int get_lprio(int ldes) {
    STATWORD ps;
    disable(ps);
    int rdr_qtail = q[ltab[ldes].rdr_qtail].qprev;
    int max_pprio = 0;
    while (q[rdr_qtail].qkey>0){
        if (proctab[rdr_qtail].pinh <= proctab[rdr_qtail].pprio ){
            if (proctab[rdr_qtail].pprio > max_pprio){
                max_pprio = proctab[rdr_qtail].pprio;
            }
        } else {
            if (proctab[rdr_qtail].pinh > max_pprio){
                max_pprio = proctab[rdr_qtail].pinh;
            }
        }
        rdr_qtail = q[rdr_qtail].qprev;
    }

    int wrt_qtail = q[ltab[ldes].wrt_qtail].qprev;
    while (q[wrt_qtail].qkey>0){
        if (proctab[wrt_qtail].pinh <= proctab[wrt_qtail].pprio ){
            if (proctab[wrt_qtail].pprio > max_pprio){
                max_pprio = proctab[q[wrt_qtail].qprev].pprio;
            }
        } else {
            if (proctab[wrt_qtail].pinh > max_pprio){
                max_pprio = proctab[q[wrt_qtail].qprev].pinh;
            }
        }
        wrt_qtail = q[wrt_qtail].qprev;
    }
    restore(ps);
    return max_pprio;
}

int update_pinh (int ldes){
    int i;
    struct pentry *pptr;
    STATWORD ps;
    disable(ps);
    for (i=0;i<NPROC;i++){
        if (ltab[ldes].lhold[i] > 0 &&  ltab[ldes].lprior > ltab[ldes].lhold[i]){
            pptr = &proctab[i];
            //check for highest lprior vlaue 
            int j, max_lprio;
            max_lprio = ltab[ldes].lprior;
            for (j=0;j<NLOCKS;j++){
                if (proctab[i].acqlck[j] == 1 && ltab[j].lprior > max_lprio){
                    max_lprio = ltab[j].lprior;
                }
            }
            pptr->pinh = max_lprio;
        }
        if(ltab[ldes].lprior == 0){
            pptr = &proctab[i];
            int j, max_lprio;
            max_lprio = ltab[ldes].lprior;
            for (j=0;j<NLOCKS;j++){
                if (proctab[i].acqlck[j] == 1 && ltab[j].lprior > max_lprio){
                    max_lprio = ltab[j].lprior;
                }
            }
            pptr->pinh = max_lprio;
        }
    }
    restore(ps);
    return OK;
}