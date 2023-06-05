#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>  
#include <unistd.h>    
#include <string.h> 
#include <signal.h>
#include <time.h>
#include <stdbool.h>
#include "structure.h"

struct shrd_mem *shm;

void sighandler(int signum) {
	printf("Child: Caught signal %d\n", signum);
	shmdt(shm);
	exit(1);
}

void enter_cs(int num){
	shm->choosing[num] = true;
	int max = 0;
	for (int i = 0; i < MAXPIDS; i++){
		if (shm->numbers[i] > max)
			max = shm->numbers[i];
	}
	shm->numbers[num] = max + 1;
	shm->choosing[num] = false;
	
	for (int k = 0; k < MAXPIDS; k++){
		while (shm->choosing[k])
			;
		while (shm->numbers[k] != 0 && 
		       (shm->numbers[k] < shm->numbers[num]) ||
		       (shm->numbers[k] == shm->numbers[num] && k < num))
		    ;
	}
}

void log_event(int num, const char* message) {
	char filename[32];
	snprintf(filename, sizeof(filename), "logfile.%d", num);
	FILE* f = fopen(filename, "a");
	time_t timer;
    struct tm* tm_info;
	timer = time(NULL);
    tm_info = localtime(&timer);
    char buffer[26];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", tm_info);
    fprintf(f, "%s: %s\n", buffer, message);
    fclose(f);
}

int main (int argc, char *argv[]) {
	signal(SIGINT, sighandler);
	
	srand(time(NULL));

	int shmid;
	
	key_t key_glock = ftok("master.c", 420);
	
	shmid = shmget(key_glock, sizeof(struct shrd_mem), 0);
	shm = shmat(shmid, 0, 0);
	
	int num = atoi(argv[1]);
	
	time_t timer;
    char buffer[26];
    struct tm* tm_info;
    
	printf("%ld\n", (long)getpid());
	
	for(int i = 0; i < 3; i++) {
		log_event(num, "Entering critical section...");
		enter_cs(num);
		log_event(num, "Entered critical section.");
		sleep(1 + rand()%5);
		timer = time(NULL);
    	tm_info = localtime(&timer);
    	strftime(buffer, sizeof(buffer), "%H:%M:%S", tm_info);
		FILE* f = fopen("cstest", "a");
		fprintf(f, "%s Queue %d File modified by process number %d\n", buffer, num, (int)getpid());
		fclose(f);
		sleep(1 + rand()%5);
		log_event(num, "Exiting critical section...");
		shm->numbers[num] = 0;
	}
	
	sleep(2);
	shmdt(shm);
	return 0;
}		