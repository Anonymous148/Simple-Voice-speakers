#include "common.h"
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include "light.h"
#include "paint.h"

int V_wait; //等待语音线程
int V_I,T_I; //接收语音接收的返回值 
pthread_t pt1,pt2,pt3,pt4;
int sockfd;
int Info,Awake=0,AwakeTime=0;
char *commandbuf="null";
int isdoor=0,islight=0,ismusic=0; //记录当前状态

//LED灯事件
void LED(int num){
	int n;
	if(num==8)  n=1; //全开
	if(num==7)  n=0; //全关
	if(num>=9&&num<=11) n=num+9; //部分关
	if(num>=12&&num<=14) n=num-4; //部分开
	DisplayLight(n);
}
//播放音乐
void *PlayMusic(void *arg){
	pthread_detach(pthread_self());
	//printf("playmusic thread\n");
	system("madplay -Q ./music/bgm.mp3");//音乐路径
	return NULL;
}
//关闭音乐
void CloseMusic(){
	//printf("stopmusic thread\n");
	system("pkill madplay");
}
//接收语音线程
void *Sent_commandbuf(void *arg){
	//create socket
	int sockFd = socket(PF_INET,SOCK_STREAM,0);
	if(sockFd < 0)
	{
		perror("create socket error!");
		return -1;
	}
	printf("create socket OK!\n");
	//请求连接
	struct sockaddr_in servAddr = {0};
	servAddr.sin_family = PF_INET;
	servAddr.sin_port = htons(4008);
	servAddr.sin_addr.s_addr = inet_addr("192.168.1.100");
	int ret = connect(sockFd,(struct sockaddr *)&servAddr,sizeof(servAddr));
	if(ret < 0)
	{
		perror("connect error!");
		close(sockFd);
		return -1;
	}
	//printf("connect OK!\n");
	//write
	//printf("write to server:");
	//char buf[2048];
	while(1)
	{
		if(strcmp(commandbuf,"null")){
			ret = write(sockFd,commandbuf,50);
			if(ret < 0)
			{
				perror("write error!");
				close(sockFd);
				return -1;
			}
			commandbuf="null";
		}
	}
	//close
	close(sockFd);
}

//接收语音线程
void *Accept_asr(void *arg){
	//printf("accept_asr thread\n");
	pthread_detach(pthread_self());
	while(1){	
		V_I=run_asr(sockfd);
	}
	return NULL;
}
void *TickScreen(void *arg){
	pthread_detach(pthread_self());
	struct input_event ev;
	int x=-1, y=-1;
	int touch_fd = open("/dev/input/event0", O_RDONLY);
	if(touch_fd < 0)
	{
		perror("open touch fail");
		return -1;
	}
	while(1){
		read(touch_fd, &ev, sizeof(ev));
		if(ev.type == EV_ABS && ev.code == ABS_X)
			x = ev.value*800/1024;
		if(ev.type == EV_ABS && ev.code == ABS_Y)
			y = ev.value*800/1024;
		if(ev.type == EV_KEY && ev.code == BTN_TOUCH)
		{
			if(ev.value == 1)
			{
				printf("%d %d\n",x,y);
				if(Awake==0){
					T_I=100; //唤醒
				}
				else if(Awake==1&&x>500&&x<=800&&y>90&&y<450){
					//根据点击的范围确定Info的取值
					Info=4;
					if(y>90&&y<=200) //开关门
					{
						if(isdoor==0) isdoor=1,T_I=4;
						else if(isdoor==1) isdoor=0,T_I=3;
					}
					if(y>200&&y<=300) //开关灯
				    {
						if(islight==0) islight=1,T_I=8;
						else if(islight==1) islight=0,T_I=7;
					}
					if(y>300&&y<450) //播放停止音乐
					{
						if(ismusic==0) ismusic=1,T_I=5;
						else if(ismusic==1) ismusic=0,T_I=6;
					}
				}
				x=-1; y=-1;
			}
		}
	}
	close(touch_fd);
	return NULL;
}
void Operate(int num){ //4开门， 3关门 5开音乐 6关音乐 7关灯 8开灯
	if(num==3){
		commandbuf="closedoor";
		readpic("./picture/closedoor.bmp");
		draw_white(0,0); //绘制图片
	}else if(num==4){
		commandbuf="opendoor";
		readpic("./picture/opendoor.bmp");
		draw_white(0,0); //绘制图片
	}else if(num==5){
		commandbuf="startmusic";
		readpic("./picture/musicstart.bmp");
		draw_white(0,0); //绘制图片
		pthread_create(&pt3,NULL,PlayMusic,NULL);
	}else if(num==6){
		commandbuf="stopmusic";
		readpic("./picture/musicstop.bmp");
		draw_white(0,0); //绘制图片
		CloseMusic();
	}else if(num==7){
		commandbuf="lightoff";
		readpic("./picture/lightoff.bmp");
		draw_white(0,0); //绘制图片
		LED(num);
	}else if(num==8){
		commandbuf="lighton";
		readpic("./picture/lighton.bmp");
		draw_white(0,0); //绘制图片
		LED(num);
	}else if(num>=3&&num<=11){ //关部分灯
		readpic("./picture/lightoff.bmp");
		draw_white(0,0); //绘制图片
		LED(num);
	}else if(num>=3&&num<=14){ //开部分灯
		readpic("./picture/lighton.bmp");
		draw_white(0,0); //绘制图片
		LED(num);
	}else if(num==999) //退出
	{
		Awake=0;
	}
}

int main(int argc,char *argv[]){
	int vs=3,ve=14;
	//struct input_event ev;
	sockfd = init_asr(argv[1]); //初始化语言接收
	T_I=-1;V_I=-1;
	lcd_init();
	pthread_create(&pt1,NULL,Accept_asr,NULL);
	pthread_create(&pt2,NULL,TickScreen,NULL);
	pthread_create(&pt4,NULL,Sent_commandbuf,NULL);
	//置屏幕为黑色
	clear_screen();
	while(1){
		//printf("===============================================================\n");
		//重置参数
		if(Awake==0&&(V_I==100 || T_I==100)){
			Awake=1;
			readpic("./picture/operate.bmp");
			draw_white(500,0); //绘制图片
			readpic("./picture/robot.bmp");
			draw_white(0,0);
			T_I=-1;	V_I=-1;
		}else if(V_I==999){
			Awake=0;
			clear_screen();
			V_I=-1;
		}else if(Awake==1){
			if(V_I>=vs&&V_I<=ve){
				Info=V_I; 
				if(Info==4) isdoor=1; if(Info==3) isdoor=0;
				if(Info==8) islight=1; if(Info==7) islight=0;
				if(Info==5) ismusic=1; if(Info==6) isdoor=0; 
				Operate(Info);			
			}
			if(T_I>=vs&&T_I<=ve){
				Info=T_I; 
				if(Info==4) isdoor=1; if(Info==3) isdoor=0;
				if(Info==8) islight=1; if(Info==7) islight=0;
				if(Info==5) ismusic=1; if(Info==6) isdoor=0; 
				Operate(Info);
				T_I=-1;V_I=-1;
			}		
		}
		
	}

	/*while(1){
		Operate(V_I);
	}*/
		
	uinit_asr(sockfd);    //语音接收
	lcd_uninit();
	return 0;
}

