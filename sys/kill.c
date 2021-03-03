/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev, i ,rflag;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}

	//Get all the locks that the process has and deque from both queue
	
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	if (proctab[pid].pwaitret >= 0){
		//the process is waiting on the lock, dequeuing it and upadating lprio.
		//Read Queuue
		int ldes = proctab[pid].pwaitret;
		int rdr_qtail = q[ltab[ldes].rdr_qtail].qprev;
		while (rdr_qtail > 0){
			
			if (rdr_qtail == pid){
				//dequeue
				dequeue(rdr_qtail);
				ltab[ldes].lprior = get_lprio(ldes);
        		update_pinh(ldes);
			}
			rdr_qtail = q[rdr_qtail].qprev;
		}

		int wrt_qtail = q[ltab[ldes].wrt_qtail].qprev;
		while (wrt_qtail > 0){
			if (wrt_qtail == pid){
				//dequeue
				dequeue(wrt_qtail);
				ltab[ldes].lprior = get_lprio(ldes);
        		update_pinh(ldes);
			}
			wrt_qtail = q[wrt_qtail].qprev;
		}

		//If the process has acquired locks, releasing those lock.
		int i;
		for (i=0;i<NLOCKS;i++){
			if (proctab[pid].acqlck[i] == 1) {
				release(pid, i);
			}
		}
	}
	switch (pptr->pstate) {

	case PRCURR:
			pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:
		dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:
			pptr->pstate = PRFREE;
	}

	restore(ps);
	return(OK);
}
