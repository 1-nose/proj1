#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/input.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/i2c-dev.h>
#include <sys/time.h>


// peripheral register physical address
#define GPIO_PHY_BASEADDR  0x3F200000
#define GPIO_AREA_SIZE	4096	// PAGE_SIZE
#define GPIO_GPFSEL0	0x0000	// for gpio 0..9, MSB 2bits are reserved
// omit GPFSEL1..GPFSEL5
#define GPIO_GPSET0	0x001C	// gpio 0..31
#define GPIO_GPSET1	0x0020	// gpio 32..53
#define GPIO_GPCLR0	0x0028	// gpio 0..31
#define GPIO_GPCLR1	0x002C	// gpio 32..53
#define GPIO_GPLEV0	0x0034	// gpio 0..31
#define GPIO_GPLEV1	0x0038	// gpio 32..53
int SWITCH=0;

int clear(int fd);
void sig_handler(int signum);
void initLCD(int fd);
unsigned int memread(void *baseaddr, int offset);
int lcd_cmdwrite(int fd, unsigned char dat);
void sig_handler(int signum);
int lcd_datawrite(int fd, char dat[]);

int main(){
	/*unsigned int gpfsel0 = *((unsigned int *)(gpio_baseaddr+GPIO_GPFSEL0));
	gpsel0 = gpfsel | (1<<5);
	*((unsigned int *)(gpio_baseaddr+GPIO_GPFSEL0)) = gpfsel0;
	*((unsigned int *)(gpio_baseaddr+GPIO_GPFSEL0)) = (1<<5);
	sleep(1);
	*((unsigned int *)(gpio_baseaddr+GPIO_GPFSEL0)) = (1<<5);*/

	struct itimerval timval;
	timval.it_interval.tv_sec=0;	//一回のみのタイマー
	timval.it_interval.tv_usec=0;
	timval.it_value.tv_sec=1;
	timval.it_value.tv_usec=0;

	signal(SIGALRM,sig_handler);

	int fd;
	unsigned int p,q;
	void *gpio_baseaddr;
	fd=open("/dev/mem",O_RDWR);
	printf("a\n");
	if(fd==-1){
		perror("open:");
		exit(1);
	}
	gpio_baseaddr=mmap(NULL,4096,PROT_WRITE,MAP_SHARED,fd,GPIO_PHY_BASEADDR);

	int i2c,val,i;
	i2c=open("/dev/i2c-1",O_RDWR);
	printf("b\n");
	if(i2c==-1){
		perror("Error;open");
		exit(1);
	}
	val=ioctl(i2c,I2C_SLAVE,0x3e);
	initLCD(i2c);

	val=clear(i2c);
	while(1){
		p=memread(gpio_baseaddr,GPIO_GPLEV0);
		p=p&0x00400000;
		printf("%x\n",p);
		if(p==0){
			if(SWITCH==0){
				val=lcd_datawrite(i2c,"hello");
				printf("c\n");
				if(val==-1){
					perror("val");
					exit(1);
				}
				SWITCH=1;
			}
			setitimer(ITIMER_REAL,&timval,NULL);
		}
		if(SWITCH==0) {
			val=clear(i2c);
		}	
	}
	val=close(i2c);
	val=close(fd);
	return 0;
}

unsigned int memread(void *baseaddr, int offset)
{
    unsigned int *p;
    p = baseaddr+offset;
    return *p;	// read memory-mapped register
}

void initLCD(int fd)
{
	int i;
	unsigned char init1[]={ 0x38, 0x39, 0x14, 0x70, 0x56, 0x6c };
	unsigned char init2[]={ 0x38, 0x0c, 0x01 };

	usleep(100000);	// wait 100ms
	for (i=0;i<sizeof(init1)/sizeof(unsigned char);i++) {
		if(lcd_cmdwrite(fd, init1[i])!=2){
			printf("internal error1\n");
			exit(1);
		}
		usleep(50); // wait 50us
	}

	usleep(300000);	// wait 300ms

	for (i=0;i<sizeof(init2)/sizeof(unsigned char);i++) {
		if(lcd_cmdwrite(fd, init2[i])!=2){
			printf("internal error2\n");
			exit(1);
		}
		usleep(50);
	}
	usleep(2000);	// wait 2ms
}

int lcd_cmdwrite(int fd, unsigned char dat)
{
	unsigned char buff[2];
	buff[0] = 0;
	buff[1] = dat;
	return write(fd,buff,2);
}

void sig_handler(int signum){
	printf("hello\n");
	SWITCH=0;	
}

int lcd_datawrite(int fd, char dat[])
{
	int len;
	char buff[100];

	len = strlen(dat);  // don't count EOS (Null char)
	if (len>99) {printf("too long string\n"); exit(1); }
	memcpy(buff+1, dat, len);	// shift 1 byte, ignore EOS
	buff[0] = 0x40;	// DATA Write command
	return write(fd, buff, len+1);
}

int clear(int fd)
{
	int val = lcd_cmdwrite(fd, 1);
	usleep(1000);	// wait 1ms
	return val;
}