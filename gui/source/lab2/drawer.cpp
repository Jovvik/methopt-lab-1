#include "lab2/drawer.h"

#include <sstream>

#include "iostream"
#include "lab1/brent.h"
#include "lab1/dichotomy.h"
#include "lab1/fibonacci.h"
#include "lab1/parabola.h"
#include "lab2/conjugate_gradient.h"
#include "lab2/fastest_descent.h"
#include "lab2/gradient_descent.h"

using namespace gui2;
using namespace lab2;

#define new_fastest(_optimizer_1d)                                       \
    new lab2::FastestDescent(                                            \
        [](const std::function<double(double)> &optimized_function,      \
           double epsilon, double start, double end) {                   \
            return std::make_unique<_optimizer_1d>(optimized_function,   \
                                                   epsilon, start, end); \
        })

Drawer::Drawer(QWidget *parent) : QCustomPlot(parent) {
    Drawer::setInteraction(QCP::iRangeDrag, true);
    Drawer::setInteraction(QCP::iRangeZoom, true);
    Drawer::setInteraction(QCP::iSelectPlottables, true);
    connect(this, &QCustomPlot::beforeReplot, this, &Drawer::before_replot);
    connect(this, &QCustomPlot::plottableClick, this,
            &Drawer::rescale_on_click);
}

void Drawer::recalc_optimize_points() {
    lab2::NOptimizer *n_optimizer;
    switch (method_2d) {
        case lab2::NOptimizers::GRADIENT_DESCENT:
            n_optimizer = new lab2::GradientDescent(0.1);
            break;
        case lab2::NOptimizers::CONJUGATE_GRADIENT:
            n_optimizer = new lab2::ConjugateGradient();
            break;
        case lab2::NOptimizers::FASTEST_DESCENT:
            switch (method_1d) {
                case lab1::Optimizers::DICHOTOMY:
                    n_optimizer = new_fastest(lab1::Dichotomy);
                    break;
                case lab1::Optimizers::GOLDEN_RATIO:
                    n_optimizer = new_fastest(lab1::GoldenRatio);
                    break;
                case lab1::Optimizers::FIBONACCI:
                    n_optimizer = new_fastest(lab1::Fibonacci);
                    break;
                case lab1::Optimizers::PARABOLA:
                    n_optimizer = new_fastest(lab1::Parabola);
                    break;
                case lab1::Optimizers::BRENT:
                    n_optimizer = new_fastest(lab1::Brent);
                    break;
            }
    }
    n_optimizer->optimize(*func.f, starting_point, pepsilon);
    optimize_points = n_optimizer->get_points();
}

void Drawer::setup() {
    xAxis->setRange(-10, 10);
    yAxis->setRange(-10, 10);
    recalc_optimize_points();
    clearItems();
    addGraph();
    graph(0)->setPen(QPen(Qt::red));
    graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    graph(0)->setSelectable(QCP::SelectionType::stSingleData);
    for (auto curve : curves) {
        removePlottable(curve);
    }
    curves.clear();
    for (int i = 0; i < LINE_COUNT; i++) {
        curves.emplace_back(new QCPCurve(xAxis, yAxis));
    }
    replot();
}

void Drawer::change_draw_level_lines() {
    draw_level_lines = !draw_level_lines;
    replot();
}

void Drawer::change_draw_optimize_lines() {
    draw_optimize_lines = !draw_optimize_lines;
    replot();
}

void Drawer::change() {
    setup();
    std::ostringstream temp_pepsilon, temp_x_y;
    temp_pepsilon << ":Eps=" << pepsilon;
    temp_x_y << ":{" << starting_point[0] << ", " << starting_point[1] << "}";
    emit pepsilon_changed(QString::fromStdString(temp_pepsilon.str()));
    emit x_y_changed(QString::fromStdString(temp_x_y.str()));
    emit method_changed(optimize_points.size());
}

void Drawer::set_method_2d(const QString &text) {
    method_2d = lab2::n_optimizers_table.at(text.toStdString());
    change();
}

void Drawer::set_method_1d(const QString &text) {
    method_1d = lab1::optimizers_table.at(text.toStdString());
    change();
}

void Drawer::set_func(const QString &text) {
    func = lab2::Function2D(lab2::functions_table.at(text.toStdString()));
    change();
}

void Drawer::set_count_to_draw(int iteration) {
    optimize_points_to_draw = iteration;
    replot();
}

void Drawer::set_pepsilon(const QString &text) {
    auto temp_str = text.toStdString();
    if (temp_str.empty()) {
        return;
    }
    pepsilon = atof(temp_str.c_str());
}

void Drawer::set_x(const QString &text) {
    auto temp_str = text.toStdString();
    if (temp_str.empty()) {
        return;
    }
    starting_point = lab2::Vector({atof(temp_str.c_str()), starting_point[1]});
}

void Drawer::set_y(const QString &text) {
    auto temp_str = text.toStdString();
    if (temp_str.empty()) {
        return;
    }
    starting_point = lab2::Vector({starting_point[0], atof(temp_str.c_str())});
}

void Drawer::before_replot() {
    replot_lines();
    replot_f();
}

void Drawer::replot_f() {
    if (draw_optimize_lines) {
        graph(0)->setLineStyle(QCPGraph::LineStyle::lsLine);
    } else {
        graph(0)->setLineStyle(QCPGraph::LineStyle::lsNone);
    }
    graph(0)->setData(QVector<double>(), QVector<double>());
    for (int i = 0; i < optimize_points_to_draw; i++) {
        graph(0)->addData(optimize_points[i][0], optimize_points[i][1]);
    }
}

void Drawer::replot_lines() {
    auto [start, end] = xAxis->range();
    double step       = (end - start) / COUNT;
    double z          = FIRST_LINE;
    for (int index = 0; index < LINE_COUNT; index++) {
        try {
            std::vector<double> x_up, x_down, y_up, y_down;
            for (double point = start; point < end; point += step) {
                auto temp  = func.get_y(point, z);
                double y_d = temp.first, y_u = temp.second;
                if (y_d > y_u) {
                    std::swap(y_d, y_u);
                }
                if (y_d == y_d) {
                    x_down.insert(x_down.begin(), point);
                    y_down.insert(y_down.begin(), y_d);
                }
                if (y_u == y_u) {
                    x_up.emplace_back(point);
                    y_up.emplace_back(y_u);
                }
            }
            x_up.insert(x_up.end(), x_down.begin(), x_down.end());
            y_up.insert(y_up.end(), y_down.begin(), y_down.end());
            if (!x_up.empty()) {
                x_up.emplace_back(x_up.front());
                y_up.emplace_back(y_up.front());
            }
            if (draw_level_lines) {
                curves[index]->setLineStyle(QCPCurve::LineStyle::lsLine);
            } else {
                curves[index]->setLineStyle(QCPCurve::LineStyle::lsNone);
            }
            curves[index]->setData(QVector<double>::fromStdVector(x_up),
                                   QVector<double>::fromStdVector(y_up));
            z *= LINE_STEP;
        } catch (int a) {
        }
    }
}

void Drawer::rescale_on_click(QCPAbstractPlottable *plottable, int,
                              QMouseEvent *) {
    plottable->rescaleAxes();
}