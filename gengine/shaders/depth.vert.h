// header file generated by lasagnaphil
#ifndef  TXT_HEADER_depth_vert_shader
#define  TXT_HEADER_depth_vert_shader


const char depth_vert_shader [] =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"\n"
"uniform mat4 dirLightSpaceMatrix;\n"
"uniform mat4 model;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = dirLightSpaceMatrix * model * vec4(aPos, 1.0);\n"
"}\n"
;


#endif  // #ifdef TXT_HEADER_depth_vert_shader
