#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
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

#define MAX_CLIENTS 5
#define BUFFER_SIZE 18
#define SEM_MODE 0666
#define SEM_KEY 123456789
#define SHM_KEY 987654321

int sem;
char buffer[BUFFER_SIZE];
char operation[10];
int amount;
int times;
int *totalmoney;
//*totalmoney = 0;
//int *ptr = &totalmoney;

// acquire
int P(int s){
	struct sembuf sop; // the operation parameters
	sop.sem_num = 0;
	sop.sem_op = -1;
	sop.sem_flg = 0;
	if (semop (s, &sop, 1) < 0) {
		fprintf(stderr,"P(): semop failed: %s\n",strerror(errno));
		return -1;
	} else {
		return 0;
	}
}

// release
int V(int s){
	struct sembuf sop;    // the operation parameters
	sop.sem_num = 0;
	sop.sem_op = 1;
	sop.sem_flg = 0;
	if (semop(s, &sop, 1) < 0) {
		fprintf(stderr,"V(): semop failed: %s\n",strerror(errno));
		return -1;
	} else {
		return 0;
	}
}


// clear zombie
void cleanzombie(int signum) {    
	while (waitpid(-1, NULL, WNOHANG) > 0);
	return;
}

// remove semaphore
void cleansem(int signum) { 
    if (semctl (sem, 0, IPC_RMID, 0) < 0){
		fprintf (stderr, "unable to remove sem %d\n", SEM_KEY);
		exit(1);
	}
	printf("Semaphore %d has been remove\n", SEM_KEY);
	
	exit(0);
}


void bank(int operation, int amount, int times){
	for (; times > 0; times--){
		P(sem);
		*totalmoney += operation * amount;
		if (operation == 1){
			printf("After deposit : %d\n", *totalmoney);
		} else {
			printf("After withdraw : %d\n", *totalmoney);	
		}
		V(sem);	
		sleep(0.1);
	}	
	return;
}

void handle_client(int client_socket) {
	while (1){
		while (recv(client_socket, buffer, BUFFER_SIZE, 0) > 0){
			//printf("%s\n", buffer);
			sscanf(buffer, "%s %d %d", operation, &amount, &times);
			
			// deposit
			if (strcmp(operation, "deposit") == 0){
				bank(1, amount, times);
				return;
			// withdraw	
			} else {
				bank(-1, amount, times);
				return;
			}
		}
	}	
}


void setup_signal_handler() {
    struct sigaction sa;
    sa.sa_handler = cleansem;  // set handle signal function = cleanup
    sigemptyset(&sa.sa_mask);  // clear signal???
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) { // set SIGINT handle signal
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}


void main(int argc, char** argv) {
	if (argc != 2){
		printf("Usage: ./server <port>");
		exit(EXIT_FAILURE);
	}
	int port = atoi(argv[1]);
    int server_socket;
    int client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addrlen = sizeof(client_addr);
    int opt = 1;
    int status;
    int shm_id;
    
    // set up signal handler
    setup_signal_handler();
    
    // create semaphore
    sem = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | SEM_MODE);
	if (sem < 0){
		fprintf(stderr, "Sem %d creation failed: %s\n", SEM_KEY,
		strerror(errno));
		exit(-1);
	}
	
	// initial semaphore value to 1 (binary semaphore)
	if ( semctl(sem, 0, SETVAL, 1) < 0 ){
		fprintf(stderr, "Unable to initialize Sem: %s\n", strerror(errno));
		exit(0);
	}
	printf("Semaphore %d has been created & initialized to 1\n", SEM_KEY);

    
    // create shared memory
    shm_id = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0666);
    if (shm_id < 0){
        fprintf(stderr, "Shared memory creation failed: %s\n", strerror(errno));
        exit(-1);
    }
    
    totalmoney = shmat(shm_id, NULL, 0);
    if (totalmoney == (void *) -1){
        fprintf(stderr, "Shared memory attach failed: %s\n", strerror(errno));
        exit(-1);
    }
    *totalmoney = 0;
    
    
    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to allow address reuse
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind socket to address and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);
    
    signal(SIGCHLD, cleanzombie);   // when child process ends
    
    while (1) {
        // Accept incoming connection
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        // Fork to handle client
        pid_t pid = fork();  
        if (pid >= 0){       	 		
		    if (pid == 0) {
		        // Child process            
		        //close(server_socket); // Close unused socket in child process
		        handle_client(client_socket);
		        waitpid(pid, &status, 0);
		        //break;
		        exit(EXIT_SUCCESS);
		    } else {
				// Parent process
				//handle_client(client_socket);
				//break;
				//exit(EXIT_SUCCESS);
		        close(client_socket); // Close unused socket in parent process
		    }
		} else {
			perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        
	}
    return;
}
