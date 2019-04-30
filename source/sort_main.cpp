//complie g++ -std=c++11 chronometer.cpp data_set.cpp pthread_func.cpp sort_main.cpp -lpthread -o hw3

#include "../header/sort_parallel.h"
#include <iostream>
using namespace std;

extern Chronometer A;

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
