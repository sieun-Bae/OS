#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/msg.h>

#include <sys/types.h>

#define MAX 10

/* SIGNAL FLAG */
volatile sig_atomic_t term = 0;

typedef struct pcb {
	long pid;
	int priority;
	
	int cpu_time;
	int burst_time;

	int total_sec;
	int total_nano;

	int running;
	int ready;
	int duration;
	int done;

	int type;
} PCB; //PCB

typedef struct msgq {
	long type;
	char str[200];
} MSGQ; //MSG

typedef struct timer {
	unsigned int secs;
	unsigned int nanos;
} TIMER; //TIMER

void handle_signal(int sig);

int main(int argc, char*argv[])
{
	sigset_t mask;
	sigfillset(&mask);
	sigdelset(&mask, SIGUSR1);
	sigdelset(&mask, SIGUSR2);
	sigdelset(&mask, SIGTERM);
	sigprocmask(SIG_SETMASK, &mask, NULL);
	signal(SIGUSR1, handle_signal);
	signal(SIGUSR2, handle_signal);
	
	/* WAIT-QUEUE in shared memory */
	PCB* pct;
	key_t key = ftok("/tmp", 50);
	int shmpid = shmget(key, MAX*sizeof(PCB), 0666);
	pct = shmat(shmpid, NULL, 0);
	
	/* TIMER in shared memory */
	TIMER* timer;
	key = ftok("/tmp", 35);
	int shmtid = shmget(key, sizeof(TIMER), 0666);
	timer = shmat(shmtid, NULL, 0);

	/* MSG-QUEUE */
	MSGQ message;
	key = ftok("/tmp", 65);
	int msgid = msgget(key, 0666);

	//flag
	int done = 0;
	//time recording
	int start;
	int end;

	int round=0;

	srand(getpid());
	int ran;

	int index = atoi(argv[1]);

	while(done==0&&term==0)
	{	
		msgrcv(msgid, &message, sizeof(message), (long)pct[index].pid, 0);
		round++;
		//Timer setting
		if (round==1)	{ start = timer->secs*1000000000+timer->nanos; }
		
		ran = rand()%499+1;
		if(ran<5)
		{
			pct[index].burst_time = rand()%(pct[index].burst_time-2)+1;
			if((pct[index].burst_time+pct[index].cpu_time)>=pct[index].duration)
				{ pct[index].burst_time = pct[index].duration-pct[index].cpu_time; }
			
			pct[index].cpu_time += pct[index].burst_time;
			pct[index].done=1;

			timer->nanos += pct[index].burst_time;
			while(timer->nanos>1000000000) { timer->nanos -= 1000000000; timer->secs++; }
			timer->secs+=pct[index].burst_time;

			end = timer->secs*1000000000 + timer->nanos;
			int childNanos = end-start;
			int childSecs = 0;
			while (childNanos >= 1000000000) { childNanos -= childNanos; childSecs++;}
			pct[index].total_sec = childSecs;
			pct[index].total_nano = childNanos;

			message.type = (long)getppid();
			msgsnd(msgid, &message, sizeof(message), 0);

			shmdt(timer);
			shmdt(pct);
			return -1;
		}

		ran = rand()%99 + 1;
		if (ran>=20)
		{
			if((pct[index].burst_time+pct[index].cpu_time)>=pct[index].duration)
				{ pct[index].burst_time = pct[index].duration-pct[index].cpu_time; 
					pct[index].done = 1;
					done=1;}
			
			timer->nanos += pct[index].burst_time;
			while(timer->nanos>1000000000) { timer->nanos -= 1000000000; timer->secs++; }
			
			pct[index].cpu_time += pct[index].burst_time;

			message.type = (long)getppid();
			msgsnd(msgid, &message, sizeof(message), 0);
		}
		else ////block
		{
			pct[index].ready = 0;

			pct[index].burst_time = rand()%(pct[index].burst_time-2)+1;
			if((pct[index].burst_time+pct[index].cpu_time)>=pct[index].duration)
				{ pct[index].burst_time = pct[index].duration-pct[index].cpu_time; 
					pct[index].done = 1;
					done=1;}

			timer->nanos += pct[index].burst_time;
			while(timer->nanos > 1000000000) { timer->nanos -= 1000000000; timer->secs++; }
			
			pct[index].cpu_time += pct[index].burst_time;

			message.type = (long)getppid();
			msgsnd(msgid, &message, sizeof(message), 0);
		}

	}

	/* Timer setting */
	end = timer->secs*1000000000+timer->nanos;
	int childNanos = end-start;
	int childSecs = 0;

	while(childNanos >= 1000000000) { childNanos -= childNanos; childSecs++; }

	pct[index].total_sec = childSecs;
	pct[index].total_nano = childNanos;
	/* Detach */
	shmdt(timer);
	shmdt(pct);

	return 0;
}

void handle_signal(int sig) {
   printf("./USER: Parent process %ld caught signal: %d. Cleaning up and terminating.\n", (long)getpid(), sig);
   switch(sig) {
      case SIGINT:
         kill(0, SIGUSR1);
         term = 1;
         break;
      case SIGALRM:
         kill(0, SIGUSR2);
         term = 2;
         break;
   }
}
