#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <exception>

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
