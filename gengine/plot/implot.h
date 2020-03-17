//
// Created by lasagnaphil on 20. 1. 1..
//

#ifndef GENGINE_IMPLOT_H
#define GENGINE_IMPLOT_H

#include <span.hpp>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "glmx/rect.h"
#include "Colors.h"
#include "Shader.h"

namespace plt {

struct Point2D {
    glm::vec2 pos;
    glm::vec3 color;
    float size;
};

struct Line2D {
    std::vector<glm::vec2> pos;
    glm::vec3 color;
    float lineWidth;
    bool drawPoints;
    float pointSize;
};

struct ImPlot2DResult {
    glm::vec2 mousePos;
    std::optional<uint32_t> clickedPointIdx = {};
    std::optional<uint32_t> clickedLineIdx = {};
};

struct ImPlot2DContext {
    std::vector<Point2D> points2D;
    std::vector<Line2D> lines2D;

    glmx::rect bounds;
    glm::mat4 viewMat;
    glm::mat4 projMat;

    uint32_t fbo = 0;
    uint32_t rbo = 0;

    uint32_t pointVAO;
    uint32_t pointVBO;

    uint32_t lineVAO;
    uint32_t lineVBO;
    std::vector<glm::vec2> entireLinePos;
    std::vector<uint32_t> lineStartIndices;

    Ref<Shader> point2DShader;
    Ref<Shader> line2DShader;

    uint32_t tex = 0;
    float sizeX;
    float sizeY;

    float grabRadius = 4.0f;

    bool renderFinished = false;

    bool autoscaleEnabled = true;

    static ImPlot2DContext create(float sizeX, float sizeY);

    void plotPoint(const Point2D& point) {
        points2D.push_back(point);
    }

    void plotPoint(glm::vec2 pos, glm::vec3 color, float size) {
        points2D.push_back({pos, color, size});
    }

    void plotPoints(nonstd::span<Point2D> points) {
        points2D.insert(points2D.begin(), points.begin(), points.end());
    }

    void plotPoints(nonstd::span<float> X, nonstd::span<float> Y, glm::vec3 color = colors::Blue, float size = 1.0f) {
        assert(X.size() == Y.size());
        points2D.reserve(points2D.size() + X.size());
        for (int i = 0; i < X.size(); i++) {
            points2D.push_back(Point2D { {X[i], Y[i]}, color, size });
        }
    }

    void plotPoints(nonstd::span<glm::vec2> points, glm::vec3 color = colors::Blue, float size = 1.0f) {
        points2D.reserve(points2D.size() + points.size());
        for (auto& point : points) {
            points2D.push_back(Point2D { point, color, size });
        }
    }

    void plotLine(const Line2D& line) {
        lines2D.push_back(line);
    }

    void plotLine(nonstd::span<glm::vec2> pos,
            glm::vec3 color = colors::Blue, float lineWidth = 1.0f, bool drawPoints = true, float pointSize = 2.0f) {
        lines2D.push_back(Line2D {
            std::vector<glm::vec2>(pos.begin(), pos.end()), color, lineWidth, drawPoints, pointSize
        });
    }

    void plotLine(nonstd::span<float> X, nonstd::span<float> Y,
            glm::vec3 color = colors::Blue, float lineWidth = 1.0f, bool drawPoints = true, float pointSize = 2.0f) {
        assert(X.size() == Y.size());
        std::vector<glm::vec2> pos(X.size());
        for (int i = 0; i < X.size(); i++) {
            pos[i].x = X[i];
            pos[i].y = Y[i];
        }
        lines2D.push_back(Line2D {
            pos, color, lineWidth, drawPoints, pointSize
        });
    }

    void plotLines(nonstd::span<Line2D> lines) {
        lines2D.insert(lines2D.begin(), lines.begin(), lines.end());
    }

    void clear();

    // Methods related to rendering

    ImPlot2DResult show();

    void autoscale();

    void saveToImage(const std::string& filename);
};

}

#endif //GENGINE_IMPLOT_H
