#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

double worker_output;
void *worker(void *arg)
{
	int i;
	double temp=0;
	worker_output=0;
	for(i=1000000;i<2000000;i++)
	{
		temp = i%2 ? 1.0 : -1.0;
		worker_output+=temp/(i*2-1);
	}
	return NULL;
}

double master_output;
void master()
{
	int i;
	double temp=0;
	master_output=0;
	for(i=1;i<1000000;i++)
	{
		temp = i%2 ? 1.0 : -1.0;
		master_output+=temp/(i*2-1);
	}
}

int main()
{
	pthread_t worker_tid;
	double total,PI;

	pthread_create(&worker_tid, NULL, worker, NULL);
	master();
	pthread_join(worker_tid, NULL);//等待子线程结束
	total = worker_output+master_output;
	PI = total*4;

	printf("worker_output=%.10lf\n",worker_output);
	printf("master_output=%.10lf\n",master_output);
	printf("PI=%.10lf\n",PI);
	return 0;
}
