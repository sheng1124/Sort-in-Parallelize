//complie g++ -std=c++11 hw3.cpp -lpthread -o hw3

#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <vector>
#include <queue>
#include <unistd.h>
#include <time.h>
#define NUM_THREAD 8

void * pd1(void *aug);
void * pd2(void *aug);
void * pd3(void *aug);
pthread_t threads[NUM_THREAD];
pthread_mutex_t critial_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_mutex[NUM_THREAD] = PTHREAD_MUTEX_INITIALIZER;
volatile int running_threads = 0;
volatile int notpass_share[2] = {1,1};
volatile int semaphore1 = NUM_THREAD, semaphore2 = NUM_THREAD, semaphore3 = NUM_THREAD;
volatile int waitingThread = 0, gate1 = 0, gate2 = 0, gate3 = 0; 
using namespace std;

class DataSet
{
public:
	vector<int> list;
	
	DataSet(int size);
	void print() const;
	const DataSet & randomize();
	void setStartTime(){ clock_gettime( CLOCK_REALTIME, &t_start); }
	double getExeTime();
	bool isSame(const DataSet & co) const;
	
	const DataSet & copy(DataSet & co);
	const DataSet & oddEvenSort();
	const DataSet & oddEvenPthread();
	const DataSet & quickSort();
	const DataSet & qSortThreadMerge();
private:
	static struct timespec t_start, t_end;
	static double elapsedTime;
	void swap(int *a, int *b);
	int partition(int front, int end);
	void quickSortRecur(int front, int end);
};

struct threadAug
{
	vector<int> *listPtr;
	int threadIndex;
	int listSize;
	DataSet *threadDataSet;
	vector<int> (*threadMergeQueue)[NUM_THREAD - 2];
	DataSet (*threadMergeList)[NUM_THREAD];
};

void * pd2(void *aug)
{
	struct threadAug *aug_ = static_cast<struct threadAug*>(aug) ;
	vector<int> *listPtr = (*aug_).listPtr;
	int threadIndex = (*aug_).threadIndex;
	DataSet *threadDataSet = (*aug_).threadDataSet;
	int size = (*listPtr).size();
	
	(*threadDataSet).list.reserve(size / NUM_THREAD + 1);
	
	for(int i = threadIndex; i < size; i += NUM_THREAD)
	{
		(*threadDataSet).list.push_back((*listPtr)[i]); 
	}
	
	(*threadDataSet).quickSort();
	/*
	for(int i = 0; i < (*threadDataSet).list.size(); i++)
	{
		pthread_mutex_lock(&critial_mutex);
		cout << "id:" << threadIndex<<" contain " << (*threadDataSet).list[i] << endl;
		pthread_mutex_unlock(&critial_mutex);
	}*/
	
	pthread_mutex_lock(&critial_mutex);
	running_threads--;
	pthread_mutex_unlock(&critial_mutex);
	pthread_exit(NULL);
}

void * pd3(void *aug)
{ 
	struct threadAug *aug_ = static_cast<struct threadAug*>(aug) ;
	vector<int> *listPtr = (*aug_).listPtr;
	int threadIndex = (*aug_).threadIndex;
	DataSet (*threadDataSet)[NUM_THREAD] = (*aug_).threadMergeList;
	vector<int> (*threadMergeQueue)[NUM_THREAD - 2] = (*aug_).threadMergeQueue;
	int listSize = (*aug_).listSize;
	
	if(threadIndex < NUM_THREAD / 2)
	{
		
		int i = 0, j = 0;
		vector<int> *listPtri = &((*threadDataSet)[2 * threadIndex].list);
		vector<int> *listPtrj = &((*threadDataSet)[2 * threadIndex + 1].list);
		
		int sizei = (*threadDataSet)[2 * threadIndex].list.size();
		int sizej = (*threadDataSet)[2 * threadIndex + 1].list.size();
		(*threadMergeQueue)[threadIndex].reserve(sizei + sizej);
		
		while(i < sizei && j < sizej)
		{
			if((*listPtri)[i] < (*listPtrj)[j])
			{
				(*threadMergeQueue)[threadIndex].push_back((*listPtri)[i]);
				i++;
			}
			else
			{
				(*threadMergeQueue)[threadIndex].push_back((*listPtrj)[j]);
				j++;
			}
		}
		
		while(i < sizei)
		{
			(*threadMergeQueue)[threadIndex].push_back((*listPtri)[i]);
			i++;
		}
		
		while(j < sizej)
		{
			(*threadMergeQueue)[threadIndex].push_back((*listPtrj)[j]);
			j++;
		}
		/*
		for(int i = 0; (*threadMergeQueue)[threadIndex].size() != 0; i++)
		{
			pthread_mutex_lock(&critial_mutex);
			int x = (*threadMergeQueue)[threadIndex].front();
			(*threadMergeQueue)[threadIndex].pop();
			cout << "id:" << threadIndex<<" contain " << x << endl;
			pthread_mutex_unlock(&critial_mutex);
		}*/
		
	}
	else if(threadIndex != NUM_THREAD - 2)
	{
		int queueiIndex = 2 * threadIndex - NUM_THREAD;
		int queuejIndex = 2 * threadIndex - NUM_THREAD + 1;
		vector<int> *queuePtri = &((*threadMergeQueue)[queueiIndex]);
		vector<int> *queuePtrj = &((*threadMergeQueue)[queuejIndex]);
		
		int sizei = (*threadDataSet)[queueiIndex * 2].list.size() + (*threadDataSet)[queueiIndex * 2 + 1].list.size();
		int sizej = (*threadDataSet)[queuejIndex * 2].list.size() + (*threadDataSet)[queuejIndex * 2 + 1].list.size();
		
		(*threadMergeQueue)[threadIndex].reserve(sizei + sizej);
		
		int i = 0, j = 0;
		do
		{
			while( i >= (*queuePtri).size() || j >= (*queuePtrj).size() )
			{
				if(i == sizei || j == sizej)
				{
					break;
				}
			}
			
			if((*queuePtri).size() > i && (*queuePtrj).size() > j)
			{
				if((*queuePtri)[i] < (*queuePtrj)[j])
				{
					(*threadMergeQueue)[threadIndex].push_back((*queuePtri)[i]);
					i++;
				}
				else
				{
					(*threadMergeQueue)[threadIndex].push_back((*queuePtrj)[j]);
					j++;
				}
			}
		}
		while(i < sizei && j < sizej);
		
		while(i < sizei)
		{
			(*threadMergeQueue)[threadIndex].push_back((*queuePtri)[i]);
			i++;
		}
		while(j < sizej)
		{
			(*threadMergeQueue)[threadIndex].push_back((*queuePtrj)[j]);
			j++;
		}
		
		/*
		for(int i = 0; i < (*threadMergeQueue)[threadIndex].size() ; i++)
		{
			pthread_mutex_lock(&critial_mutex);
			
			cout << "id:" << threadIndex<<"index "<< i <<" contain " << (*threadMergeQueue)[threadIndex][i] << endl;
			pthread_mutex_unlock(&critial_mutex);
		}*/
		
	}
	else
	{
		int queueiIndex = threadIndex - 2;
		int queuejIndex = threadIndex - 1;
		
		vector<int> *queuePtri = &(*threadMergeQueue)[queueiIndex];
		vector<int> *queuePtrj = &(*threadMergeQueue)[queuejIndex];
		
		int sizei = 0;
		int sizej = 0;
		
		for(int i = 0; i < NUM_THREAD / 2; i++)
		{
			sizei += (*threadDataSet)[i].list.size();
		}
		
		for(int j = NUM_THREAD / 2; j < NUM_THREAD; j++)
		{
			sizej += (*threadDataSet)[j].list.size();
		}
		
		(*listPtr).reserve(sizei + sizej);
		
		int i = 0, j =0;
		
		do
		{
			while( i >= (*queuePtri).size() || j >= (*queuePtrj).size() )
			{
				if(i == sizei || j == sizej)
				{
					break;
				}
			}
			
			if((*queuePtri).size() > i && (*queuePtrj).size() > j)
			{
				if((*queuePtri)[i] < (*queuePtrj)[j])
				{
					(*listPtr).push_back((*queuePtri)[i]);
					i++;
				}
				else
				{
					(*listPtr).push_back((*queuePtrj)[j]);
					j++;
				}
			}
		}
		while(i < sizei && j < sizej);
		
		while(i < sizei)
		{
			(*listPtr).push_back((*queuePtri)[i]);
			i++;
		}
		while(j < sizej)
		{
			(*listPtr).push_back((*queuePtrj)[j]);
			j++;
		}
		/*
		for(int i = 0; i < (*listPtr).size() ; i++)
		{
			pthread_mutex_lock(&critial_mutex);
			
			cout << "id:" << threadIndex<<"index "<< i <<" contain " << (*listPtr)[i] << endl;
			pthread_mutex_unlock(&critial_mutex);
		}*/
	}

	pthread_mutex_lock(&critial_mutex);
	running_threads--;
	cout << threadIndex << " exit"<<endl;
	pthread_mutex_unlock(&critial_mutex);
	pthread_exit(NULL);
}

const DataSet & DataSet::qSortThreadMerge()
{
	DataSet threadDataSet[NUM_THREAD](0);
	
	for(int i = 0; i < NUM_THREAD; i++)
	{
		struct threadAug *aug = new struct threadAug;
		(*aug).listPtr = &list;
		(*aug).threadIndex = i;
		(*aug).threadDataSet = &threadDataSet[i];
		
		void *aug_ = static_cast<void*>(aug);
		
		pthread_mutex_lock(&critial_mutex);
		running_threads++;
		pthread_mutex_unlock(&critial_mutex);
		pthread_create(&threads[i], NULL, pd2, aug_);		
	}
	
	while(running_threads > 0)
	{
		;
	}
	
	/*for(int i = 0; i < NUM_THREAD; i++)
	{
		threadDataSet[i].print();
		cout << endl;
	}*/

	vector<int> mergeQueue[NUM_THREAD - 2];
	int listSize = list.size();
	list.clear();
	for(int i = 0; i < NUM_THREAD - 1; i++)
	{
		struct threadAug *aug = new struct threadAug;
		(*aug).listPtr = &list;
		(*aug).threadIndex = i;
		(*aug).listSize = listSize;
		(*aug).threadMergeQueue = &mergeQueue;
		(*aug).threadMergeList = &threadDataSet;
		
		void *aug_ = static_cast<void*>(aug);
		
		pthread_mutex_lock(&critial_mutex);
		running_threads++;
		pthread_mutex_unlock(&critial_mutex);
		pthread_create(&threads[i], NULL, pd3, aug_);		
	}
	
	while(running_threads > 0)
	{
		;
	}
	return *this;
}

int DataSet::partition(int front, int end)
{
	int pivot = list[end];
	int i = front - 1;
	int j;
	for(j = front; j < end; j++)
	{
		if(list[j] < pivot)
		{
			i++;
			swap(&list[i], &list[j]);
		}
	}
	//swap pivot to middle, list[j] is pivot now,list[i] will be greater then pivot
	i++;
	swap(&list[i], &list[j]);
	return i;
}

void DataSet::quickSortRecur(int front, int end)
{
	if(front < end)
	{
		int pivotIndex = partition(front, end);
		quickSortRecur(pivotIndex + 1, end);
		quickSortRecur(front, pivotIndex - 1);
	}
}

int qcompare(const void *a, const void *b)
{
	int a_ = *(int *)a;
	int b_ = *(int *)b;
	
	return (a_ - b_);
}

const DataSet & DataSet::quickSort()
{
	int size = list.size();
	//(*this).quickSortRecur(0, size - 1);
	qsort(&list[0], size, sizeof(int), qcompare);
	
	return *this;
}


struct timespec DataSet::t_start, DataSet::t_end;
double DataSet::elapsedTime;

double DataSet::getExeTime()
{
	clock_gettime( CLOCK_REALTIME, &t_end);

	// compute and print the elapsed time in millisec
	elapsedTime = (t_end.tv_sec - t_start.tv_sec) * 1000.0;
	elapsedTime += (t_end.tv_nsec - t_start.tv_nsec) / 1000000.0;
	cout << "Sequential elapsedTime: " << elapsedTime << " ms" << endl;
	return elapsedTime;
}

void * pd1(void *aug)
{
	struct threadAug *aug_ = static_cast<struct threadAug*>(aug) ;
	vector<int> *listPtr = (*aug_).listPtr;
	int threadIndex = (*aug_).threadIndex;
	int size = (*listPtr).size();
	int turn = 0;//0 even, 1 odd 
	
	do
	{
		//需要集合 thread，確保進入判斷式
		pthread_mutex_lock(&critial_mutex);
		gate1 = 0;
		waitingThread++;
		pthread_mutex_unlock(&critial_mutex);
		while(waitingThread < NUM_THREAD && gate1 == 0)
		{
			//cout <<"11:" << waitingThread  <<endl;
			//sleep(1);
		}
		//保證進入判斷式 所有人都在 waitingThread++ 之後
		pthread_mutex_lock(&critial_mutex);
		notpass_share[turn % 2] = 0;//需等待所有 thread 進入回圈判斷式才能執行
		if(waitingThread == NUM_THREAD)
		{
			//只有第一個進入此區的 thread 會清空 waiting,打開大門
			waitingThread -= NUM_THREAD;
			gate1 = 1;
		}
		pthread_mutex_unlock(&critial_mutex);

		int notpass = 0;
		for(int j = (threadIndex * 2) + (turn % 2); j < size - 1; j += (2 * NUM_THREAD))
		{
			if((*listPtr)[j] > (*listPtr)[j + 1])
			{
				int temp = (*listPtr)[j];
				(*listPtr)[j] = (*listPtr)[j + 1];
				(*listPtr)[j + 1] = temp;
				notpass++;
			}
			
			//cout<<" tid" << threadIndex << " : " << (*listPtr)[j] << "  " << (*listPtr)[j + 1] << endl;
		}
		
		pthread_mutex_lock(&critial_mutex);
		notpass_share[turn % 2] += notpass;
		pthread_mutex_unlock(&critial_mutex);
		
		turn += 1;
		//cout << "notpass_share " << notpass_share << " notpass " << notpass << endl;
		
		
		//需要集合 thread，notpass_share
		pthread_mutex_lock(&critial_mutex);
		gate2 = 0;
		waitingThread++;
		pthread_mutex_unlock(&critial_mutex);
		while(waitingThread < NUM_THREAD && gate2 == 0)
		{
			//cout <<"22:" << waitingThread <<endl;
			//sleep(1);
		}
		//保證 notpass_share 是完整的，所有人都在 waitingThread++ 之後
		pthread_mutex_lock(&critial_mutex);
		
		if(waitingThread == NUM_THREAD)
		{
			//只有第一個進入此區的 thread 會清空 waiting,打開大門
			waitingThread -= NUM_THREAD;
			gate2 = 1;
		}
		pthread_mutex_unlock(&critial_mutex);
		
	}while(notpass_share[0] > 0 || notpass_share[1] > 0);
	pthread_mutex_lock(&critial_mutex);
	running_threads--;
	pthread_mutex_unlock(&critial_mutex);
	pthread_exit(NULL);
}

const DataSet & DataSet::oddEvenPthread()
{	
	for(int i = 0; i < NUM_THREAD; i++)
	{
		struct threadAug *aug = new struct threadAug;
		(*aug).listPtr = &list;
		(*aug).threadIndex = i;
		
		void *aug_ = static_cast<void*>(aug);
		
		pthread_mutex_lock(&critial_mutex);
		running_threads++;
		pthread_mutex_unlock(&critial_mutex);
		pthread_create(&threads[i], NULL, pd1, aug_);		
	}
	
	while(running_threads > 0)
	{
		;
	}
	
	return *this;
}

const DataSet & DataSet::oddEvenSort()
{
	int size = list.size();
	
	for(int i = 0, notpass = 1; notpass > 0; i++)
	{
		notpass = 0;
		for(int j = i % 2; j < size - 1; j += 2)
		{
			if(list[j] > list[j + 1])
			{
				(*this).swap(&list[j], &list[j + 1]);
				notpass++;
			}
		}
	}
	
	return *this;
}

void DataSet::swap(int *a, int *b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

DataSet::DataSet(int size)
{
	list.assign(size, 0);
}

const DataSet & DataSet::copy(DataSet & co)
{
	int size = list.size();
	for(int i = 0; i < size; i++)
	{
		list[i] = co.list[i];
	}
	return *this;
}

bool DataSet::isSame(const DataSet & co) const
{
	int size = list.size();
	for(int i = 0; i < size; i++)
	{
		if(list[i] != co.list[i])
			return false;
	}
	return true;
}

const DataSet & DataSet::randomize()
{
	int size = list.size();
	
	for(int i = 0; i < size; i++)
	{
		list[i] = rand() % 100;
	}
	return *this;
}

void DataSet::print() const
{
	int size = list.size();
	for(int i = 0; i < size; i++)
	{
		cout << list[i] << " ";
	}
	cout << endl;
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
		if(n < 100)
		{
			cout << "原本的陣列" << endl;
			test.randomize().print();
		}
		
		golden.copy(test);
		cout << "Sorted by no parallel" <<endl;
		
		golden.setStartTime();
		//golden.oddEvenSort();
		golden.quickSort();
		golden.getExeTime();
		cout << endl << "Sorted by parallel" <<endl;
		test.setStartTime();
		//test.oddEvenPthread();
		//test.oddEvenPthread();
		test.qSortThreadMerge();
		test.getExeTime();
		
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
