#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE "/dev/etx_device"

int c;
int fd, ret;
char input;
int array[4] = {0};
int number;
int flag = 0;
// 0 main list
// 1 distance (shop list)
// 2 shop name (order)
// 3 menu_dessert
// 4 menu_beverage
// 5 menu_diner
// 6 confirm	
			
//flag = 0;
void main_manu(){  
	printf("\n1. shop list\n");
	printf("2. order\n");
	while ((c = getchar()) != '\n' && c != EOF);  // clear buffer
	scanf("%c", &input);	
	if (input == '1'){
		flag = 1;
		return;
	}else if (input == '2'){
		flag = 2;
		return;
	}	
	return;
}

//flag = 1
void show_distance(){  
	printf("\nDessert shop: 3km\n");
	printf("Beverage shop: 5km\n");
	printf("Diner: 8km\n");
	while ((c = getchar()) != '\n' && c != EOF);
	scanf("%c", &input);
	flag = 0;
	return;
}

//flag = 2;
void shop_name(){ 
	printf("\nPlease choose from 1~3\n");
	printf("1. Dessert shop\n");
	printf("2. Beverage shop\n");
	printf("3. Diner\n");
	while ((c = getchar()) != '\n' && c != EOF);
	//sleep(1);
	scanf("%c", &input);
	if (input == '1'){
		flag = 3;
		return;
	}else if (input == '2'){
		flag = 4;
		return;
	}else if (input == '3'){
		flag = 5;
		return;
	}
	return;
}

int confirm(int *array, int prize1, int prize2){
	int total_distance[2] = {0};
	int total = 0;
	printf("\nPlease wait for a few minutes...\n");
	total = array[0] * array[1] * prize1 + array[2] * array[3] * prize2;
	total_distance[0] = total;
	total_distance[1] = flag;
	total = 0;
	ret = write(fd, total_distance, sizeof(total_distance));
	if (ret < 0) {
	    perror("Failed to write to the device file");
	    close(fd);
	    return -1;
	}
	return 0;
}

//flag = 3;
void dessert_shop(){ 
	printf("\nPlease choose from 1~4\n");
	printf("1. cookie: $60\n");
	printf("2. cake: $80\n");
	printf("3. confirm\n");
	printf("4. cancle\n");
	while ((c = getchar()) != '\n' && c != EOF);
	scanf("%c", &input);	
	if (input == '1'){
		printf("How many?\n");
		while ((c = getchar()) != '\n' && c != EOF);
		scanf("%d", &number);
		array[0] = 1;
		array[1] += number;
		return;
	}else if (input == '2'){
		printf("How many?\n");
		while ((c = getchar()) != '\n' && c != EOF);
		scanf("%d", &number);
		array[2] = 1;
		array[3] += number;
		return;
	}else if (input == '3'){
		if ((array[0] == 0) && (array[2] == 0)){
			flag = 0;
			return;
		}
		confirm(array, 60, 80);
		for(int i = 0; i < 4; i++){
			array[i] = 0;
		}
		flag = 0;
		sleep(3);
		return;
	}else if (input == '4'){
		for(int i = 0; i < 4; i++){
			array[i] = 0;
		}
		flag = 0;
		return;
	}
	return;
}

//flag = 4;
void beverage_shop(){ 
	printf("\nPlease choose from 1~4\n");
	printf("1. tea: $40\n");
	printf("2. boba: $70\n");
	printf("3. confirm\n");
	printf("4. cancle\n");
	while ((c = getchar()) != '\n' && c != EOF);
	scanf("%c", &input);	
	if (input == '1'){
		printf("How many?\n");
		while ((c = getchar()) != '\n' && c != EOF);
		scanf("%d", &number);
		array[0] = 1;
		array[1] += number;
		return;
	}else if (input == '2'){
		printf("How many?\n");
		while ((c = getchar()) != '\n' && c != EOF);
		scanf("%d", &number);
		array[2] = 1;
		array[3] += number;
		return;
	}else if (input == '3'){
		if ((array[0] == 0) && (array[2] == 0)){
			flag = 0;
			return;
		}
		confirm(array, 40, 70);
		for(int i = 0; i < 4; i++){
			array[i] = 0;
		}
		flag = 0;
		sleep(5);
		return;
	}else if (input == '4'){
		for(int i = 0; i < 4; i++){
			array[i] = 0;
		}
		flag = 0;
		return;
	}
	return;
}

//flag = 5;
void diner(){ 
	printf("\nPlease choose from 1~4\n");
	printf("1. fried rice: $120\n");
	printf("2. egg-drop soup: $50\n");
	printf("3. confirm\n");
	printf("4. cancle\n");
	while ((c = getchar()) != '\n' && c != EOF);
	scanf("%c", &input);	
	if (input == '1'){
		printf("How many?\n");
		while ((c = getchar()) != '\n' && c != EOF);
		scanf("%d", &number);
		array[0] = 1;
		array[1] += number;
		return;
	}else if (input == '2'){
		printf("How many?\n");
		while ((c = getchar()) != '\n' && c != EOF);
		scanf("%d", &number);
		array[2] = 1;
		array[3] += number;
		return;
	}else if (input == '3'){
		if ((array[0] == 0) && (array[2] == 0)){
			flag = 0;
			return;
		}
		confirm(array, 120, 50);
		for(int i = 0; i < 4; i++){
			array[i] = 0;
		}
		flag = 0;
		sleep(8);
		return;
	}else if (input == '4'){
		for(int i = 0; i < 4; i++){
			array[i] = 0;
		}
		flag = 0;
		return;
	}
	return;
}

int _flag(){
	switch (flag){
		case 0:
			main_manu();
			return 0;
		case 1:
			show_distance();
			return 0;
		case 2:
			shop_name();
			return 0;
		case 3:
			dessert_shop();
			return 0;
		case 4:
			beverage_shop();
			return 0;
		case 5:
			diner();
			return 0;
	}
	return 0;
}


int main(int argc, char *argv[]) {
    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device file");
        return -1;
    }  
	while (1){
		_flag();
	}
	return 0;
}
