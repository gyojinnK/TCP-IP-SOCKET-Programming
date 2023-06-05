#include <stdio.h>
#include <unistd.h>
int gval=10;

int main(int argc, char *argv[])
{
	pid_t pid;
	int lval=20;
	gval++, lval+=5;
	
	pid=fork();	// 프로세스를 복사하는 함수	
	if(pid==0)	// if Child Process
		gval+=2, lval+=2;
	else			// if Parent Process
		gval-=2, lval-=2;
	
	if(pid==0)
		printf("Child Proc: [%d, %d] \n", gval, lval);
	else
		printf("Parent Proc: [%d, %d] \n", gval, lval);

	sleep(10);

	return 0;
}
// 복사된 자식 프로세스와 부모 프로세스의 return값은 다르다.
// 자식의 return 값 : 0
// 부모의 return 값 : 자식 pid 