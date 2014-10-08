#ifndef NUMERICAL_H
#define NUMERICAL_H

#define ARMA_USE_BLAS
#define ARMA_USE_LAPACK
#include <armadillo>

using arma::mat;
using arma::vec;

#include "levmar.h"

namespace Numerical {

vec solve(const mat &A, const vec &b) {
    return arma::solve(A, b);
}

}

#endif // NUMERICAL_H
