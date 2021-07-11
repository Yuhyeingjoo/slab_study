#include<pthread.h>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include<signal.h>
#include<stdbool.h>
#define DEBUG 1
char  instruct[128];
int turn, mode=0;

void get_char(char*);
void find_wake(int);
pthread_mutex_t mutex, mutex2, instruct_m,turn_m, atomic_inst;
int count = 0;
struct list{
        int id;
        int conn;
	pthread_cond_t cond;
        struct list* next;
};
struct list* front = 0x0;
int list_n=0;
void close_all(){
	struct list* cur ;
	pthread_mutex_lock(&mutex2);
	cur = front;
	while(cur!=NULL){
		close(cur->conn);
		cur= cur->next;
	}

	pthread_mutex_unlock(&mutex2);
}

void list_search(){
        struct list * cur;
        cur = front;
        while(cur!=NULL){
                printf("list search :%d\n",cur->id);
                cur = cur->next;
        }

}


struct list* new_list(int sock){
        struct list * New;
        New = (struct list*)malloc(sizeof(struct list));
        New->id = list_n++;
        New->conn = sock;
        New->next = NULL;
	
	pthread_cond_t cd;
	New->cond = cd;
	pthread_cond_init(&New->cond,NULL);
        struct list* cur = 0x0;
	pthread_mutex_lock(&mutex2);
        cur = front;

        if(cur==0x0){
                front = New;
		pthread_mutex_unlock(&mutex2);
        }
        else{
                while(1){
                        if(cur->next== NULL){
                                cur->next = New;
				pthread_mutex_unlock(&mutex2);
                                break;
                        }
                        cur = cur->next;
                }
        }
//        list_search();
	return New;
}

bool find_id(int ID, int sock){
        struct list * cur, *orig;
	orig = cur;

	pthread_mutex_lock(&mutex2);
        cur = front;
        while(cur!=NULL){
                if(cur->id == ID && cur->conn == sock){
			free(orig);
			pthread_mutex_unlock(&mutex2);
                        return true;
                }
        }
	pthread_mutex_unlock(&mutex2);
        return false;

}


void handler(int sig){
	mode = 1;	

}

int recv_fixed_len(int sock_fd, int len, void* buf){
        buf = (char*)buf;
        char *tmp = buf;
        int s;
        while(len>0 && (s = recv(sock_fd,tmp,len,0))>0){
#if DEBUG==0
        printf("DEBUG in FUNCTION: %s\n",tmp);
#endif
                tmp+=s;
                len-=s;
                }
	return s;
        
}

void send_mes(int conn, void* data, int len){
        int s;
        char* checker;
        checker =(char*) data;
        while(len>0 && (s=send(conn,checker,len,0))>0){
                checker+=s;
                len-=s;
        }
}

void get_char(char * instruct){
	int i=0;
	char c;
	pthread_mutex_lock(&instruct_m);
	for(;(c = getc(stdin)) !='\n'; i++){
		instruct[i] =c;
	}
	instruct[i] =0x0;

	pthread_mutex_unlock(&instruct_m);
}
void send_cmd(int conn){
	pthread_mutex_lock(&instruct_m);
	int cmd_len = strlen(instruct) ;
	send_mes(conn,&cmd_len,sizeof(int));
	send_mes(conn,instruct, cmd_len);
	pthread_mutex_unlock(&instruct_m);
}


void* receiver(void* sock){
	int conn = *(int*)sock;
	char buffer[128];
	int s;
	while((s=recv(conn,buffer,127,0)) >0){
		while(mode==1);
		buffer[s] = 0x0;
		printf("%s",buffer);
	
	}
	
	shutdown(conn, SHUT_WR) ;
	
}


void find_wake(int turn){
	struct list* c;
	c = front;
	while(c!=NULL){
		if(turn==c->id){
			pthread_cond_signal(&c->cond);
		}
		c=c->next;
	}
}
void
*child_proc(void* sock_fd)
{	

	int conn = *(int*)sock_fd;

	struct list* New = new_list(conn);
	while(1){	
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&New->cond,&mutex);
#if DEBUG==0
	printf("inttruction : %s\nlen :%d",instruct,str_len);	
#endif
		send_cmd(conn);
		pthread_mutex_unlock(&atomic_inst);
		pthread_mutex_unlock(&mutex);
	}
		
}
void quit(char* inst){
	if(strcmp(inst,"q")==0){
		close_all();
		exit(1);	
	}
		
}
void* UI ( void* s){
	char blank;
	while(1){
		if(mode==1){
			pthread_mutex_lock(&atomic_inst);
			get_char(instruct);
			quit(instruct);
		        scanf("%d",&turn);
		        scanf("%c",&blank);
			find_wake(turn);
			mode=0;
		}
	}
 }
int ipv4_tcp(){
	int listen_fd;
	listen_fd = socket(AF_INET /*IPv4*/, SOCK_STREAM /*TCP*/, 0 /*IP*/) ;
	if (listen_fd == 0)  { 
		perror("socket failed : "); 
		exit(EXIT_FAILURE); 
	}
	return listen_fd;
}
int 
main(int argc, char const *argv[]) 
{ 
	signal(SIGINT, (void*)handler);
	pthread_mutex_init(&mutex,NULL);
	pthread_mutex_init(&atomic_inst,NULL);
	pthread_t ui_thread;
	if(pthread_create(&ui_thread,0x0, UI,(void*)NULL)<0)
	{printf("ui error\n"); exit(1);}

	int listen_fd, new_socket ; 
	struct sockaddr_in address; 
	int port = atoi(argv[1]);
	int addrlen = sizeof(address); 
	listen_fd = ipv4_tcp();
	memset(&address, '0', sizeof(address)); 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY /* the localhost*/ ; 
	address.sin_port = htons(port); 
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
		pthread_t threadd, thread2;
		if(pthread_create(&thread2,0x0, receiver, (void*)&new_socket)<0){
			exit(1);
		} 
		if(pthread_create(&threadd, 0x0, child_proc, (void*)&new_socket)<0)
			exit(1);
	}
} 
