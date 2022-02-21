#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include "paint.h"

lcd_fd = 0;

int readpic(char *filename){
	int i,j;
	for(i=0;i<800;i++)
		for(j=0;j<480;j++)
			COLOR[i][j]=0;
	char bm[2];
	short bit;
	int c;
	FILE *file;
	file=fopen(filename,"r");
	fread(bm,sizeof(char),2,file);
	if(bm[0]!='B'||bm[1]!='M')//判断文件类型，若不是BMP图像，则停止下面操作
		return 0;
	fseek(file,18,0);
	fread(&CW,sizeof(int),1,file);//读取bmp图像的宽
	fread(&CH,sizeof(int),1,file);//读取bmp图像的高
	fseek(file,28,0);	
	fread(&bit,sizeof(short),1,file);
	if (bit!=24){
		printf("%i\n",bit);
		return 1;
	}
	fseek(file,52,0);
	c=CW*3%4;
	int count=0,y,x;
	if(CH<=480&&CW<=800){
		for(y=CH-1;y>=0;y--){//读取图像三通道像素，并保存在数组中
			for (x=0;x<CW;x++){
				unsigned char r,g,b;
				fread(&g,sizeof(unsigned char),1,file);
				fread(&r,sizeof(unsigned char),1,file);
				fread(&b,sizeof(unsigned char),1,file);
				COLOR[x][y]=256*256*256*255+256*256*r+256*g+b;
				count++;
				fseek(file,52+count*3,0);
			}
			if(c)	fseek(file,4-c,SEEK_CUR);
		}
	}else return -1;
	fclose(file);
	return 2;
}
int lcd_init()
{
	lcd_fd = open("/dev/fb0", O_RDWR);
	if(lcd_fd == -1)
	{
		perror("open lcd fail");
		return -1;
	}
	plcd = mmap(NULL, 800*480*4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
	if(plcd == NULL)
	{
		perror("mmap fail");
		return -1;
	}
	return 0;
}

void lcd_uninit()
{
	munmap(plcd, 800*480*4);
	close(lcd_fd);
}

void lcd_draw_point(unsigned int color, int x, int y)
{
	if(x>=0 && x<800 && y>=0 && y<480)
		*(plcd + y*800 + x) = color;	
}

void draw_white(int dx,int dy)
{	
	int i,j;
	if(CH+dy<=480 && CW+dx<=800){
		for(i=0; i<CH; i++)
		{
			for(j=0; j<CW; j++)
			{
				lcd_draw_point(COLOR[j][i], j+dx, i+dy);
			}
		}
	}
}

void clear_screen(){
	int i,j;
	for(i=0; i<480; i++)
	{
		for(j=0; j<800; j++)
		{
			lcd_draw_point(0xFF000000, j, i);
		}
	}
}
