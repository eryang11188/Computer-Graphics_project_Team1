
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm> 

#include "WorldConfig.h"
#include "TransformUtils.h"


float yaw = 0.0f;                  
float pitch = glm::radians(20.0f);
float radius = 35.0f;
float angularSpeed = 1.5f;


float zoomSpeed = 2.0f;
float radiusMin = 6.0f;
float radiusMax = 120.0f;

static void glfw_error_callback(int code, const char* desc) {
    std::cerr << "[GLFW ERROR] " << code << " : " << (desc ? desc : "") << "\n";
}

void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* /*window*/, double, double yoffset) {
    radius -= (float)yoffset * zoomSpeed;
    if (radius < radiusMin) radius = radiusMin;
    if (radius > radiusMax) radius = radiusMax;
}

void processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) yaw -= angularSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) yaw += angularSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pitch += angularSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) pitch -= angularSpeed * deltaTime;

    float limit = glm::radians(89.0f);
    if (pitch > limit) pitch = limit;
    if (pitch < -limit) pitch = -limit;
}

GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success = 0;
    char infoLog[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        std::cerr << "Shader compile error:\n" << infoLog << "\n";
    }
    return shader;
}

GLuint linkProgram(GLuint vs, GLuint fs) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    int ok = 0;
    char infoLog[1024];
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        glGetProgramInfoLog(prog, 1024, nullptr, infoLog);
        std::cerr << "Program link error:\n" << infoLog << "\n";
    }
    return prog;
}

struct RenderItem {
    GLuint vao;
    bool indexed;   
    int count;    
    glm::mat4 model;
    glm::vec3 color;
};


static const char* skyVertexShaderSrc = R"(
#version 330 core
layout (location=0) in vec2 aPos;
out vec2 vUV;
void main(){
    vUV = aPos * 0.5 + 0.5;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

static const char* skyFragmentShaderSrc = R"(
#version 330 core
in vec2 vUV;
out vec4 FragColor;

float hash(vec2 p){
    p = fract(p * vec2(123.34, 345.45));
    p += dot(p, p + 34.345);
    return fract(p.x * p.y);
}

float noise(vec2 p){
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = hash(i);
    float b = hash(i + vec2(1,0));
    float c = hash(i + vec2(0,1));
    float d = hash(i + vec2(1,1));
    vec2 u = f*f*(3.0-2.0*f);
    return mix(a,b,u.x) + (c-a)*u.y*(1.0-u.x) + (d-b)*u.x*u.y;
}

void main(){
    float t = clamp(vUV.y, 0.0, 1.0);

    vec3 top    = vec3(0.20, 0.45, 0.85);
    vec3 bottom = vec3(0.72, 0.90, 1.00);
    vec3 col = mix(bottom, top, smoothstep(0.0, 1.0, t));

    float haze = exp(-t * 6.0);
    col += haze * vec3(0.08, 0.10, 0.12);

    float n = noise(vUV * vec2(3.0, 1.6));
    float clouds = smoothstep(0.48, 0.70, n);
    float density = mix(0.25, 0.60, 1.0 - t);
    clouds *= density;

    col = mix(col, vec3(1.0), clouds * 0.65);
    FragColor = vec4(col, 1.0);
}
)";

int main() {
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    const int WIN_W = 1600;
    const int WIN_H = 900;

    GLFWwindow* window = glfwCreateWindow(WIN_W, WIN_H, "World (Borderless Window)", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    int mx, my;
    glfwGetMonitorPos(monitor, &mx, &my);
    int x = mx + (mode->width - WIN_W) / 2;
    int y = my + (mode->height - WIN_H) / 2;
    glfwSetWindowPos(window, x, y);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        glfwTerminate();
        return -1;
    }

    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    glViewport(0, 0, fbW, fbH);

    glEnable(GL_DEPTH_TEST);

    
    float cubeVertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f
    };

    unsigned int cubeIndices[] = {
        0,1,2, 2,3,0,
        4,5,6, 6,7,4,
        0,4,7, 7,3,0,
        1,5,6, 6,2,1,
        0,1,5, 5,4,0,
        3,2,6, 6,7,3
    };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    const char* vertexShaderSrc = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        void main() { gl_Position = projection * view * model * vec4(aPos, 1.0); }
    )";

    const char* fragmentShaderSrc = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 uColor;
        void main() { FragColor = vec4(uColor, 1.0); }
    )";

    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    GLuint shaderProgram = linkProgram(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    glUseProgram(shaderProgram);
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");

    
    GLuint skyVS = compileShader(GL_VERTEX_SHADER, skyVertexShaderSrc);
    GLuint skyFS = compileShader(GL_FRAGMENT_SHADER, skyFragmentShaderSrc);
    GLuint skyProgram = linkProgram(skyVS, skyFS);
    glDeleteShader(skyVS);
    glDeleteShader(skyFS);

    GLuint skyVAO = 0, skyVBO = 0;
    {
        float quad[] = {
            -1.f, -1.f,
             1.f, -1.f,
             1.f,  1.f,
            -1.f, -1.f,
             1.f,  1.f,
            -1.f,  1.f
        };
        glGenVertexArrays(1, &skyVAO);
        glGenBuffers(1, &skyVBO);
        glBindVertexArray(skyVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    
    GLuint roofVAO = 0, roofVBO = 0;
    {
        float v[] = {
            
            -0.5f, 0.0f,  0.5f,
             0.5f, 0.0f,  0.5f,
             0.0f, 1.0f,  0.0f,

             
              0.5f, 0.0f, -0.5f,
             -0.5f, 0.0f, -0.5f,
              0.0f, 1.0f,  0.0f,

              
              -0.5f, 0.0f, -0.5f,
              -0.5f, 0.0f,  0.5f,
               0.0f, 1.0f,  0.0f,

               
                0.5f, 0.0f,  0.5f,
                0.5f, 0.0f, -0.5f,
                0.0f, 1.0f,  0.0f,

                
                -0.5f, 0.0f, -0.5f,
                 0.5f, 0.0f, -0.5f,
                 0.5f, 0.0f,  0.5f,

                -0.5f, 0.0f, -0.5f,
                 0.5f, 0.0f,  0.5f,
                -0.5f, 0.0f,  0.5f,
        };

        glGenVertexArrays(1, &roofVAO);
        glGenBuffers(1, &roofVBO);

        glBindVertexArray(roofVAO);
        glBindBuffer(GL_ARRAY_BUFFER, roofVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    
    std::vector<RenderItem> items;

    const float groundY = 0.0f;
    const float overlayY = groundY + 0.002f;

    
    items.push_back({
        VAO, true, 36,
        MakeModel_BottomPivot(glm::vec3(0.0f, groundY, 0.0f),
                              glm::vec3(0.0f),
                              glm::vec3(80.0f, 0.05f, 80.0f)),
        glm::vec3(0.20f, 0.45f, 0.20f)
        });

    
    const float roadW = 10.0f;
    const float roadL = 60.0f;
    items.push_back({
        VAO, true, 36,
        MakeModel_BottomPivot(glm::vec3(0.0f, overlayY, 0.0f),
                              glm::vec3(0.0f),
                              glm::vec3(roadW, 0.03f, roadL)),
        glm::vec3(0.12f, 0.12f, 0.12f)
        });

    
    const float sidewalkW = 6.0f;
    const float sidewalkL = roadL;
    {
        float xLeft = -(roadW * 0.5f + sidewalkW * 0.5f);
        float xRight = (roadW * 0.5f + sidewalkW * 0.5f);
        glm::vec3 rot(0.0f);
        glm::vec3 scale(sidewalkW, 0.12f, sidewalkL);

        items.push_back({ VAO, true, 36, MakeModel_BottomPivot(glm::vec3(xLeft,  overlayY, 0.0f), rot, scale), glm::vec3(0.55f) });
        items.push_back({ VAO, true, 36, MakeModel_BottomPivot(glm::vec3(xRight, overlayY, 0.0f), rot, scale), glm::vec3(0.55f) });
    }

    
    glm::vec3 shinHouseCenter(0.0f, groundY, -8.0f);

    glm::vec3 wallColor = glm::vec3(0.92f, 0.88f, 0.80f);
    glm::vec3 floorColor = glm::vec3(0.80f, 0.76f, 0.70f);

    const float baseY = overlayY + 0.06f;
    const float H1 = 5.0f;

   
    glm::vec3 S1_pos = glm::vec3(shinHouseCenter.x - 0.5f, baseY, shinHouseCenter.z - 0.5f);
    glm::vec3 S1_scl = glm::vec3(8.5f, H1, 8.5f);

    
    glm::vec3 S2_pos = glm::vec3(shinHouseCenter.x + 2.6f, baseY, shinHouseCenter.z - 0.8f);
    glm::vec3 S2_scl = glm::vec3(6.8f, H1, 5.0f);

    
    const float S3_extendDown = 3.0f;   

    glm::vec3 S3_pos = glm::vec3(
        shinHouseCenter.x - 3.8f,
        baseY,
        (shinHouseCenter.z - 3.6f) + (S3_extendDown * 0.5f)  
    );

    glm::vec3 S3_scl = glm::vec3(
        5.2f,
        H1,
        2.3f + S3_extendDown                                  
    );
    
    glm::vec3 S4_pos = glm::vec3(shinHouseCenter.x - 2.4f, baseY, shinHouseCenter.z + 3.6f);
    glm::vec3 S4_scl = glm::vec3(4.7f, H1, 5.0f);

    
    auto minf = [](float a, float b) { return (a < b) ? a : b; };
    auto maxf = [](float a, float b) { return (a > b) ? a : b; };

    auto slabMinX = [](const glm::vec3& p, const glm::vec3& s) { return p.x - s.x * 0.5f; };
    auto slabMaxX = [](const glm::vec3& p, const glm::vec3& s) { return p.x + s.x * 0.5f; };
    auto slabMinZ = [](const glm::vec3& p, const glm::vec3& s) { return p.z - s.z * 0.5f; };
    auto slabMaxZ = [](const glm::vec3& p, const glm::vec3& s) { return p.z + s.z * 0.5f; };

    float minX = slabMinX(S1_pos, S1_scl);
    float maxX = slabMaxX(S1_pos, S1_scl);
    float minZ = slabMinZ(S1_pos, S1_scl);
    float maxZ = slabMaxZ(S1_pos, S1_scl);

    
    minX = minf(minX, slabMinX(S2_pos, S2_scl));
    maxX = maxf(maxX, slabMaxX(S2_pos, S2_scl));
    minZ = minf(minZ, slabMinZ(S2_pos, S2_scl));
    maxZ = maxf(maxZ, slabMaxZ(S2_pos, S2_scl));

    minX = minf(minX, slabMinX(S3_pos, S3_scl));
    maxX = maxf(maxX, slabMaxX(S3_pos, S3_scl));
    minZ = minf(minZ, slabMinZ(S3_pos, S3_scl));
    maxZ = maxf(maxZ, slabMaxZ(S3_pos, S3_scl));

    minX = minf(minX, slabMinX(S4_pos, S4_scl));
    maxX = maxf(maxX, slabMaxX(S4_pos, S4_scl));
    minZ = minf(minZ, slabMinZ(S4_pos, S4_scl));
    maxZ = maxf(maxZ, slabMaxZ(S4_pos, S4_scl));

    
    const float plinthMarginX = 1.3f; 
    const float plinthMarginZ = 1.3f; 
    const float plinthThk = 0.18f;

    float plinthW = (maxX - minX) + plinthMarginX * 2.0f;
    float plinthD = (maxZ - minZ) + plinthMarginZ * 2.0f;

    glm::vec3 plinthPos(
        (minX + maxX) * 0.5f,
        baseY - 0.02f,                
        (minZ + maxZ) * 0.5f
    );

    items.push_back({
        VAO, true, 36,
        MakeModel_BottomPivot(plinthPos, glm::vec3(0.0f), glm::vec3(plinthW, plinthThk, plinthD)),
        glm::vec3(0.78f, 0.74f, 0.68f) 
        });

    
    items.push_back({ VAO, true, 36, MakeModel_BottomPivot(S1_pos, glm::vec3(0), S1_scl), wallColor });
    items.push_back({ VAO, true, 36, MakeModel_BottomPivot(S2_pos, glm::vec3(0), S2_scl), wallColor });
    items.push_back({ VAO, true, 36, MakeModel_BottomPivot(S3_pos, glm::vec3(0), S3_scl), wallColor });
    items.push_back({ VAO, true, 36, MakeModel_BottomPivot(S4_pos, glm::vec3(0), S4_scl), wallColor });

    const float floorThk = 0.12f;
    items.push_back({ VAO, true, 36, MakeModel_BottomPivot(glm::vec3(S1_pos.x, baseY - 0.01f, S1_pos.z), glm::vec3(0), glm::vec3(S1_scl.x, floorThk, S1_scl.z)), floorColor });
    items.push_back({ VAO, true, 36, MakeModel_BottomPivot(glm::vec3(S2_pos.x, baseY - 0.01f, S2_pos.z), glm::vec3(0), glm::vec3(S2_scl.x, floorThk, S2_scl.z)), floorColor });
    items.push_back({ VAO, true, 36, MakeModel_BottomPivot(glm::vec3(S3_pos.x, baseY - 0.01f, S3_pos.z), glm::vec3(0), glm::vec3(S3_scl.x, floorThk, S3_scl.z)), floorColor });
    items.push_back({ VAO, true, 36, MakeModel_BottomPivot(glm::vec3(S4_pos.x, baseY - 0.01f, S4_pos.z), glm::vec3(0), glm::vec3(S4_scl.x, floorThk, S4_scl.z)), floorColor });

    
    items.push_back({
        VAO, true, 36,
        MakeModel_BottomPivot(glm::vec3(shinHouseCenter.x + 3.8f, baseY - 0.01f, shinHouseCenter.z + 3.0f),
                              glm::vec3(0),
                              glm::vec3(3.0f, floorThk, 2.0f)),
        glm::vec3(0.65f, 0.62f, 0.58f)
        });

    
    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, deltaTime);

        
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(skyProgram);
        glBindVertexArray(skyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        
        glEnable(GL_DEPTH_TEST);
        glClear(GL_DEPTH_BUFFER_BIT);

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        float aspect = (h == 0) ? 1.0f : (float)w / (float)h;

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 200.0f);

        glm::vec3 cameraPos;
        cameraPos.x = shinHouseCenter.x + radius * cosf(pitch) * sinf(yaw);
        cameraPos.y = shinHouseCenter.y + radius * sinf(pitch);
        cameraPos.z = shinHouseCenter.z + radius * cosf(pitch) * cosf(yaw);

        glm::mat4 view = glm::lookAt(cameraPos, shinHouseCenter, glm::vec3(0, 1, 0));

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        for (const auto& it : items) {
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(it.model));
            glUniform3fv(colorLoc, 1, glm::value_ptr(it.color));

            glBindVertexArray(it.vao);
            if (it.indexed) glDrawElements(GL_TRIANGLES, it.count, GL_UNSIGNED_INT, 0);
            else           glDrawArrays(GL_TRIANGLES, 0, it.count);
        }
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

   
    glDeleteVertexArrays(1, &skyVAO);
    glDeleteBuffers(1, &skyVBO);
    glDeleteProgram(skyProgram);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glDeleteVertexArrays(1, &roofVAO);
    glDeleteBuffers(1, &roofVBO);

    glfwTerminate();
    return 0;
}