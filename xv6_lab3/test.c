#include "types.h"
#include "stat.h"
#include "user.h"

void work(int n, int g) {

	for( ; ; ){
		printf(1, "%d:%d ", n, g);
	}

	exit(1);
}

	
int
main (int argc, char * argv[])
{

	int num_proc;

	if(argc < 2){
		num_proc = 5;
	} else {
		num_proc = argc;
	}

	int pid[num_proc];
	for(int i = 0 ; i < num_proc ; i++){
		pid[i] = fork();
		if( pid[i] < 0 ){
			exit(1);
		} else if ( pid[i] == 0 ){
			int prio = 0;
			int ret_g = 0;
			if( (ret_g = getpriority(pid[i], &prio)) < 0 ){
				exit(1);
			}
			printf(1, "getprio result :%d | %d\n", ret_g, prio);
			int p = getpid();
			work(p, prio);
		}
	}
	sleep(1000000);


	for(  int i = 0 ; i < num_proc ; i++ ){
	       int ret_s = 0;

	       int prio = 10;
       	       if( (ret_s = setpriority(pid[i], prio)) < 0 ){
			exit(1);
	       }
		
	}	       
	wait(0x0);
	exit(0);
}
