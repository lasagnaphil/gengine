//
// Created by lasagnaphil on 20. 1. 1..
//

#include "gengine/App.h"
#include "gengine/plot/implot.h"
#include <fmt/core.h>

struct ExamplePlotApp : public App {
    plt::ImPlot2DContext plt;

    ExamplePlotApp() : App() {}

    void loadResources() override {
        // TODO: need static camera
        FlyCamera* camera = dynamic_cast<FlyCamera*>(this->camera.get());
        plt = plt::ImPlot2DContext::create(400.0f, 400.0f);
    }

    void update(float dt) override {
    }

    void render() override {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("ImPlotApp");

        std::vector<float> X = {10, 20, 30, 40, 50};
        std::vector<float> Y = {10, 30, 60, 100, 150};
        plt.plotPoints(X, Y, colors::Blue, 5.0f);
        auto res = plt.show();
        if (res.clickedPointIdx) {
            fmt::print("Point {} clicked!\n", *res.clickedPointIdx);
        }

        X = {10, 20, 30, 40, 50};
        Y = {150, 100, 60, 30, 10};
        plt.plotLine(X, Y, colors::Red, 2.0f, true, 4.0f);

        ImGui::End();
    }

    void release() override {
        plt.saveToImage("graph.png");
    }
};

int main() {
    auto app = ExamplePlotApp();
    app.load();
    app.startMainLoop();
    app.release();
}
