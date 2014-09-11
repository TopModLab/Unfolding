#ifndef UTILS_HPP
#define UTILS_HPP

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

template <typename Container, typename Func>
Container map(Container c, Func f) {
  Container res;
  for(auto x : c) {
      res.push_back(f(x));
  }
  return res;
}

}

#endif // UTILS_HPP
