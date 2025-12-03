# Rasterización Optimizada de Escenarios

A real-time 3D rendering project demonstrating optimized scene rasterization with shadow mapping, texture mapping, and Phong lighting using OpenGL 4.3 and GLSL shaders.

## Features

- **OBJ Model Loading**: Custom OBJ file parser supporting vertices, normals, and texture coordinates
- **Shadow Mapping**: Real-time shadow rendering using depth maps (2048x2048 resolution)
- **PBR-style Lighting**: Phong shading with customizable ambient, diffuse, and specular components
- **Texture Mapping**: Support for base color textures with gamma correction
- **Interactive Controls**: Keyboard-based object manipulation and camera controls
- **Ground Plane**: Procedurally generated ground with proper shadow reception

## Technical Stack

- **Language**: C++ with OpenGL 4.3 Core Profile
- **Libraries**:
  - FreeGLUT (windowing and input)
  - GLEW (OpenGL extension loading)
  - stb_image.h (texture loading)
- **Shaders**: GLSL 4.3 (vertex and fragment shaders)

## Project Structure

```
├── main.cpp                      # Main application logic
├── Utils.c / Utils.h             # Matrix operations and shader utilities
├── SimpleShader.vertex.glsl      # Main vertex shader
├── SimpleShader.fragment.glsl    # Main fragment shader with lighting
├── Shadow.vertex.glsl            # Shadow map vertex shader
├── Shadow.fragment.glsl          # Shadow map fragment shader
├── stb_image.h                   # Image loading library
├── backpack_house.obj            # 3D model file
└── T_CartoonHouse_Base_color1.jpg # Base color texture
```

## Building the Project

### Requirements
- C++ compiler with C++11 support
- OpenGL 4.3+ compatible graphics card
- FreeGLUT development libraries
- GLEW development libraries

### Compilation (Windows)
```bash
g++ -o rasterization main.cpp Utils.c -lglew32 -lfreeglut -lopengl32 -lglu32 -std=c++11
```

### Compilation (Linux)
```bash
g++ -o rasterization main.cpp Utils.c -lGLEW -lglut -lGL -lGLU -std=c++11
```

## Controls

| Key | Action |
|-----|--------|
| `A` | Move object left |
| `D` | Move object right |
| `R` | Toggle automatic rotation |
| `Q` | Rotate manually left (when auto-rotation is off) |
| `E` | Rotate manually right (when auto-rotation is off) |
| `C` | Center object and reset rotation |
| `ESC` | Exit application |

## Implementation Highlights

### Shadow Mapping Pipeline
1. **Shadow Pass**: Renders scene from light's perspective to shadow framebuffer
2. **Main Pass**: Renders scene normally with shadow map lookup in fragment shader
3. **PCF Filtering**: 3x3 kernel for smooth shadow edges

### Lighting Model
- **Ambient**: Base illumination (25% intensity)
- **Diffuse**: Lambertian reflection based on surface normal
- **Specular**: Phong highlights with shininess = 32
- **Shadow attenuation**: 85% darkness for shadowed areas

### Matrix Operations
Custom matrix library in `Utils.c` provides:
- Matrix multiplication
- Perspective projection
- Transformation matrices (translate, rotate, scale)

### OBJ Loading
The `LoadOBJ` function handles:
- Vertices with positions, normals, and UVs
- Face triangulation for n-gons
- Index optimization using hash map deduplication

## Performance Optimizations

1. **Indexed Rendering**: Uses Element Buffer Objects (EBO) to minimize vertex duplication
2. **Shadow Map Resolution**: Configurable via `SHADOW_WIDTH` and `SHADOW_HEIGHT` constants
3. **Frustum Culling**: Orthographic light projection bounds scene tightly
4. **Texture Mipmapping**: Automatic mipmap generation for texture filtering

## Rendering Pipeline

```
Initialize()
  ├── CreateOBJ()          # Load model, compile shaders, setup VAO/VBO/EBO
  ├── CreateGround()       # Generate ground plane geometry
  └── CreateShadowMap()    # Setup shadow framebuffer and light matrices
  
RenderFunction() (per frame)
  ├── RenderShadowPass()   # Render to shadow map
  │   ├── Bind ShadowFBO
  │   ├── Render object from light view
  │   └── Render ground from light view
  └── Main Pass
      ├── DrawOBJ()        # Render model with shadows
      └── DrawGround()     # Render ground with shadows
```

## Shader Uniforms

### Main Shader
- Transform matrices: ModelMatrix, ViewMatrix, ProjectionMatrix
- LightSpaceMatrix: Shadow map transformation
- Lighting: LightDir, LightColor, AmbientColor
- Textures: BaseColor (texture sampler), ShadowMap (depth texture)
- UseTexture: Toggle between texture and material color

### Shadow Shader
- LightSpaceMatrix: Light's view-projection matrix
- ModelMatrix: Object transformation

## Known Limitations

- Only supports triangular faces in OBJ files (n-gons are triangulated)
- Single directional light source
- No support for multiple objects (single model + ground)
- Fixed camera position (modifiable in code)

## Future Enhancements

- [ ] Multiple light sources
- [ ] Normal mapping support
- [ ] Cascaded shadow maps for larger scenes
- [ ] FPS camera controls
- [ ] Material system with multiple textures (roughness, AO, metallic)
- [ ] Scene graph for multiple objects

## Credits

- **stb_image.h**: Sean Barrett (public domain image loader)
- **Model/Texture**: Ensure you have proper licensing for included assets

## License

This project is provided as-is for educational purposes. Please ensure proper licensing for any third-party assets used.

---

**Author**: Alondra Soto  
**OpenGL Version**: 4.3 Core Profile  
**Last Updated**: December 2024
