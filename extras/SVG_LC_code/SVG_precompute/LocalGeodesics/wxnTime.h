#ifndef _WXNTIME_H_
#define _WXNTIME_H_

#include <Windows.h>
#include <ctime>

class ElapasedTime{
	double PCFreq;
	__int64 CounterStart;

public:
	ElapasedTime(bool startNow = true){
		if( startNow == false){
			PCFreq = 0.0;
			CounterStart = 0.0;
			return;
		}
		LARGE_INTEGER li;
		if( !QueryPerformanceFrequency(&li) ){
			cout << "QueryPerformanceFrequency failed!\n";
			return;
		}
		PCFreq = double(li.QuadPart) / 1000.0;
		//printf( "inipcfreq %lf\n" , (double)PCFreq );
		QueryPerformanceCounter(&li);
		CounterStart = li.QuadPart;
	}
	void start(){
		LARGE_INTEGER li;
		if( !QueryPerformanceFrequency(&li) ){
			cout << "QueryPerformanceFrequency failed!\n";
			return;
		}
		PCFreq = double(li.QuadPart) / 1000.0;
		QueryPerformanceCounter(&li);
		CounterStart = li.QuadPart;
		//printf( "startpcfreq %lf\n" , (double)PCFreq );
	}
	double getTime(){
	    LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		
		return double(li.QuadPart-CounterStart)/PCFreq/1000.0;
	}
	void printTime(string tmpMessage = ""){
	    LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		//printf( "li %lf\n" , (double)li.QuadPart);
		//printf( "pcfreq %lf\n" , PCFreq );
		if( tmpMessage.size() != 0 ) 
			tmpMessage = tmpMessage +",";
		printf( "%s time : %lf seconds\n\r" ,  tmpMessage.c_str() , double(li.QuadPart-CounterStart)/PCFreq/1000.0 );
		return;
	}
	
};


#endif

