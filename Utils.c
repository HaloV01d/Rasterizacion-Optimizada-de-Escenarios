#include "Utils.h" // Incluye el archivo de cabecera Utils.h

const Matrix IDENTITY_MATRIX = {{ // Definición de la matriz identidad
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
}};

float Cotangent(float angle) { // Función para calcular la cotangente de un ángulo
    return (float)(1.0f / tan(angle));
}

float DegreesToRadians(float degrees) { // Función para convertir grados a radianes
    return degrees * (float)(PI / 180.0f);
}   

float RadiansToDegrees(float radians) { // Función para convertir radianes a grados
    return radians * (float)(180.0f / PI);
}

Matrix MultiplyMatrices(const Matrix* m1, const Matrix* m2) // Función para multiplicar dos matrices
{
Matrix out = IDENTITY_MATRIX;
unsigned int row, column, row_offset;

  for (row = 0, row_offset = row * 4; row < 4; ++row, row_offset = row * 4)
    for (column = 0; column < 4; ++column)
    out.m[row_offset + column] =
        (m1->m[row_offset + 0] * m2->m[column + 0]) +
        (m1->m[row_offset + 1] * m2->m[column + 4]) +
        (m1->m[row_offset + 2] * m2->m[column + 8]) +
        (m1->m[row_offset + 3] * m2->m[column + 12]);

return out;
}

void ScaleMatrix(Matrix* m, float x, float y, float z) // Función para escalar una matriz
{
    Matrix scale = IDENTITY_MATRIX;

    scale.m[0] = x;
    scale.m[5] = y;
    scale.m[10] = z;

    memcpy(m->m, MultiplyMatrices(m, &scale).m, sizeof(m->m));
}

void TranslateMatrix(Matrix* m, float x, float y, float z) // Función para trasladar una matriz
{
    Matrix translation = IDENTITY_MATRIX;

    translation.m[12] = x;
    translation.m[13] = y;
    translation.m[14] = z;

    memcpy(m->m, MultiplyMatrices(m, &translation).m, sizeof(m->m));
}

void RotateAboutxAxis(Matrix* m, float angle) // Función para rotar una matriz alrededor del eje x
{
    Matrix rotation = IDENTITY_MATRIX;
    float c = cosf(angle);
    float s = sinf(angle);

    rotation.m[5] = c;
    rotation.m[6] = -s;
    rotation.m[9] = s;
    rotation.m[10] = c;

    memcpy(m->m, MultiplyMatrices(m, &rotation).m, sizeof(m->m)); // Copia el resultado de la multiplicación de matrices en m
}

void RotateAboutyAxis(Matrix* m, float angle) // Función para rotar una matriz alrededor del eje y
{
    Matrix rotation = IDENTITY_MATRIX;
    float c = cosf(angle);
    float s = sinf(angle);

    rotation.m[0] = c;
    rotation.m[8] = s;
    rotation.m[2] = -s;
    rotation.m[10] = c;

    memcpy(m->m, MultiplyMatrices(m, &rotation).m, sizeof(m->m)); // Copia el resultado de la multiplicación de matrices en m
}

void RotateAboutzAxis(Matrix* m, float angle) // Función para rotar una matriz alrededor del eje z
{
    Matrix rotation = IDENTITY_MATRIX;
    float c = cosf(angle);
    float s = sinf(angle);

    rotation.m[0] = c;
    rotation.m[1] = -s;
    rotation.m[4] = s;
    rotation.m[5] = c;

    memcpy(m->m, MultiplyMatrices(m, &rotation).m, sizeof(m->m));
}

Matrix CreateProjectionMatrix( // Función para crear una matriz de proyección
float fovy,
float aspect_ratio,
float near_plane,
float far_plane
)
{
Matrix out = { { 0 } };

const float
    y_scale = Cotangent(DegreesToRadians(fovy / 2)),
    x_scale = y_scale / aspect_ratio,
    frustum_length = far_plane - near_plane;

out.m[0] = x_scale;
out.m[5] = y_scale;
out.m[10] = -((far_plane + near_plane) / frustum_length);
out.m[11] = -1;
out.m[14] = -((2 * near_plane * far_plane) / frustum_length);

return out;
}

void ExitOnGLError(const char* message) // Función para salir en caso de error de OpenGL
{
    GLenum error = glGetError();

    if (error != GL_NO_ERROR)
    {
        fprintf(
            stderr,
            "%s: %s\n",
            message,
            gluErrorString(error)
        );
        exit(-1);
    }
}

GLuint LoadShader(const char* filename, GLenum shader_type) // Función para cargar un shader desde un archivo
{
GLuint shader_id = 0;
FILE* file;
long file_size = -1;
char* glsl_source;

if (NULL != (file = fopen(filename, "rb")) &&
    0 == fseek(file, 0, SEEK_END) &&
    -1 != (file_size = ftell(file)))
{
    rewind(file);
    
    if (NULL != (glsl_source = (char*)malloc(file_size + 1)))
    {
    if (file_size == (long)fread(glsl_source, sizeof(char), file_size, file))
    {
        glsl_source[file_size] = '\0';

        if (0 != (shader_id = glCreateShader(shader_type)))
        {
        glShaderSource(shader_id, 1, &glsl_source, NULL);
        glCompileShader(shader_id);
        ExitOnGLError("Could not compile a shader");
        }
        else
        fprintf(stderr, "ERROR: Could not create a shader.\n");
    }
    else
        fprintf(stderr, "ERROR: Could not read file %s\n", filename);

    free(glsl_source);
    }
    else
    fprintf(stderr, "ERROR: Could not allocate %i bytes.\n", file_size);

    fclose(file);
}
else
{
    if (NULL != file)
    fclose(file);
    fprintf(stderr, "ERROR: Could not open file %s\n", filename);
}

return shader_id;
}


