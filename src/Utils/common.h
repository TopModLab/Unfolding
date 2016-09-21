#ifdef _MSC_VER
#pragma once
#pragma warning (disable:4996) // scanf_s error
#endif // _MSVC
#pragma warning (disable : 4305) // double constant assigned to float
#pragma warning (disable : 4244) // int -> float conversion
#pragma warning (disable : 4267) // size_t -> unsigned int conversion

#ifndef COMMON_H
#define COMMON_H

#include <cstdlib>
#include <cstdint>
#include <cmath>
#include <assert.h>
#include <string>
#include <list>
#include <vector>
#include <queue>
#include <stack>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <numeric>
#include <memory>
#include <limits>
#include <math.h>
#include <utility>

using namespace std;

//#if !defined(QT_NO_DEBUG) && !defined(QT_DEBUG)
#if !defined(NDEBUG) && !defined(_DEBUG)
#	define _DEBUG
#endif

#ifdef DOUBLE_AS_FLOAT
using Float = double;
#else
using Float = float;
#endif // DOUBLE_AS_FLOAT

using floats_t = vector<float>;
using doubles_t = vector<double>;
using ui8s_t = vector<uint8_t>;
using ui16s_t = vector<uint16_t>;
using ui32s_t = vector<uint32_t>;
using int8s_t = vector<int8_t>;
using int16s_t = vector<int16_t>;
using int32s_t = vector<int32_t>;


typedef std::unordered_map<string, Float> confMap;

#endif // COMMON_H
