#ifndef UTILS_HPP
#define UTILS_HPP

#include "common.h"

#include <QColor>

namespace Utils {

template <typename Container, typename Pred>
Container filter(Container c, Pred p) {
  Container res;
  for(auto x : c) {
    if( p(x) )
      res.push_back(x);
  }
  return res;
}

template <typename Container, typename Pred>
Container filter_set(Container c, Pred p) {
  Container res;
  for(auto x : c) {
    if( p(x) )
      res.insert(x);
  }
  return res;
}

template <typename Container, typename Func>
Container map(Container c, Func f) {
  Container res;
  for(auto x : c) {
      res.push_back(f(x));
  }
  return res;
}

template <typename Container, typename Func>
Container map_set(Container c, Func f) {
  Container res;
  for(auto x : c) {
      res.insert(f(x));
  }
  return res;
}

template <typename Container>
void print(Container c, ostream& os = cout) {
  for(auto x : c) {
    os << x << ' ';
  }
  os << endl;
}

inline void printColor(QColor clr) {
  cout << '(' << clr.red() << ' ' << clr.green() << ' ' << clr.blue() << ')' << endl;
}

}

#endif // UTILS_HPP
