#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define BUFFER_SIZE 18

void main(int argc, char** argv) {
	if (argc != 6){
		printf("Usage: ./client <ip> <port> <deposit/withdraw> <amount> <times>");
		exit(EXIT_FAILURE);
	}
	char *ip = argv[1];
	int port = atoi(argv[2]);    // string to int
	char *operation = argv[3];
	char *amount = argv[4];
	char *times = argv[5];
	
    //int server_socket;
    int client_socket;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr;

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // transform ip to binary
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("error ip");
        exit(EXIT_FAILURE);
    }

    // connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }
    
    printf("connect successfully, ip = %s, port = %d\n", ip, port);
    
    sprintf(buffer, "%s %s %s", operation, amount, times);
    
    if (send(client_socket, buffer, BUFFER_SIZE, 0) < 0){
		perror("send error");
		close(client_socket);
        exit(EXIT_FAILURE);
	}
	
	printf("message send : %s\n", buffer);
    memset(buffer, 0, BUFFER_SIZE);
    
    close(client_socket);
    return;
}
    
    
    
    
    
    
    
    
    
    
    
    
    

	
