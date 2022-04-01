#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <GL/glew.h>
#include <iostream>
#include <math.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>

GLuint VBO;
GLuint gScaleLocation;
GLuint gWorldLocation;

using namespace glm;
//gl_Position = vec4(gScale * Position.x, gScale * Position.y, Position.z, 1.0);  \n\
//gl_Position = gWorld * vec4(Position, 1.0);

static const char* pVS = "                                                    \n\
#version 330                                                                  \n\
                                                                              \n\
layout (location = 0) in vec3 Position;                                       \n\
uniform mat4 gWorld;                                                          \n\
                                                                              \n\
void main()                                                                   \n\
{                                                                             \n\
    gl_Position = gWorld * vec4(Position, 1.0);                               \n\
}";

static const char* pFS = "                                                    \n\
#version 330                                                                  \n\
                                                                              \n\
out vec4 FragColor;                                                           \n\
                                                                              \n\
void main()                                                                   \n\
{                                                                             \n\
    FragColor = vec4(0.0, 1.0, 0.0, 1.0);                                     \n\
}";


void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT); // очистка

    static float Scale = 0.0f;
    Scale += 0.001f; // переменная, которую будет постепенно увеличивать

    //glUniform1f(gScaleLocation, sinf(Scale));

    mat4 World;// матрица для перемножения

    /*World[0][0] = sinf(Scale); World[0][1] = 0.0f; World[0][2] = 0.0f; World[0][3] = 0.0f;
    World[1][0] = 0.0f; World[1][1] = cosf(Scale); World[1][2] = 0.0f; World[1][3] = 0.0f;
    World[2][0] = 0.0f; World[2][1] = 0.0f; World[2][2] = sinf(Scale); World[2][3] = 0.0f;
    World[3][0] = 0.0f; World[3][1] = 0.0f; World[3][2] = 0.0f; World[3][3] = 1.0f;*/

    /*World[0][0] = 1.0f; World[0][1] = 0.0f; World[0][2] = 0.0f; World[0][3] = sinf(Scale);
    World[1][0] = 0.0f; World[1][1] = 1.0f; World[1][2] = 0.0f; World[1][3] = 0.0f;
    World[2][0] = 0.0f; World[2][1] = 0.0f; World[2][2] = 1.0f; World[2][3] = 0.0f;
    World[3][0] = 0.0f; World[3][1] = 0.0f; World[3][2] = 0.0f; World[3][3] = 1.0f;*/

    World[0][0] = cosf(Scale); World[0][1] = -sinf(Scale); World[0][2] = 0.0f; World[0][3] = 0.0f;
    World[1][0] = sinf(Scale); World[1][1] = cosf(Scale); World[1][2] = 0.0f; World[1][3] = 0.0f;
    World[2][0] = 0.0f; World[2][1] = 0.0f; World[2][2] = 1.0f; World[2][3] = 0.0f;
    World[3][0] = 0.0f; World[3][1] = 0.0f; World[3][2] = 0.0f; World[3][3] = 1.0f;

    glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, &World[0][0]); // для загрузки в юниформ переменную

    glEnableVertexAttribArray(0);// индексация
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // снова привязка 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // передаём колво объектов и то, как их рисовать
    glDrawArrays(GL_TRIANGLES, 0, 3);// рисуем всё
    glDisableVertexAttribArray(0); // отключение индексации

    glutSwapBuffers(); // меняю буфер используемый с заготовленным

}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
    GLuint ShaderObj = glCreateShader(ShaderType);// создаём оба шейдера

    if (ShaderObj == 0) 
    {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }

    const GLchar* p[1];
    p[0] = pShaderText;
    GLint Lengths[1];
    Lengths[0] = strlen(pShaderText);
    glShaderSource(ShaderObj, 1, p, Lengths);//принимает тип шейдера как параметр и предоставляет 
                                             //гибкость в плане указания исходного кода шейдера

    glCompileShader(ShaderObj); //компиляция шейдера

    GLint success; // часть кода, отвечающая за отображение ошибок в ходе компиляции
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }

    glAttachShader(ShaderProgram, ShaderObj); //присоединяем скомпилированный объект шейдера к объекту программы
}

static void CompileShaders()
{
    GLuint ShaderProgram = glCreateProgram();//создание программного объекта

    if (ShaderProgram == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    AddShader(ShaderProgram, pVS, GL_VERTEX_SHADER);
    AddShader(ShaderProgram, pFS, GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glLinkProgram(ShaderProgram); // линкуем шейдеры

    // та же самая проверка, но немного другая
    // glGetShaderiv -> glGetProgramiv, glGetShaderInfoLog -> glGetProgramInfoLog
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glValidateProgram(ShaderProgram);// итоговая проверка: сможет ли вообще всё запуститься

    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glUseProgram(ShaderProgram);// для использования отлинкованной программы
                                // шейдеров мы назначаем её для конвейера

    //gScaleLocation = glGetUniformLocation(ShaderProgram, "gScale");
    gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld"); // обращаемся к адресу переменной шейдера
    assert(gWorldLocation != 0xFFFFFFFF);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv); // инициализация глут
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(1024, 768); // создаем окно с параметрами размер и позиции
    glutInitWindowPosition(100, 100);
    glutCreateWindow("ыыыыыыыыыы");// имя окна

    glutDisplayFunc(RenderSceneCB); // вызов функции 
    glutIdleFunc(RenderSceneCB); // вызов функции для анимации

    glClearColor(0.6f, 0.2f, 0.3f, 0.4f); // смена цвета

    GLenum res = glewInit(); // инициализируем GLEW и проверяем на ошибки

    // правильно ли иницилизирован глут 
    if (res != GLEW_OK) // если нет, ошибка
    {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glm::vec3 Vertices[3]; // инициализация точек
    Vertices[0] = glm::vec3(0.0f, 0.5f, 0.0f); // X Y Z
    Vertices[1] = glm::vec3(0.5f, 0.0f, 0.0f);
    Vertices[2] = glm::vec3(-0.5f, 0.0f, 0.0f);

    glGenBuffers(1, &VBO);// создаём буффер для помещения объетов
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // буфер будет хранить массив вершин
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW); // наполнение буфера данными
                                                                               // источник для команд рисования 

    CompileShaders();

    glutMainLoop();

    return 0;
}
