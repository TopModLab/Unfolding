#ifndef MATHUTILS_HPP
#define MATHUTILS_HPP

#include "math.h"

static const double PI = 3.14159265358979323846;
static const double PI2 = PI * 2.0;


template <typename T>
__forceinline void encodeIndex(int idx, T& r, T& g, T& b, float scaler = 255.0) {
  r = ((idx >> 16) & 0xff) / scaler;
  g = ((idx >>  8) & 0xff) / scaler;
  b = ( idx		 & 0xff) / scaler;
}

template <typename T>
__forceinline int decodeIndex(T r, T g, T b, float scaler = 255.0) {
  int ri = r * scaler;
  int gi = g * scaler;
  int bi = b * scaler;
  return (ri << 16) | (gi << 8) | bi;
}

template <typename T>
__forceinline T clamp(T val, T lower, T upper) {
  if( val < lower ) return lower;
  if( val > upper ) return upper;
  return val;
}

#endif // MATHUTILS_HPP
