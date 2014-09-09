#ifndef COMMON_H
#define COMMON_H

#include <cstdlib>
#include <string>
#include <list>
#include <vector>
#include <set>
#include <unordered_set>
#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

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
