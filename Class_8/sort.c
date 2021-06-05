#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>
#include <time.h> 
#include <stdlib.h>

#define NR_CPU 2
#define RANGE 10
#define MAX_LENGTH 40

struct param {
	int start;
	int end;
	int *nums;
};

void *worker(void *args)
{
	//辅助线程中使用选择排序
	struct param *param=(struct param *)args;
	int i,j,k;
	for(i=param->start;i<param->end-1;i++)
	{
		j=i+1;
		k=i;
		for(;j<param->end;j++)
		{
			if(param->nums[j] < param->nums[k]) k=j;
		}
		if(k!=i)
		{
			j=param->nums[i];
			param->nums[i]=param->nums[k];
			param->nums[k]=j;
		}
	}
}

//输出数组
void showNums(int *nums)
{
	int i;
	for(i=0;i<MAX_LENGTH;i++)
	{
		printf("%d ",nums[i]);
	}
	printf("\n");
	return ;
}

//二路归并排序
void mergeSort(int *nums, int low, int mid, int high)
{
	int *temp = (int *)malloc(sizeof(int)*(high-low+1));
	int i,j,k;

	for(i=low,j=mid,k=0;i<mid && j<high ;)
	{
		if(nums[i] < nums[j])	temp[k++]=nums[i++];
		else	temp[k++]=nums[j++];
	}
	while(j<high)
	{
		temp[k++]=nums[j++];
	}
	while(i<mid)
	{
		temp[k++]=nums[i++];
	}
	
	for(i=low,k=0;i<high;)
	{
		//将临时数组里的值复制到原数组中
		nums[i++]=temp[k++];
	}
	free(temp);
}

//为数组生成随机数
void randNums(int *nums)
{
	int i;
	srand( (unsigned)time(NULL) );
	for(i=0;i<MAX_LENGTH;i++)
	{
		nums[i]=rand( )%MAX_LENGTH;
	}
}

int main()
{
	pthread_t workers[NR_CPU];
	struct param params[NR_CPU];

	int nums[MAX_LENGTH];
	randNums(nums);

	printf("Before sort\n");
	showNums(nums);

	int i;
	for(i=0;i<NR_CPU;i++)
	{
		struct param *param;
		param=&param[i];
		param->start=i*(MAX_LENGTH/NR_CPU);
		param->end=(i+1)*(MAX_LENGTH/NR_CPU);
		param->nums=nums;
		pthread_create(&workers[i], NULL, worker, param);
	}

	for(i=0;i<NR_CPU;i++)
	{
		//等待子线程结束
		pthread_join(workers[i], NULL);
	}

	printf("\nAfter selection sort\n");
	showNums(nums);
	mergeSort(nums, 0, MAX_LENGTH/2, MAX_LENGTH);

	printf("\nAfter merge sort\n");
	showNums(nums);

	return 0;
}
