#include <shader-utils.h>
#include <stdio.h>
#include <stdlib.h>

GLuint quadVAO, quadVBO;
GLuint screenShaderProgram;

const float quadVertices[24] = {  // 6 verts * 4 floats
    // First triangle
    -1.0f, -1.0f,  0.0f, 0.0f,  // Bottom-left
     1.0f,  1.0f,  1.0f, 1.0f,  // Top-right
    -1.0f,  1.0f,  0.0f, 1.0f,  // Top-left
    // Second triangle
    -1.0f, -1.0f,  0.0f, 0.0f,  // Bottom-left
     1.0f, -1.0f,  1.0f, 0.0f,  // Bottom-right
     1.0f,  1.0f,  1.0f, 1.0f   // Top-right
};

static void PrintShaderError(GLuint shader, const char *type) {
    GLint len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    if (len > 1) {
        char *log = malloc((size_t)len);
        if (log) {
            glGetShaderInfoLog(shader, len, NULL, log);
            fprintf(stderr, "%s compile error: %s\n", type, log);
            free(log);
        }
    }
}

GLuint CreateShaderProgram(const char *vertexSrc, const char *fragmentSrc) {
    // 1. Compile Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSrc, NULL);
    glCompileShader(vertexShader);
    GLint ok = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        PrintShaderError(vertexShader, "vertexShader");
        glDeleteShader(vertexShader);
        return 0;
    }

    // 2. Compile Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSrc, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        PrintShaderError(fragmentShader, "fragmentShader");
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    // 3. Link Program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check link status (optional, but good practice)
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        if (len > 1) {
            char *log = malloc((size_t)len);
            if (log) {
                glGetProgramInfoLog(program, len, NULL, log);
                fprintf(stderr, "Program link error: %s\n", log);
                free(log);
            }
        }
        glDeleteProgram(program);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    // 4. Cleanup individual shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

void SetupScreenQuad(void) {
    const char *screenVertexSource =
        "#version 330 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "layout (location = 1) in vec2 aTexCoord;\n"
        "out vec2 TexCoord;\n"
        "void main(){\n"
        "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "    TexCoord = aTexCoord;\n"
        "}\n";

    const char *screenFragmentSource =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D screenTexture;\n"
        "void main() {\n"
        "    FragColor = texture(screenTexture, TexCoord);\n"
        "}\n";

    screenShaderProgram = CreateShaderProgram(screenVertexSource, screenFragmentSource);
    if (screenShaderProgram == 0) {
        fprintf(stderr, "Failed to create screen shader program\n");
        return;
    }

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}
