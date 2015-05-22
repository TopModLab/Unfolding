#ifndef MATHUTILS_HPP
#define MATHUTILS_HPP

#include "math.h"

#include <QColor>

static const double Pi = 3.14159265358979323846;
static const double PI2 = Pi * 2.0;


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

inline QColor operator*(QColor c, double v) {
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
T interpolate(double c, T v1, T v2) {
	return v1 * (1.0-c) + v2 * c;
}

template <typename T>
T interpolate(double c, T c1, T c2, T c3) {
	if( c > 0.0 ) {
	return c2 * (1.0 - c) + c3 * c;
	}
	else {
	return c2 * (1.0 + c) + c1 * (-c);
	}
}

#endif // MATHUTILS_HPP
