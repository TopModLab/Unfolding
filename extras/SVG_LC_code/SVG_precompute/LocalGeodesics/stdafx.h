// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#define _AFXDLL
#include <afx.h>
#include <Windows.h>
#include "targetver.h"
#include <stdio.h>
#include <tchar.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <iomanip>
#include <vector>
#include <map>
#include <stack>
#include <set>
#include <list>
#include <queue>
#include <limits>
#include <cassert>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <cassert>
#include <omp.h>
//#include <ompassem.h>
#include <iomanip>

//#pragma comment(lib, "glut32.lib")
using namespace std;

const double DOUBLE_EPSILON = 1e-10;
const double LENGTH_EPSILON_CONTROL = 1e-6;
const double PI = 3.14159265359;
const double RateOfNormalShift = 1.5e-3;
const double ToleranceOfConvexAngle = 5e-3;
// TODO: reference additional headers your program requires here
