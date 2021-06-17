#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define CAPACITY 4

int buffer1[CAPACITY];
int in1;
int out1;
int buffer2[CAPACITY];
int in2;
int out2;

int buffer1_is_empty()
{
	return in1 == out1;
}

int buffer1_is_full()
{
	return (in1 + 1)%CAPACITY == out1;
}

int get_item1()
{
	int item1;
	item1 = buffer1[out1];
	out1 = (out1+1)%CAPACITY;
	return item1;
}

void put_item1(int item1)
{
	buffer1[in1] = item1;
	in1 = (in1 + 1)%CAPACITY;
}

int buffer2_is_empty()
{
	return in2 == out2;
}

int buffer2_is_full()
{
	return (in2 + 2)%CAPACITY == out2;
}

int get_item2()
{
	int item2;
	item2 = buffer2[out2];
	out2 = (out2+2)%CAPACITY;
	return item2;
}

void put_item2(int item2)
{
	buffer2[in2] = item2;
	in2 = (in2 + 2)%CAPACITY;
}
pthread_mutex_t mutex1;
pthread_cond_t wait_empty_buffer1;
pthread_cond_t wait_full_buffer1;

pthread_mutex_t mutex2;
pthread_cond_t wait_empty_buffer2;
pthread_cond_t wait_full_buffer2;

#define ITEM_COUNT (CAPACITY*2)

//消费者
void *consume(void *arg)
{
	int i;
	int item;
	for(i=0;i<ITEM_COUNT;i++)
	{
		pthread_mutex_lock(&mutex2);
		while (buffer2_is_empty())
			pthread_cond_wait(&wait_full_buffer2, &mutex2);

		item = get_item2();
		printf("        consume item: %c\n",item);

		pthread_cond_signal(&wait_empty_buffer2);
		pthread_mutex_unlock(&mutex2);
	}
	return NULL;
}

//计算者
void *calculate(void *arg)
{
	int i;
	int item;
	for(i=0;i<ITEM_COUNT;i++)
	{
		//从buffer1中读入字符
		pthread_mutex_lock(&mutex1);
		while (buffer1_is_empty())
			pthread_cond_wait(&wait_full_buffer1, &mutex1);

		item = get_item1();
		//printf("    calc get item1: %c\n",item);

		pthread_cond_signal(&wait_empty_buffer1);
		pthread_mutex_unlock(&mutex1);
		
		//将小写字符转换为大写字符
		item -= 32;

		//从buffer2中写入字符
		pthread_mutex_lock(&mutex2);
		while (buffer2_is_full())
			pthread_cond_wait(&wait_empty_buffer2, &mutex2);

		put_item2(item);
		//printf("    calc put item2: %c\n",item);

		pthread_cond_signal(&wait_full_buffer2);
		pthread_mutex_unlock(&mutex2);

	}
	return NULL;
}

//生产者
void *produce(void *arg)
{
	int i;
	int item;

	for(i=0;i<ITEM_COUNT;i++)
	{
		pthread_mutex_lock(&mutex1);
		while (buffer1_is_full())
			pthread_cond_wait(&wait_empty_buffer1, &mutex1);

		item = 'a' + i;
		put_item1(item);
		printf("produce  item: %c\n",item);

		pthread_cond_signal(&wait_full_buffer1);
		pthread_mutex_unlock(&mutex1);
	}
	return NULL;

}

int main()
{

	pthread_t consumer_tid;
	pthread_t calculator_tid;

	pthread_mutex_init(&mutex1, NULL);
	pthread_cond_init(&wait_empty_buffer1, NULL);
	pthread_cond_init(&wait_full_buffer1, NULL);

	pthread_mutex_init(&mutex2, NULL);
	pthread_cond_init(&wait_empty_buffer2, NULL);
	pthread_cond_init(&wait_full_buffer2, NULL);

	pthread_create(&calculator_tid, NULL, calculate, NULL);
	pthread_create(&consumer_tid, NULL, consume, NULL);
	produce(NULL);
	pthread_join(consumer_tid, NULL);

	return 0;
}













