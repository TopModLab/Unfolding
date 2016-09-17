#pragma once
#ifndef UNFOLDING_APP_EXCEPTION
#define UNFOLDING_APP_EXCEPTION

#include "Utils/common.h"
class UnfoldingAppException : public exception {
public:
	UnfoldingAppException(const string& msg) :msg(msg) {}
	virtual ~UnfoldingAppException() throw() {}
	virtual const char* what() const throw() {
		return msg.c_str();
	}

private:
	string msg;
};

#endif