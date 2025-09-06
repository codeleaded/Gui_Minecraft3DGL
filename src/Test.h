#include "/home/codeleaded/System/Static/Library/WindowEngineGL1.0.h"
#include "/home/codeleaded/System/Static/Library/Files.h"

char* vertexShaderSource = NULL;
char* fragmentShaderSource = NULL;

float vertices[] = {
    // positions        // texture coords
     0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f, 0.0f,  0.0f, 1.0f
};
unsigned int indices[] = {
    0, 1, 3,
    1, 2, 3
};
GLuint VBO, VAO, EBO;
GLuint texture;

GLuint vertexShader;
GLuint fragmentShader;
GLuint shaderProgram;

void checkCompileErrors(GLuint shader, const char* type) {
    GLint success;
    GLchar infoLog[512];
    if (strcmp(type, "PROGRAM") != 0) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            printf("ERROR: Shader Compilation [%s]\n%s\n", type, infoLog);
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 512, NULL, infoLog);
            printf("ERROR: Program Linking\n%s\n", infoLog);
        }
    }
}

void Setup(AlxWindow* w){
    vertexShaderSource = Files_Read("./src/vertexShader.glsl");
    if(!vertexShaderSource){
        printf("[vertexShader]: Error\n");
        window.Running = 0;
        return;
    }

    fragmentShaderSource = Files_Read("./src/fragmentShader.glsl");
    if(!fragmentShaderSource){
        printf("[fragmentShader]: Error\n");
        window.Running = 0;
        return;
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const GLchar * const*)&vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkCompileErrors(vertexShader, "VERTEX");

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, (const GLchar * const*)&fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader, "FRAGMENT");

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkCompileErrors(shaderProgram, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Image_Enable_FlipV();
    Sprite sp = Sprite_Load_BGR("./data/Atlas.png");
    if(sp.img){
        GLenum format = GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, sp.w, sp.h, 0, format, GL_UNSIGNED_BYTE, sp.img);
        glGenerateMipmap(GL_TEXTURE_2D);
        Sprite_Free(&sp);
    }
}
void Update(AlxWindow* w){
    glClearColor(0.5f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
void Delete(AlxWindow* w){
    if(vertexShaderSource)      free(vertexShaderSource);
    if(fragmentShaderSource)    free(fragmentShaderSource);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
}

int main(){
    if(Create("OpenGL Test",2500,1300,1,1,Setup,Update,Delete))
        Start();
    return 0;
}
