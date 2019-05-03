//
// Created by lasagnaphil on 2017-03-27.
//

#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "GLUtils.h"
#include "Material.h"
#include "Camera.h"

GLuint compileShader(GLenum type, const GLchar *source) {
    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_TRUE) {
        std::cout << "Shader (type " << type << ") is compiled successfully!" << std::endl;
    } else {
        std::cout << "Shader (type " << type << ") compile failed!" << std::endl;
    }

    std::cout << "Compile log: " << std::endl;
    char compileInfo[512];
    glGetShaderInfoLog(shader, 512, NULL, compileInfo);
    std::cout << compileInfo << std::endl;

    return shader;
}

static std::string loadFile(const std::string& path) {
    std::string contents;
    std::ifstream fs;

    fs.open(path, std::ios::in);
    if (!fs) {
        std::cerr << "Error while loading file " << path << ":" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::stringstream buf;
    buf << fs.rdbuf();
    fs.close();
    return buf.str();
}

void Shader::compile() {
    bool hasGeom = !geometryPath.empty();

    program = glCreateProgram();

    std::string vertexCode = loadFile(vertexPath);
    std::string fragmentCode = loadFile(fragmentPath);

    std::string geometryCode;
    if (hasGeom) {
        geometryCode = loadFile(geometryPath);
    }

    const GLchar* vertexCodePtr = vertexCode.data();
    const GLchar* fragmentCodePtr = fragmentCode.data();

    GLuint vertexShaderPtr = compileShader(GL_VERTEX_SHADER, vertexCodePtr);
    GLuint fragmentShaderPtr = compileShader(GL_FRAGMENT_SHADER, fragmentCodePtr);
    glAttachShader(program, vertexShaderPtr);
    glAttachShader(program, fragmentShaderPtr);

    GLuint geometryShaderPtr;
    if (hasGeom) {
        const GLchar* geometryCodePtr = geometryCode.data();
        geometryShaderPtr = compileShader(GL_GEOMETRY_SHADER, geometryCodePtr);
        glAttachShader(program, geometryShaderPtr);
    }

    glLinkProgram(program);
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "Error: shader program linking failed" << infoLog << std::endl;
    }

    glDeleteShader(vertexShaderPtr);
    glDeleteShader(fragmentShaderPtr);
    if (hasGeom) glDeleteShader(geometryShaderPtr);
}

void Shader::use() const {
    glUseProgram(this->program);
}

void Shader::setBool(const char* name, bool value) const {
    glUniform1i(glGetUniformLocation(program, name), (int)value);
}

void Shader::setBool(GLint uniID, bool value) const {
    glUniform1i(uniID, (int)value);
}

void Shader::setInt(const char* name, int value) const {
    glUniform1i(glGetUniformLocation(program, name), value);
}

void Shader::setInt(GLint uniID, int value) const {
    glUniform1i(uniID, value);
}

void Shader::setFloat(const char* name, float value) const {
    glUniform1f(glGetUniformLocation(program, name), value);
}

void Shader::setFloat(GLint uniID, float value) const {
    glUniform1f(uniID, value);
}

void Shader::setMat4(const char* name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMat4(GLint uniID, const glm::mat4& value) const {
    glUniformMatrix4fv(uniID, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec3(const char* name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(program, name), 1, glm::value_ptr(value));
}

void Shader::setVec3(GLint uniID, const glm::vec3& value) const {
    glUniform3fv(uniID, 1, glm::value_ptr(value));
}

void Shader::setVec4(const char* name, const glm::vec4& value) const {
    glUniform4fv(glGetUniformLocation(program, name), 1, glm::value_ptr(value));
}

void Shader::setVec4(GLint uniID, const glm::vec4& value) const {
    glUniform4fv(uniID, 1, glm::value_ptr(value));
}

void Shader::setMaterial(const Material &material) const {
    setVec4("material.ambient", material.ambient);
    setFloat("material.shininess", material.shininess);
    setBool("material.useTexDiffuse", !material.texDiffuse.isNull());
    setBool("material.useTexSpecular", !material.texSpecular.isNull());
    if (material.texDiffuse) {
        glActiveTexture(GL_TEXTURE0);
        material.texDiffuse->bind();
    }
    else {
        setVec4("material.diffuse", material.diffuse);
    }
    if (material.texSpecular) {
        glActiveTexture(GL_TEXTURE1);
        material.texSpecular->bind();
    }
    else {
        setVec4("material.specular", material.specular);
    }
    setInt("material.texDiffuse", 0);
    setInt("material.texSpecular", 1);
}

void Shader::setCamera(const Camera &camera) const {
    setMat4("proj", camera.getPerspectiveMatrix());
    setMat4("view", camera.getViewMatrix());
    setVec3("viewPos", camera.transform->getPosition());
}

GLint Shader::getUniformLocation(const char* name) {
    return glGetUniformLocation(program, name);
}

Ref<Shader> Shaders::line2D = {};
Ref<Shader> Shaders::line3D = {};
Ref<Shader> Shaders::point = {};
Ref<Shader> Shaders::phong = {};
Ref<Shader> Shaders::depth = {};
Ref<Shader> Shaders::depthDebug = {};

void Shaders::init() {
    Shaders::line2D = Resources::make<Shader>("gengine/shaders/line2d.vert", "gengine/shaders/line2d.frag");
    Shaders::line2D->compile();

    Shaders::line3D = Resources::make<Shader>("gengine/shaders/line3d.vert", "gengine/shaders/line3d.frag");
    Shaders::line3D->compile();

    Shaders::point = Resources::make<Shader>("gengine/shaders/point.vert", "gengine/shaders/point.frag");
    Shaders::point->compile();

    Shaders::phong = Resources::make<Shader>("gengine/shaders/phong_shadows.vert", "gengine/shaders/phong_shadows.frag");
    Shaders::phong->compile();

    Shaders::depth = Resources::make<Shader>("gengine/shaders/depth.vert", "gengine/shaders/depth.frag");
    Shaders::depth->compile();

    Shaders::depthDebug = Resources::make<Shader>("gengine/shaders/depth_debug.vert", "gengine/shaders/depth_debug.frag");
    Shaders::depthDebug->compile();
}
