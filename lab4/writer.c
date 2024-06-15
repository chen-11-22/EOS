#include <stdio.h>        // fprintf(), perror()
#include <stdlib.h>       // exit()
#include <string.h>       // memset()
#include <signal.h>       // signal()
#include <fcntl.h>        // open()
#include <unistd.h>       // read(), write(), close()

# define DEVICE_NAME "/dev/my_dev"

int main(int argc, char *argv[]) {
    int fd, ret;
    char data[5] = {'\0'};
    char letter;

    if (argc != 2) {
        printf("Usage: %s <name>\n", argv[0]);
        return -1;
    }

    // open
    fd = open(DEVICE_NAME, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device file");
        return -1;
    }

    // write
    strcpy(data, argv[1]);  
    for (int i = 0; i < 5; i++){
    	if (data[i] == '\0'){
			break;
		}else{
			ret = write(fd, &data[i], sizeof(char));
			if (ret < 0) {
				perror("Failed to write to the device file");
				close(fd);
				return -1;
			}
		}
		printf("%c ", data[i]);
		sleep(1);
    }
    printf("Data written successfully\n");

    // close driver
    close(fd);

    return 0;
}

