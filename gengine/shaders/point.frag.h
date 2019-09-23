// header file generated by lasagnaphil
#ifndef  TXT_HEADER_point_frag_shader
#define  TXT_HEADER_point_frag_shader


const char point_frag_shader [] =
"#version 330 core\n"
"\n"
"uniform vec4 color;\n"
"\n"
"out vec4 fragColor;\n"
"\n"
"void main() {\n"
"    vec2 circCoord = 2.0 * gl_PointCoord - 1.0;\n"
"    if (dot(circCoord, circCoord) > 1.0) {\n"
"        discard;\n"
"    }\n"
"    fragColor = color;\n"
"}\n"
;


#endif  // #ifdef TXT_HEADER_point_frag_shader