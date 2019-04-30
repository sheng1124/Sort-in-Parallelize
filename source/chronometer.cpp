//complie g++ -std=c++11 chronometer.cpp data_set.cpp pthread_func.cpp sort_main.cpp -lpthread -o hw3

#include "../header/sort_parallel.h"
#include <iostream>
#include <cstdlib>
using namespace std;

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


