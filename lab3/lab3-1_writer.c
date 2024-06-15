#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define LED_DEVICE "/dev/etx_device" 

int main(int argc, char *argv[]) {
    int fd, ret;
    char data[9];
    int num;

    if (argc != 2) {
        printf("Usage: %s <number>\n", argv[0]);
        return -1;
    }

    // open
    fd = open(LED_DEVICE, O_RDWR);  // 開啟資料夾(可讀寫)
    if (fd < 0) {
        perror("Failed to open the device file");
        return -1;
    }

    // write
    strcpy(data, argv[1]);
    for (int i = 0; i < 9; i++){
        int num = data[i] - '0';
		ret = write(fd, &num, sizeof(int));
		if (ret < 0) {
		    perror("Failed to write to the device file");
		    close(fd);
		    return -1;
		}
		printf("%d ", num);
		sleep(1);
    }
    printf("Data written successfully\n");

    // close
    close(fd);
    return 0;
}

