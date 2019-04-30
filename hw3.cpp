//complie g++ -std=c++11 hw3.cpp -lpthread -o hw3

#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <vector>
#include <time.h>
#define NUM_THREAD 8
using namespace std;

class Chronometer;
class DataSet;
struct ThreadPara;
int qcompare(const void *a, const void *b);
void * pd1(void *para);
void * pd2(void *para);

pthread_t threads[NUM_THREAD];
pthread_mutex_t critial_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int running_threads = 0;

struct ThreadPara
{
	int threadIndex;
	DataSet *motherDataSetPtr;
	DataSet *threadISetPtr;
	
	DataSet (*sortedThreadSetPtr)[NUM_THREAD];
	DataSet (*mergeSetPtr)[NUM_THREAD - 2];
};

class Chronometer
{
public:
	const Chronometer & setStartTime();
	double getExeTime();
private:
	struct timespec t_start, t_end;
	double elapsedTime;
};
Chronometer A;
const Chronometer & Chronometer::setStartTime()
{
	clock_gettime( CLOCK_REALTIME, &t_start); 
	return *this;
}

double Chronometer::getExeTime()
{
	clock_gettime( CLOCK_REALTIME, &t_end);
	// compute and print the elapsed time in millisec
	elapsedTime = (t_end.tv_sec - t_start.tv_sec) * 1000.0;
	elapsedTime += (t_end.tv_nsec - t_start.tv_nsec) / 1000000.0;
	cout << "Sequential elapsedTime: " << elapsedTime << " ms" << endl;
	return elapsedTime;
}

class DataSet
{
public:
	vector<int> list;
	int size_;
	int maxSize_;
	
	DataSet(int size);
	const DataSet & set(int size, int value);
	const DataSet & randomize();
	void print() const;
	bool isSame(const DataSet & co) const;
	const DataSet & assign(const DataSet & co);
	const DataSet & qSort();
	
	const DataSet & qSortThreadMerge();
private:
	void swap(int *a, int *b);
};

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



int main()
{
	int n = 0;
	cout << "輸入列表規模 -1離開" << endl;
	cin >> n;
	while(n != -1)
	{
		DataSet test(n);
		DataSet golden(n);
		test.randomize();
		golden.assign(test);
		
		if(n < 100)
		{
			cout << "原本的陣列" << endl;
			test.print();
		}
		
		cout << "Sorted by no parallel" <<endl;
		A.setStartTime();
		golden.qSort();
		A.getExeTime();
		cout << endl << "Sorted by parallel" <<endl;
		A.setStartTime();
		test.qSortThreadMerge();
		A.getExeTime();
		cout << "HW2測試結果: ";
		if(test.isSame(golden))
		{
			cout << "結果一致"<< endl;
		}
		else
		{
			cout << "不一致" << endl;
		}
		
		cout << "答案：" << endl;
		if(n < 100)
		{
			golden.print();
			cout << endl;
			test.print();
		}
		
		cout << "輸入列表規模 -1離開" << endl;
		cin >> n;
	}	
	return 0;
}
