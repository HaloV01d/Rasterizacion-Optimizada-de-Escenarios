#include "Utils.h" // Para funciones de matrices y carga de shaders
#include <vector> // Para std::vector
#include <string> // Para std::string
#include <fstream> // Para std::ifstream
#include <sstream> // Para std::stringstream
#include <iostream> // Para std::cout, std::endl
#include <algorithm> // Para std::count
#include <map> // Para std::map

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // Para carga de imágenes

#define WINDOW_TITLE_PREFIX "Rasterizacion_Optimizada_de_Escenarios" // Título de la ventana

int CurrentWidth = 800;  // Ancho actual de la ventana
int CurrentHeight = 600; // Alto actual de la ventana
int WindowHandle = 0; // Manejador de la ventana GLUT

size_t IndexCount = 0; // Número de índices para el objeto principal
size_t GroundIndexCount = 0; // Número de índices para el suelo

unsigned FrameCount = 0; // Contador de frames renderizados
unsigned TotalFrameCount = 0; // Contador total de frames
float FPS = 0.0f; // FPS actuales
clock_t FPSLastTime = 0; // Tiempo del último cálculo de FPS

GLuint
ProjectionMatrixUniformLocation, // Ubicación uniforme de la matriz de proyección
ViewMatrixUniformLocation, // Ubicación uniforme de la matriz de vista
ModelMatrixUniformLocation, // Ubicación uniforme de la matriz de modelo
LightDirUniformLocation, // Ubicación uniforme de la dirección de la luz
LightColorUniformLocation, // Ubicación uniforme del color de la luz
AmbientColorUniformLocation, // Ubicación uniforme del color ambiental
MaterialColorUniformLocation, // Ubicación uniforme del color del material
ViewPosUniformLocation; // Ubicación uniforme de la posición de la cámara

GLuint BufferIds[3] = {0}; // VAO, VBO, IBO para el objeto principal
GLuint ShaderIds[3] = {0}; // IDs de shaders (vertex, fragment, program)    

GLuint GroundVAO = 0, GroundVBO = 0, GroundIBO = 0; // VAO, VBO, IBO para el suelo

GLuint BaseColorTex = 0, NormalTex = 0, RoughnessTex = 0, AOTex = 0; // Texturas del modelo

// Después de las texturas (línea ~38), añadir:
GLuint ShadowFBO = 0;           // Framebuffer para sombras
GLuint ShadowMap = 0;           // Textura de profundidad para sombras
const int SHADOW_WIDTH = 2048;  // Resolución del mapa de sombras
const int SHADOW_HEIGHT = 2048;

GLuint ShadowShaderIds[3] = {0}; // Shader separado para generar sombras
Matrix LightProjectionMatrix;   // Matriz de proyección desde la luz
Matrix LightViewMatrix;          // Matriz de vista desde la luz

Matrix ProjectionMatrix; // Matriz de proyección
Matrix ViewMatrix; // Matriz de vista
Matrix ModelMatrix; // Matriz de modelo

float CubeRotationAngle = 0; // Ángulo de rotación del objeto principal   
clock_t LastTime = 0; // Tiempo del último frame

float ObjectPositionX = 0.0f; // Posición X del objeto
float ManualRotationAngle = 0.0f; // Ángulo de rotación manual
bool AutoRotate = true; // Flag para rotación automática

// =======================================================================
// OBJ Loader
// =======================================================================
bool LoadOBJ(const std::string& path, // Carga un modelo OBJ desde archivo
            std::vector<Vertex>& outVertices,
            std::vector<GLuint>& outIndices)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "ERROR: no se pudo abrir: " << path << std::endl;
        return false;
    }

    std::vector<float> px, py, pz;
    std::vector<float> tu, tv;
    std::vector<float> nx, ny, nz;

    struct Packed {
        int v, vt, vn;
        bool operator<(Packed const& o) const {
            if (v != o.v) return v < o.v;
            if (vt != o.vt) return vt < o.vt;
            return vn < o.vn;
        }
    };

    std::map<Packed, GLuint> indexMap;

    std::string line;
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string h;
        ss >> h;

        if (h == "v")
        {
            float x, y, z;
            ss >> x >> y >> z;
            px.push_back(x);
            py.push_back(y);
            pz.push_back(z);
        }
        else if (h == "vt")
        {
            float u, v;
            ss >> u >> v;
            tu.push_back(u);
            tv.push_back(v);
        }
        else if (h == "vn")
        {
            float x, y, z;
            ss >> x >> y >> z;
            nx.push_back(x);
            ny.push_back(y);
            nz.push_back(z);
        }
        else if (h == "f")
        {
            std::vector<Packed> verts;
            std::string tok;

            while (ss >> tok)
            {
                int v = -1, vt = -1, vn = -1;

                if (tok.find("//") != std::string::npos)
                {
                    // formato v//vn
                    sscanf(tok.c_str(), "%d//%d", &v, &vn);
                }
                else if (tok.find('/') != std::string::npos)
                {
                    // formato v/vt/vn o v/vt
                    int count = std::count(tok.begin(), tok.end(), '/');
                    if (count == 2)
                        sscanf(tok.c_str(), "%d/%d/%d", &v, &vt, &vn);
                    else
                        sscanf(tok.c_str(), "%d/%d", &v, &vt);
                }
                else
                {
                    // formato solo v
                    sscanf(tok.c_str(), "%d", &v);
                }

                verts.push_back({ v - 1, vt - 1, vn - 1 });
            }

            // triangulación de cara N-lados
            for (size_t i = 1; i + 1 < verts.size(); i++)
            {
                Packed tri[3] = { verts[0], verts[i], verts[i + 1] };

                for (int k = 0; k < 3; k++)
                {
                    Packed p = tri[k];

                    if (!indexMap.count(p))
                    {
                        Vertex v;

                        // posición (siempre existe)
                        v.position[0] = px[p.v];
                        v.position[1] = py[p.v];
                        v.position[2] = pz[p.v];

                        // UV si existen
                        if (p.vt >= 0)
                        {
                            v.uv[0] = tu[p.vt];
                            v.uv[1] = tv[p.vt];
                        }
                        else
                        {
                            v.uv[0] = 0.0f;
                            v.uv[1] = 0.0f;
                        }

                        // normales si existen
                        if (p.vn >= 0)
                        {
                            v.normal[0] = nx[p.vn];
                            v.normal[1] = ny[p.vn];
                            v.normal[2] = nz[p.vn];
                        }
                        else
                        {
                            v.normal[0] = 0;
                            v.normal[1] = 1;
                            v.normal[2] = 0;
                        }

                        outVertices.push_back(v);
                        indexMap[p] = outVertices.size() - 1;
                    }

                    outIndices.push_back(indexMap[p]);
                }
            }
        }
    }

    std::cout << "OBJ CARGADO OK. Vertices: "
            << outVertices.size() << "  Indices: "
            << outIndices.size() << std::endl;

    return true;
}


// =======================================================================
// Load Texture
// =======================================================================
GLuint LoadTexture(const char* path) // Carga una textura desde archivo
{
    int w, h, comp;
    unsigned char* data = stbi_load(path, &w, &h, &comp, STBI_rgb_alpha);

    if (!data) {
        std::cout << "ERROR cargando textura: " << path << std::endl;
        return 0;
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return tex;
}

// =======================================================================
// Update Window Title with Stats
// =======================================================================
void UpdateWindowTitle() // Actualizar título de la ventana con estadísticas
{
    char title[256];
    
    // Calcular total de triángulos
    size_t totalTriangles = (IndexCount / 3) + (GroundIndexCount / 3);
    size_t totalVertices = IndexCount + GroundIndexCount;
    
    // Formato: Título | FPS | Triángulos | Vértices | Shadow Map
    sprintf(title, "%s | FPS: %.1f | Tris: %zu | Verts: %zu | Shadow: %dx%d | Rot: %s",
            WINDOW_TITLE_PREFIX,
            FPS,
            totalTriangles,
            totalVertices,
            SHADOW_WIDTH,
            SHADOW_HEIGHT,
            AutoRotate ? "AUTO" : "MANUAL");
    
    glutSetWindowTitle(title);
}

// =======================================================================
// Prototipos
// =======================================================================
void Initialize(int, char*[]); // Inicialización
void InitWindow(int, char*[]); // Ventana
void ResizeFunction(int, int); // Resize
void RenderFunction(void); // Render
void TimerFunction(int); // Timer
void IdleFunction(void); // Idle
void CleanUp(void); // Limpieza
void CreateOBJ(void); // Crear objeto
void DrawOBJ(void); // Dibujar objeto
void CreateGround(void); // Crear suelo
void DrawGround(void); // Dibujar suelo
void KeyboardFunction(unsigned char, int, int); // Función de teclado
void CreateShadowMap(void); // Crear mapa de sombras
void RenderShadowPass(void); // Renderizar pase de sombras 
void UpdateWindowTitle(void); // Actualizar título de ventana
// =======================================================================
// MAIN
// =======================================================================
int main(int argc, char* argv[]) // Función principal
{
    Initialize(argc, argv); // Inicialización
    glutMainLoop(); // Bucle principal de GLUT
    return 0;
}

// =======================================================================
// Inicialización
// =======================================================================
void Initialize(int argc, char* argv[]) // Inicialización
{
    InitWindow(argc, argv);

    if (glewInit() != GLEW_OK)
    {
        std::cout << "Error inicializando GLEW\n";
        exit(1);
    }

    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));

    ModelMatrix      = IDENTITY_MATRIX;
    ProjectionMatrix = IDENTITY_MATRIX;
    ViewMatrix       = IDENTITY_MATRIX;

    // Cámara ajustada para mejor vista
    TranslateMatrix(&ViewMatrix, 0.0f, -1.8f, -7.5f);

    // Fondo más cálido/beige como la imagen
    glClearColor(0.82f, 0.76f, 0.65f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    CreateOBJ();
    CreateGround();
    CreateShadowMap();
    
    // Inicializar tiempo para FPS
    FPSLastTime = clock();
}

// =======================================================================
// Ventana
// =======================================================================
void InitWindow(int argc, char* argv[]) // Inicializa la ventana GLUT
{
    glutInit(&argc, argv);
    glutInitContextVersion(4, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitWindowSize(CurrentWidth, CurrentHeight);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

    WindowHandle = glutCreateWindow(WINDOW_TITLE_PREFIX);

    glutReshapeFunc(ResizeFunction);
    glutDisplayFunc(RenderFunction);
    glutIdleFunc(IdleFunction);
    glutTimerFunc(0, TimerFunction, 0);
    glutCloseFunc(CleanUp);
    glutKeyboardFunc(KeyboardFunction); // AGREGAR ESTA LÍNEA
}

// =======================================================================
// Resize
// =======================================================================
void ResizeFunction(int W, int H) // Función de resize
{
    CurrentWidth  = W;
    CurrentHeight = H;

    glViewport(0,0,W,H);

    ProjectionMatrix = CreateProjectionMatrix(
        60.0f,
        (float)W / (float)H,
        0.1f,
        200.0f
    );
}

// =======================================================================
// Render Loop
// =======================================================================
void RenderFunction(void) // Función de render
{
    FrameCount++;
    TotalFrameCount++;
    
    // Calcular FPS cada 0.5 segundos
    clock_t currentTime = clock();
    float deltaTime = (float)(currentTime - FPSLastTime) / CLOCKS_PER_SEC;
    
    if (deltaTime >= 0.5f) // Actualizar FPS cada medio segundo
    {
        FPS = FrameCount / deltaTime;
        FrameCount = 0;
        FPSLastTime = currentTime;
        
        // Actualizar título de ventana con estadísticas
        UpdateWindowTitle();
    }
    
    // 1. Renderizar pase de sombras
    RenderShadowPass();
    
    // 2. Renderizar escena normal
    glViewport(0, 0, CurrentWidth, CurrentHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DrawOBJ();
    DrawGround();

    glutSwapBuffers();
    glutPostRedisplay();
}

// =======================================================================
// Idle
// =======================================================================
void IdleFunction(void) // Función de idle
{
    glutPostRedisplay();
}

// =======================================================================
// Timer
// =======================================================================
void TimerFunction(int Value) // Función de timer
{
    glutTimerFunc(250, TimerFunction, 1);
}

// =======================================================================
// Clean
// =======================================================================
void CleanUp(void) // Función de limpieza
{
    glDeleteProgram(ShaderIds[0]);
}

// =======================================================================
void CreateOBJ() // Crear modelo OBJ
{
    printf("Cargando modelo OBJ...\n");

    std::vector<Vertex> verts;
    std::vector<GLuint> idx;

    if (!LoadOBJ("backpack_house.obj", verts, idx))
    {
        printf("ERROR cargando OBJ.\n");
        exit(1);
    }

    IndexCount = idx.size();

    // Crear shaders
    ShaderIds[0] = glCreateProgram();
    ShaderIds[1] = LoadShader("SimpleShader.fragment.glsl", GL_FRAGMENT_SHADER);
    ShaderIds[2] = LoadShader("SimpleShader.vertex.glsl",   GL_VERTEX_SHADER);

    glAttachShader(ShaderIds[0], ShaderIds[1]);
    glAttachShader(ShaderIds[0], ShaderIds[2]);
    glLinkProgram(ShaderIds[0]);

    // Obtener ubicaciones de uniforms
    ModelMatrixUniformLocation      = glGetUniformLocation(ShaderIds[0], "ModelMatrix");
    ViewMatrixUniformLocation       = glGetUniformLocation(ShaderIds[0], "ViewMatrix");
    ProjectionMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ProjectionMatrix");

    LightDirUniformLocation     = glGetUniformLocation(ShaderIds[0], "LightDir");
    LightColorUniformLocation   = glGetUniformLocation(ShaderIds[0], "LightColor");
    AmbientColorUniformLocation = glGetUniformLocation(ShaderIds[0], "AmbientColor");
    MaterialColorUniformLocation= glGetUniformLocation(ShaderIds[0], "MaterialColor");
    ViewPosUniformLocation      = glGetUniformLocation(ShaderIds[0], "ViewPos"); 

    
    printf("Cargando textura...\n");
    BaseColorTex = LoadTexture("T_CartoonHouse_Base_color1.jpg");  // Cambia esto por el nombre de tu textura
    if (BaseColorTex == 0) {
        printf("ERROR: No se pudo cargar la textura\n");
        printf("Asegurate que el archivo este en el mismo directorio que el .exe\n");
    } else {
        printf("Textura cargada exitosamente (ID: %d)\n", BaseColorTex);
    }

    // Asignar unidades de textura
    glUseProgram(ShaderIds[0]);
    glUniform1i(glGetUniformLocation(ShaderIds[0], "BaseColor"), 0);
    glUniform1i(glGetUniformLocation(ShaderIds[0], "ShadowMap"), 1);
    glUseProgram(0);

    // Crear VAO/VBO/IBO
    glGenVertexArrays(1, &BufferIds[0]);
    glBindVertexArray(BufferIds[0]);

    glGenBuffers(1, &BufferIds[1]);
    glBindBuffer(GL_ARRAY_BUFFER, BufferIds[1]);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                        sizeof(Vertex),
                        (void*)(sizeof(float)*3));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                    sizeof(Vertex),
                    (void*)(sizeof(float)*6));

    glGenBuffers(1, &BufferIds[2]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferIds[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size()*sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

// =======================================================================
// Create Shadow Map
// =======================================================================
void CreateShadowMap() // Crear mapa de sombras
{
    // Crear shader de sombras
    ShadowShaderIds[0] = glCreateProgram();
    ShadowShaderIds[1] = LoadShader("Shadow.fragment.glsl", GL_FRAGMENT_SHADER);
    ShadowShaderIds[2] = LoadShader("Shadow.vertex.glsl", GL_VERTEX_SHADER);
    
    glAttachShader(ShadowShaderIds[0], ShadowShaderIds[1]);
    glAttachShader(ShadowShaderIds[0], ShadowShaderIds[2]);
    glLinkProgram(ShadowShaderIds[0]);

    // Verificar si el shader compiló correctamente
    GLint success;
    glGetProgramiv(ShadowShaderIds[0], GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(ShadowShaderIds[0], 512, NULL, infoLog);
        printf("ERROR: Shadow shader link failed:\n%s\n", infoLog);
    } else {
        printf("Shadow shader compilado OK\n");
    }

    // Crear framebuffer para sombras
    glGenFramebuffers(1, &ShadowFBO);
    
    // Crear textura de profundidad
    glGenTextures(1, &ShadowMap);
    glBindTexture(GL_TEXTURE_2D, ShadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 
                SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Adjuntar textura al framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, ShadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ShadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    // Verificar que el framebuffer esté completo
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("ERROR: Shadow framebuffer no está completo!\n");
    } else {
        printf("Shadow framebuffer OK\n");
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // CREAR MATRIZ DE LUZ CORRECTAMENTE
    // Usamos CreateProjectionMatrix en lugar de construirla manualmente
    LightProjectionMatrix = IDENTITY_MATRIX;
    
    // Proyección ortográfica manual
    float left = -10.0f;
    float right = 10.0f;
    float bottom = -10.0f;
    float top = 10.0f;
    float nearPlane = 1.0f;
    float farPlane = 25.0f;
    
    LightProjectionMatrix.m[0] = 2.0f / (right - left);
    LightProjectionMatrix.m[5] = 2.0f / (top - bottom);
    LightProjectionMatrix.m[10] = -2.0f / (farPlane - nearPlane);
    LightProjectionMatrix.m[12] = -(right + left) / (right - left);
    LightProjectionMatrix.m[13] = -(top + bottom) / (top - bottom);
    LightProjectionMatrix.m[14] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    LightProjectionMatrix.m[15] = 1.0f;
    
    // Vista de luz - colocar la luz mirando hacia el centro
    LightViewMatrix = IDENTITY_MATRIX;
    
    // Posición de la luz: arriba y ligeramente al lado
    // Dirección de luz es (0.3, 1.0, 0.5), así que posicionamos opuesta
    TranslateMatrix(&LightViewMatrix, -3.0f, -10.0f, -5.0f);
    
    // Rotar para que mire hacia abajo al centro
    RotateAboutxAxis(&LightViewMatrix, DegreesToRadians(-60.0f));
    RotateAboutyAxis(&LightViewMatrix, DegreesToRadians(-17.0f));
    
    printf("Shadow mapping creado\n");
    printf("Luz posicionada en: (-3, -10, -5) mirando hacia (0, -1, 0)\n");
}

// =======================================================================
// Render Shadow Pass
// =======================================================================
void RenderShadowPass() // Renderizar pase de sombras
{
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, ShadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    // Habilitar culling para evitar peter panning
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT); // Renderizar caras traseras para sombras
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glUseProgram(ShadowShaderIds[0]);
    
    Matrix lightSpaceMatrix = MultiplyMatrices(&LightProjectionMatrix, &LightViewMatrix);
    
    GLuint lightSpaceLoc = glGetUniformLocation(ShadowShaderIds[0], "LightSpaceMatrix");
    GLuint modelLoc = glGetUniformLocation(ShadowShaderIds[0], "ModelMatrix");
    
    if (lightSpaceLoc == -1 || modelLoc == -1) {
        printf("ERROR: No se encontraron uniforms en shadow shader\n");
    }
    
    glUniformMatrix4fv(lightSpaceLoc, 1, GL_FALSE, lightSpaceMatrix.m);

    // Renderizar objeto
    ModelMatrix = IDENTITY_MATRIX;
    float angle = AutoRotate ? DegreesToRadians(CubeRotationAngle) : DegreesToRadians(ManualRotationAngle);
    TranslateMatrix(&ModelMatrix, ObjectPositionX, -1.0f, 0.0f);
    RotateAboutyAxis(&ModelMatrix, angle);
    ScaleMatrix(&ModelMatrix, 0.045f, 0.045f, 0.045f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, ModelMatrix.m);
    glBindVertexArray(BufferIds[0]);
    glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, 0);
    
    // Renderizar suelo
    ModelMatrix = IDENTITY_MATRIX;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, ModelMatrix.m);
    glBindVertexArray(GroundVAO);
    glDrawElements(GL_TRIANGLES, GroundIndexCount, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
    glUseProgram(0);
    
    // Restaurar culling
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Restaurar viewport
    glViewport(0, 0, CurrentWidth, CurrentHeight);
}

// =======================================================================
// Keyboard Handler
// =======================================================================
void KeyboardFunction(unsigned char key, int x, int y) // Función de teclado
{
    switch (key)
    {
        case 'a': // Mover a la izquierda
        case 'A':
            ObjectPositionX -= 0.2f;
            break;
            
        case 'd': // Mover a la derecha
        case 'D':
            ObjectPositionX += 0.2f;
            break;
            
        case 'r': // Activar/desactivar rotación automática
        case 'R':
            AutoRotate = !AutoRotate;
            if (AutoRotate) {
                LastTime = 0; // Reiniciar el tiempo para rotación suave
            }
            printf("Rotación automática: %s\n", AutoRotate ? "ON" : "OFF");
            UpdateWindowTitle(); // Actualizar título inmediatamente
            break;
            
        case 'q': // Rotar manualmente a la izquierda
        case 'Q':
            if (!AutoRotate) {
                ManualRotationAngle -= 5.0f;
            }
            break;
            
        case 'e': // Rotar manualmente a la derecha
        case 'E':
            if (!AutoRotate) {
                ManualRotationAngle += 5.0f;
            }
            break;
            
        case 'c': // Centrar objeto
        case 'C':
            ObjectPositionX = 0.0f;
            ManualRotationAngle = 0.0f;
            printf("Objeto centrado\n");
            break;
            
        case 27: // ESC para salir
            glutLeaveMainLoop();
            break;
    }
    
    glutPostRedisplay();
}

// =======================================================================
// Create Ground
// =======================================================================
void CreateGround() // Crear suelo
{
    // Crear un plano simple en Y=0
    std::vector<Vertex> groundVerts;
    std::vector<GLuint> groundIdx;

    float size = 10.0f;

    // 4 vértices del suelo
    Vertex v0, v1, v2, v3;

    // Vértice 0 (esquina inferior izquierda)
    v0.position[0] = -size; v0.position[1] = -1.5f; v0.position[2] = -size;
    v0.normal[0] = 0.0f; v0.normal[1] = 1.0f; v0.normal[2] = 0.0f;
    v0.uv[0] = 0.0f; v0.uv[1] = 0.0f;

    // Vértice 1 (esquina inferior derecha)
    v1.position[0] = size; v1.position[1] = -1.5f; v1.position[2] = -size;
    v1.normal[0] = 0.0f; v1.normal[1] = 1.0f; v1.normal[2] = 0.0f;
    v1.uv[0] = 1.0f; v1.uv[1] = 0.0f;

    // Vértice 2 (esquina superior derecha)
    v2.position[0] = size; v2.position[1] = -1.5f; v2.position[2] = size;
    v2.normal[0] = 0.0f; v2.normal[1] = 1.0f; v2.normal[2] = 0.0f;
    v2.uv[0] = 1.0f; v2.uv[1] = 1.0f;

    // Vértice 3 (esquina superior izquierda)
    v3.position[0] = -size; v3.position[1] = -1.5f; v3.position[2] = size;
    v3.normal[0] = 0.0f; v3.normal[1] = 1.0f; v3.normal[2] = 0.0f;
    v3.uv[0] = 0.0f; v3.uv[1] = 1.0f;

    groundVerts.push_back(v0);
    groundVerts.push_back(v1);
    groundVerts.push_back(v2);
    groundVerts.push_back(v3);

    // Dos triángulos
    groundIdx.push_back(0);
    groundIdx.push_back(1);
    groundIdx.push_back(2);

    groundIdx.push_back(0);
    groundIdx.push_back(2);
    groundIdx.push_back(3);

    GroundIndexCount = groundIdx.size();

    // Crear VAO/VBO/IBO
    glGenVertexArrays(1, &GroundVAO);
    glBindVertexArray(GroundVAO);

    glGenBuffers(1, &GroundVBO);
    glBindBuffer(GL_ARRAY_BUFFER, GroundVBO);
    glBufferData(GL_ARRAY_BUFFER, 
                groundVerts.size() * sizeof(Vertex),
                groundVerts.data(),
                GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                        sizeof(Vertex),
                        (void*)(sizeof(float)*3));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                        sizeof(Vertex),
                        (void*)(sizeof(float)*6));

    glGenBuffers(1, &GroundIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GroundIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                groundIdx.size() * sizeof(GLuint),
                groundIdx.data(),
                GL_STATIC_DRAW);

    glBindVertexArray(0);

    printf("Suelo creado\n");
}

// =======================================================================
// Draw OBJ
// =======================================================================
void DrawOBJ() // Dibujar modelo OBJ
{
    float angle;
    clock_t now = clock();

    if (AutoRotate)
    {
        if (LastTime == 0)
            LastTime = now;

        CubeRotationAngle += 15.0f * ((float)(now - LastTime) / CLOCKS_PER_SEC);
        angle = DegreesToRadians(CubeRotationAngle);
        LastTime = now;
    }
    else
    {
        angle = DegreesToRadians(ManualRotationAngle);
    }

    ModelMatrix = IDENTITY_MATRIX;

    TranslateMatrix(&ModelMatrix, ObjectPositionX, -1.0f, 0.0f);
    RotateAboutyAxis(&ModelMatrix, angle);
    ScaleMatrix(&ModelMatrix, 0.045f, 0.045f, 0.045f);

    glUseProgram(ShaderIds[0]);
    glUniformMatrix4fv(ModelMatrixUniformLocation, 1, GL_FALSE, ModelMatrix.m);
    glUniformMatrix4fv(ViewMatrixUniformLocation, 1, GL_FALSE, ViewMatrix.m);
    glUniformMatrix4fv(ProjectionMatrixUniformLocation, 1, GL_FALSE, ProjectionMatrix.m);
    
    // LightSpaceMatrix para sombras
    Matrix lightSpaceMatrix = MultiplyMatrices(&LightProjectionMatrix, &LightViewMatrix);
    GLuint lightSpaceLoc = glGetUniformLocation(ShaderIds[0], "LightSpaceMatrix");
    glUniformMatrix4fv(lightSpaceLoc, 1, GL_FALSE, lightSpaceMatrix.m);

    glUniform1i(glGetUniformLocation(ShaderIds[0], "UseTexture"), 1);

    // Solo BaseColor y ShadowMap
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, BaseColorTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ShadowMap);

    // Posición de la cámara (inversa de ViewMatrix translation)
    glUniform3f(ViewPosUniformLocation, 0.0f, 1.8f, 7.5f);

    glUniform3f(LightDirUniformLocation, 0.3f, 1.0f, 0.5f);
    glUniform3f(LightColorUniformLocation, 1.0f, 0.98f, 0.95f);
    glUniform3f(AmbientColorUniformLocation, 0.25f, 0.23f, 0.20f); // Reducir ambiente para ver sombras mejor
    glUniform3f(MaterialColorUniformLocation, 1.0f, 1.0f, 1.0f);

    glBindVertexArray(BufferIds[0]);
    glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}

// =======================================================================
// Draw Ground
// =======================================================================
void DrawGround() // Dibujar suelo
{
    ModelMatrix = IDENTITY_MATRIX;

    glUseProgram(ShaderIds[0]);

    glUniformMatrix4fv(ModelMatrixUniformLocation, 1, GL_FALSE, ModelMatrix.m);
    glUniformMatrix4fv(ViewMatrixUniformLocation, 1, GL_FALSE, ViewMatrix.m);
    glUniformMatrix4fv(ProjectionMatrixUniformLocation, 1, GL_FALSE, ProjectionMatrix.m);
    
    Matrix lightSpaceMatrix = MultiplyMatrices(&LightProjectionMatrix, &LightViewMatrix);
    glUniformMatrix4fv(glGetUniformLocation(ShaderIds[0], "LightSpaceMatrix"), 1, GL_FALSE, lightSpaceMatrix.m);

    glUniform1i(glGetUniformLocation(ShaderIds[0], "UseTexture"), 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ShadowMap);

    glUniform3f(ViewPosUniformLocation, 0.0f, 1.8f, 7.5f);
    glUniform3f(LightDirUniformLocation, 0.3f, 1.0f, 0.5f);
    glUniform3f(LightColorUniformLocation, 1.0f, 0.98f, 0.95f);
    glUniform3f(AmbientColorUniformLocation, 0.25f, 0.23f, 0.20f); // Reducir ambiente
    glUniform3f(MaterialColorUniformLocation, 0.75f, 0.70f, 0.62f);

    glBindVertexArray(GroundVAO);
    glDrawElements(GL_TRIANGLES, GroundIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}
