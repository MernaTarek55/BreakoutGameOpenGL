#include <iostream>
#include <gl/glew/glew.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include<gl\glm\glm.hpp>
#include<gl\glm\gtc\type_ptr.hpp>
#include<gl\glm\gtx\transform.hpp>
#include <vector>
#include <fstream>
#include <string>
#include <SFML/Window.hpp>
#define M_PI 3.14159265358979323846


using namespace std;
using namespace glm;



const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 fragColor;

uniform mat4 model;

void main() {
    gl_Position = model * vec4(aPos, 1.0);
    fragColor = aColor;
}
)glsl";

const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor; // Output color

uniform vec3 color; // Color uniform (passed from the C++ code)

void main() {
    FragColor = vec4(color, 1.0); // Use the color passed to the shader
}

)glsl";

GLuint CompileShader(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::VERTEX_SHADER_COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::FRAGMENT_SHADER_COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::PROGRAM_LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint CreateBuffer(const std::vector<GLfloat>& vertices) {
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    return VBO;
}

void Render(GLuint VBO, int vertexCount) {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}
bool isWaitingForRestart = false;
int main() {
    sf::Window window(sf::VideoMode(800, 600), "Breakout Game", sf::Style::Default, sf::ContextSettings(32));
    window.setFramerateLimit(60);
    glewInit();

    GLuint shaderProgram = CompileShader(vertexShaderSource, fragmentShaderSource);
    glUseProgram(shaderProgram);

    std::vector<GLfloat> paddleVertices = {
        -0.15f, -0.9f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.15f, -0.9f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.15f, -0.85f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.15f, -0.9f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.15f, -0.85f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.15f, -0.85f, 0.0f, 1.0f, 0.0f, 0.0f
    };
    GLuint VBO_Paddle = CreateBuffer(paddleVertices);

    std::vector<GLfloat> ballVertices;
    int segments = 30;
    float radius = 0.03f;
    for (int i = 0; i < segments; ++i) {
        float angle1 = 2.0f * M_PI * i / segments;
        float angle2 = 2.0f * M_PI * (i + 1) / segments;

        ballVertices.push_back(0.0f); ballVertices.push_back(0.0f); ballVertices.push_back(0.0f); 
        ballVertices.push_back(0.0f); ballVertices.push_back(1.0f); ballVertices.push_back(0.0f);

        ballVertices.push_back(radius * cos(angle1)); ballVertices.push_back(radius * sin(angle1)); ballVertices.push_back(0.0f);
        ballVertices.push_back(0.0f); ballVertices.push_back(1.0f); ballVertices.push_back(0.0f);

        ballVertices.push_back(radius * cos(angle2)); ballVertices.push_back(radius * sin(angle2)); ballVertices.push_back(0.0f);
        ballVertices.push_back(0.0f); ballVertices.push_back(1.0f); ballVertices.push_back(0.0f);
    }
    GLuint VBO_Ball = CreateBuffer(ballVertices);

    std::vector<GLfloat> brickVertices = {
        -0.075f, -0.025f, 0.0f, 0.0f, 0.0f, 1.0f,
         0.075f, -0.025f, 0.0f, 0.0f, 0.0f, 1.0f,
         0.075f,  0.025f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.075f, -0.025f, 0.0f, 0.0f, 0.0f, 1.0f,
         0.075f,  0.025f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.075f,  0.025f, 0.0f, 0.0f, 0.0f, 1.0f
    };
    GLuint VBO_Bricks = CreateBuffer(brickVertices);

    std::vector<bool> brickStates(90, true); 

    vec3 ballPosition(0.0f, -0.8f, 0.0f);
    vec3 ballVelocity(0.03f, 0.03f, 0.0f);
    float paddleX = 0.0f;
    float switchnum = 0.0f;
    float shiftOffset = 0.0f;
    float shiftSpeed = 0.005f;
    bool movingRight = true;

    while (window.isOpen()) {
        switchnum += shiftSpeed;

        if (movingRight) {
            shiftOffset += shiftSpeed;
            if (switchnum > 0.24f) {
                movingRight = false;
                switchnum = 0;
            }
        }
        else {
            shiftOffset -= shiftSpeed;
            if (switchnum > 0.24f) {
                movingRight = true;
                switchnum = 0;
            }
        }
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (isWaitingForRestart && event.type == sf::Event::MouseButtonPressed) {
                isWaitingForRestart = false;
                ballVelocity = vec3(0.03f, 0.03f, 0.0f);
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            if (isWaitingForRestart) {
                ballPosition.x -= 0.02f;
                if (ballPosition.x < -0.85f) ballPosition.x = -0.85f;
            }
            paddleX -= 0.02f;
            if (paddleX < -0.85f) paddleX = -0.85f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            if (isWaitingForRestart) {
                ballPosition.x += 0.02f;
                if (ballPosition.x > 0.85f) ballPosition.x = 0.85f;
            }
            paddleX += 0.02f;
            if (paddleX > 0.85f) paddleX = 0.85f;
        }

        if (!isWaitingForRestart) {
            
            ballPosition += ballVelocity;
            if (ballPosition.x > 1.0f || ballPosition.x < -1.0f) ballVelocity.x = -ballVelocity.x;
            if (ballPosition.y > 1.0f) ballVelocity.y = -ballVelocity.y;

            if (ballPosition.y < -0.85f && ballPosition.y > -0.9f &&
                ballPosition.x > paddleX - 0.15f && ballPosition.x < paddleX + 0.15f) {
                ballVelocity.y = -ballVelocity.y;
            }

            for (int i = 0; i < brickStates.size(); ++i) {
                if (!brickStates[i]) continue;
                int row = i / 10;
                int col = i % 10;
                float x = -.9f + col * (0.15f + 0.02f) + shiftOffset;
                float y = 0.9f - row * (0.05f + 0.02f);

                if (ballPosition.x > x && ballPosition.x < x + 0.15f &&
                    ballPosition.y < y && ballPosition.y > y - 0.05f) {

                    brickStates[i] = false;
                    ballVelocity.y = -ballVelocity.y;
                    //ballVelocity.x = -ballVelocity.x;
                    break;
                }

                
            }

            if (ballPosition.y < -1.0f) {
                ballPosition = vec3(paddleX, -0.8f, 0.0f);
                isWaitingForRestart = true;
                ballVelocity = vec3(0.0f, 0.0f, 0.0f);
            }
        }

        glClear(GL_COLOR_BUFFER_BIT);

        mat4 paddleModel = translate(vec3(paddleX, 0.0f, 0.0f));
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(paddleModel));
        Render(VBO_Paddle, paddleVertices.size() / 6);

        mat4 ballModel = translate(ballPosition);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ballModel));
        Render(VBO_Ball, ballVertices.size() / 6);
        std::vector<vec3> brickColors = {
            vec3(0.851f, 0.051f, 0.682f),
            vec3(0.694f, 0.051f, 0.851f),
            vec3(0.0f, 0.729f, 0.839f)
        };
        
        for (int i = 0; i < brickStates.size(); ++i) {
            if (!brickStates[i]) continue;

            int row = i / 10;
            int col = i % 10;

            mat4 brickModel = translate(vec3(-0.9f + col * (0.15f + 0.02f) + shiftOffset,
                0.9f - row * (0.05f + 0.02f), 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(brickModel));

            int colorIndex = i % brickColors.size(); 
            glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, glm::value_ptr(brickColors[colorIndex]));

            Render(VBO_Bricks, brickVertices.size() / 6);
        }

        window.display();
    }

    // Clean up buffers
    glDeleteBuffers(1, &VBO_Paddle);
    glDeleteBuffers(1, &VBO_Ball);
    glDeleteBuffers(1, &VBO_Bricks);
    glDeleteProgram(shaderProgram);

    return 0;
}

