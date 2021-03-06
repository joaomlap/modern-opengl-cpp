#include <cmath>
#include <stdio.h>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Window dimensions
const GLint WIDTH = 800, HEIGHT = 600;

GLuint VAO, VBO, shader, uniformModel;

bool direction = true;
float triOffset = 0.0f;
float triMaxOffset = 0.7f;
float triIncrement = 0.001f;

// Vertex Shader
static const char *vShader = "                                      \n\
#version 410                                                        \n\
layout (location = 0) in vec3 pos;                                  \n\
                                                                    \n\
out vec4 vCol;                                                      \n\
                                                                    \n\
uniform mat4 model;                                                 \n\
                                                                    \n\
void main() {                                                       \n\
  gl_Position = model * vec4(pos.x, pos.y, pos.z, 1.0);             \n\
  vCol = vec4(clamp(pos, 0.0f, 1.0f), 1.0f);                        \n\
}";

// Fragment Shader
static const char *fShader = "                              \n\
#version 410                                                \n\
                                                            \n\
in vec4 vCol;                                               \n\
                                                            \n\
out vec4 colour;                                            \n\
                                                            \n\
void main() {                                               \n\
  colour = vCol;                                            \n\
}";

void createTriangle() {
  GLfloat vertices[] = {
      -1.0, -1.0f, 0.0f, 1.0, -1.0f, 0.0f, 0.0, 1.0f, 0.0f,
  };

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);
}

void addShader(GLuint program, const char *shaderCode, GLenum shaderType) {
  GLuint currShader = glCreateShader(shaderType);
  const GLchar *code[1];
  code[0] = shaderCode;

  GLint codeLength[1];
  codeLength[0] = strlen(shaderCode);

  glShaderSource(currShader, 1, code, codeLength);
  glCompileShader(currShader);

  GLint result = 0;
  GLchar eLog[1024] = {0};

  glGetShaderiv(currShader, GL_COMPILE_STATUS, &result);
  if (!result) {
    glGetShaderInfoLog(currShader, sizeof(eLog), NULL, eLog);
    printf("Error: Could not compile %d shader: %s\n", shaderType, eLog);

    return;
  }

  glAttachShader(program, currShader);
}

void compileShaders() {
  shader = glCreateProgram();

  if (!shader) {
    printf("Error: Could not create shader program!\n");
    return;
  }

  addShader(shader, vShader, GL_VERTEX_SHADER);
  addShader(shader, fShader, GL_FRAGMENT_SHADER);

  GLint result = 0;
  GLchar eLog[1024] = {0};

  glLinkProgram(shader);
  glGetProgramiv(shader, GL_LINK_STATUS, &result);

  if (!result) {
    glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
    printf("Error: Could not link program: %s\n", eLog);

    return;
  }

  glValidateProgram(shader);
  glGetProgramiv(shader, GL_VALIDATE_STATUS, &result);

  if (!result) {
    glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
    printf("Error: Could not validate program: %s\n", eLog);

    return;
  }

  uniformModel = glGetUniformLocation(shader, "model");
}

int main() {
  if (!glfwInit()) {
    printf("Error: GLFW initialisation failed!\n");
    glfwTerminate();

    return 1;
  }

  // Setup GLfw window properties
  // OpenGL version
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  // Core profile means no backwards compatibility
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // Allow forward compatibility
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow *mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL João", NULL, NULL);

  if (!mainWindow) {
    printf("Error: GLFW window creation failed!\n");
    glfwTerminate();

    return 1;
  }

  // Get buffer size information
  int bufferWidth, bufferHeight;
  glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

  // Set context for GLEW to use
  glfwMakeContextCurrent(mainWindow);

  // Allow modern extension features
  glewExperimental = GL_TRUE;

  if (glewInit() != GLEW_OK) {
    printf("Error: GLEW initialisation failed!\n");
    glfwDestroyWindow(mainWindow);
    glfwTerminate();

    return 1;
  }

  // Setup viewport size
  glViewport(0, 0, bufferWidth, bufferHeight);

  createTriangle();
  compileShaders();

  // Loop until window close
  while (!glfwWindowShouldClose(mainWindow)) {
    // Get and handle user input events
    glfwPollEvents();

    if (direction) {
      triOffset += triIncrement;
    } else {
      triOffset -= triIncrement;
    }

    if (abs(triOffset) >= triMaxOffset) {
      direction = !direction;
    }

    // Clear window
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(triOffset, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.4, 0.4, 1));

    // bind uniform variable
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    // glUseProgram(0);

    glfwSwapBuffers(mainWindow);
  }

  // Cleanup VBO
  glDeleteBuffers(1, &VBO);
  glDeleteVertexArrays(1, &VAO);
  glDeleteProgram(shader);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  return 0;
}
