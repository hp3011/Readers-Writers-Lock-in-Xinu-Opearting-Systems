
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>

void reader_pinh (char *msg, int lck)
{
    int     ret;

    kprintf ("  Reader %s: to acquire lock: %d\n", msg, currpid);
    lock (lck, READ, 20);
    kprintf ("  Reader %s: acquired lock, sleep 10s\n", msg);
    sleep(10);
    int i;
    // kprintf("\n");
    for (i=0;i<10;i++){
        kprintf("%s",msg);
    }
    kprintf("\n");
    kprintf ("  Reader %s: to release lock\n", msg);
    releaseall (1, lck);
}

void writer_pinh (char *msg, int lck)
{
    kprintf ("  Writer %s: to acquire lock: %d\n", msg, currpid);
    lock (lck, WRITE, 20);
    kprintf ("  Writer %s: acquired lock\n", msg);
    int i;
    // kprintf("\n");
    for (i=0;i<10;i++){
        kprintf("%s",msg);
    }
    kprintf("\n");
    kprintf ("  Writer %s: to release lock\n", msg);
    releaseall (1, lck);
}

void reader_sem (char *msg, int sem)
{
    kprintf ("  Reader %s: to acquire sem\n", msg);
	wait(sem);
	kprintf ("  Reader %s: acquired sem, sleep 10s\n", msg);
    sleep(10);
    int i;
    // kprintf("\n");
    for (i=0;i<10;i++){
        kprintf("%s",msg);
    }
    kprintf("\n");
    kprintf ("  Reader %s: to release sem\n", msg);
	signal(sem);
}

void writer_sem (char *msg, int sem)
{
    kprintf ("  Writer %s: to acquire sem\n", msg);
    wait(sem);
    kprintf ("  Writer %s: acquired sem\n", msg);
    int i;
    // kprintf("\n");
    for (i=0;i<10;i++){
        kprintf("%s",msg);
    }
    kprintf("\n");
    kprintf ("  Writer %s: to release sem\n", msg);
    signal(sem);
}
void testsem()
{
	int sem;
	int rd1, rd2, rd3;
	int wr1, wr2;
	
	sem = screate(1);
	
	wr1 = create(writer_sem, 2000, 20, "writerA", 2, "A", sem);

    wr2 = create(writer_sem, 2000, 30, "writerB", 2, "B", sem);
    rd1 = create(reader_sem, 2000, 10, "reader", 2, "R", sem);
    	
	kprintf("-start writer, then sleep 1s. sem granted to writer (prio 10), pid = %d\n", rd1);
    resume(rd1);
    sleep (1);

    kprintf("-start reader A, then sleep 1s. reader A(prio 20) blocked on the sem, pid = %d\n", wr1);
    resume(wr1);
    sleep (1);

    kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the sem, pid = %d\n", wr2);
    resume(wr2);
	sleep (1);	

    sleep(10);			
}

void testpinh()
{
	int lck;
	int rd1, rd2, rd3;
	int wr1, wr2;
	
	lck = lcreate();

    wr1 = create(writer_pinh, 2000, 20, "writerA", 2, "A", lck);

    wr2 = create(reader_pinh, 2000, 30, "writerB", 2, "B", lck);
    rd1 = create(reader_pinh, 2000, 10, "reader", 2, "R", lck);
	
	kprintf("-start writer, then sleep 1s. lock granted to writer (prio 10), pid = %d\n",rd1);
    resume(rd1);
    sleep (1);
	
    kprintf("-start reader A, then sleep 1s. reader A(prio 20) blocked on the lock, pid = %d\n",wr1);
    resume(wr1);
    sleep (1);

    kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock, pid = %d\n",wr2);
    resume(wr2);
	sleep (1);	

    sleep(10);		
}

int task1()
{
	kprintf("\n********** Semaphore approach **********\n");
	testsem();
	kprintf("\n****************************************\n");
	kprintf("\n********** Priority Inheritance approach **********\n");
	testpinh();
	kprintf("\n******************************\n");
	
}