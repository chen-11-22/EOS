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
#define MAX_COMMAND_LEN 256
#define SEM_MODE 0666
#define SEM_KEY 123456789   // semaphore
#define SHM_KEY1 987654321   // share memory
#define SHM_KEY2 987654322   // share memory

int server_socket;
int client_socket;
struct sockaddr_in server_addr, client_addr;
int addrlen = sizeof(client_addr);
int opt = 1;
int status;
int shm_id;
int port;

char object[15];
int number;
int totalmoney = 0;
int money;
int shopname1 = 10;
int shopname2;
// 1 : dessert shop
// 2 : beverage shop
// 3 : diver
int *times1;
int *times2;
char totalmeal[26] = {0};
char totalmeal_temp[24] = {0};
char command[MAX_COMMAND_LEN];
char response[MAX_COMMAND_LEN];
int mealnumber[6] = {0};
int ind;
int order;
int sem;
//bool yes = false;
int deliveryman;

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

// remove semaphore
void cleansem(int signum) { 
    if (semctl (sem, 0, IPC_RMID, 0) < 0){
		fprintf (stderr, "unable to remove sem %d\n", SEM_KEY);
		exit(1);
	}
	printf("Semaphore %d has been remove\n", SEM_KEY);
	
	exit(0);
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

void create_semaphore(){
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
   
    // create shared memory 1
    shm_id = shmget(SHM_KEY1, sizeof(int), IPC_CREAT | 0666);
    if (shm_id < 0){
        fprintf(stderr, "Shared memory creation failed: %s\n", strerror(errno));
        exit(-1);
    }
    
    times1 = shmat(shm_id, NULL, 0);
    if (times1 == (void *) -1){
        fprintf(stderr, "Shared memory attach failed: %s\n", strerror(errno));
        exit(-1);
    }
    *times1 = 0;
    
    // create shared memory 2
    shm_id = shmget(SHM_KEY2, sizeof(int), IPC_CREAT | 0666);
    if (shm_id < 0){
        fprintf(stderr, "Shared memory creation failed: %s\n", strerror(errno));
        exit(-1);
    }
    
    times2 = shmat(shm_id, NULL, 0);
    if (times2 == (void *) -1){
        fprintf(stderr, "Shared memory attach failed: %s\n", strerror(errno));
        exit(-1);
    }
    *times2 = 0;
    return;
}

void create_socket(){
	// Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to allow address reuse
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind socket to address and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    return;
}

void sendtoclient(int client_socket, char *res){
	if (send(client_socket, res, 256, 0) < 0){
		perror("send error");
	}
	//printf("%s\n", res);
    memset(res, 0, sizeof(res));
    return;
}

void handle_client(int client_socket){
	while (recv(client_socket, command, MAX_COMMAND_LEN, 0) > 0){
    	memset(response, '\0', sizeof(response));       
    	//printf("Received command from client: %s\n", command);
    	
    	// yes
    	if (strcmp(command, "Yes\n") == 0){
			strcpy(response, "Please wait a few minutes...\n");
			sendtoclient(client_socket, response);
			if (deliveryman == 1){
				sleep(*times1);
			}else {
				sleep(*times2);
			}
			strcpy(response, "Delivery has arrived and you need to pay ");
			sprintf(response + strlen(response), "%d$\n", totalmoney);
			sendtoclient(client_socket, response);
			
			shopname1 = 10;
			totalmoney = 0;
			memset(totalmeal, 0, sizeof(totalmeal));
			close(client_socket);   	
    	//confirm
    	} else if (strcmp(command, "confirm\n") == 0){    // +\n
			if (totalmeal[0] != '\0'){
				// first delivery man
				if (*times1 <= *times2){
					//printf("confirm\n");
					deliveryman = 1;
					P(sem);
					if (shopname1 == 1){
						*times1 += 3;
					} else if (shopname1 == 2){
						*times1 += 6;
					} else {
						*times1 += 8;
					}
					V(sem);	
					// over 30 sec
					if (*times1 >= 30){
						strcpy(response, "Your delivery will take a long time, do you want to wait?\n");
						sendtoclient(client_socket, response);
					} else {
						strcpy(response, "Please wait a few minutes...\n");
						sendtoclient(client_socket, response);
						sleep(*times1);
						if (shopname1 == 1){
							*times1 -= 3;
						} else if (shopname1 == 2){
							*times1 -= 6;
						} else {
							*times1 -= 8;
						}
						strcpy(response, "Delivery has arrived and you need to pay ");
						sprintf(response + strlen(response), "%d$\n", totalmoney);
						sendtoclient(client_socket, response);
						
						shopname1 = 10;
						totalmoney = 0;
						memset(totalmeal, 0, sizeof(totalmeal));
						close(client_socket);
						//yes = 1;
					}
					
				//second delivery man
				} else {
					//printf("confirm\n");
					deliveryman = 2;
					P(sem);
					if (shopname1 == 1){
						*times2 += 3;
					} else if (shopname1 == 2){
						*times2 += 6;
					} else {
						*times2 += 8;
					}
					V(sem);	
					// over 30 sec
					if (*times2 >= 30){
						strcpy(response, "Your delivery will take a long time, do you want to wait?\n");
						sendtoclient(client_socket, response);
					} else {
						strcpy(response, "Please wait a few minutes...\n");
						sendtoclient(client_socket, response);
						sleep(*times2);
						if (shopname1 == 1){
							*times2 -= 3;
						} else if (shopname1 == 2){
							*times2 -= 6;
						} else {
							*times2 -= 8;
						}
						strcpy(response, "Delivery has arrived and you need to pay ");
						sprintf(response + strlen(response), "%d$\n", totalmoney);
						sendtoclient(client_socket, response);
						
						shopname1 = 10;
						totalmoney = 0;
						memset(totalmeal, 0, sizeof(totalmeal));
						close(client_socket);
					}
				}

				
			} else {
				strcpy(response, "Please order some meals\n");
				sendtoclient(client_socket, response);
			
			}
		// shop list
		} else if (strcmp(command, "shop list") == 0) {
			strcpy(response, "Dessert shop:3km\n- cookie:$60|cake:$80\nBeverage shop:5km\n- tea:$40|boba:$70\nDiner:8km\n- fried-rice:$120|Egg-drop-soup:$50\n");
			sendtoclient(client_socket, response);
		// cancel	
		} else if (strcmp(command, "cancel") == 0 || strcmp(command, "No") == 0){
			memset(totalmeal, 0, sizeof(totalmeal));
			shopname1 = 10;
			totalmoney = 0;
			close(client_socket);
		// order	
		} else if (sscanf(command, "order %s %d", object, &number) == 2){
			if (strcmp(object, "cookie") == 0){
				order = 1;
				ind = 0;
				mealnumber[0] += number;   // the number of this meal have been ordered
				shopname2 = 1;
				money = 60;
			} else if (strcmp(object, "cake") == 0){
				order = 2;
				ind = 1;
				mealnumber[1] += number;
				shopname2 = 1;
				money = 80;
			} else if (strcmp(object, "tea") == 0){
				order = 1;
				ind = 2;
				mealnumber[2] += number;
				shopname2 = 2;
				money = 40;
			} else if (strcmp(object, "boba") == 0){
				order = 2;
				ind = 3;
				mealnumber[3] += number;
				shopname2 = 2;
				money = 70;
			} else if (strcmp(object, "fried-rice") == 0){
				order = 1;
				ind = 4;
				mealnumber[4] += number;
				shopname2 = 3;
				money = 120;
			} else if (strcmp(object, "Egg-drop-soup") == 0){
				order = 2;
				ind = 5;
				mealnumber[5] += number;
				shopname2 = 3;
				money = 50;
			}
			// order for the first time
			if (shopname1 == 10){   
				shopname1 = shopname2;
				sprintf(totalmeal + strlen(totalmeal), "%s %d\n", object, number);
				strcpy(response, totalmeal);
				sendtoclient(client_socket, response);
				totalmoney += money * number;	
			} else{
				if (shopname1 == shopname2){
					// no repeat meal
					if (strstr(totalmeal, object) == NULL) {
						// order inverse
						if (order == 1){  
							strcpy(totalmeal_temp, totalmeal);
							sprintf(totalmeal, "%s %d|%s", object, number, totalmeal_temp);
						} else {
							sprintf(totalmeal + strlen(totalmeal) - 1, "|%s %d\n", object, number);
						}
						strcpy(response, totalmeal);
						sendtoclient(client_socket, response);
						totalmoney += money * number;
					// repeat meal	
					} else { 
						int d = strstr(totalmeal, object) - totalmeal;
						// number of meal > 10
						if (mealnumber[ind] >= 10 && (mealnumber[ind]-number) < 10){
							strcpy(totalmeal_temp, totalmeal + d + 2 + strlen(object));
							//sprintf(totalmeal + d + 1 + strlen(object), "%d%d%s", mealnumber[ind]/10, mealnumber[ind]%10, totalmeal_temp);
							sprintf(totalmeal + d + 1 + strlen(object), "%d%s", mealnumber[ind], totalmeal_temp);
						} else if (mealnumber[ind] >= 10){
							strcpy(totalmeal_temp, totalmeal + d + 3 + strlen(object));
							sprintf(totalmeal + d + 1 + strlen(object), "%d%s", mealnumber[ind], totalmeal_temp);
						}else{
							sprintf(totalmeal + d + 1 + strlen(object), "%d%s", mealnumber[ind], totalmeal + d + 2 + strlen(object));
							
						}
						strcpy(response, totalmeal);
						sendtoclient(client_socket, response);
						totalmoney += money * number;
					}
						
				}else {
				strcpy(response, totalmeal);
				sendtoclient(client_socket, response);
				}
			}
		}
    	memset(command, 0, sizeof(command));
    	memset(response, '\0', sizeof(response));	
    }
	return;
}


void main(int argc, char** argv) {
	if (argc != 2){
		printf("Usage: ./hw2 [port]");
		exit(EXIT_FAILURE);
	}
	port = atoi(argv[1]);
    
    // set up signal handler
    setup_signal_handler();
    // create semaphore and two share memory
    create_semaphore();
    // Create socket
    create_socket();
    

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    while (1) {
        // Accept incoming connection
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
  
        memset(command, 0, sizeof(command));
        
        pid_t pid = fork();  
        if (pid >= 0){       	 		
		    if (pid == 0) {
		        // Child process            
		        //close(server_socket); // Close unused socket in child process
		        handle_client(client_socket);
		        waitpid(pid, &status, 0);
		        exit(EXIT_SUCCESS);
		    } else {
				// Parent process
		        close(client_socket); // Close unused socket in parent process
		    }
		} else {
			perror("Fork failed");
            exit(EXIT_FAILURE);
        }
 
    }
    return;
}

