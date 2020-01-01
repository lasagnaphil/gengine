//
// Created by lasagnaphil on 20. 1. 1..
//

#include "plot/implot_window.h"
#include <thread>

int main() {
    auto implot = ImPlotApp();
    auto thread = std::thread([&implot] {implot.start(); implot.release();});
    // TODO: wait until app is initialized
    std::vector<float> X = {1, 2, 3, 4, 5};
    std::vector<float> Y = {1 ,3, 6, 10, 15};
    implot.plt().plotPoints(X, Y);
}
