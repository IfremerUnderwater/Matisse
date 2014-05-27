// AlgoMock.cpp : Simulateur de code utilisant MATLAB
//
#include "stdafx.h"
#include <iostream>

using namespace std;


#define DLLEXPORT  extern "C" __declspec(dllexport)


DLLEXPORT void init() {
#ifdef WIN64
	cout << "Init in AlgoMock(x64)"  << endl;
#else
	cout << "Init in AlgoMock(Win32)"  << endl;
#endif
}

DLLEXPORT void doWork() {
	cout << "doWork in AlgoMock. Pretend to use MATLAB Here..."  << endl;
}

DLLEXPORT void stop() {
	cout << "stop in AlgoMock"  << endl;
}
