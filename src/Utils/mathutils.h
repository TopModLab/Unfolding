#ifndef MATHUTILS_HPP
#define MATHUTILS_HPP

#include <math.h>
#include <limits>

//#include <QtGui/QColor>
#include <QColor>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
const Float Pi = 3.14159265358979323846;
const Float PI2 = 6.283185307179586476925286766559;//Pi * 2.0;
const Float M_HALFPI = 1.570796326794896619231321691640;// Pi * 0.5;
const Float PI_EPS = 1.0E-6;


template <typename T>
__forceinline void encodeIndex(int idx, T& r, T& g, T& b, Float scaler = 255.0) {
	r = ((idx >> 16) & 0xff) / scaler;
	g = ((idx >>  8) & 0xff) / scaler;
	b = ( idx		 & 0xff) / scaler;
}

template <typename T>
__forceinline int decodeIndex(T r, T g, T b, Float scaler = 255.0) {
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

inline QColor operator*(QColor c, Float v) {
	return QColor(clamp<int>(c.red() * v, 0, 255),
				clamp<int>(c.green() * v, 0, 255),
				clamp<int>(c.blue() * v, 0, 255));
}

inline QColor operator+(QColor c1, QColor c2) {
	return QColor(clamp<int>(c1.red() + c2.red(), 0, 255),
				clamp<int>(c1.green() + c2.green(), 0, 255),
				clamp<int>(c1.blue() + c2.blue(), 0, 255));
}

template <typename T>
__forceinline T sqr(T val)
{
	return val * val;
}

template <typename T>
T interpolate(Float c, T v1, T v2) {
	return v1 * (1.0-c) + v2 * c;
}

template <typename T>
T interpolate(Float c, T c1, T c2, T c3) {
	if(c > 0.0)	return c2 * (1.0 - c) + c3 * c;
	else		return c2 * (1.0 + c) + c1 * (-c);
}

template <typename T>
inline T RadianToDegree(T radian)
{
	return radian * 180.0 / M_PI;
}
template <typename T>
inline T DegreeToRadian(T degree)
{
	return degree * M_PI / 180.0;
}

template <typename T>
__forceinline bool FuzzyEqual(T val, T target, T eps = std::numeric_limits<T>::epsilon())
{
	return val > (target + eps) && val < (target + eps);
}
#endif // MATHUTILS_HPP
