#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>

#define NR_CPU 5
#define RANGE 1000000

struct param {
	int start;
	int end;
};

struct result {
	double sum;
};

void *worker(void *arg)
{
	struct param *param=(struct param *)arg;
	struct result *result=(struct result *)malloc(sizeof(struct result));
	result->sum=0;
	int i;
	double temp=0;
	//printf("start=%d,end=%d\n",param->start,param->end);
	for(i=param->start;i<param->end;i++)
	{
		temp = i%2 ? 1.0 : -1.0;
		result->sum+=temp/(i*2-1);
	}
	return result;
}

int main()
{
	pthread_t workers[NR_CPU];
	struct param params[NR_CPU];

	double total,PI;
	int i;
	for(i=0;i<NR_CPU;i++)
	{
		struct param *param;
		param=&params[i];
		param->start=i*RANGE+1;
		param->end=(i+1)*RANGE+1;
		//printf("i=%d,start=%d,end=%d\n",i,param->start,param->end);
		pthread_create(&workers[i], NULL, worker, param);
	}

	for(i=0;i<NR_CPU;i++)
	{
		struct result *result;
		pthread_join(workers[i],(void **)&result);
		//printf("worker[%d].result=%.10lf\n",i,result->sum);
		total += result->sum;
		free(result);
	}
	//printf("total=%.10lf\n",total);
	
	PI = total*4;
	printf("NR_CPU=%d RANGE=%d\n",NR_CPU,RANGE);
	printf("PI=%.10lf\n",PI);
	return 0;
}
