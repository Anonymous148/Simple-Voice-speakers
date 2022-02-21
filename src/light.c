#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "light.h"

//输入参数 num=0时灯全灭，为1时灯全亮， 输入8-10时对应的1-3号灯亮 输入18-20时对应的1-3灯灭
int DisplayLight(int num){
	int fd,ret,i;
	char led_ctrl[2]; 
	fd = open("/dev/led_drv",O_RDWR);
	if(fd < 0){
		printf("open led_drv failed\n");
		return -1;
	}
	if(num==0){
		for(i=8;i<11;i++){
			led_ctrl[0] = 0,led_ctrl[1] = i; 
		 	ret = write(fd,led_ctrl,sizeof(led_ctrl));	
		}
	}else if(num==1){
		for(i=8;i<11;i++){
			led_ctrl[0] = 1,led_ctrl[1] = i; 
		 	ret = write(fd,led_ctrl,sizeof(led_ctrl));	
		}
	}else if(num>=8&&num<=10){
		led_ctrl[0] = 1,led_ctrl[1] = num; 
		ret = write(fd,led_ctrl,sizeof(led_ctrl));		
	}else if(num>=18&&num<=20){
		led_ctrl[0] = 0,led_ctrl[1] = num-10; 
		ret = write(fd,led_ctrl,sizeof(led_ctrl));	
	}
	close(fd);
	return 0;
}
