//
// Created by lasagnaphil on 19. 9. 30..
//

#ifndef GENGINE_DEBUGRENDERER_H
#define GENGINE_DEBUGRENDERER_H

#include "Camera.h"
#include "Shader.h"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glad/glad.h>
#include <span.hpp>

namespace colors {
    static const glm::vec3 AliceBlue = {0.941176f, 0.972549f, 1.000000f};
    static const glm::vec3 AntiqueWhite = {0.980392f, 0.921569f, 0.843137f};
    static const glm::vec3 Aquamarine = {0.498039f, 1.000000f, 0.831373f};
    static const glm::vec3 Azure = {0.941176f, 1.000000f, 1.000000f};
    static const glm::vec3 Beige = {0.960784f, 0.960784f, 0.862745f};
    static const glm::vec3 Bisque = {1.000000f, 0.894118f, 0.768627f};
    static const glm::vec3 Black = {0.000000f, 0.000000f, 0.000000f};
    static const glm::vec3 BlanchedAlmond = {1.000000f, 0.921569f, 0.803922f};
    static const glm::vec3 Blue = {0.000000f, 0.000000f, 1.000000f};
    static const glm::vec3 BlueViolet = {0.541176f, 0.168627f, 0.886275f};
    static const glm::vec3 Brown = {0.647059f, 0.164706f, 0.164706f};
    static const glm::vec3 BurlyWood = {0.870588f, 0.721569f, 0.529412f};
    static const glm::vec3 CadetBlue = {0.372549f, 0.619608f, 0.627451f};
    static const glm::vec3 Chartreuse = {0.498039f, 1.000000f, 0.000000f};
    static const glm::vec3 Chocolate = {0.823529f, 0.411765f, 0.117647f};
    static const glm::vec3 Coral = {1.000000f, 0.498039f, 0.313726f};
    static const glm::vec3 CornflowerBlue = {0.392157f, 0.584314f, 0.929412f};
    static const glm::vec3 Cornsilk = {1.000000f, 0.972549f, 0.862745f};
    static const glm::vec3 Crimson = {0.862745f, 0.078431f, 0.235294f};
    static const glm::vec3 Cyan = {0.000000f, 1.000000f, 1.000000f};
    static const glm::vec3 DarkBlue = {0.000000f, 0.000000f, 0.545098f};
    static const glm::vec3 DarkCyan = {0.000000f, 0.545098f, 0.545098f};
    static const glm::vec3 DarkGoldenRod = {0.721569f, 0.525490f, 0.043137f};
    static const glm::vec3 DarkGray = {0.662745f, 0.662745f, 0.662745f};
    static const glm::vec3 DarkGreen = {0.000000f, 0.392157f, 0.000000f};
    static const glm::vec3 DarkKhaki = {0.741176f, 0.717647f, 0.419608f};
    static const glm::vec3 DarkMagenta = {0.545098f, 0.000000f, 0.545098f};
    static const glm::vec3 DarkOliveGreen = {0.333333f, 0.419608f, 0.184314f};
    static const glm::vec3 DarkOrange = {1.000000f, 0.549020f, 0.000000f};
    static const glm::vec3 DarkOrchid = {0.600000f, 0.196078f, 0.800000f};
    static const glm::vec3 DarkRed = {0.545098f, 0.000000f, 0.000000f};
    static const glm::vec3 DarkSalmon = {0.913725f, 0.588235f, 0.478431f};
    static const glm::vec3 DarkSeaGreen = {0.560784f, 0.737255f, 0.560784f};
    static const glm::vec3 DarkSlateBlue = {0.282353f, 0.239216f, 0.545098f};
    static const glm::vec3 DarkSlateGray = {0.184314f, 0.309804f, 0.309804f};
    static const glm::vec3 DarkTurquoise = {0.000000f, 0.807843f, 0.819608f};
    static const glm::vec3 DarkViolet = {0.580392f, 0.000000f, 0.827451f};
    static const glm::vec3 DeepPink = {1.000000f, 0.078431f, 0.576471f};
    static const glm::vec3 DeepSkyBlue = {0.000000f, 0.749020f, 1.000000f};
    static const glm::vec3 DimGray = {0.411765f, 0.411765f, 0.411765f};
    static const glm::vec3 DodgerBlue = {0.117647f, 0.564706f, 1.000000f};
    static const glm::vec3 FireBrick = {0.698039f, 0.133333f, 0.133333f};
    static const glm::vec3 FloralWhite = {1.000000f, 0.980392f, 0.941176f};
    static const glm::vec3 ForestGreen = {0.133333f, 0.545098f, 0.133333f};
    static const glm::vec3 Gainsboro = {0.862745f, 0.862745f, 0.862745f};
    static const glm::vec3 GhostWhite = {0.972549f, 0.972549f, 1.000000f};
    static const glm::vec3 Gold = {1.000000f, 0.843137f, 0.000000f};
    static const glm::vec3 GoldenRod = {0.854902f, 0.647059f, 0.125490f};
    static const glm::vec3 Gray = {0.501961f, 0.501961f, 0.501961f};
    static const glm::vec3 Green = {0.000000f, 0.501961f, 0.000000f};
    static const glm::vec3 GreenYellow = {0.678431f, 1.000000f, 0.184314f};
    static const glm::vec3 HoneyDew = {0.941176f, 1.000000f, 0.941176f};
    static const glm::vec3 HotPink = {1.000000f, 0.411765f, 0.705882f};
    static const glm::vec3 IndianRed = {0.803922f, 0.360784f, 0.360784f};
    static const glm::vec3 Indigo = {0.294118f, 0.000000f, 0.509804f};
    static const glm::vec3 Ivory = {1.000000f, 1.000000f, 0.941176f};
    static const glm::vec3 Khaki = {0.941176f, 0.901961f, 0.549020f};
    static const glm::vec3 Lavender = {0.901961f, 0.901961f, 0.980392f};
    static const glm::vec3 LavenderBlush = {1.000000f, 0.941176f, 0.960784f};
    static const glm::vec3 LawnGreen = {0.486275f, 0.988235f, 0.000000f};
    static const glm::vec3 LemonChiffon = {1.000000f, 0.980392f, 0.803922f};
    static const glm::vec3 LightBlue = {0.678431f, 0.847059f, 0.901961f};
    static const glm::vec3 LightCoral = {0.941176f, 0.501961f, 0.501961f};
    static const glm::vec3 LightCyan = {0.878431f, 1.000000f, 1.000000f};
    static const glm::vec3 LightGoldenYellow = {0.980392f, 0.980392f, 0.823529f};
    static const glm::vec3 LightGray = {0.827451f, 0.827451f, 0.827451f};
    static const glm::vec3 LightGreen = {0.564706f, 0.933333f, 0.564706f};
    static const glm::vec3 LightPink = {1.000000f, 0.713726f, 0.756863f};
    static const glm::vec3 LightSalmon = {1.000000f, 0.627451f, 0.478431f};
    static const glm::vec3 LightSeaGreen = {0.125490f, 0.698039f, 0.666667f};
    static const glm::vec3 LightSkyBlue = {0.529412f, 0.807843f, 0.980392f};
    static const glm::vec3 LightSlateGray = {0.466667f, 0.533333f, 0.600000f};
    static const glm::vec3 LightSteelBlue = {0.690196f, 0.768627f, 0.870588f};
    static const glm::vec3 LightYellow = {1.000000f, 1.000000f, 0.878431f};
    static const glm::vec3 Lime = {0.000000f, 1.000000f, 0.000000f};
    static const glm::vec3 LimeGreen = {0.196078f, 0.803922f, 0.196078f};
    static const glm::vec3 Linen = {0.980392f, 0.941176f, 0.901961f};
    static const glm::vec3 Magenta = {1.000000f, 0.000000f, 1.000000f};
    static const glm::vec3 Maroon = {0.501961f, 0.000000f, 0.000000f};
    static const glm::vec3 MediumAquaMarine = {0.400000f, 0.803922f, 0.666667f};
    static const glm::vec3 MediumBlue = {0.000000f, 0.000000f, 0.803922f};
    static const glm::vec3 MediumOrchid = {0.729412f, 0.333333f, 0.827451f};
    static const glm::vec3 MediumPurple = {0.576471f, 0.439216f, 0.858824f};
    static const glm::vec3 MediumSeaGreen = {0.235294f, 0.701961f, 0.443137f};
    static const glm::vec3 MediumSlateBlue = {0.482353f, 0.407843f, 0.933333f};
    static const glm::vec3 MediumSpringGreen = {0.000000f, 0.980392f, 0.603922f};
    static const glm::vec3 MediumTurquoise = {0.282353f, 0.819608f, 0.800000f};
    static const glm::vec3 MediumVioletRed = {0.780392f, 0.082353f, 0.521569f};
    static const glm::vec3 MidnightBlue = {0.098039f, 0.098039f, 0.439216f};
    static const glm::vec3 MintCream = {0.960784f, 1.000000f, 0.980392f};
    static const glm::vec3 MistyRose = {1.000000f, 0.894118f, 0.882353f};
    static const glm::vec3 Moccasin = {1.000000f, 0.894118f, 0.709804f};
    static const glm::vec3 NavajoWhite = {1.000000f, 0.870588f, 0.678431f};
    static const glm::vec3 Navy = {0.000000f, 0.000000f, 0.501961f};
    static const glm::vec3 OldLace = {0.992157f, 0.960784f, 0.901961f};
    static const glm::vec3 Olive = {0.501961f, 0.501961f, 0.000000f};
    static const glm::vec3 OliveDrab = {0.419608f, 0.556863f, 0.137255f};
    static const glm::vec3 Orange = {1.000000f, 0.647059f, 0.000000f};
    static const glm::vec3 OrangeRed = {1.000000f, 0.270588f, 0.000000f};
    static const glm::vec3 Orchid = {0.854902f, 0.439216f, 0.839216f};
    static const glm::vec3 PaleGoldenRod = {0.933333f, 0.909804f, 0.666667f};
    static const glm::vec3 PaleGreen = {0.596078f, 0.984314f, 0.596078f};
    static const glm::vec3 PaleTurquoise = {0.686275f, 0.933333f, 0.933333f};
    static const glm::vec3 PaleVioletRed = {0.858824f, 0.439216f, 0.576471f};
    static const glm::vec3 PapayaWhip = {1.000000f, 0.937255f, 0.835294f};
    static const glm::vec3 PeachPuff = {1.000000f, 0.854902f, 0.725490f};
    static const glm::vec3 Peru = {0.803922f, 0.521569f, 0.247059f};
    static const glm::vec3 Pink = {1.000000f, 0.752941f, 0.796078f};
    static const glm::vec3 Plum = {0.866667f, 0.627451f, 0.866667f};
    static const glm::vec3 PowderBlue = {0.690196f, 0.878431f, 0.901961f};
    static const glm::vec3 Purple = {0.501961f, 0.000000f, 0.501961f};
    static const glm::vec3 RebeccaPurple = {0.400000f, 0.200000f, 0.600000f};
    static const glm::vec3 Red = {1.000000f, 0.000000f, 0.000000f};
    static const glm::vec3 RosyBrown = {0.737255f, 0.560784f, 0.560784f};
    static const glm::vec3 RoyalBlue = {0.254902f, 0.411765f, 0.882353f};
    static const glm::vec3 SaddleBrown = {0.545098f, 0.270588f, 0.074510f};
    static const glm::vec3 Salmon = {0.980392f, 0.501961f, 0.447059f};
    static const glm::vec3 SandyBrown = {0.956863f, 0.643137f, 0.376471f};
    static const glm::vec3 SeaGreen = {0.180392f, 0.545098f, 0.341176f};
    static const glm::vec3 SeaShell = {1.000000f, 0.960784f, 0.933333f};
    static const glm::vec3 Sienna = {0.627451f, 0.321569f, 0.176471f};
    static const glm::vec3 Silver = {0.752941f, 0.752941f, 0.752941f};
    static const glm::vec3 SkyBlue = {0.529412f, 0.807843f, 0.921569f};
    static const glm::vec3 SlateBlue = {0.415686f, 0.352941f, 0.803922f};
    static const glm::vec3 SlateGray = {0.439216f, 0.501961f, 0.564706f};
    static const glm::vec3 Snow = {1.000000f, 0.980392f, 0.980392f};
    static const glm::vec3 SpringGreen = {0.000000f, 1.000000f, 0.498039f};
    static const glm::vec3 SteelBlue = {0.274510f, 0.509804f, 0.705882f};
    static const glm::vec3 Tan = {0.823529f, 0.705882f, 0.549020f};
    static const glm::vec3 Teal = {0.000000f, 0.501961f, 0.501961f};
    static const glm::vec3 Thistle = {0.847059f, 0.749020f, 0.847059f};
    static const glm::vec3 Tomato = {1.000000f, 0.388235f, 0.278431f};
    static const glm::vec3 Turquoise = {0.250980f, 0.878431f, 0.815686f};
    static const glm::vec3 Violet = {0.933333f, 0.509804f, 0.933333f};
    static const glm::vec3 Wheat = {0.960784f, 0.870588f, 0.701961f};
    static const glm::vec3 White = {1.000000f, 1.000000f, 1.000000f};
    static const glm::vec3 WhiteSmoke = {0.960784f, 0.960784f, 0.960784f};
    static const glm::vec3 Yellow = {1.000000f, 1.000000f, 0.000000f};
    static const glm::vec3 YellowGreen = {0.603922f, 0.803922f, 0.196078f};
}

struct ImStringData {
    glm::vec3 color;
    float posX;
    float posY;
    float scaling;
    std::string text;
    bool centered;
};

struct ImPointData {
    glm::vec3 position;
    glm::vec3 color;
    float size;
    bool depthEnabled;
};

struct ImLineData {
    glm::vec3 from;
    glm::vec3 to;
    glm::vec3 color;
    bool depthEnabled;
};

union ImDrawVertex {
    struct {
        glm::vec3 pos;
        glm::vec3 color;
        float size;
    } point;

    struct {
        glm::vec3 pos;
        glm::vec3 color;
    } line;

    struct {
        glm::vec2 pos;
        glm::vec2 uv;
        glm::vec3 color;
    } glyph;
};

#define IM_VERTEX_BUFFER_SIZE 1024

struct DebugRenderer {

    DebugRenderer(Camera* camera = nullptr) : camera(camera) {}

    void setCamera(Camera* camera) { this->camera = camera; }

    void init();
    void render();

    void drawDebugPoints();
    void drawDebugLines();

    // Immediate-mode rendering functions
    void drawPoint(glm::vec3 pos, glm::vec3 color, float size, bool depthEnabled);
    void drawLine(glm::vec3 from, glm::vec3 to, glm::vec3 color, bool depthEnabled);
    void drawText(std::string_view str, glm::vec3 pos, glm::vec3 color, float scaling);
    void drawAxisTriad(glm::mat4 transform, float size, float length, bool depthEnabled);
    void drawArrow(glm::vec3 from, glm::vec3 to, glm::vec3 color, float size, bool depthEnabled);
    void drawCross(glm::vec3 center, float length, bool depthEnabled);
    void drawCircle(glm::vec3 center, glm::vec3 planeNormal, glm::vec3 color, float radius, int numSteps, bool depthEnabled);
    void drawPlane(glm::vec3 center, glm::vec3 planeColor,
            glm::vec3 planeNormal, glm::vec3 normalVecColor, float planeScale, float normalVecScale, bool depthEnabled);
    void drawSphere(glm::vec3 center, glm::vec3 color, float radius, bool depthEnabled);
    void drawCone(glm::vec3 apex, glm::vec3 dir, glm::vec3 color,
                    float baseRadius, float apexRadius, bool depthEnabled);
    void drawBox(nonstd::span<glm::vec3> points, glm::vec3 color, bool depthEnabled);
    void drawBox(glm::vec3 center, glm::vec3 color, float width, float height, float depth, bool depthEnabled);
    void drawFrustrum(glm::mat4 invClipMatrix, glm::vec3 color, bool depthEnabled);
    void drawVertexNormal(glm::vec3 origin, glm::vec3 normal, float length, bool depthEnabled);
    void drawTangentBasis(glm::vec3 origin, glm::vec3 normal, glm::vec3 tangent, glm::vec3 bitangent, float lengths, bool depthEnabled);
    void drawXZSquareGrid(float mins, float maxs, float y, float step, glm::vec3 color, bool depthEnabled);

private:
    void pushPointVert(const ImPointData& point);
    void pushLineVert(const ImLineData& line);

    void flushPointVerts(bool depthEnabled);
    void flushLineVerts(bool depthEnabled);

    Camera* camera;

    std::vector<ImStringData> stringData;
    std::vector<ImPointData> pointData;
    std::vector<ImLineData> lineData;

    std::array<ImDrawVertex, IM_VERTEX_BUFFER_SIZE> vertexBuffer;
    int vertexBufferUsed = 0;

    Ref<Shader> imPointShader;
    Ref<Shader> imLineShader;

    GLuint pointVao;
    GLuint lineVao;

    GLuint vbo;
};

#endif //GENGINE_DEBUGRENDERER_H
