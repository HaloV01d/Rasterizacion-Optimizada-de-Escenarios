#include "Utils.h" // Incluye el archivo de cabecera Utils.h

#define WINDOW_TITLE_PREFIX "Rasterizacion_Optimizada_de_Escenarios" // Define el prefijo del título de la ventana

int CurrentWidth = 800; // Ancho inicial de la ventana
int CurrentHeight = 600; // Altura inicial de la ventana
int WindowHandle = 0; // Manejador de la ventana

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
void CreateCube(void);
void DestroyCube(void);
void DrawCube(void);
void KeyboardFunction(unsigned char, int, int);
void CleanUp(void);


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

    CreateCube(); // Llama a la función para crear el cubo
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

    DrawCube(); // Llama a dibujar el cubo

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
void CreateCube() {

    const Vertex Vertices[8] =
    {
        { { -.5f, -.5f,  .5f, 1 }, { 0, 0, 1, 1 } },
        { { -.5f,  .5f,  .5f, 1 }, { 1, 0, 0, 1 } },
        { {  .5f,  .5f,  .5f, 1 }, { 0, 1, 0, 1 } },
        { {  .5f, -.5f,  .5f, 1 }, { 1, 1, 0, 1 } },
        { { -.5f, -.5f, -.5f, 1 }, { 1, 1, 1, 1 } },
        { { -.5f,  .5f, -.5f, 1 }, { 1, 0, 0, 1 } },
        { {  .5f,  .5f, -.5f, 1 }, { 1, 0, 1, 1 } },
        { {  .5f, -.5f, -.5f, 1 }, { 0, 0, 1, 1 } }
    };

    const GLuint Indices[36] =
    {
        0,2,1,  0,3,2,
        4,3,0,  4,7,3,
        4,1,5,  4,0,1,
        3,6,2,  3,7,6,
        1,6,5,  1,2,6,
        7,5,6,  7,4,5
    };

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

    glGenVertexArrays(1, &BufferIds[0]);
    glBindVertexArray(BufferIds[0]);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glGenBuffers(2, &BufferIds[1]);

    glBindBuffer(GL_ARRAY_BUFFER, BufferIds[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertices[0]), (GLvoid*)0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertices[0]), (GLvoid*)sizeof(Vertices[0].Position));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferIds[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}


// ===============================
//  DIBUJAR EL CUBO ROTANDO
// ===============================
void DrawCube(void) {

    float CubeAngle;
    clock_t Now = clock();
    if (LastTime == 0) {
        LastTime = Now;
    }

    CubeRotationAngle += 45.0f * ((float)(Now - LastTime) / CLOCKS_PER_SEC);
    CubeAngle = DegreesToRadians(CubeRotationAngle);
    LastTime = Now;

    ModelMatrix = IDENTITY_MATRIX;
    RotateAboutyAxis(&ModelMatrix, CubeAngle);
    RotateAboutxAxis(&ModelMatrix, CubeAngle);

    glUseProgram(ShaderIds[0]);

    glUniformMatrix4fv(ModelMatrixUniformLocation, 1, GL_FALSE, ModelMatrix.m);
    glUniformMatrix4fv(ViewMatrixUniformLocation, 1, GL_FALSE, ViewMatrix.m);
    glUniformMatrix4fv(ProjectionMatrixUniformLocation, 1, GL_FALSE, ProjectionMatrix.m);

    glBindVertexArray(BufferIds[0]);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLvoid*)0);

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
