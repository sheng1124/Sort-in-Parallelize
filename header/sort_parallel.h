//complie g++ -std=c++11 chronometer.cpp data_set.cpp pthread_func.cpp sort_main.cpp -lpthread -o hw3

#ifndef SORT_PARALLEL_H_
#define SORT_PAPALLEL_H_
#include <vector>
#include <cstdlib>

#define NUM_THREAD 8

struct ThreadPara;
class Chronometer;
class DataSet;

int qcompare(const void *a, const void *b);
void * pd1(void *para);
void * pd2(void *para);

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

class DataSet
{
public:
	std::vector<int> list;
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

#endif
