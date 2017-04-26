#ifndef SHADERS_HPP
#define SHADERS_HPP

const char VERTEX_SHADER[] = ""
"#version 330 core \n"
"layout(location = 0) in vec2 pos;\n"
"layout(location = 1) in vec4 color;\n"
"out vec4 vertexColor;\n"
"uniform mat4 orthoMatrix;\n"
"void main(){\n"
"	gl_Position = orthoMatrix * vec4(pos, 1.0f, 1.0f);\n"
"	vertexColor = color;\n"
"}\n";


const char FRAGMENT_SHADER[] = ""
"#version 330 core \n"
"in vec4 vertexColor;\n"
"layout(location = 0) out vec4 fragmentColor;\n"
"void main(){\n"
"	fragmentColor = vertexColor;\n"
"}\n";


const char VERTEX_TEXTURE_SHADER[] = ""
"#version 330 core \n"
"layout(location = 0) in vec2 pos;\n"
"layout(location = 1) in vec2 texCoord;\n"
"out vec2 outCoord;\n"
"uniform mat4 orthoMatrix;\n"
"void main() {\n"
"	gl_Position = orthoMatrix * vec4(pos, 1.0f, 1.0f);\n"
"	outCoord = texCoord;\n"
"}\n";

const char FRAGMENT_TEXTURE_SHADER[] = ""
"#version 330 core \n"
"in vec2 outCoord;\n"
"layout(location = 0) out vec4 fragmentColor;\n"
"uniform sampler2D tex;\n"
"void main(){\n"
"	fragmentColor = texture(tex, outCoord);\n"
"}\n";

#endif
