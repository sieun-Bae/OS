/* REF
 * signal.h
   https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.1.0/com.ibm.zos.v2r1.bpxbd00/rtsigde.htm#rtsigde
 * key (ftok)
   https://codeday.me/ko/qa/20190403/230371.html
 * message equeue
   https://www.it-note.kr/98
*/
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/msg.h>

#include <sys/types.h>


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


/* Time */
int quantum = 200000;
int i; // for forloop
int NPROC;

void handle_signal(int sig);

int main()
{
	sigset_t mask;
	sigfillset(&mask);
	sigdelset(&mask, SIGINT);
	sigdelset(&mask, SIGALRM);
	sigdelset(&mask, SIGTERM);
	sigprocmask(SIG_SETMASK, &mask, NULL);
	signal(SIGINT, handle_signal);
	signal(SIGALRM, handle_signal);
	
	/* WAIT-QUEUE in shared memory */
	
	NPROC = rand() % 40 + 10;

	long roundRobin[NPROC]; //RUN-QUEUE
	PCB* pct;
	key_t key = ftok("/tmp", 50);
	int shmpid = shmget(key, NPROC*sizeof(PCB), IPC_CREAT | 0666);
	pct = shmat(shmpid, NULL, 0);
	for(i=0; i<NPROC; i++) { pct[i].ready = -1; }

	/* TIMER in shared memory */
	TIMER* timer;
	key = ftok("/tmp", 35);
	int shmtid = shmget(key, sizeof(TIMER), IPC_CREAT | 0666);
	timer = shmat(shmtid, NULL, 0);
	timer->secs = 0;
	timer->nanos = 0;

	/* RUN-QUEUE */
	for(i=0; i<NPROC; i++) { roundRobin[i] = 0; }
	
	/* MSG-QUEUE */
	MSGQ message;
	key = ftok("/tmp", 65);
	int msgid = msgget(key, IPC_CREAT | 0666);

	int termTime=20;

	alarm(termTime);
	
	srand(getpid());
	unsigned int ran;
	
	unsigned int secs;
	unsigned int nanos;

	int total = 0; //total num of processes
	int count = 0; //current num of processes
	int index = 0;
	pid_t pid;

	int status;

	/* Log file */
	char* filename = "schedule_dump.txt";
	FILE* log;
	log = fopen(filename, "w");

	printf("Computing logs written in %s log file.. termination time: %d\n", filename, termTime);

	while (term == 0 || count>0) //전체 terminate 아니거나 프로세스 남아있을 때
	{
		timer->secs += rand()%1;
		timer->nanos += rand()%1000;
		while(timer->nanos>1000000000) { timer->nanos -=1000000000; timer->secs++; }
		
		/* FORK */
		if (count<NPROC && total<100)
		{
			//find index to fork new process in WAIT-QUEUE
			for(i=0;i<NPROC;i++) { if(pct[i].ready==-1){ index = i; break; }}
			pid = fork();
			total++;
			count++;

			if(total==100) term=3;

			switch(pid){
				case -1:
					fprintf(stderr, "Error. Fork Failed....\n"); 
					count--; 
					break;
				case 0:
					
					msgrcv(msgid, &message, sizeof(message), 1, 0);
					
					printf("Executing %ld child process!\n", (long)getpid());
					
					message.type = 2;
					msgsnd(msgid, &message, sizeof(message), 0);
					
					char I[3];
					sprintf(I, "%d", index);
					char* args[] = {"./Final_RR_user_", I, NULL};
					if(execv(args[0], args)==-1) 
						fprintf(stderr, "Executing %ld child process Failed.....\n", (long)getpid()); 

					exit(1); 

				default:
					pct[index].pid = (long)pid;
					for(i=0;i<NPROC;i++){
						if(roundRobin[i]==0) {
							roundRobin[i]=pct[index].pid; 
							pct[index].burst_time = quantum; 
							break;
						}
					}

					pct[index].duration = rand()%99999998+1;
					pct[index].cpu_time = 0;
					pct[index].total_sec = 0;
					pct[index].total_nano = 0;
					pct[index].running = 0;
					pct[index].ready = 1;
					pct[index].done = 0;

					//////////////////write log////////////
					snprintf(message.str, sizeof(message.str), "At time %d.%d, Generating Process %ld!\n", timer->secs, timer->nanos, pct[index].pid);
					fprintf(log, "%s", message.str);

					message.type = 1;
					msgsnd(msgid, &message, sizeof(message), 0);
					msgrcv(msgid, &message, sizeof(message), 2, 0);
					break;	
			}
			
		}
		
		timer->nanos+=rand()%9900+100;
		while (timer->nanos > 1000000000){ timer->nanos -= 1000000000; timer->secs++; }
		
		if(roundRobin[0] != 0)
		{
			//find Process to dispatch			
			for(i=0;i<NPROC;i++) { 
				if(pct[i].pid == roundRobin[0]) 
					break; 
			}
			
			

			pct[i].burst_time = quantum;
			
			////////////write log//////////////
			snprintf(message.str, sizeof(message.str), "At time %d.%d, Dispatching Process %ld!\n", timer->secs, timer->nanos, pct[i].pid);
			fprintf(log, "%s", message.str);

			message.type = (long)roundRobin[0];
			msgsnd(msgid, &message, sizeof(message), 0);

			msgrcv(msgid, &message, sizeof(message), (long)getpid(), 0);

			if(pct[i].done == 1) //done
			{
				count--;
				printf("At time %d.%d, Process %ld finished. %d child processes running.\n", timer->secs, timer->nanos, pct[i].pid, count);
				waitpid(pct[i].pid, &status, 0);

				snprintf(message.str, sizeof(message.str), "At time %d.%d, Process %ld finished. %d child processes running\n", timer->secs, timer->nanos, pct[i].pid, count);
				fprintf(log, "%s", message.str);

				roundRobin[0] = 0;
				pct[i].ready=-1;
			}
			else if(pct[i].ready == 0) //blocked
			{
				snprintf(message.str, sizeof(message.str), "At time %d.%d, Blocking Process %ld.. it was ran for %ds..\n", timer->secs, timer->nanos, pct[i].pid, pct[index].burst_time);
				fprintf(log, "%s", message.str);
				pct[i].priority++;
			}
			else //still running
			{
				snprintf(message.str, sizeof(message.str), "At time %d.%d, Recieving Process %ld ran for %ds!\n", timer->secs, timer->nanos, pct[i].pid, pct[index].burst_time);
				fprintf(log, "%s", message.str);
				pct[i].priority++;
			}

			/* Update priority queue */
			
			long temp = roundRobin[0];
			long least = 0;
			for (i=0; i<(NPROC-1); i++) 
				roundRobin[i] = roundRobin[i+1];
			roundRobin[NPROC-1] = 0;
			for (i=0; i<(NPROC); i++) { 
				if (roundRobin[i]==0) {
					roundRobin[i] = temp; break;
				} 
			}
		}
	}

	while( (pid=wait(&status))>0 )
	{
		count--;
		printf("Process %ld finished.\n", pct[i].pid);
	}
	
	/* FREE */
	shmdt(timer);
	shmctl(shmtid, IPC_RMID, NULL);
	shmdt(pct);
	shmctl(shmpid, IPC_RMID, NULL);
	msgctl(msgid, IPC_RMID, NULL);

	fclose(log);

	return 0;
}


void handle_signal(int sig) {
   printf("./RR: Parent process %ld caught signal: %d. Cleaning up and terminating.\n", (long)getpid(), sig);
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
















