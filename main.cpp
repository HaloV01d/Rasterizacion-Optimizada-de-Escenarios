#include "Utils.h" // Incluye el archivo de cabecera Utils.h
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#define WINDOW_TITLE_PREFIX "Rasterizacion_Optimizada_de_Escenarios" // Define el prefijo del título de la ventana


int CurrentWidth = 800; // Ancho inicial de la ventana
int CurrentHeight = 600; // Altura inicial de la ventana
int WindowHandle = 0; // Manejador de la ventana
size_t IndexCount = 0;

unsigned FrameCount = 0; // Contador de frames

GLuint
ProjectionMatrixUniformLocation, 
ViewMatrixUniformLocation,
ModelMatrixUniformLocation,
BufferIds[3] = { 0 },
ShaderIds[3] = { 0 };

Matrix
ProjectionMatrix,
ViewMatrix,
ModelMatrix;

float CubeRotationAngle = 0; // Ángulo de rotación del cubo
clock_t LastTime = 0; // Tiempo del último frame


// Funciones
void Initialize(int, char*[]);
void InitWindow(int, char*[]);
void ResizeFunction(int, int);
void RenderFunction(void);
void TimerFunction(int);
void IdleFunction(void);
void CreateOBJ(void);
void DestroyCube(void);
void DrawOBJ(void);
void KeyboardFunction(unsigned char, int, int);
void CleanUp(void);

bool LoadOBJ(const std::string& path,
            std::vector<Vertex>& outVertices,
            std::vector<GLuint>& outIndices)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "No pude abrir el OBJ: " << path << std::endl;
        return false;
    }

    std::vector<GLfloat> tempPositions;
    std::vector<GLfloat> tempNormals;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v") {
            float x,y,z;
            ss >> x >> y >> z;
            tempPositions.push_back(x);
            tempPositions.push_back(y);
            tempPositions.push_back(z);
        }
        else if (type == "vn") {
            float x,y,z;
            ss >> x >> y >> z;
            tempNormals.push_back(x);
            tempNormals.push_back(y);
            tempNormals.push_back(z);
        }
        else if (type == "f") {
            // Asumimos TRIÁNGULOS
            for (int i = 0; i < 3; ++i) {
                std::string vert;
                ss >> vert;          // ejemplo "12//5" o "12/3/5" o "12"

                // separar por '/'
                int vIndex = 0, nIndex = 0;
                {
                    std::replace(vert.begin(), vert.end(), '/', ' ');
                    std::istringstream vs(vert);
                    vs >> vIndex;   // índice de posición
                    if (!(vs >> std::ws).eof()) {
                        int tmp;    // posible vt
                        if (vs >> tmp) {       // vt o vn
                            if (!(vs >> std::ws).eof()) {
                                vs >> nIndex;  // vn
                            } else {
                                nIndex = tmp;  // si solo hay vn
                            }
                        }
                    }
                }

                // OBJ es 1-based → C++ 0-based
                vIndex -= 1;
                nIndex -= 1;

                Vertex v{};
                v.position[0] = tempPositions[3 * vIndex + 0];
                v.position[1] = tempPositions[3 * vIndex + 1];
                v.position[2] = tempPositions[3 * vIndex + 2];

                if (!tempNormals.empty() && nIndex >= 0) {
                    v.normal[0] = tempNormals[3 * nIndex + 0];
                    v.normal[1] = tempNormals[3 * nIndex + 1];
                    v.normal[2] = tempNormals[3 * nIndex + 2];
                } else {
                    // Por ahora normal (0,1,0) si no hay; luego se pueden calcular bien
                    v.normal[0] = 0.0f;
                    v.normal[1] = 1.0f;
                    v.normal[2] = 0.0f;
                }

                outVertices.push_back(v);
                outIndices.push_back(static_cast<GLuint>(outVertices.size() - 1));
            }
        }
    }

    std::cout << "OBJ cargado: " 
            << outVertices.size() << " vertices, "
            << outIndices.size()  << " indices.\n";

    return true;
}

int main(int argc, char* argv[]) {

    printf("Entrando a main...\n");
    Initialize(argc, argv); // Llama a la función de inicialización
    printf("Despues de Initialize...\n");
    glutMainLoop(); // Entra en el bucle principal de GLUT

    printf("Despues de glutMainLoop (solo si algun dia sale)...\n");
    getchar(); // Mantiene la consola abierta si glutMainLoop retorna

    exit(EXIT_SUCCESS); // Sale del programa exitosamente
}


void Initialize(int argc, char* argv[]) { // Función de inicialización
    GLenum GlewInitResult; // Variable para almacenar el resultado de la inicialización de GLEW

    InitWindow(argc, argv); // Llama a la función para inicializar la ventana

    GlewInitResult = glewInit(); // Inicializa GLEW

    if (GLEW_OK != GlewInitResult) {
        fprintf(stderr, "ERROR: %s\n", glewGetErrorString(GlewInitResult)); // Imprime un error si GLEW no se inicializa correctamente
        exit(EXIT_FAILURE); // Sale del programa con fallo
    }

    fprintf(stdout, "INFO: OPENGL Version: %s\n", glGetString(GL_VERSION)); // Imprime la versión de OpenGL

    ModelMatrix = IDENTITY_MATRIX; // Inicializa la matriz modelo como la matriz identidad
    ProjectionMatrix = IDENTITY_MATRIX; // Inicializa la matriz de proyección como la matriz identidad
    ViewMatrix = IDENTITY_MATRIX; // Inicializa la matriz vista como la matriz identidad
    TranslateMatrix(&ViewMatrix, 0, 0, -2); // Traslada la matriz vista

    // Habilitar OpenGL
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Establece el color
    glEnable(GL_DEPTH_TEST); // Habilita la prueba de profundidad
    glDepthFunc(GL_LESS); // Establece la función de profundidad

    glEnable(GL_CULL_FACE); // Habilita el recorte de caras
    glCullFace(GL_BACK); // Establece las caras traseras para el recorte
    glFrontFace(GL_CCW); // Establece el sentido antihorario como frontal

    CreateOBJ(); // Llama a la función para crear el cubo
}


void InitWindow(int argc, char* argv[]) { // Función para inicializar la ventana
    glutInit(&argc, argv); // Inicializa GLUT
    glutInitContextVersion(4, 3); // Establece la versión del contexto OpenGL
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE); // Establece las banderas del contexto OpenGL
    glutInitContextProfile(GLUT_CORE_PROFILE); // Establece el perfil del contexto OpenGL

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    glutInitWindowSize(CurrentWidth, CurrentHeight); // Inicializa el tamaño de la ventana
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA); // Inicializa el modo de visualización
    WindowHandle = glutCreateWindow(WINDOW_TITLE_PREFIX); // Crea la ventana

    if (WindowHandle < 1) {
        fprintf(stderr, "ERROR: Could not create a new rendering window.\n");
        exit(EXIT_FAILURE);
    }

    glutReshapeFunc(ResizeFunction); 
    glutDisplayFunc(RenderFunction); 
    glutIdleFunc(IdleFunction);
    glutTimerFunc(0, TimerFunction, 0);
    glutKeyboardFunc(KeyboardFunction);
    glutCloseFunc(CleanUp);
}


void KeyboardFunction(unsigned char Key, int X, int Y) {
    // No usamos teclas ahora
}


void ResizeFunction(int Width, int Height) {
    CurrentWidth = Width;
    CurrentHeight = Height;

    glViewport(0, 0, CurrentWidth, CurrentHeight);

    ProjectionMatrix = CreateProjectionMatrix(
        60,
        (float)CurrentWidth / (float)CurrentHeight,
        0.1f,
        100.0f
    );
}


void RenderFunction(void) {
    ++FrameCount;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DrawOBJ(); // Llama a dibujar el cubo

    glutSwapBuffers();
    glutPostRedisplay();
}


void IdleFunction(void) {
    glutPostRedisplay();
}


void TimerFunction(int Value) {
    if (Value != 0) {
        char* TempString = (char*)malloc(512 + strlen(WINDOW_TITLE_PREFIX));

        sprintf(
            TempString,
            "%s: %d FPS @ %d x %d",
            WINDOW_TITLE_PREFIX,
            FrameCount * 4,
            CurrentWidth,
            CurrentHeight
        );

        glutSetWindowTitle(TempString);
        free(TempString);
    }

    FrameCount = 0;
    glutTimerFunc(250, TimerFunction, 1);
}


void CleanUp(void) {
    DestroyCube();
}


// ===============================
//  CREAR EL CUBO
// ===============================
void CreateOBJ()
{
    printf("Cargando modelo OBJ...\n");

    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    if (!LoadOBJ("witchs_house.obj", vertices, indices)) {
        printf("ERROR: No se pudo cargar el modelo OBJ.\n");
        exit(-1);
    }

    printf("Modelo cargado: %zu vertices, %zu indices\n",
        vertices.size(), indices.size());

    // SHADERS
    ShaderIds[0] = glCreateProgram();
    {
        ShaderIds[1] = LoadShader("SimpleShader.fragment.glsl", GL_FRAGMENT_SHADER);
        ShaderIds[2] = LoadShader("SimpleShader.vertex.glsl", GL_VERTEX_SHADER);
        glAttachShader(ShaderIds[0], ShaderIds[1]);
        glAttachShader(ShaderIds[0], ShaderIds[2]);
    }
    glLinkProgram(ShaderIds[0]);

    ModelMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ModelMatrix");
    ViewMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ViewMatrix");
    ProjectionMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ProjectionMatrix");

    // VAO + VBO + IBO
    glGenVertexArrays(1, &BufferIds[0]);
    glBindVertexArray(BufferIds[0]);

    glGenBuffers(1, &BufferIds[1]);
    glBindBuffer(GL_ARRAY_BUFFER, BufferIds[1]);
    glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vertex),
        vertices.data(),
        GL_STATIC_DRAW);

    // Atributos
    glEnableVertexAttribArray(0); // posición
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex),
        (GLvoid*)0);

    glEnableVertexAttribArray(1); // normal
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex),
        (GLvoid*)(sizeof(float) * 3));

    glGenBuffers(1, &BufferIds[2]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferIds[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(GLuint),
        indices.data(),
        GL_STATIC_DRAW);

    // Guardar conteo global
    IndexCount = indices.size();

    glBindVertexArray(0);
}


// ===============================
//  DIBUJAR EL CUBO ROTANDO
// ===============================
void DrawOBJ(void)
{
    float CubeAngle;
    clock_t Now = clock();
    if (LastTime == 0) {
        LastTime = Now;
    }

    CubeRotationAngle += 45.0f * ((float)(Now - LastTime) / CLOCKS_PER_SEC);
    float Angle = DegreesToRadians(CubeRotationAngle);
    LastTime = Now;

    ModelMatrix = IDENTITY_MATRIX;
    RotateAboutyAxis(&ModelMatrix, Angle);
    RotateAboutxAxis(&ModelMatrix, Angle);

    glUseProgram(ShaderIds[0]);

    glUniformMatrix4fv(ModelMatrixUniformLocation, 1, GL_FALSE, ModelMatrix.m);
    glUniformMatrix4fv(ViewMatrixUniformLocation, 1, GL_FALSE, ViewMatrix.m);
    glUniformMatrix4fv(ProjectionMatrixUniformLocation, 1, GL_FALSE, ProjectionMatrix.m);

    glBindVertexArray(BufferIds[0]);
    glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, (GLvoid*)0);

    glBindVertexArray(0);
    glUseProgram(0);
}



// ===============================
//  DESTRUIR CUBO
// ===============================
void DestroyCube(void) {

    glDetachShader(ShaderIds[0], ShaderIds[1]);
    glDetachShader(ShaderIds[0], ShaderIds[2]);
    glDeleteShader(ShaderIds[1]);
    glDeleteShader(ShaderIds[2]);
    glDeleteProgram(ShaderIds[0]);

    glDeleteBuffers(2, &BufferIds[1]);
    glDeleteVertexArrays(1, &BufferIds[0]);
}
