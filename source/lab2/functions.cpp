#include "lab2/functions.h"

#include "lab2/matrix.h"
#include "lab2/vector.h"

using namespace lab2;

QuadraticFunction Functions::paraboloid() {
    std::vector<double> zero_vector{0, 0};
    return QuadraticFunction(new Matrix({{2, 1}, {1, 2}}, 2),
                             Vector({1, 2}), 1);
}
QuadraticFunction Functions::f1() {
    return QuadraticFunction(new Matrix({{2, -1}, {-1, 2}}, 3), Vector({4, 3}),
                             -1);
}
