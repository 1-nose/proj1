include<stdio.h>
include<unistd.h>
include<signal.h>
int main()
{
	int n;
	struct itimerval timval;
	timval.it_interval.tv_sec=1;
	timval.it_interval.tv_usec=1;
	timval.it_value.tv_sec=1;
	timval.it_value.tv_usec=0;
	settimer(ITIMER_REAL,&timval,NULL);
	if(signal(SIGALRM,sig_handler)== SIG_ERR) perror("コメント");
	while(1)
	{
		printf("z");
		printf("hello");
		sleep(1);
	}
}
