#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(char *message);
void read_childproc(int sig);

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	
	pid_t pid;
	struct sigaction act;
	socklen_t adr_sz;
	int str_len, state;
	char buf[BUF_SIZE];
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	act.sa_handler=read_childproc;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	state=sigaction(SIGCHLD, &act, 0);
	// 너 자식 죽었다는 신호가 오면 sa_handler를 실행하라는 의미
	// read_childproc는 waitpid를 호출하는 것, 즉 자식이 리턴한 값을 받기만하면 좀비 프로세스는 생기지 않는다.
	
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	while(1)
	{
		adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
				// accept() 반복 허용을 이용해서 iter서버 구현
				// 클라이언트의 요청을 수락하면 아래 fork() 실행
		if(clnt_sock==-1)
			continue;
		else
			puts("new client connected...");
		pid=fork();	// 이후 프로세스 복사
					// 위의 accept를 실행한 정보를 갖고 복사된다.
		if(pid==-1)
		{
			close(clnt_sock);
			continue;
		}
		if(pid==0)	// 자식 프로세스가 동작하는 로직, 즉 입출력 서버 역할 수행
		{
			close(serv_sock);	// serv_sock도 마찬가지로 clnt_sock처럼 불필요하기 때문에 리소스 해제
			while((str_len=read(clnt_sock, buf, BUF_SIZE))!=0)
				write(clnt_sock, buf, str_len);
			
			close(clnt_sock);
			puts("client disconnected...");
			return 0;	// waitpid()로 넘어가는 종료 파라미터 0
		}
		else	// 부모 프로세스가 동작하는 로직
			close(clnt_sock);
			// clnt_sock는 accept시 생성되는데 자식 프로세스를 fork로 생성할 때 함께 복사되어 자식에게서도 존재
			// 그러므로 부모 프로세스에서의 clnt_sock은 필요없기 때문에 리소스를 해제하는 것!
	}
	close(serv_sock);
	return 0;
}

void read_childproc(int sig)
{
	pid_t pid;
	int status;
	pid=waitpid(-1, &status, WNOHANG); // 좀비가 안생기게 하기위한 코드, 자식이 죽을 때 날리는 리턴값이 오기만하면 좀비 안생김
	printf("removed proc id: %d \n", pid);
}
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}