#include "stringutils.h"

string toLower( const string& s )
{
    string res = s;
    std::transform(res.begin(), res.end(), res.begin(), std::ptr_fun(::tolower));
    return res;
}

string toUpper( const string& s )
{
    string res = s;
    std::transform(res.begin(), res.end(), res.begin(), std::ptr_fun(::toupper));
    return res;
}

bool stringComp(const string& s1, const string& s2, CaseSensitivity cs)
{
    switch( cs )
    {
    case CaseSensitive:
        return (s1 == s2);
    case CaseInsensitive:
        return (toLower(s1) == toLower(s2));
    default:
        return (s1 == s2);
    }
}

stringlist split(const string& line, const string& sep) {
    stringlist sl;
    int pos = 0;
    while( pos < line.size() )
    {
        int lpos = pos;
        pos = line.find_first_of(sep, lpos);
        if( pos < 0 )
            pos = line.size();
        string ss = line.substr(lpos, pos - lpos);
        //cout << ss << endl;
        sl.push_back( ss );
        pos++;
    }

    return sl;
}

bool beginsWith(const string& s, char c)
{
    return (s.find_first_of(c) == 0);
}

bool beginsWith(const string& s, const string& sub)
{
    return (s.find_first_of(sub) == 0);
}

bool endsWith(const string& s, char c)
{
    return (s.find_last_of(c) == s.size() - 1);
}

bool endsWith(const string& s, const string& sub)
{
    return (s.find_last_of(sub) == s.size() - 1);
}

bool contains(const string& str, const string& substr) {
    return (str.find_first_of(substr) >= 0);
}

string padWith(const string& str, char c, int L) {
    string res = str;
    while( res.length() < L ) {
        res = c + res;
    }
    return res;
}

void printStringList( const stringlist& lst )
{
    for(auto it=lst.begin();it!=lst.end();it++) {
        cout << (*it) << ' ';
    }
    cout << endl;
}
