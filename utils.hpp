#ifndef UTILS_HPP
#define UTILS_HPP

template <typename Container, typename Pred>
Container filter(Container c, Pred p) {
  Container res;
  for(auto x : c) {
    if( p(x) )
      res.insert(x);
  }
  return res;
}

#endif // UTILS_HPP
