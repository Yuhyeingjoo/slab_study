#include "types.h"
#include "stat.h"
#include "user.h"


int main(){

	int s[10000];

	for(int i = 0 ; i < 10000; i++){
		s[i] = i;
	}

	printf(1, "%d\n", s[9999]);
	printf(1, "COMPLETE\n");

	exit(0);	

}
