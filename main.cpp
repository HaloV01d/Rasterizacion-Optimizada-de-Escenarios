#include <stdlib.h> // Incluye la biblioteca estándar de C
#include <string.h> // Incluye la biblioteca estándar de C++ para manejo de cadenas
#include <stdio.h> // Incluye la biblioteca estándar de C para entrada/salida
#include <GL/glew.h> // Incluye la biblioteca GLEW
#include <GL/freeglut.h> // Incluye la biblioteca FreeGLUT

#define WINDOW_TITLE_PREFIX "Rasterizacion_Optimizada_de_Escenarios" // Define el prefijo del título de la ventana

typedef struct {
    GLfloat XYZW[4]; // Coordenadas del vértice
    GLfloat RGBA[4]; // Color del vértice
} Vertex; // Estructura para un vértice con posición y color

int CurrentWidth = 600; // Ancho inicial de la ventana
int CurrentHeight = 600; // Altura inicial de la ventana
int WindowHandle = 0; // Manejador de la ventana

unsigned FrameCount = 0; // Contador de frames

GLuint VertextShaderId; // ID del shader de vértices
GLuint FragmentShaderId; // ID del shader de fragmentos
GLuint ProgramId; // ID del programa shader
GLuint VaoId; // ID del Vertex Array Object
GLuint VboId; // ID del Vertex Buffer Object
GLuint BufferId; // ID del Buffer
GLuint IndexBufferId; // ID del Index Buffer

const GLchar* VertexShader = // Código del shader de vértices
{
"#version 430 core                                                 \n"
"layout(location = 0) in vec4 in_Position;                        \n"
"layout(location = 1) in vec4 in_Color;                           \n"
"out vec4 ex_Color;                                              \n"
"void main(void)                                                 \n"
"{                                                               \n"
"    gl_Position = in_Position;                                  \n"
"    ex_Color = in_Color;                                        \n"
"}                                                               \n"
};

const GLchar* FragmentShader = // Código del shader de fragmentos
{
"#version 430 core                                                 \n"
"in vec4 ex_Color;                                              \n"
"out vec4 out_Color;                                            \n"
"void main(void)                                                 \n"
"{                                                               \n"
"    out_Color = ex_Color;                                      \n"
"}                                                               \n"
};

void Initialize(int, char*[]); // Declaración de la función de inicialización
void InitWindow(int, char*[]); // Declaración de la función para inicializar la ventana
void ResizeFunction(int, int); // Declaración de la función de redimensionamiento
void RenderFunction(void); // Declaración de la función de renderizado
void TimerFunction(int); // Declaración de la función de temporizador
void IdleFunction(void); // Declaración de la función inactiva
void CleanUp(void); // Declaración de la función de limpieza
void CreateVBO(void); // Declaración de la función para crear el VBO
void DestroyVBO(void); // Declaración de la función para destruir el VBO
void CreateShaders(void); // Declaración de la función para crear los shaders
void DestroyShaders(void); // Declaración de la función para destruir los shaders

int main(int argc, char* argv[]) {

    Initialize(argc, argv); // Llama a la función de inicialización
    glutMainLoop(); // Entra en el bucle principal de GLUT
    exit(EXIT_SUCCESS); // Sale del programa exitosamente
    return 0; // Retorna 0 al sistema operativo
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
    CreateShaders(); // Llama a la función para crear los shaders
    CreateVBO(); // Llama a la función para crear el VBO
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Establece el color
}

void InitWindow(int argc, char* argv[]) { // Función para inicializar la ventana
    glutInit(&argc, argv); // Inicializa GLUT
    glutInitContextVersion(4, 3); // Establece la versión del contexto OpenGL
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE); // Establece las banderas del contexto OpenGL
    glutInitContextProfile(GLUT_CORE_PROFILE); // Establece el perfil del contexto OpenGL

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS); // Configura la acción al cerrar la ventana

    glutInitWindowSize(CurrentWidth, CurrentHeight); // Inicializa el tamaño de la ventana

    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA); // Inicializa el modo de visualización

    WindowHandle = glutCreateWindow(WINDOW_TITLE_PREFIX); // Crea la ventana

    if (WindowHandle < 1) {
        fprintf(stderr, "ERROR: Could not create a new rendering window.\n"); // Imprime un error si no se puede crear la ventana
        exit(EXIT_FAILURE); // Sale del programa con fallo
    }

    glutReshapeFunc(ResizeFunction); // Registra la función de redimensionamiento
    glutDisplayFunc(RenderFunction); // Registra la función de renderizado
    glutIdleFunc(IdleFunction); // Registra la función inactiva
    glutTimerFunc(0, TimerFunction, 0); // Registra la función de temporizador
    glutCloseFunc(CleanUp); // Registra la función de limpieza
}

void ResizeFunction(int Width, int Height) { // Función de redimensionamiento
    CurrentWidth = Width; // Actualiza el ancho actual
    CurrentHeight = Height; // Actualiza la altura actual
    glViewport(0, 0, CurrentWidth, CurrentHeight); // Establece el viewport
}

void RenderFunction(void) { // Función de renderizado
    ++FrameCount; // Incrementa el contador de frames
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpia el buffer de color y profundidad

    // Aquí iría el código de renderizado
    glDrawElements(GL_TRIANGLES, 48, GL_UNSIGNED_BYTE, (GLvoid*)0); // Dibuja los elementos usando índices

    glutSwapBuffers(); // Intercambia los buffers
    glutPostRedisplay(); // Solicita una nueva renderización
}

void IdleFunction(void) {
    glutPostRedisplay(); // Solicita una nueva renderización cuando el programa está inactivo
}

void TimerFunction(int Value) { // Función de temporizador
    if (0 != Value) {
        char* TempString = (char*)malloc(512 + strlen(WINDOW_TITLE_PREFIX)); // Reserva memoria para el título de la ventana

        sprintf(TempString, "%s: %d Frames Per Second @ %d x %d", WINDOW_TITLE_PREFIX, FrameCount * 4, CurrentWidth, CurrentHeight); // Formatea el título con los FPS

        glutSetWindowTitle(TempString); // Establece el título de la ventana
        free(TempString); // Libera la memoria reservada
    }
    FrameCount = 0; // Reinicia el contador de frames
    glutTimerFunc(250, TimerFunction, 1); // Vuelve a llamar a la función de temporizador después de 250 ms
}

void CleanUp(void) { // Función de limpieza
    DestroyVBO(); // Llama a la función para destruir el VBO
    DestroyShaders(); // Llama a la función para destruir los shaders
}

void CreateVBO(void) { // Función para crear el VBO
Vertex Vertices[] = // Array de vértices
{
	{ { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	// Top
	{ { -0.2f, 0.8f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
	{ { 0.2f, 0.8f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
	{ { 0.0f, 0.8f, 0.0f, 1.0f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
	{ { 0.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
	// Bottom
	{ { -0.2f, -0.8f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
	{ { 0.2f, -0.8f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
	{ { 0.0f, -0.8f, 0.0f, 1.0f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
	{ { 0.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
	// Left
	{ { -0.8f, -0.2f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
	{ { -0.8f, 0.2f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
	{ { -0.8f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
	{ { -1.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
	// Right
	{ { 0.8f, -0.2f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
	{ { 0.8f, 0.2f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
	{ { 0.8f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
	{ { 1.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }
};

GLubyte Indices[] = {
	// Top
	0, 1, 3,
	0, 3, 2,
	3, 1, 4,
	3, 4, 2,
	// Bottom
	0, 5, 7,
	0, 7, 6,
	7, 5, 8,
	7, 8, 6,
	// Left
	0, 9, 11,
	0, 11, 10,
	11, 9, 12,
	11, 12, 10,
	// Right
	0, 13, 15,
	0, 15, 14,
	15, 13, 16,
	15, 16, 14
};

    GLenum ErrorCheckValue = glGetError(); // Verifica errores antes de crear el VBO
    const size_t BufferSize = sizeof(Vertices); // Calcula el tamaño del buffer
    const size_t VertexSize = sizeof(Vertices[0]); // Calcula el tamaño de un vértice
    const size_t RgbOffset = sizeof(Vertices[0].XYZW); // Calcula el offset del color dentro de un vértice

    glGenBuffers(1, &VboId); // Genera el VBO

    glGenVertexArrays(1, &VaoId); // Genera el VAO
    glBindVertexArray(VaoId); // Vincula el VAO

    glBindBuffer(GL_ARRAY_BUFFER, VboId); // Vincula el VBO
    glBufferData(GL_ARRAY_BUFFER, BufferSize, Vertices, GL_STATIC_DRAW); // Carga los datos de los vértices en el VBO

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0); // Define el atributo de posición
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset); // Define el atributo de color

    glEnableVertexAttribArray(0); // Habilita el atributo de posición
    glEnableVertexAttribArray(1); // Habilita el atributo de color
    glGenBuffers(1, &IndexBufferId); // Genera el Index Buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId); // Vincula el Index Buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW); // Carga los datos de los índices en el Index Buffer

    ErrorCheckValue = glGetError(); // Verifica errores después de crear el VBO
    if (ErrorCheckValue != GL_NO_ERROR) //  Si hay un error
    {
        fprintf(
        stderr,
        "ERROR: Could not create a VBO: %s\n",
        gluErrorString(ErrorCheckValue)
        );

        exit(-1);
    }
}

void DestroyVBO(void) { // Función para destruir el VBO
    GLenum ErrorCheckValue = glGetError(); // Verifica errores antes de destruir el VBO

    glDisableVertexAttribArray(1); // Deshabilita el atributo de colores
    glDisableVertexAttribArray(0); // Deshabilita el atributo de vértices

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Desvincula el buffer

    glDeleteBuffers(1, &VboId); // Elimina el VBO

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Desvincula el Index Buffer
    glDeleteBuffers(1, &IndexBufferId); // Elimina el Index Buffer

    glBindVertexArray(0); // Desvincula el VAO
    glDeleteVertexArrays(1, &VaoId); // Elimina el VAO

    ErrorCheckValue = glGetError(); // Verifica errores después de destruir el VBO
    if (ErrorCheckValue != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Could not destroy the VBO: %s \n", gluErrorString(ErrorCheckValue)); // Imprime un error si no se puede destruir el VBO
        exit(-1); // Sale del programa con fallo
    }
}

void CreateShaders(void) { // Función para crear los shaders
    GLenum ErrorCheckValue = glGetError(); // Verifica errores antes de crear los shaders

    VertextShaderId = glCreateShader(GL_VERTEX_SHADER); // Crea el shader de vértices
    glShaderSource(VertextShaderId, 1, &VertexShader, NULL); // Establece el código fuente del shader de vértices
    glCompileShader(VertextShaderId); // Compila el shader de vértices

    FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER); // Crea el shader de fragmentos
    glShaderSource(FragmentShaderId, 1, &FragmentShader, NULL); // Establece el código fuente del shader de fragmentos
    glCompileShader(FragmentShaderId); // Compila el shader de fragmentos

    ProgramId = glCreateProgram(); // Crea el programa shader
        glAttachShader(ProgramId, VertextShaderId); // Adjunta el shader de vértices al programa
        glAttachShader(ProgramId, FragmentShaderId); // Adjunta el shader de fragmentos al programa
    glLinkProgram(ProgramId); // Enlaza el programa shader
    glUseProgram(ProgramId); // Usa el programa shader

    ErrorCheckValue = glGetError(); // Verifica errores después de crear los shaders
    if (ErrorCheckValue != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Could not create the shaders: %s \n", gluErrorString(ErrorCheckValue)); // Imprime un error si no se pueden crear los shaders
        exit(-1); // Sale del programa con fallo
    }
}

void DestroyShaders(void) { // Función para destruir los shaders
    GLenum ErrorCheckValue = glGetError(); // Verifica errores antes de destruir los shaders

    glUseProgram(0); // Deja de usar el programa shader

    glDetachShader(ProgramId, VertextShaderId); // Desadjunta el shader de vértices del programa
    glDetachShader(ProgramId, FragmentShaderId); // Desadjunta el shader de fragmentos del programa

    glDeleteShader(FragmentShaderId); // Elimina el shader de fragmentos
    glDeleteShader(VertextShaderId); // Elimina el shader de vértices
    glDeleteProgram(ProgramId); // Elimina el programa shader

    ErrorCheckValue = glGetError(); // Verifica errores después de destruir los shaders
    if (ErrorCheckValue != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Could not destroy the shaders: %s \n", gluErrorString(ErrorCheckValue)); // Imprime un error si no se pueden destruir los shaders
        exit(-1); // Sale del programa con fallo
    }
}