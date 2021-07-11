#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <sys/socket.h>
#include "sys/types.h"
#include <errno.h>
#include<sys/inotify.h>
#include<unistd.h>
#include "regex.h"
#include<pthread.h>
#include<stdbool.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN 	(1024*(EVENT_SIZE+16))
struct meta_data{
	int version;
	int len;
	char* name;
	struct meta_data * next;
};
struct meta_data * front = 0x0;
struct meta_data * checker=0x0;
int theNumberOf =0;
char ** modify_array;
char ** local_modify;
int ipv4Conn(char*ip, char*port);
void put_send(int sock_fd, char* file_name);

void inotify_send(char *ip_port, char* name);
void new_list(char*file_name, int v_from_server ){
        theNumberOf++;
        struct meta_data * New;
        New = (struct meta_data*)malloc(sizeof(struct meta_data));
        New->len = strlen(file_name);
        New->name = (char*)malloc(strlen(file_name));
        strcpy(New->name,file_name);
        New->version=v_from_server;
        New->next = NULL;
        struct meta_data *cur;
        cur = front;
        if(cur ==0x0){
                        front = New;
			checker = front;
        }
        else{
                while(1){
                        if(cur->next==NULL){
                                cur->next = New;
                                break;
                        }

                        cur=cur->next;

                }
        }

}


int send_mes(int sock_fd, void* header, int len);
void send_put_data(int sock_fd, char* file_name){
	FILE * fp = fopen(file_name,"r");
	if(fp==NULL){
		printf("No such FIle\n");
		exit(1);
	}
	fseek(fp, 0, SEEK_END);    // 파일 포인터를 파일의 끝으로 이동시킴
	int size = ftell(fp);          // 파일 포인터의 현재 위치를 얻음
	rewind(fp);
	char buffer[8];
	char *big_buf, *orig;
	big_buf = (char*)malloc(sizeof(char)*size);
	orig = big_buf;
	strcpy(big_buf,"");
	
	fread(big_buf,sizeof(char),size,fp);
	int s = send_mes(sock_fd, big_buf,size);
	
	
	

}
void recv_fixed_len(int sock_fd, int len, void* buf){
	buf = (char*)buf;
	int s;
	while(len>0 && (s = recv(sock_fd,buf,len,0))>0){
			buf+=s;
			len-=s;
		}
}
char*  recv_mes(int sock_fd, char* data){
	char buf[1024] ;
	int len = 0, s=0 ;
	data = 0x0;

	while ( (s = recv(sock_fd, buf, 1023, 0)) > 0 ) {
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
	return data;
}
int send_mes(int sock_fd,void* header, int len ){
	int  s=0, size=0;
	
	while (len > 0 && (s = send(sock_fd,header, len, 0)) > 0) {
		header += s ;
		len -= s ;
		size+=s ;
	}
	return size;
}
void get_file(char * file_name, char * data){
		FILE * fp =fopen(file_name,"w");
		fwrite(data,strlen(data),1,fp);
		fclose(fp);
}
void reg_check( char * ip_port){
	regex_t state;
	const char* ip4_pattern = "^[0-9]+.[0-9]+.[0-9]+.[0-9]+:[0-9]+$";
	regcomp(&state,ip4_pattern,REG_EXTENDED);
	int status = regexec(&state,ip_port,0,NULL,0);

	if(status ==0){
	}
	else{
		printf("Ip or Port error\n");
		exit(1);
	}
}
bool regular_file_check(char name[]){
	if(name[0]!='.' && strcmp("4913", name) && name[strlen(name)-1] !='~')
		return true;
	else
		return false;
}
int add_local_modify(char * name ,int len){
	for(int i=0; i<len; i++){
		if(strcmp(local_modify[i],name)==0)
			return len;
	}
	local_modify[len] = (char*)malloc(strlen(name));
	strcpy(local_modify[len],name);
	len++;
	return len;
}
void list_find(char * name){
	struct meta_data *c;
	c = front;
	while(c!=NULL){
		if(strcmp(name, c->name)==0)
			return ;
	}
	new_list(name, 0);	
}
void inotify(char * ip_port){
	int length, i=0, fd, wd, local_change_n=0, prev=0;
	char buffer[EVENT_BUF_LEN];
	fd = inotify_init();
	if(fd<0){
		perror("inotify_init");
	}
	wd = inotify_add_watch(fd,"./", IN_CREATE | IN_MODIFY);
	while(1){
		length = read(fd, buffer, EVENT_BUF_LEN);
		if(length<0)
			perror("read");
		while(i<length){
				
			struct inotify_event *event = (struct inotify_event *) &buffer[i];
				
			if( regular_file_check(event->name) && (event->mask & IN_CREATE || event->mask &IN_MODIFY) ){
				local_change_n = add_local_modify(event->name, local_change_n);
				printf(" in inofy : %s   %d\n",event->name, local_change_n);

			}
			i += EVENT_SIZE + event->len;

		}
		i=0;
		if(local_change_n>prev){
			for(int j=0; j<local_change_n; j++){
//				printf("inotify send : %d %s\n",j, local_modify[j]);
				list_find(local_modify[j]);
				inotify_send(ip_port,local_modify[i]);
				local_change_n = 0;
			}

		}
		prev =local_change_n; 
	}

}


void get_send(int sock_fd, char * file_name){
	int list_type = 2;
	int file_name_len = strlen(file_name)+1;
	send_mes(sock_fd, &list_type,4);
	send_mes(sock_fd, &file_name_len,4);
	send_mes(sock_fd, file_name,file_name_len);

}
int recv_metadata_check(int sock_fd){
	int theServerNumber, name_len, given_version, modify_array_count = 0;
	char * name;
	recv_fixed_len(sock_fd, 4,&theServerNumber);
	modify_array = (char**)malloc(sizeof(char*)*theNumberOf);
	for(int i=0; i<theServerNumber; i++){
		recv_fixed_len(sock_fd, 4, &name_len);
		name = (char*)malloc(name_len);
		recv_fixed_len(sock_fd,name_len , name);
		recv_fixed_len(sock_fd, 4, &given_version);
		
		if(theNumberOf>i){
			printf("hrere\n");
			if(checker->version!= given_version){
				modify_array[modify_array_count] = (char*)malloc(name_len);
				strcpy(modify_array[modify_array_count], name);
				modify_array_count++;
			}
			checker= checker->next;

		}
		else{
			new_list(name, given_version);
			modify_array[modify_array_count] = (char*)malloc(name_len);
			strcpy(modify_array[modify_array_count], name);
			modify_array_count++;

		}
	}

	checker = front;
	struct meta_data *c;
        c =front;
	while(c!=NULL){
                printf("%s\n",c->name);
                c=c->next;
        }
	return modify_array_count;
		
}
int update_version(char* given_name,int new_version){
        struct meta_data *c;
        c =front;
        while(c!=NULL){
                if(strcmp(c->name, given_name)==0){
                        c->version = new_version;
			return 0;
                }
                c=c->next;
        }
}

void local_update( int modify_num , char* ip, char* port){
	int version;
	char * data;
	data = 0x0;
	for(int i=0; i<modify_num; i++){
		printf("modif_change : %s\n",modify_array[i]);	
		int conn = ipv4Conn(ip,port);
		get_send(conn, modify_array[i]);

		shutdown(conn, SHUT_WR) ;
		recv_fixed_len(conn,4,&version);
		data = recv_mes(conn, data);
		get_file(modify_array[i],data);
		
		update_version(modify_array[i], version);
	}
}

void put_send(int sock_fd, char* file_name){
	int list_type = 3;
	int file_name_len = strlen(file_name)+1;
	
	send_mes(sock_fd, &list_type,4);
	send_mes(sock_fd, &file_name_len,4);
	send_mes(sock_fd, file_name,file_name_len);
	send_put_data(sock_fd, file_name);

	
}
void inotify_send(char *ip_port, char* name){
	char* tmp;
	int version;
	tmp = (char*)malloc(strlen(ip_port));
	strcpy(tmp, ip_port);
	char * ip = strtok(tmp,":");
	char * port =strtok(NULL,":");
	int sock_fd = ipv4Conn(ip,port);
	put_send(sock_fd , name);
	recv_fixed_len(sock_fd, 4, &version);	
	shutdown(sock_fd, SHUT_WR) ;
	update_version(name, version);
}
int ipv4Conn(char * ip, char* port ){
	int sock_fd;
	struct sockaddr_in serv_addr;
	char* file_name;
	int port_num=0;	
	port_num = atoi(port);	
	sock_fd = socket(AF_INET, SOCK_STREAM, 0) ;
	if (sock_fd <= 0) {
		perror("socket failed : ") ;
		exit(EXIT_FAILURE) ;
	}

	memset(&serv_addr, '\0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port_num);
	if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
		perror("inet_pton failed : ") ;
		exit(EXIT_FAILURE) ;
	}

	if (connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {

		perror("connect failed : ") ;
		exit(EXIT_FAILURE) ;
	}
	return sock_fd;

}

void* regular_check(void* ipPort){
		char* ip_port;
		ip_port = (char*)ipPort;
		char *tmp;
		tmp = (char*) malloc(strlen(ip_port));
		strcpy(tmp,ip_port);
		char * ip = strtok(tmp,":");
		char * port =strtok(NULL,":");

	while(1){

		int conn = ipv4Conn(ip,port);
		sleep(1);

		printf("regular %d \n", conn);
		int list_type = 1;
		send_mes(conn,&list_type,4);
		int s=  recv_metadata_check(conn);
		shutdown(conn,SHUT_WR);	
		local_update(s, ip, port);	
		struct meta_data *c;
		c =front;
		while(c!=NULL){
			printf("in list :%s %d\n", c->name, c->version);
			c=c->next;
		}

	}

}
int main(int argc, char* argv[]){
	local_modify = (char**)malloc(sizeof(char*));	
	reg_check(argv[1]);
	char* ipPort;
	ipPort = (char*)malloc(strlen(argv[1]));
	strcpy(ipPort,argv[1]);	
//	char * ip = strtok(argv[1],":");
//	char * port =strtok(NULL,":");

//	int sock_fd ;
//	int s, len ;
//	char buffer[1024] = {0};
//	char * data ;
//	char * instruction, *argument;
//	instruction = (char*)malloc(strlen(argv[2]));	
//	strcpy(instruction,argv[2]);

//	if(argc==4){
//		argument = (char*)malloc(strlen(argv[3]));
//		strcpy(argument, argv[3]);
//	}
//	int list_type;
	pthread_t threadd;
	
	pthread_create(&threadd, 0x0, regular_check, (void*)ipPort);

		


	inotify(ipPort);
/*	
//	while(1){	
		sock_fd  = ipv4Conn(ip,port);
		data = 0x0 ;
		if(strcmp("list",instruction)==0){
			list_type =1;
			send_mes(sock_fd,&list_type,4);
		}

		if(strcmp("get",instruction)==0){
			list_type =2;
			get_send(sock_fd, argument);
			data = recv_mes(sock_fd,data);
		}
		if(strcmp("put",instruction)==0){
			list_type =3;
			put_send(sock_fd,argument);		

		}

		shutdown(sock_fd, SHUT_WR) ;
		
		if(list_type==1){
			recv_metadata_check(sock_fd);
		}
		if(list_type==2&&data!=0x0){
			get_file(argv[3],data);	
		}
//		sleep(1);
//	}
	*/

	pthread_join(threadd,NULL);
}
