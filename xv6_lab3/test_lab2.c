#include "types.h"
#include "stat.h"
#include "user.h"

int
main (int argc, char * argv[])
{
	int pid ;
	printf(1, "test start!\n") ;

	pid = fork() ;
	if (pid < 0) {
		printf(2, "pid1: ERROR: fork") ;
		exit(1) ;
	}
	else if (pid == 0) {
		printf(1, "pid1: child run!\n") ;
		exit(0) ;
	}
	else {
		printf(1, "parent start!\n") ;

		int result = getpriority(pid);
		printf(1, "getprio result :%d\n", result);
		result = setpriority(pid, 3);
		printf(1, "setprio result :%d\n", result);
		result = getpriority(pid);
		printf(1, "getprio result :%d\n", result);
		int pid2 = fork();
		if( pid2 < 0) {
			printf(2, "pid2: ERROR: fork") ;
			exit(1);
		} else if ( pid2 == 0){
			int exit_code ;
			int child_pid = waitpid(pid, &exit_code, 0) ;
			printf(1, "pid1: child %d end w/ exit code %d\n", child_pid, exit_code) ;
			printf(1, "pid2: child run!\n");
			exit(0);
		} else {
			int exit_code ;
			int child_pid = waitpid(pid2, &exit_code, 0) ;
			printf(1, "pid2: child %d end w/ exit code %d\n", child_pid, exit_code) ;
			printf(1, "pid2: parent end!\n") ;
			exit(0);
		}
	}


}
