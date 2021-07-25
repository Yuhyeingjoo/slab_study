#include "types.h"
#include "stat.h"
#include "user.h"

void
loop(int num)
{
	while(1){
		for(int i = 0 ; i < num ; i++){
			printf(1, "  ");
		}
		printf(1,"%d\n",num);
	}
	exit(0);
}
int
main()
{
	int i;
	int pid[10];
	for(i = 0; i< 10; i++){
		pid[i] = fork();
		if(pid[i] < 0){
			printf(2,"fork failed\n");
		}
		if(pid[i] == 0){
			loop(i);
		}
	}
	sleep(500);
	if(setpriority(getpid(),1) == -1){
		printf(2,"setpriority failed(range or no pid)\n");
		exit(1);
	}
	for(i = 0; i < 10; i++) {
		if(setpriority(pid[i],1) == -1){
			printf(2,"setpriority failed(range or no pid)\n");
			exit(1);
		}
		sleep(100);
	}
	printf(1, "ALL IN!\n") ;
	for(i = 10; i >= 0; i--) {
		if (kill(pid[i]) == -1) {
			printf(2, "kill failed\n") ;
		}
		sleep(100) ;
	}
	for(i = 0; i < 10; i++) {
		wait(0x0) ;
	}
	printf(1, "ALL KILLED!\n") ;
	exit(0);
}

