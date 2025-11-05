#include <stdlib.h> // Incluye la biblioteca estándar de C
#include <string.h> // Incluye la biblioteca estándar de C++ para manejo de cadenas
#include <stdio.h> // Incluye la biblioteca estándar de C para entrada/salida
#include <GL/glew.h> // Incluye la biblioteca GLEW
#include <GL/freeglut.h> // Incluye la biblioteca FreeGLUT

#define WINDOW_TITLE_PREFIX "Rasterizacion_Optimizada_de_Escenarios" // Define el prefijo del título de la ventana

int CurrentWidth = 800; // Ancho inicial de la ventana
int CurrentHeight = 600; // Altura inicial de la ventana
int WindowHandle = 0; // Manejador de la ventana

unsigned FrameCount = 0; // Contador de frames

GLuint VertextShaderId; // ID del shader de vértices
GLuint FragmentShaderId; // ID del shader de fragmentos
GLuint ProgramId; // ID del programa shader
GLuint VaoId; // ID del Vertex Array Object
GLuint VboId; // ID del Vertex Buffer Object
GLuint ColorBufferId; // ID del Color Buffer Object

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
    glDrawArrays(GL_TRIANGLES, 0, 3); // Dibuja un triángulo como ejemplo

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
    // Aquí iría el código para crear el VBO
    GLfloat Vertices[] = {
        -0.8f, -0.8f, 0.0f, 1.0f,
         0.0f, 0.8f, 0.0f, 1.0f,
         0.8f,  -0.8f, 0.0f, 1.0f
    };
    GLfloat Colors[] = {
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f
    };

    GLenum ErrorCheckValue = glGetError(); // Verifica errores antes de crear el VBO

    glGenVertexArrays(1, &VaoId); // Genera el VAO
    glBindVertexArray(VaoId); // Vincula el VAO

    glGenBuffers(1, &VboId); // Genera el VBO
    glBindBuffer(GL_ARRAY_BUFFER, VboId); // Vincula el VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices) , Vertices, GL_STATIC_DRAW); // Carga los datos de los vértices
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0); // Define el layout de los vértices
    glEnableVertexAttribArray(0); // Habilita el atributo de vértices

    glGenBuffers(1, &ColorBufferId); // Genera el Color Buffer
    glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId); // Vincula el Color Buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(Colors), Colors, GL_STATIC_DRAW); // Carga los datos de color
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0); // Define el layout de los colores
    glEnableVertexAttribArray(1); // Habilita el atributo de colores

    ErrorCheckValue = glGetError(); // Verifica errores después de crear el VBO
    if (ErrorCheckValue != GL_NO_ERROR) {
        fprintf(stderr, "ERROR: Could not create a VBO: %s \n", gluErrorString(ErrorCheckValue)); // Imprime un error si no se puede crear el VBO
        exit(-1); // Sale del programa con fallo
    }
}

void DestroyVBO(void) { // Función para destruir el VBO
    GLenum ErrorCheckValue = glGetError(); // Verifica errores antes de destruir el VBO

    glDisableVertexAttribArray(1); // Deshabilita el atributo de colores
    glDisableVertexAttribArray(0); // Deshabilita el atributo de vértices

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Desvincula el buffer

    glDeleteBuffers(1, &ColorBufferId); // Elimina el Color Buffer
    glDeleteBuffers(1, &VboId); // Elimina el VBO

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