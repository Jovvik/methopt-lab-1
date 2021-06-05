#pragma once

#include <memory>

#include "lab2/matrix.h"
#include "lab2/n_function.h"
#include "lab2/vector.h"
#include "vector"

namespace lab2 {
    class NFunctionImpl : public NFunction {
      public:
        NFunctionImpl(int dim, const std::function<double(Vector)>& f,
                      const std::function<Vector(Vector)>& grad_f,
                      const std::function<Matrix(Vector)>& hessian_f);
        double operator()(Vector x) override;
        Vector grad(Vector x) override;
        std::size_t get_dim() override;
        Matrix hessian(Vector x) override;

        const int dim;
        const std::function<double(Vector)> f;
        const std::function<Vector(Vector)> grad_f;
        const std::function<Matrix(Vector)> hessian_f;
    };
}  // namespace lab2
