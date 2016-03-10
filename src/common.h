#ifndef COMMON_H
#define COMMON_H

#include <cstdlib>
#include <cmath>
#include <assert.h>
#include <string>
#include <list>
#include <vector>
#include <queue>
#include <stack>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <numeric>
#include <memory>
#include <limits>
#include <math.h>

using namespace std;

class UnfoldingAppException: public exception {
public:
	UnfoldingAppException(const string& msg):msg(msg) {}
	virtual ~UnfoldingAppException() throw() {}
	virtual const char* what() const throw() {
		return msg.c_str();
	}

private:
	string msg;
};

#endif // COMMON_H
