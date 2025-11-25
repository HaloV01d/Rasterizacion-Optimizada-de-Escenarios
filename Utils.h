#ifndef UTILS_H // UTILS_H
#define UTILS_H // UTILS_H
#include <stdlib.h> // Incluye la biblioteca estándar de C
#include <stdio.h> // Incluye la biblioteca estándar de C para entrada/salida
#include <string.h> // Incluye la biblioteca estándar de C++ para manejo de cadenas
#include <math.h> // Incluye la biblioteca matemática de C
#include <time.h> // Incluye la biblioteca de tiempo de C
#include <GL/glew.h> // Incluye la biblioteca GLEW
#include <GL/freeglut.h> // Incluye la biblioteca FreeGLUT

static const double PI = 3.1415926535897932384626433832795; // Define la constante PI

typedef struct {
    GLfloat position[3];
    GLfloat normal[3];
} Vertex;


typedef struct Matrix { // Estructura para una matriz 4x4
    float m[16];
} Matrix;

extern const Matrix IDENTITY_MATRIX; // Matriz identidad

float Cotangent(float angle); // Función para calcular la cotangente de un ángulo
float DegreesToRadians(float degrees); // Función para convertir grados a radianes
float RadiansToDegrees(float radians); // Función para convertir radianes a grados

Matrix MultiplyMatrices(const Matrix* m1, const Matrix* m2); // Función para multiplicar dos matrices
void RotateAboutxAxis(Matrix* m, float angle); // Función para rotar una matriz alrededor del eje x
void RotateAboutyAxis(Matrix* m, float angle); // Función para rotar una matriz alrededor del eje y
void RotateAboutzAxis(Matrix* m, float angle); // Función para rotar una matriz alrededor del eje z
void ScaleMatrix(Matrix* m, float x, float y, float z); // Función para escalar una matriz
void TranslateMatrix(Matrix* m, float x, float y, float z); // Función para trasladar una matriz

Matrix CreateProjectionMatrix(float fovY, float aspect, float nearPlane, float farPlane); // Función para crear una matriz de proyección

void ExitOnGLError(const char* message); // Función para salir en caso de error de OpenGL

GLuint LoadShader(const char* filename, GLenum shader_Type); // Función para cargar un shader desde un archivo


#endif // UTILS_H