
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>

void linit(){
    //initialise the lock
    int i;
    for (i=0;i<NLOCKS;i++){
        ltab[i].lavail = LFREE;
        ltab[i].lstatus = LFREE;
        ltab[i].rdr_count = 0;
        ltab[i].rdr_qtail = 1 + (ltab[i].rdr_qhead = newqueue());
        ltab[i].wrt_qtail = 1 + (ltab[i].wrt_qhead = newqueue());
    }
}