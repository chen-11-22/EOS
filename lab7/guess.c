// guess.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <errno.h>

typedef struct {
    int guess;
    char result[8];
} data;

data *D;
int shm_id;


void send_signal(int pid) {
    // Send SIGUSR1 signal
    if (kill(pid, SIGUSR1) == -1) {
        perror("kill");
        exit(EXIT_FAILURE);
    }
}

void create_shared_memory(int key) {
    // Attach to the existing shared memory segment
    shm_id = shmget(key, sizeof(data), 0666);
    if (shm_id < 0) {
        fprintf(stderr, "Shared memory access failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Attach to the shared memory segment
    D = (data *)shmat(shm_id, NULL, 0);
    if (D == (void *) -1) {
        fprintf(stderr, "Shared memory attach failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
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

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <key> <upper_bound> <pid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int key = atoi(argv[1]);
    int high = atoi(argv[2]);
    int game_pid = atoi(argv[3]);
    int low = 1;
    int guess;

    // Create and attach shared memory
    create_shared_memory(key);

   
    // Binary search to guess the number
    while (1) {
    	if (strcmp(D->result, "bingo") == 0){
    		break;
    	} else if(strcmp(D->result, "smaller") == 0){
    		high = D->guess;
    	} else if(strcmp(D->result, "bigger") == 0){
    		low = D->guess;
    	}   	
        guess = (low + high) / 2;
        printf("[game] Guess : %d\n", guess);
        D->guess = guess;
        send_signal(game_pid);  // Notify Game program to evaluate
        sleep(1);        
    }
    return 0;
}

