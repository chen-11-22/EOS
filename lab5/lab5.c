#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>


#define MAX_CLIENTS 5

void handler(int signum) {
	while (waitpid(-1, NULL, WNOHANG) > 0);
	return;
}

void handle_client(int client_socket) {
   	printf("Train ID: %d\n", getpid());
    dup2(client_socket, STDOUT_FILENO);
    close(client_socket);
    execlp("sl", "sl", "-l", NULL);
}

void main(int argc, char** argv) {
	if (argc != 2){
		printf("Usage: ./lab5 [port]");
		exit(EXIT_FAILURE);
	}
	int port = atoi(argv[1]);
    int server_fd;
    int client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addrlen = sizeof(client_addr);
    int opt = 1;

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to allow address reuse
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind socket to address and port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);
    
    signal(SIGCHLD, handler);

    while (1) {
        // Accept incoming connection
        if ((client_socket = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        // Fork to handle client
        pid_t pid = fork();  
        if (pid >= 0){       	 		
		    if (pid == 0) {
		        // Child process            
		        close(server_fd); // Close unused socket in child process
		        handle_client(client_socket);
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

