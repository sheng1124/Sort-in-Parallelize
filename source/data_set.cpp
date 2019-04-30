//complie g++ -std=c++11 chronometer.cpp data_set.cpp pthread_func.cpp sort_main.cpp -lpthread -o hw3

#include "../header/sort_parallel.h"
#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <vector>

using namespace std;

extern pthread_t threads[NUM_THREAD];
extern pthread_mutex_t critial_mutex;
extern volatile int running_threads;
extern Chronometer A;

DataSet::DataSet(int size)
{
	list.assign(size, 0);
	maxSize_ = size;
}

const DataSet & DataSet::set(int size, int value)
{
	list.clear();
	list.assign(size, value);
	
	maxSize_ = size;
	size_ = maxSize_;
	return *this;
}

const DataSet & DataSet::randomize()
{
	for(int i = 0; i < maxSize_; i++)
	{
		list[i] = rand() % 100000;
	}
	size_ = maxSize_;
	return *this;
}

void DataSet::print() const
{
	for(int i = 0; i < size_; i++)
	{
		cout << list[i] << " ";
	}
	cout << endl;
}

bool DataSet::isSame(const DataSet & co) const
{
	if(size_ != co.size_)
	{
		return false;
	}
	else
	{
		for(int i = 0; i < size_; i++)
		{
			if(list[i] != co.list[i])
			{
				return false;
			}
		}
	}
	return true;
}

const DataSet & DataSet::assign(const DataSet & co)
{
	if(size_ != co.size_)
	{
		size_ = co.size_;
		maxSize_ = co.maxSize_;
		list.clear();
		list.assign(size_, 0);
	}
	for(int i = 0; i < size_; i++)
	{
		list[i] = co.list[i];
	}
	return *this;
}

const DataSet & DataSet::qSort()
{
	qsort(&list[0], size_, sizeof(int), qcompare);
	return *this;
}
int qcompare(const void *a, const void *b)
{
	int a_ = *(int *)a;
	int b_ = *(int *)b;
	
	return (a_ - b_);
}

const DataSet & DataSet::qSortThreadMerge()
{
	DataSet threadISet[NUM_THREAD](0);
	
	for(int i = 0; i < NUM_THREAD; i++)
	{
		struct ThreadPara *para = new struct ThreadPara;
		(*para).threadIndex = i;
		(*para).motherDataSetPtr = this;
		(*para).threadISetPtr = &threadISet[i];
		
		void *para_ = static_cast<void*>(para);
		
		pthread_mutex_lock(&critial_mutex);
		running_threads++;
		pthread_mutex_unlock(&critial_mutex);
		pthread_create(&threads[i], NULL, pd1, para_);		
	}	
	while(running_threads > 0)
	{
		;
	}
	
	cout<< NUM_THREAD <<" threads sort time ";
	A.getExeTime();
	DataSet mergeSet[NUM_THREAD - 2](0);
		
	for(int i = 0; i < NUM_THREAD - 1; i++)
	{
		struct ThreadPara *para = new struct ThreadPara;
		(*para).threadIndex = i;
		(*para).motherDataSetPtr = this;
		(*para).sortedThreadSetPtr = &threadISet;
		(*para).mergeSetPtr = &mergeSet;
		
		void *para_ = static_cast<void*>(para);
		
		pthread_mutex_lock(&critial_mutex);
		running_threads++;
		pthread_mutex_unlock(&critial_mutex);
		pthread_create(&threads[i], NULL, pd2, para_);		
	}
	while(running_threads > 0)
	{
		;
	}
	
	return *this;
}
