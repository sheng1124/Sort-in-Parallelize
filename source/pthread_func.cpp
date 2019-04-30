//complie g++ -std=c++11 chronometer.cpp data_set.cpp pthread_func.cpp sort_main.cpp -lpthread -o hw3

#include "../header/sort_parallel.h"
#include <iostream>
#include <pthread.h>
#include <vector>

using namespace std;

pthread_t threads[NUM_THREAD];
pthread_mutex_t critial_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int running_threads = 0;

void * pd1(void *para)
{
	struct ThreadPara *para_ = static_cast<struct ThreadPara*>(para);
	
	DataSet *threadISet = (*para_).threadISetPtr;
	DataSet *motherDataSet = (*para_).motherDataSetPtr;
	
	int motherSize_ = (*motherDataSet).size_;
	int index = (*para_).threadIndex;
	int start = index * (motherSize_ / NUM_THREAD);
	int end;
	
	if(index != NUM_THREAD - 1)
	{
		end = (index + 1) * (motherSize_ / NUM_THREAD);
	}
	else
	{
		end = motherSize_;
	}
	
	(*threadISet).set(end - start, 0);
	
	for(int i = start; i < end; i++)
	{
		(*threadISet).list[i - start] = (*motherDataSet).list[i];
	}
	
	(*threadISet).qSort();

	pthread_mutex_lock(&critial_mutex);
	running_threads--;
	pthread_mutex_unlock(&critial_mutex);
	pthread_exit(NULL);
}

void * pd2(void *para)
{ 

	struct ThreadPara *para_ = static_cast<struct ThreadPara*>(para) ;
	DataSet *motherDataSet = (*para_).motherDataSetPtr;
	DataSet (*sortedThreadSet)[NUM_THREAD] = (*para_).sortedThreadSetPtr;
	DataSet (*mergeSet)[NUM_THREAD - 2] = (*para_).mergeSetPtr;
	
	int motherSize_ = (*motherDataSet).size_;
	int threadIndex = (*para_).threadIndex;
	
	if(threadIndex < NUM_THREAD / 2)//8set merge into 4set
	{
	
		vector<int> *listPtri = &((*sortedThreadSet)[2 * threadIndex].list);
		vector<int> *listPtrj = &((*sortedThreadSet)[2 * threadIndex + 1].list);
		
		int sizei = (*sortedThreadSet)[2 * threadIndex].size_;
		int sizej = (*sortedThreadSet)[2 * threadIndex + 1].size_;
		int *size_ = &((*mergeSet)[threadIndex].size_);
		
		(*mergeSet)[threadIndex].set(sizei + sizej, 0);
		int i = 0, j = 0;
		*size_ = 0;
	
		while(i < sizei && j < sizej)
		{
			if((*listPtri)[i] <= (*listPtrj)[j])
			{ 
				(*mergeSet)[threadIndex].list[*size_] = (*listPtri)[i];
				(*size_)++;
				i++;
			}
			else
			{
				(*mergeSet)[threadIndex].list[*size_] = (*listPtrj)[j];
				(*size_)++;
				j++;
			}
		}
		
		while(i < sizei)
		{
			(*mergeSet)[threadIndex].list[*size_] = (*listPtri)[i];
			(*size_)++;
			i++;
		}
		
		while(j < sizej)
		{
			(*mergeSet)[threadIndex].list[*size_] = (*listPtrj)[j];
			(*size_)++;
			j++;
		}		
	}
	else if(threadIndex != NUM_THREAD - 2)//4set merge into 2set
	{
		int queueiIndex = 2 * threadIndex - NUM_THREAD;
		int queuejIndex = 2 * threadIndex - NUM_THREAD + 1;
		DataSet *queuePtri = &((*mergeSet)[queueiIndex]);
		DataSet *queuePtrj = &((*mergeSet)[queuejIndex]);
		
		int sizei = (*sortedThreadSet)[queueiIndex * 2].size_ + (*sortedThreadSet)[queueiIndex * 2 + 1].size_;
		int sizej = (*sortedThreadSet)[queuejIndex * 2].size_ + (*sortedThreadSet)[queuejIndex * 2 + 1].size_;
		int *size_ = &((*mergeSet)[threadIndex].size_);
		
		(*mergeSet)[threadIndex].set(sizei + sizej, 0);
		int i = 0, j = 0;
		*size_ = 0;
		
		do
		{
			if((*queuePtri).size_ > i && (*queuePtrj).size_ > j)
			{
				if((*queuePtri).list[i] <= (*queuePtrj).list[j])
				{
					(*mergeSet)[threadIndex].list[*size_] = (*queuePtri).list[i];
					(*size_)++;
					i++;
				}
				else
				{
					(*mergeSet)[threadIndex].list[*size_] = (*queuePtrj).list[j];
					(*size_)++;
					j++;
				}
			}
		}
		while(i < sizei && j < sizej);
		
		while(i < sizei)
		{
			(*mergeSet)[threadIndex].list[*size_] = (*queuePtri).list[i];
			(*size_)++;
			i++;
		}
		while(j < sizej)
		{
			(*mergeSet)[threadIndex].list[*size_] = (*queuePtrj).list[j];
			(*size_)++;
			j++;
		}
	}
	else//2set merge into 1set
	{
		int queueiIndex = threadIndex - 2;
		int queuejIndex = threadIndex - 1;
		
		DataSet *queuePtri = &((*mergeSet)[queueiIndex]);
		DataSet *queuePtrj = &((*mergeSet)[queuejIndex]);
		
		int sizei = 0;
		int sizej = 0;
		for(int i = 0; i < NUM_THREAD / 2; i++)
		{
			sizei += (*sortedThreadSet)[i].size_;
		}
		
		for(int j = NUM_THREAD / 2; j < NUM_THREAD; j++)
		{
			sizej += (*sortedThreadSet)[j].size_;
		}
		
		int *size_ = &((*motherDataSet).size_);
		int i = 0, j =0;
		*size_ = 0;
		do
		{
			if((*queuePtri).size_ > i && (*queuePtrj).size_ > j)
			{
				if((*queuePtri).list[i] <= (*queuePtrj).list[j])
				{
					(*motherDataSet).list[*size_] = (*queuePtri).list[i];
					(*size_)++;
					i++;
				}
				else
				{
					(*motherDataSet).list[*size_] = (*queuePtrj).list[j];
					(*size_)++;
					j++;
				}
			}
		}
		while(i < sizei && j < sizej);
		
		while(i < sizei)
		{
			(*motherDataSet).list[*size_] = (*queuePtri).list[i];
			(*size_)++;
			i++;
		}
		while(j < sizej)
		{
			(*motherDataSet).list[*size_] = (*queuePtrj).list[j];
			(*size_)++;
			j++;
		}
	}
	
	pthread_mutex_lock(&critial_mutex);
	running_threads--;
	cout << threadIndex << " exit"<<endl;
	pthread_mutex_unlock(&critial_mutex);
	pthread_exit(NULL);
}

