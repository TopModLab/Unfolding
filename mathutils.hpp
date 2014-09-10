#ifndef MATHUTILS_HPP
#define MATHUTILS_HPP

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

#endif // MATHUTILS_HPP
