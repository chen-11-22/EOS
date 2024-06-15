#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define MAX_CLIENTS 5
#define MAX_COMMAND_LEN 256


char object[15];
int number;
int totalmoney = 0;
int money;
int shopname1 = 10;
int shopname2;
// 1 : dessert shop
// 2 : beverage shop
// 3 : diver
int time;
char totalmeal[26] = {0};
char totalmeal_temp[13] = {0};
char response[MAX_COMMAND_LEN];
int mealnumber[6] = {0};
int ind;
int order;

void sendtoclient(int client_socket, char *res){
	if (send(client_socket, res, 256, 0) < 0){
		perror("send error");
	}
    memset(res, 0, sizeof(res));
    return;
}

void main(int argc, char** argv) {
	if (argc != 2){
		printf("Usage: ./hw2 [port]");
		exit(EXIT_FAILURE);
	}
	int port = atoi(argv[1]);
    int server_socket;
    int client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addrlen = sizeof(client_addr);
    int opt = 1;
    char command[MAX_COMMAND_LEN];

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

        while (recv(client_socket, command, MAX_COMMAND_LEN, 0) > 0){
        	memset(response, '\0', sizeof(response));       
        	printf("Received command from client: %s\n", command);
        	
        	//confirm
        	if (strcmp(command, "confirm") == 0){
				if (totalmeal[0] != '\0'){
					strcpy(response, "Please wait a few minutes...\n");
					sendtoclient(client_socket, response);

					if (shopname1 == 1){
						sleep(3);
					} else if (shopname1 == 2){
						sleep(6);
					} else {
						sleep(8);
					}
					strcpy(response, "Delivery has arrived and you need to pay ");
					sprintf(response + strlen(response), "%d$\n", totalmoney);
					sendtoclient(client_socket, response);
					
					shopname1 = 10;
					totalmoney = 0;
					memset(totalmeal, 0, sizeof(totalmeal));
					close(client_socket);
				} else {
					strcpy(response, "Please order some meals\n");
					sendtoclient(client_socket, response);
				
				}
			// shop list	
			} else if (strcmp(command, "shop list") == 0) {
				strcpy(response, "Dessert shop:3km\n- cookie:$60|cake:$80\nBeverage shop:5km\n- tea:$40|boba:$70\nDiner:8km\n- fried-rice:$120|Egg-drop-soup:$50\n");
				sendtoclient(client_socket, response);
			// cancel	
			} else if (strcmp(command, "cancel") == 0){
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
							sprintf(totalmeal + d + 1 + strlen(object), "%d%s", mealnumber[ind], totalmeal + d + 2 + strlen(object));
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
    }
    return;
}

