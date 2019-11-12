//
// Created by lasagnaphil on 19. 11. 12..
//

#ifndef IMGUI_PLOT_H
#define IMGUI_PLOT_H

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include <glm/vec3.hpp>
#include <glmx/rect.h>
#include <span.hpp>

namespace ImGui {
    const glm::vec3 PlotDefaultColors[22] = {
            {0.901961, 0.098039, 0.294118},
            {0.235294, 0.705882, 0.294118},
            {1.000000, 0.882353, 0.098039},
            {0.262745, 0.388235, 0.847059},
            {0.960784, 0.509804, 0.192157},
            {0.568627, 0.117647, 0.705882},
            {0.274510, 0.941176, 0.941176},
            {0.941176, 0.196078, 0.901961},
            {0.737255, 0.964706, 0.047059},
            {0.980392, 0.745098, 0.745098},
            {0.000000, 0.501961, 0.501961},
            {0.901961, 0.745098, 1.000000},
            {0.603922, 0.388235, 0.141176},
            {1.000000, 0.980392, 0.784314},
            {0.501961, 0.000000, 0.000000},
            {0.666667, 1.000000, 0.764706},
            {0.501961, 0.501961, 0.000000},
            {1.000000, 0.847059, 0.694118},
            {0.000000, 0.000000, 0.458824},
            {0.501961, 0.501961, 0.501961},
            {1.000000, 1.000000, 1.000000},
            {0.000000, 0.000000, 0.000000},
    };

    struct ScatterPlotDetails {
        std::vector<float> X;
        std::vector<float> Y;
        std::vector<uint8_t> colors;
        std::vector<glm::vec3> colorMap;
        std::vector<std::string> tags;
        glmx::rect bounds;
        float gridSize = 1.0f;
        float pointSize = 6.0f;
        float grabRadius = pointSize;
    };

    // Draw a scatter plot.
    // If a point is clicked, then returns the index of that point.
    // Else, return -1.
    int Scatter(const char* label, const ScatterPlotDetails& details) {
        assert(details.X.size() == details.Y.size());
        assert(details.X.size() == details.colors.size());

        const ImGuiStyle& style = ImGui::GetStyle();
        const ImGuiIO& io = ImGui::GetIO();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return -1;

        const float avail = GetContentRegionAvailWidth();
        const float dim = ImMin(avail, 512.f);
        ImVec2 canvas(dim, dim);

        ImRect bb(window->DC.CursorPos, window->DC.CursorPos + canvas);
        ItemSize(bb);
        if (!ItemAdd(bb, NULL)) return -1;

        RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg, 1), true, style.FrameRounding);

        const float gridSizeX = details.gridSize * (bb.Max.x - bb.Min.x) / (details.bounds.max.x - details.bounds.min.x);
        for (int i = 0; i <= canvas.x; i += gridSizeX) {
            drawList->AddLine(
                    ImVec2(bb.Min.x + i, bb.Min.y),
                    ImVec2(bb.Min.x + i, bb.Max.y),
                    GetColorU32(ImGuiCol_TextDisabled));
        }
        const float gridSizeY = details.gridSize * (bb.Max.y - bb.Min.y) / (details.bounds.max.y - details.bounds.min.y);
        for (int i = 0; i <= canvas.y; i += gridSizeY) {
            drawList->AddLine(
                    ImVec2(bb.Min.x, bb.Min.y + i),
                    ImVec2(bb.Max.x, bb.Min.y + i),
                    GetColorU32(ImGuiCol_TextDisabled));
        }

        // handle grabbers
        float screenToRealX = (details.bounds.max.x - details.bounds.min.x) / (bb.Max.x - bb.Min.x);
        float screenToRealY = (details.bounds.max.y - details.bounds.min.y) / (bb.Max.y - bb.Min.y);

        ImVec2 mouse = GetIO().MousePos;
        float relMouseX = (mouse.x - bb.Min.x) / (bb.Max.x - bb.Min.x);
        float mX = details.bounds.min.x + screenToRealX * (mouse.x - bb.Min.x);
        float mY = details.bounds.max.y - screenToRealY * (mouse.y - bb.Min.y);

        int selected = -1;
        bool mouseClicked = false;

        float distMin = std::numeric_limits<float>::max();
        for (int i = 0; i < details.X.size(); i++) {
            float x = details.X[i];
            float y = details.Y[i];
            float dist = (x - mX) * (x - mX) + (y - mY) * (y - mY);
            if (dist < distMin) {
                distMin = dist;
                selected = i;
            }
        }

        float threshold = details.grabRadius * details.grabRadius * screenToRealX * screenToRealY;
        if (distMin < threshold)
        {
            if (details.tags.empty()) {
                SetTooltip("(%4.3f, %4.3f)", details.X[selected], details.Y[selected]);
            }
            else {
                SetTooltip("%s: (%4.3f, %4.3f)", details.tags[selected].c_str(), details.X[selected], details.Y[selected]);
            }

            mouseClicked = IsMouseClicked(0);
        }

        for (int i = 0; i < details.X.size(); i++) {
            float x = details.X[i];
            float y = details.Y[i];
            if (!details.bounds.isInside({x, y})) continue;
            float boundX = details.bounds.max.x - details.bounds.min.x;
            float boundY = details.bounds.max.y - details.bounds.min.y;
            float sx = bb.Min.x + (bb.Max.x - bb.Min.x) * (x - details.bounds.min.x) / boundX;
            float sy = bb.Max.y + (bb.Min.y - bb.Max.y) * (y - details.bounds.min.y) / boundY;
            glm::vec3 color = details.colorMap[details.colors[i]];
            drawList->AddCircleFilled(ImVec2(sx, sy), details.pointSize, ImColor(color.x, color.y, color.z));
        }

        if (mouseClicked) return selected;
        else return -1;
    }
}

#endif //IMGUI_PLOT_H
