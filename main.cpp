#include <GL/glew.h>   
#include <GLFW/glfw3.h> 
#include <vector>       
#include <cmath>  
#include <iostream>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Vertex {
    double x;
    double y;
    double z;
};

void RungeKuttaMethod(Vertex* pos, double sigma, double r, double b, double h) {
    double k[4][3];
    double x = pos->x, y = pos->y, z = pos->z;
    double x_1 = x, y_1 = y, z_1 = z;

    for (int i = 0; i < 4; i++) {
        k[i][0] = h * (sigma * (y_1 - x_1));
        k[i][1] = h * (x_1 * (r - z_1) - y_1);
        k[i][2] = h * (x_1 * y_1 - b * z_1);

        if (i == 2) {
            x_1 = x + k[i][0];
            y_1 = y + k[i][1];
            z_1 = z + k[i][2];
        } else {
            x_1 = x + k[i][0] / 2.0;
            y_1 = y + k[i][1] / 2.0;
            z_1 = z + k[i][2] / 2.0;
        }
    }

    pos->x = x + (k[0][0] + 2 * k[1][0] + 2 * k[2][0] + k[3][0]) / 6.0;
    pos->y = y + (k[0][1] + 2 * k[1][1] + 2 * k[2][1] + k[3][1]) / 6.0;
    pos->z = z + (k[0][2] + 2 * k[1][2] + 2 * k[2][2] + k[3][2]) / 6.0;
}

unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);
    
    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return id;
}

unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}


int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "Lorenz Attractor - Free Camera", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(0.2, 0.8, 1.0, 1.0);
        }
    )";

    unsigned int shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);


    int max_points = 100000;
    double sigma = 10.0;
    double r = 28.0;
    double b = 8.0 / 3.0;
    Vertex pos = {1.75, 1.87, 1.35};

    GLuint VBO, VAO;
    std::vector<GLfloat> trajectory;
    trajectory.reserve(max_points * 3); 
    
    trajectory.push_back(static_cast<GLfloat>(pos.x));
    trajectory.push_back(static_cast<GLfloat>(pos.y));
    trajectory.push_back(static_cast<GLfloat>(pos.z));

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, max_points * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW); 

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    
    double dt = 1.0 / 60.0;       
    double accumulator = 0.0;     
    double currentTime = glfwGetTime(); 
    int current_point_count = 1;

    float yaw = 0.0f;                      
    float pitch = glm::radians(30.0f);     
    float radius = 50.0f;                 
    
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 25.0f); 

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        double newTime = glfwGetTime();
        double frameTime = newTime - currentTime;
        currentTime = newTime;
        if (frameTime > 0.25) frameTime = 0.25; 
        accumulator += frameTime;

        float moveSpeed = 15.0f * frameTime;  
        float rotSpeed = 1.5f * frameTime;    

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  yaw -= rotSpeed;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) yaw += rotSpeed;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    pitch += rotSpeed;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  pitch -= rotSpeed;

        float maxPitch = glm::radians(89.0f);
        float minPitch = glm::radians(-89.0f);
        if (pitch > maxPitch) pitch = maxPitch;
        if (pitch < minPitch) pitch = minPitch;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) radius -= moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) radius += moveSpeed;
        if (radius < 5.0f) radius = 5.0f;
        if (radius > 150.0f) radius = 150.0f;

        float x = radius * glm::cos(pitch) * glm::sin(yaw);
        float y = radius * glm::sin(pitch);
        float z = radius * glm::cos(pitch) * glm::cos(yaw);
        glm::vec3 cameraPos = target + glm::vec3(x, y, z);

        glm::vec3 forward = glm::normalize(target - cameraPos);
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);      
        glm::vec3 right = glm::normalize(glm::cross(forward, worldUp)); 

        glm::vec3 up = glm::normalize(glm::cross(right, forward)); 

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) target -= right * moveSpeed; 
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) target += right * moveSpeed; 
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) target += up * moveSpeed;    
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) target -= up * moveSpeed;    
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) target -= forward * moveSpeed; 
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) target += forward * moveSpeed;

        int points_added_this_frame = 0;
        while (accumulator >= dt && current_point_count < max_points) {
            RungeKuttaMethod(&pos, sigma, r, b, dt); 
            trajectory.push_back(static_cast<GLfloat>(pos.x));
            trajectory.push_back(static_cast<GLfloat>(pos.y));
            trajectory.push_back(static_cast<GLfloat>(pos.z));
            current_point_count++;
            points_added_this_frame++;
            accumulator -= dt; 
        }
        
        if (points_added_this_frame > 0) {
            GLsizeiptr new_size = points_added_this_frame * 3 * sizeof(GLfloat);
            GLsizeiptr gpu_offset = (current_point_count - points_added_this_frame) * 3 * sizeof(GLfloat);
            const void* cpu_offset = &trajectory[(current_point_count - points_added_this_frame) * 3];

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, gpu_offset, new_size, cpu_offset);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);
        
        glm::mat4 model = glm::mat4(1.0f); 

        glm::mat4 view = glm::lookAt(cameraPos, target, glm::vec3(0.0f, 1.0f, 0.0f));
        
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 200.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glLineWidth(2.0f);
        glDrawArrays(GL_LINE_STRIP, 0, current_point_count);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}