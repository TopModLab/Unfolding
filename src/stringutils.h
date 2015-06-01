#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include "common.h"

enum CaseSensitivity{
	CaseSensitive,
	CaseInsensitive
};

typedef list<string> stringlist;

string toLower(const string& s);
string toUpper(const string& s);
bool stringComp(const string& s1, const string& s2, CaseSensitivity cs = CaseSensitive);
stringlist split(const string& line, const string& separaters);
bool contains(const string& str, const string& substr);
bool beginsWith(const string& s, char c);
bool endsWith(const string& s, char c);
bool beginsWith(const string& s, const string& sub);
bool endsWith(const string& s, const string& sub);

template <typename T>
string toString(const T& t) {
	stringstream ss;
	ss << t;
	return ss.str();
}

static string toString(const bool& v) {
	stringstream ss;
	ss << (v?"true":"false");
	return ss.str();
}

string padWith(const string& str, char c, int L);
void printStringList(const stringlist& lst);

#endif // STRINGUTILS_H
