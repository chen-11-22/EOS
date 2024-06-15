#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <errno.h>

typedef struct {
    int guess;
    char result[8];
} data;


data *D;
int key;
int target;
int shm_id;
int game = 1;


void create_share_memory(){
	// create shared memory 1
    shm_id = shmget(key, sizeof(data), IPC_CREAT | 0666);
    if (shm_id < 0){
        fprintf(stderr, "Shared memory creation failed: %s\n", strerror(errno));
        exit(-1);
    }
    
    D = (data *)shmat(shm_id, NULL, 0);
    if (D == (void *) -1){
        fprintf(stderr, "Shared memory attach failed: %s\n", strerror(errno));
        exit(-1);
    }
}

void remove_share_memory(int shm_id, data *shm_addr) {
    // Detach from shared memory
    if (shmdt(shm_addr) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    // Remove shared memory segment
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }
}


void signal_handler(int signo){
	if (signo == SIGUSR1) {
        //printf("Received SIGUSR1 signal from Guess program.\n");

        // guess
        if (target == D->guess) {
            sprintf(D->result, "bingo");
        } else if (D->guess > target) {
            sprintf(D->result, "smaller");
        } else {
            sprintf(D->result, "bigger");
        } 
        printf("[game] Guess %d, %s\n", D->guess, D->result);
        
        // if bingo clear share memory
        if (target == D->guess) {
            remove_share_memory(shm_id, D);
            game = 0;
            
        }
    }
}


void main(int argc, char** argv) {
	if (argc != 3){
		printf("Usage: ./game <key> <guess>");
		exit(EXIT_FAILURE);
	}

	printf("[Game] Game PID : %d\n", getpid());
	
	D = (data *)malloc(sizeof(data));    // initialized D
	
	key = atoi(argv[1]);
	target = atoi(argv[2]);
	
	memset(D->result, 0, sizeof(D->result));
	
	
	// create share memory
    create_share_memory();
    
    
    // signal
    struct sigaction action;
	memset(&action, 0, sizeof (action));
	action.sa_flags = SA_SIGINFO;
	action.sa_handler = signal_handler;
	if(sigaction(SIGUSR1, &action, NULL) == -1){
		perror("sigaction");
        exit(EXIT_FAILURE);
    }

    
	// printf("Game program started. Waiting for guesses...\n");
	
	while (game) {
        sleep(1);  // Wait for 1 second
    }
 
    return;
}
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
