//////////////////////////////////////////////////////////////////
//
//  Copyright(C), 2013-2017, GEC Tech. Co., Ltd.
//
//  File name: GPLE/voicectl.c
//
//  Author: GEC
//
//  Date: 2017-01
//  
//  Description: 获取语音指令，根据指令完成相应动作
//
//
//////////////////////////////////////////////////////////////////

#include "common.h"
#include <sys/ioctl.h>
  
#define TEST_MAGIC 'x'                           //定义幻数
#define TEST_MAX_NR 2                            //定义命令的最大序数

//定义LED的魔幻数
#define LED1 _IO(TEST_MAGIC, 0)              
#define LED2 _IO(TEST_MAGIC, 1)
#define LED3 _IO(TEST_MAGIC, 2)
#define LED4 _IO(TEST_MAGIC, 3)



//#define REC_CMD  "./arecord -d4 -c1 -r16000 -traw -fS16_LE cmd.pcm"
#define REC_CMD  "arecord -d3 -c1 -r16000 -traw -fS16_LE cmd.pcm"
#define PCM_FILE "./cmd.pcm"
/* -d：录音时长（duration）
-c：音轨（channels）
-r：采样频率（rate）
-t：封装格式（type）
-f：量化位数（format） */
int said=0;

void catch(int sig)
{
	if(sig == SIGPIPE)
	{
		printf("killed by SIGPIPE\n");
		exit(0);
	}
}

int init_asr(char* argv)
{
	int sockfd = init_sock(argv); //tcp
	return sockfd;
}

int run_asr(int sockfd) // ./wav2pcm ubuntu-IP
{
	int id_num=0;
	//初始化UDP的socket
	//int udp_sockfd=udp_init();

	// 1，调用arecord来录一段音频
	printf("please to start REC in 3s...\n");

	// 在程序中执行一条命令  “录音的命令”
	system(REC_CMD);

	// 2，将录制好的PCM音频发送给语音识别引擎
	send_pcm(sockfd, PCM_FILE);

	// 3，等待对方回送识别结果（字符串ID）
	xmlChar *id = wait4id(sockfd);
	
	if(id == NULL)
	{
		printf("arecord fail\n");
		return -1;
	}
	id_num=atoi((char *)id);
	return id_num;
	
}

void uinit_asr(int sockfd)
{
	close(sockfd);
}
/*
int main(int argc,char*argv[])
{
	//if(argc == 2)
		printf("123456argc == %d\n", argc);
	int sockfd = init_asr(argv[1]);
	int id = run_asr(sockfd);
	printf("id == %d\n",id);
	
	uinit_asr(sockfd);

	return 0;
} 
*/