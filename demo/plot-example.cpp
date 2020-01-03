//
// Created by lasagnaphil on 20. 1. 1..
//

#include "App.h"
#include "plot/implot.h"
#include <fmt/core.h>

struct ExamplePlotApp : public App {
    plt::ImPlot2DContext plt;

    ExamplePlotApp() : App(false) {}

    void loadResources() override {
        // TODO: need static camera
        FlyCamera* camera = initCamera<FlyCamera>();
        plt = plt::ImPlot2DContext::create(400.0f, 400.0f);
    }

    void update(float dt) override {
    }

    void render() override {
        std::vector<float> X = {10, 20, 30, 40, 50};
        std::vector<float> Y = {10, 30, 60, 100, 150};
        plt.plotPoints(X, Y, colors::Blue, 5.0f);
        plt.show();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("ImPlotApp");

        ImGui::Image((void*)plt.tex, ImVec2(plt.sizeX, plt.sizeY));

        /*
        auto mousePos = ImGui::GetMousePos();
        if (ImGui::IsItemClicked()) {
            plt.mouseClick(mousePos.x, mousePos.y, SDL_BUTTON_LEFT);
        }
        if (ImGui::IsItemHovered()) {
            if (ImGui::IsMouseDragging()) {
                auto dd = ImGui::GetMouseDragDelta();
                plt.mouseDrag(dd.x, dd.y);
            }
            else {
                plt.mouseHover(mousePos.x, mousePos.y);
            }
        }
         */

        ImGui::End();
    }

    void release() override {
        plt.saveToImage("graph.png");
    }
};

int main() {
    auto app = ExamplePlotApp();
    app.start();
    app.release();
}
