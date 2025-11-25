#include "Utils.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <map>

#define WINDOW_TITLE_PREFIX "Rasterizacion_Optimizada_de_Escenarios"

int CurrentWidth = 800;
int CurrentHeight = 600;
int WindowHandle = 0;

size_t IndexCount = 0;
size_t GroundIndexCount = 0;

unsigned FrameCount = 0;

GLuint
ProjectionMatrixUniformLocation,
ViewMatrixUniformLocation,
ModelMatrixUniformLocation,
LightDirUniformLocation,
LightColorUniformLocation,
AmbientColorUniformLocation,
MaterialColorUniformLocation;

GLuint BufferIds[3] = {0};
GLuint ShaderIds[3] = {0};

GLuint GroundVAO = 0, GroundVBO = 0, GroundIBO = 0;

Matrix ProjectionMatrix;
Matrix ViewMatrix;
Matrix ModelMatrix;

float CubeRotationAngle = 0;
clock_t LastTime = 0;

// =======================================================================
// OBJ Loader
// =======================================================================
bool LoadOBJ(const std::string& path,
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
// Prototipos
// =======================================================================
void Initialize(int, char*[]);
void InitWindow(int, char*[]);
void ResizeFunction(int, int);
void RenderFunction(void);
void TimerFunction(int);
void IdleFunction(void);
void CleanUp(void);

void CreateOBJ(void);
void DrawOBJ(void);
void CreateGround(void);
void DrawGround(void);

// =======================================================================
// MAIN
// =======================================================================
int main(int argc, char* argv[])
{
    Initialize(argc, argv);
    glutMainLoop();
    return 0;
}

// =======================================================================
// Inicialización
// =======================================================================
void Initialize(int argc, char* argv[])
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

    // 👁 CAMARA FIX: Más lejos y más arriba
    TranslateMatrix(&ViewMatrix, 0.0f, -3.0f, -40.0f);

    glClearColor(0.4f, 0.7f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    CreateOBJ();
    CreateGround();
}

// =======================================================================
// Ventana
// =======================================================================
void InitWindow(int argc, char* argv[])
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
}

// =======================================================================
// Resize
// =======================================================================
void ResizeFunction(int W, int H)
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
void RenderFunction(void)
{
    FrameCount++;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DrawOBJ();
    DrawGround();

    glutSwapBuffers();
    glutPostRedisplay();
}

// =======================================================================
// Idle
// =======================================================================
void IdleFunction(void)
{
    glutPostRedisplay();
}

// =======================================================================
// Timer
// =======================================================================
void TimerFunction(int Value)
{
    FrameCount = 0;
    glutTimerFunc(250, TimerFunction, 1);
}

// =======================================================================
// Clean
// =======================================================================
void CleanUp(void)
{
    glDeleteProgram(ShaderIds[0]);
}

// =======================================================================
// Crear OBJ
// =======================================================================
void CreateOBJ()
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

    ShaderIds[0] = glCreateProgram();
    ShaderIds[1] = LoadShader("SimpleShader.fragment.glsl", GL_FRAGMENT_SHADER);
    ShaderIds[2] = LoadShader("SimpleShader.vertex.glsl",   GL_VERTEX_SHADER);

    glAttachShader(ShaderIds[0], ShaderIds[1]);
    glAttachShader(ShaderIds[0], ShaderIds[2]);
    glLinkProgram(ShaderIds[0]);

    ModelMatrixUniformLocation      = glGetUniformLocation(ShaderIds[0], "ModelMatrix");
    ViewMatrixUniformLocation       = glGetUniformLocation(ShaderIds[0], "ViewMatrix");
    ProjectionMatrixUniformLocation = glGetUniformLocation(ShaderIds[0], "ProjectionMatrix");

    LightDirUniformLocation     = glGetUniformLocation(ShaderIds[0], "LightDir");
    LightColorUniformLocation   = glGetUniformLocation(ShaderIds[0], "LightColor");
    AmbientColorUniformLocation = glGetUniformLocation(ShaderIds[0], "AmbientColor");
    MaterialColorUniformLocation= glGetUniformLocation(ShaderIds[0], "MaterialColor");

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

    glGenBuffers(1, &BufferIds[2]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferIds[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size()*sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

// =======================================================================
// Draw OBJ
// =======================================================================
void DrawOBJ()
{
    float angle;
    clock_t now = clock();

    if (LastTime == 0)
        LastTime = now;

    CubeRotationAngle += 15.0f * ((float)(now - LastTime) / CLOCKS_PER_SEC);
    angle = DegreesToRadians(CubeRotationAngle);
    LastTime = now;

    ModelMatrix = IDENTITY_MATRIX;

    // Primero trasladar a la posición sobre el suelo
    TranslateMatrix(&ModelMatrix, 0.0f, -1.5f, 0.0f);
    
    // Rotación para animación (solo en Y para mantenerlo vertical)
    RotateAboutyAxis(&ModelMatrix, angle);
    
    // Escalar el modelo
    ScaleMatrix(&ModelMatrix, 0.1f, 0.1f, 0.1f);

    glUseProgram(ShaderIds[0]);

    glUniformMatrix4fv(ModelMatrixUniformLocation,      1, GL_FALSE, ModelMatrix.m);
    glUniformMatrix4fv(ViewMatrixUniformLocation,       1, GL_FALSE, ViewMatrix.m);
    glUniformMatrix4fv(ProjectionMatrixUniformLocation, 1, GL_FALSE, ProjectionMatrix.m);

    glUniform3f(LightDirUniformLocation,     0.5f, 1.0f, 0.3f);
    glUniform3f(LightColorUniformLocation,   1.0f, 1.0f, 1.0f);
    glUniform3f(AmbientColorUniformLocation, 0.2f, 0.2f, 0.25f);
    glUniform3f(MaterialColorUniformLocation,0.7f, 0.4f, 0.2f);

    glBindVertexArray(BufferIds[0]);
    glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}

// =======================================================================
// Crear terreno
// =======================================================================
void CreateGround()
{
    Vertex V[4];

    V[0].position[0] = -15; V[0].position[1] = -1.5f; V[0].position[2] = -15;
    V[1].position[0] =  15; V[1].position[1] = -1.5f; V[1].position[2] = -15;
    V[2].position[0] =  15; V[2].position[1] = -1.5f; V[2].position[2] =  15;
    V[3].position[0] = -15; V[3].position[1] = -1.5f; V[3].position[2] =  15;

    for (int i=0;i<4;i++)
    {
        V[i].normal[0] = 0;
        V[i].normal[1] = 1;
        V[i].normal[2] = 0;
    }

    GLuint I[6] = {0,1,2, 2,3,0};
    GroundIndexCount = 6;

    glGenVertexArrays(1, &GroundVAO);
    glBindVertexArray(GroundVAO);

    glGenBuffers(1, &GroundVBO);
    glBindBuffer(GL_ARRAY_BUFFER, GroundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(V), V, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(sizeof(float)*3));

    glGenBuffers(1, &GroundIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GroundIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(I), I, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

// =======================================================================
// Draw Ground
// =======================================================================
void DrawGround()
{
    ModelMatrix = IDENTITY_MATRIX;

    glUseProgram(ShaderIds[0]);

    glUniformMatrix4fv(ModelMatrixUniformLocation,      1, GL_FALSE, ModelMatrix.m);
    glUniformMatrix4fv(ViewMatrixUniformLocation,       1, GL_FALSE, ViewMatrix.m);
    glUniformMatrix4fv(ProjectionMatrixUniformLocation, 1, GL_FALSE, ProjectionMatrix.m);

    glUniform3f(LightDirUniformLocation,     0.5f, 1.0f, 0.3f);
    glUniform3f(LightColorUniformLocation,   1.0f, 1.0f, 1.0f);
    glUniform3f(AmbientColorUniformLocation, 0.2f, 0.2f, 0.25f);
    glUniform3f(MaterialColorUniformLocation,0.1f, 0.5f, 0.1f);

    glBindVertexArray(GroundVAO);
    glDrawElements(GL_TRIANGLES, GroundIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}
