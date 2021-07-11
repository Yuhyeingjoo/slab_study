#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include<pthread.h>
#include <netinet/tcp.h>
pthread_mutex_t mutex;
int theNumberOf = 0;
struct socket_list{
	struct socket_list * prev;
	int conn;
	struct socket_list *next;
};
struct socket_list * front = 0x0;
void newSocket(int new_socket){
	struct socket_list* New;
	New = (struct socket_list*)malloc(sizeof(struct socket_list));
	New -> conn = new_socket;
//	pthread_mutex_lock(&mutex);
	if(theNumberOf==0){
		front = New;
		theNumberOf++;
	}
	else{
		struct socket_list *cur = front; 
		while(1){
			if(cur->next==NULL){
				cur->next = New;
				New->prev = cur;
				theNumberOf++;
				break;
			}
			cur = cur->next;
		}
	}
//	pthread_mutex_unlock(&mutex);
}

void echoAll(char* data, int len){
//	pthread_mutex_lock(&mutex);
	struct socket_list *cur = front;
	while(cur!=NULL){
		if(send(cur->conn,data,len,0)<len){
			printf("Not sended all data\n");
		}
		cur=cur->next;
	}
//	pthread_mutex_unlock(&mutex);
}

void delete_socket(int dead){
//	pthread_mutex_lock(&mutex);
	struct socket_list *cur = front;
	while(cur!=NULL){
		if(cur->conn == dead){
			if(front ==cur){
				if(cur->next!=NULL){
					front = cur->next;
					front->prev = NULL;
				}
			}
			else if(cur->next ==NULL){
				cur->prev->next = NULL;
			}
			else{
				cur->prev->next = cur->next;
				cur->next->prev = cur->prev;
			}
			free(cur);
			theNumberOf--;
			break;	
		}
		cur = cur->next;
	}
//	pthread_mutex_lock(&mutex);
}
struct thread_argument{
	int con;
	char *key;
};



void receive_echo(int conn){
	char buf[1024]; char *data = 0x0, *orig = 0x0;
	int len, s;
	while(1){
		data = 0x0;

		if ( (s = recv(conn, buf, 1023, 0)) > 0 ) {
			buf[s] = 0x0 ;
			if (data == 0x0) {
				data = strdup(buf) ;
				len = s ;
			}
			else {
				data = realloc(data, len + s + 1) ;
				strncpy(data + len, buf, s) ;
				data[len + s] = 0x0 ;
				len += s ;
			}
	
		}
		if(data == 0x0){
			char *ret;
			delete_socket(conn);
			pthread_exit(ret);
		}
		printf(">%s\n", data) ;
		
		
		
		orig = data ;
		echoAll(data,len);
	}
	free(orig);
}

void
*child_proc(void *received)
{
	char buf[1024] ;
	struct	thread_argument *t_argu = (struct thread_argument*)received;
	int conn = t_argu->con;
	char * data = 0x0, * orig = 0x0 ;
	int len = 0 ;
	int s ;
	receive_echo(conn);
	if (orig != 0x0) 
		free(orig) ;
}

int 
main(int argc, char const *argv[]) 
{ 
	int listen_fd, new_socket ; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 

	char buffer[1024] = {0}; 

	listen_fd = socket(AF_INET /*IPv4*/, SOCK_STREAM /*TCP*/, 0 /*IP*/) ;
	if (listen_fd == 0)  { 
		perror("socket failed : "); 
		exit(EXIT_FAILURE); 
	}
	
	memset(&address, '0', sizeof(address)); 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY /* the localhost*/ ; 
	address.sin_port = htons(8095); 
	if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed : "); 
		exit(EXIT_FAILURE); 
	} 

	while (1) {
		if (listen(listen_fd, 16 /* the size of waiting queue*/) < 0) { 
			perror("listen failed : "); 
			exit(EXIT_FAILURE); 
		} 
		
		new_socket = accept(listen_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen) ;
		
		if (new_socket < 0) {
			perror("accept"); 
			exit(EXIT_FAILURE); 
		} 
		
		newSocket(new_socket);

		int opt_val = 1;
		int stat = setsockopt(new_socket, IPPROTO_TCP, TCP_NODELAY, (void*)&opt_val, sizeof(opt_val));
	
		pthread_t threadd;
		struct thread_argument * t_argu;
		t_argu = (struct thread_argument*)malloc(sizeof(struct thread_argument));
		t_argu->con = new_socket;

		if(pthread_create(&threadd, 0x0,child_proc,(void*)t_argu)<0){
			printf("thread error\n");
			exit(1);
		}
	}
} 

