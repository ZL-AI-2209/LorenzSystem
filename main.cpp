#include <GL/glew.h>   
#include <math.h> 


struct Vertex {
    double x;
    double y;
    double z;
}typedef;


void RungeKuttaMethod (Vertex* pos, double sigma, double r, double b, double h){
    double k [4][3];
    Vertex f;

    double x = pos -> x;
    double y = pos -> y;
    double z = pos -> z;

    double x_1 = x;
    double y_1 = y;
    double z_1 = z;

    for (int i = 0; i < 4; i ++ ){

        k [i][0]= h * (sigma * (y_1 - x_1));
        k [i][1] = h * ( x_1 * (r - z_1) - y_1);
        k [i][2]  = h * (x_1 * y_1 - b * z_1);

        if (i == 2){
            x_1 = x + k[i][0];
            y_1 = y + k[i][1];
            z_1 = z + k[i][2]; 
        } else {
            x_1 = x + k[i][0]/2.0;
            y_1 = y + k[i][1]/2.0;
            z_1 = z + k[i][2]/2.0;
        }
    } 

    pos -> x = pos -> x + (k[0][0] + 2 * k[1][0] + 2 * k [2][0] + k[3][0]) / 6;
    pos -> y = pos -> y + (k[0][1] + 2 * k[1][1] + 2 * k [2][1] + k[3][1]) / 6;
    pos -> z = pos -> z + (k[0][2] + 2 * k[1][2] + 2 * k [2][2] + k[3][2]) / 6;
}


int main () { 

    int period_sim = 1000;
    double sigma = 10;
    double r = 28;
    double b = 8/3;

    Vertex pos = {1.75, 1.87, 1.35};

    GLuint VBO, VAO;
    std::vector<GLfloat> trajectory;
    
    trajectory.push_back(pos.x);
    trajectory.push_back(pos.y);
    trajectory.push_back(pos.z);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, period_sim * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW); 

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    
    double dt = 1.0 / 60.0;       
    double accumulator = 0.0;     
    double currentTime = glfwGetTime(); 

    while (!glfwWindowShouldClose(window)) {
        double newTime = glfwGetTime();
        double frameTime = newTime - currentTime;
        currentTime = newTime;

        if (frameTime > 0.25) frameTime = 0.25; 

        accumulator += frameTime;

        while (accumulator >= dt) {
            RungeKuttaMethod(&pos, sigma, r, b, dt); 

            trajectory.push_back(pos.x);
            trajectory.push_back(pos.y);
            trajectory.push_back(pos.z);

            current_point_count++;
            
            accumulator -= dt; 

            if (current_point_count >= 100000) break; 
        }
        
        if (points_per_f    rame > 0) {
            GLsizeiptr new_size = points_per_frame * 3 * sizeof(GLfloat);
            GLsizeiptr offset = (current_point_count - points_per_frame) * 3 * sizeof(GLfloat);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, offset, new_size, trajectory.data());
        }

        render(); 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

}