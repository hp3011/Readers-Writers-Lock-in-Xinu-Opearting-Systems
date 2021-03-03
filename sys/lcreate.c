#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>

int lcreate(){
    //Creation of lock
    int i;
    STATWORD ps;
    disable(ps);
    for(i=0;i<50;i++){
        if (ltab[i].lavail == LFREE) {
            ltab[i].lavail = LASSIGN;
            restore(ps);
            return i;
        }
    }
    for(i=0;i<50;i++){
        if (ltab[i].lavail == DELETED) {
            ltab[i].lavail = LASSIGN;
            restore(ps);
            return i;
        }
    }
    restore(ps);
    return SYSERR;

}