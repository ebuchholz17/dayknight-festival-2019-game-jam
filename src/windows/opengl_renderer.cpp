#include <gl\GL.h>

#include "opengl_renderer.h"
#include "glsl_shaders.h"

static int loadWGLExtensions (HDC deviceContext) {
    wglGetExtensionsStringARB = (wgl_get_extensions_string_arb *)wglGetProcAddress("wglGetExtensionsStringARB");
    // TODO(ebuchholz): Check for error

    char *wglExtensions = wglGetExtensionsStringARB(deviceContext);
    if (wglExtensions == 0) {
        return 1;
    }
    char *index = wglExtensions;
    char *endIndex;
    int numSupported = 0; // we will require all of the specified extensions, and quit if they are not all available
    while (*index) {
            while ((*index) == ' ') { ++index; };
            endIndex = index;
            while (*endIndex && *endIndex != ' ') { ++endIndex; }

            int length = (int)(endIndex - index);
            if (stringsAreEqual(length, index, "WGL_ARB_create_context")) { ++numSupported; }
            else if (stringsAreEqual(length, index, "WGL_ARB_pixel_format")) { ++numSupported; }
            else if (stringsAreEqual(length, index, "WGL_EXT_swap_control")) { ++numSupported; }

            index = endIndex;
    }
    if (numSupported != 3) {
        return 2;
    }
    // Load the functions we need
    wglCreateContextAttribsARB = (wgl_create_context_attribs_arb *)wglGetProcAddress("wglCreateContextAttribsARB");
    wglChoosePixelFormatARB = (wgl_choose_pixel_format_arb *)wglGetProcAddress("wglChoosePixelFormatARB");
    wglSwapIntervalEXT = (wgl_swap_interval_ext *)wglGetProcAddress("wglSwapIntervalEXT");

    return 0;
}

static void loadOpenGLFunctions () {
    glCreateShader = (gl_create_shader *)wglGetProcAddress("glCreateShader");
    glShaderSource = (gl_shader_source *)wglGetProcAddress("glShaderSource");
    glCompileShader = (gl_compile_shader *)wglGetProcAddress("glCompileShader");
    glCreateProgram = (gl_create_program *)wglGetProcAddress("glCreateProgram");
    glAttachShader = (gl_attach_shader *)wglGetProcAddress("glAttachShader");
    glLinkProgram = (gl_link_program *)wglGetProcAddress("glLinkProgram");
    glGetShaderiv = (gl_get_shader_iv *)wglGetProcAddress("glGetShaderiv");
    glGetProgramiv = (gl_get_program_iv *)wglGetProcAddress("glGetProgramiv");
    glGetShaderInfoLog = (gl_get_shader_info_log *)wglGetProcAddress("glGetShaderInfoLog");
    glGetProgramInfoLog = (gl_get_program_info_log *)wglGetProcAddress("glGetProgramInfoLog");
    glDetachShader = (gl_detach_shader *)wglGetProcAddress("glDetachShader");

    glGetAttribLocation = (gl_get_attrib_location *)wglGetProcAddress("glGetAttribLocation");
    glGenVertexArrays = (gl_gen_vertex_arrays *)wglGetProcAddress("glGenVertexArrays");
    glBindVertexArray = (gl_bind_vertex_array *)wglGetProcAddress("glBindVertexArray");
    glGenBuffers = (gl_gen_buffers *)wglGetProcAddress("glGenBuffers");
    glBindBuffer = (gl_bind_buffer *)wglGetProcAddress("glBindBuffer");
    glBufferData = (gl_buffer_data *)wglGetProcAddress("glBufferData");
    glEnableVertexAttribArray = (gl_enable_vertex_attrib_array *)wglGetProcAddress("glEnableVertexAttribArray");
    glDisableVertexAttribArray = (gl_disable_vertex_attrib_array *)wglGetProcAddress("glDisableVertexAttribArray");
    glVertexAttribPointer = (gl_vertex_attrib_pointer *)wglGetProcAddress("glVertexAttribPointer");
    glUseProgram = (gl_use_program *)wglGetProcAddress("glUseProgram");

    glGetUniformLocation = (gl_get_uniform_location *)wglGetProcAddress("glGetUniformLocation");
    glUniform1i = (gl_uniform_1i *)wglGetProcAddress("glUniform1i");
    glUniform1f = (gl_uniform_1f *)wglGetProcAddress("glUniform1f");
    glUniformMatrix4fv = (gl_uniform_matrix_4fv *)wglGetProcAddress("glUniformMatrix4fv");

    glActiveTexture = (gl_active_texture *)wglGetProcAddress("glActiveTexture");
    glGenerateMipmap = (gl_generate_mipmap *)wglGetProcAddress("glGenerateMipmap");
}

static void loadShader(GLuint *shaderHandle, int shaderType, char *shaderSource, 
                       renderer_memory *memory) 
{
    *shaderHandle = glCreateShader(shaderType);
    const char *constShaderSource = (const char *)shaderSource;
    glShaderSource(*shaderHandle, 1, &constShaderSource, NULL);
    glCompileShader(*shaderHandle);
    int result;
    glGetShaderiv(*shaderHandle, GL_COMPILE_STATUS, &result);
    if (result != 1) {
        memory->debugPrintString("Failed to compile shader\n");
        int length;
        int actualLength;
        glGetShaderiv(*shaderHandle, GL_INFO_LOG_LENGTH, &length);

        char *infoLog = (char *)memory->memory; // completely wreck memory and crash the program
        glGetShaderInfoLog(*shaderHandle, length, &actualLength, infoLog);
        memory->debugPrintString(infoLog);
        assert(0); // Force a crash
    }
}

void loadRendererMesh (renderer_memory *memory, loaded_mesh_asset *loadedMesh) {
    openGL_renderer *renderer = (openGL_renderer *)memory->memory;
    assert(renderer->numMeshes < MAX_OPENGL_MESHES);
    // TODO(ebuchholz): triple check that the keys will line up this way
    openGL_mesh *mesh = &renderer->meshes[loadedMesh->key];
    renderer->numMeshes++;

    glGenBuffers(1, &mesh->positionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, loadedMesh->positions.count * sizeof(float), loadedMesh->positions.values, GL_STATIC_DRAW);

    glGenBuffers(1, &mesh->texCoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->texCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, loadedMesh->texCoords.count * sizeof(float), loadedMesh->texCoords.values, GL_STATIC_DRAW);

    glGenBuffers(1, &mesh->normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, loadedMesh->normals.count * sizeof(float), loadedMesh->normals.values, GL_STATIC_DRAW);

    glGenBuffers(1, &mesh->indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, loadedMesh->indices.count * sizeof(int), loadedMesh->indices.values, GL_STATIC_DRAW);

    mesh->numIndices = loadedMesh->indices.count;
}

void loadRendererTexture (renderer_memory *memory, loaded_texture_asset *loadedTexture) {
    openGL_renderer *renderer = (openGL_renderer *)memory->memory;
    assert(renderer->numTextures < MAX_OPENGL_TEXTURES);
    // TODO(ebuchholz): triple check that the keys will line up this way
    openGL_texture *texture = &renderer->textures[loadedTexture->key];
    texture->width = loadedTexture->width;
    texture->height = loadedTexture->height;
    renderer->numTextures++;

    glGenTextures(1, &texture->textureID);
    glBindTexture(GL_TEXTURE_2D, texture->textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, loadedTexture->width, loadedTexture->height, 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, loadedTexture->pixels);
    //glGenerateMipmap(GL_TEXTURE_2D);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

static void createShaderProgram(openGL_renderer *renderer, shader_type type,
                                char *vertexSource, char *fragmentSource, 
                                renderer_memory *memory) 
{
    // shader_type enum must map inside of the shader array
    assert(type == renderer->numShaders);
    shader_program *shaderProgram = &renderer->shaders[type];
    renderer->numShaders++;

    loadShader(&shaderProgram->vertexShader, GL_VERTEX_SHADER, vertexSource, memory);
    loadShader(&shaderProgram->fragmentShader, GL_FRAGMENT_SHADER, fragmentSource, memory);

    shaderProgram->program = glCreateProgram();
    glAttachShader(shaderProgram->program, shaderProgram->vertexShader);
    glAttachShader(shaderProgram->program, shaderProgram->fragmentShader);
    glLinkProgram(shaderProgram->program);
    int result;
    glGetProgramiv(shaderProgram->program, GL_LINK_STATUS, &result);
    if (result != 1) {
        memory->debugPrintString("Failed to link shader\n");
        int length;
        int actualLength;
        glGetProgramiv(shaderProgram->program, GL_INFO_LOG_LENGTH, &length);

        char *infoLog = (char *)memory->memory;
        glGetProgramInfoLog(shaderProgram->program, length, &actualLength, infoLog);
        memory->debugPrintString(infoLog);
        assert(0); // Force a crash
    }
    glDetachShader(shaderProgram->program, shaderProgram->vertexShader);
    glDetachShader(shaderProgram->program, shaderProgram->fragmentShader);
}

int initOpenGL (HWND window, renderer_memory *memory) {
    openGL_renderer *renderer = (openGL_renderer *)memory->memory;

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;

    HDC deviceContext = GetDC(window);
    renderer->deviceContext = deviceContext;
    int pixelFormat = ChoosePixelFormat(deviceContext, &pfd);
    SetPixelFormat(deviceContext, pixelFormat, &pfd);

    HGLRC glRenderContext = wglCreateContext(deviceContext);
    wglMakeCurrent(deviceContext, glRenderContext);

    int error = loadWGLExtensions(deviceContext);
    if (error != 0) {
        return error;
    }
    int attribList[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 0,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB, // TODO: maybe remove debug
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
        0
    };
    HGLRC betterContext = wglCreateContextAttribsARB(deviceContext, 0, attribList);

    assert(betterContext);

    wglMakeCurrent(deviceContext, betterContext);
    wglDeleteContext(glRenderContext);
    renderer->context = betterContext;

    // TODO(ebuchholz): See if we need to destroy the window and make a new one with different settings
    // Use wglChoosePixelFormatARB, etc.

    // OpenGL 3.x core functions
    loadOpenGLFunctions();
    wglSwapIntervalEXT(1);

    createShaderProgram(renderer, SHADER_TYPE_DEFAULT,
                        defaultVertexShaderSource, defaultFragmentShaderSource, memory);
    createShaderProgram(renderer, SHADER_TYPE_LINES,
                        lineVertexShaderSource, lineFragmentShaderSource, memory);
    createShaderProgram(renderer, SHADER_TYPE_SPRITE,
                        spriteVertexShaderSource, spriteFragmentShaderSource, memory);
    createShaderProgram(renderer, SHADER_TYPE_BACKGROUND_VISUALIZATION,
                        backgroundVisualizationVertexShaderSource, backgroundVisualizationFragmentShaderSource, memory);

    // sprite drawing
    glGenBuffers(1, &renderer->spritePositionBuffer);
    glGenBuffers(1, &renderer->spriteTextureBuffer);
    glGenBuffers(1, &renderer->spriteColorBuffer);
    glGenBuffers(1, &renderer->spriteIndexBuffer);

    // index buffer doesn't change
    // TODO(ebuchholz): temporary memory, don't need to keep it in renderer
    int indexIndex = 0;
    for (int i = 0; i < MAX_SPRITES_PER_BATCH; ++i) {
        int vertexNum = i * 4;
        renderer->spriteIndices[indexIndex++] = vertexNum;
        renderer->spriteIndices[indexIndex++] = vertexNum+3;
        renderer->spriteIndices[indexIndex++] = vertexNum+1;
        renderer->spriteIndices[indexIndex++] = vertexNum;
        renderer->spriteIndices[indexIndex++] = vertexNum+2;
        renderer->spriteIndices[indexIndex++] = vertexNum+3;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->spriteIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_SPRITES_PER_BATCH * 3 * 2 * sizeof(int), renderer->spriteIndices, GL_STATIC_DRAW);

    glGenBuffers(1, &renderer->fullScreenQuadBuffer);

    float quadPositions[12];
    quadPositions[0] = -1.0f;
    quadPositions[1] = 1.0f;
    quadPositions[2] = -1.0f;
    quadPositions[3] = -1.0f;
    quadPositions[4] = 1.0f;
    quadPositions[5] = 1.0f;

    quadPositions[6] = -1.0f;
    quadPositions[7] = -1.0f;
    quadPositions[8] = 1.0f;
    quadPositions[9] = -1.0f;
    quadPositions[10] = 1.0f;
    quadPositions[11] = 1.0f;

    glBindBuffer(GL_ARRAY_BUFFER, renderer->fullScreenQuadBuffer);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), quadPositions, GL_STATIC_DRAW);

    // For line drawing, maybe other commands that don't have a buffer ready ahead of time and supply data on demand
    glGenBuffers(1, &renderer->debugPositionBuffer);

    // TODO(ebuchholz): use the right blend mode depending on what's being drawn, this is just for default 2d drawing
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return 0;
}

void setCamera (openGL_renderer *renderer, render_command_set_camera *setCameraCommand) {
    renderer->viewMatrix = setCameraCommand->viewMatrix;
    renderer->projMatrix = setCameraCommand->projMatrix;
}

void drawModel (openGL_renderer *renderer, GLuint program, render_command_model *modelCommand) {
    openGL_mesh *mesh = &renderer->meshes[modelCommand->meshKey];

    // TODO(ebuchholz): have a way to avoid finding and settings attributes/uniforms for each
    // time we draw a mesh
    glBindBuffer(GL_ARRAY_BUFFER, mesh->positionBuffer);
    GLint positionLocation = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->texCoordBuffer);
    GLint texCoordLocation = glGetAttribLocation(program, "texCoord");
    glEnableVertexAttribArray(texCoordLocation);
    glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->normalBuffer);
    GLint normalLocation = glGetAttribLocation(program, "normal");
    glEnableVertexAttribArray(normalLocation);
    glVertexAttribPointer(normalLocation, 3, GL_FLOAT, FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);

    // NOTE(ebuchholz): run_around_math matrices memory-order is
    // x0 y0 z0 
    // x1 y1 z1
    // x2 y2 z2
    // but OpenGL memory-order is
    // x0 x1 x2
    // y0 y1 y2
    // z0 z1 z2
    // so glUniformMatrixNfv calls are transposed, and both in the run_around_game code and 
    // opengl shaders, matrices are multiplied like column major matrices (proj * view * model)
    // TODO(ebuchholz): better to multiply these ahead of time
    GLint modelMatrixLocation = glGetUniformLocation(program, "modelMatrix");
    glUniformMatrix4fv(modelMatrixLocation, 1, true, modelCommand->modelMatrix.m);

    GLint viewMatrixLocation = glGetUniformLocation(program, "viewMatrix");
    glUniformMatrix4fv(viewMatrixLocation, 1, true, renderer->viewMatrix.m);

    GLint projMatrixLocation = glGetUniformLocation(program, "projMatrix");
    glUniformMatrix4fv(projMatrixLocation, 1, true, renderer->projMatrix.m);

    openGL_texture *texture = &renderer->textures[modelCommand->textureKey];

    glActiveTexture(GL_TEXTURE0);
    GLuint textureLocation = glGetUniformLocation(program, "texture");
    glUniform1i(textureLocation, 0);
    glBindTexture(GL_TEXTURE_2D, texture->textureID);

    glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_INT, 0);
}

// TODO(ebuchholz): batch
void drawSprite (openGL_renderer *renderer, GLuint program, render_command_sprite *spriteCommand, 
                 float screenWidth, float screenHeight) 
{
    renderer->spriteVertexPositions[0] = spriteCommand->x;
    renderer->spriteVertexPositions[1] = spriteCommand->y;
    renderer->spriteVertexPositions[2] = spriteCommand->x + spriteCommand->width;
    renderer->spriteVertexPositions[3] = spriteCommand->y;
    renderer->spriteVertexPositions[4] = spriteCommand->x;
    renderer->spriteVertexPositions[5] = spriteCommand->y + spriteCommand->height;
    renderer->spriteVertexPositions[6] = spriteCommand->x + spriteCommand->width;
    renderer->spriteVertexPositions[7] = spriteCommand->y + spriteCommand->height;

    glBindBuffer(GL_ARRAY_BUFFER, renderer->spritePositionBuffer);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), renderer->spriteVertexPositions, GL_DYNAMIC_DRAW);

    GLint positionLocation = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, FALSE, 0, 0);

    renderer->spriteTextureCoords[0] = 0.0f;
    renderer->spriteTextureCoords[1] = 1.0f;
    renderer->spriteTextureCoords[2] = 1.0f;
    renderer->spriteTextureCoords[3] = 1.0f;
    renderer->spriteTextureCoords[4] = 0.0f;
    renderer->spriteTextureCoords[5] = 0.0f;
    renderer->spriteTextureCoords[6] = 1.0f;
    renderer->spriteTextureCoords[7] = 0.0f;

    glBindBuffer(GL_ARRAY_BUFFER, renderer->spriteTextureBuffer);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), renderer->spriteTextureCoords, GL_DYNAMIC_DRAW);

    GLint texCoordLocation = glGetAttribLocation(program, "texCoord");
    glEnableVertexAttribArray(texCoordLocation);
    glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->spriteIndexBuffer);

    openGL_texture *texture = &renderer->textures[spriteCommand->textureKey];

    GLint screenWidthLocation = glGetUniformLocation(program, "screenWidth");
    glUniform1f(screenWidthLocation, screenWidth);
    GLint screenHeightLocation = glGetUniformLocation(program, "screenHeight");
    glUniform1f(screenHeightLocation, screenHeight);

    glActiveTexture(GL_TEXTURE0);
    GLuint textureLocation = glGetUniformLocation(program, "texture");
    glUniform1i(textureLocation, 0);
    glBindTexture(GL_TEXTURE_2D, texture->textureID);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void flushSprites (openGL_renderer *renderer, GLuint program, int numSpritesBatched, 
                   float screenWidth, float screenHeight, int textureKey) 
{
    glBindBuffer(GL_ARRAY_BUFFER, renderer->spritePositionBuffer);
    glBufferData(GL_ARRAY_BUFFER, numSpritesBatched * 8 * sizeof(float), renderer->spriteVertexPositions, GL_DYNAMIC_DRAW);

    GLint positionLocation = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->spriteTextureBuffer);
    glBufferData(GL_ARRAY_BUFFER, numSpritesBatched * 8 * sizeof(float), renderer->spriteTextureCoords, GL_DYNAMIC_DRAW);

    GLint texCoordLocation = glGetAttribLocation(program, "texCoord");
    glEnableVertexAttribArray(texCoordLocation);
    glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->spriteColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, numSpritesBatched * 16 * sizeof(float), renderer->spriteColors, GL_DYNAMIC_DRAW);

    GLint colorLocation = glGetAttribLocation(program, "color");
    glEnableVertexAttribArray(colorLocation);
    glVertexAttribPointer(colorLocation, 4, GL_FLOAT, FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->spriteIndexBuffer);

    openGL_texture *texture = &renderer->textures[textureKey];

    GLint screenWidthLocation = glGetUniformLocation(program, "screenWidth");
    glUniform1f(screenWidthLocation, screenWidth);
    GLint screenHeightLocation = glGetUniformLocation(program, "screenHeight");
    glUniform1f(screenHeightLocation, screenHeight);

    //GLint textureWidthLocation = glGetUniformLocation(program, "textureWidth");
    //glUniform1f(textureWidthLocation, (float)texture->width);
    //GLint textureHeightLocation = glGetUniformLocation(program, "textureHeight");
    //glUniform1f(textureHeightLocation, (float)texture->height);

    glActiveTexture(GL_TEXTURE0);
    GLuint textureLocation = glGetUniformLocation(program, "texture");
    glUniform1i(textureLocation, 0);
    glBindTexture(GL_TEXTURE_2D, texture->textureID);

    glDrawElements(GL_TRIANGLES, numSpritesBatched * 6, GL_UNSIGNED_INT, 0);
}

// TODO(ebuchholz): have game prepare memory directly? and feed it to buffer with offset
void drawSpriteList (openGL_renderer *renderer, GLuint program, render_command_sprite_list *spriteListCommand, 
                 float screenWidth, float screenHeight) 
{
    if (spriteListCommand->numSprites > 0) {
        render_sprite *sprites = spriteListCommand->sprites;
        int currentTextureKey = sprites[0].textureKey;
        int numSpritesBatched = 0;

        for (int i = 0; i < spriteListCommand->numSprites; ++i) {
            render_sprite *sprite = &sprites[i];

            if (sprite->textureKey != currentTextureKey || numSpritesBatched >= MAX_SPRITES_PER_BATCH) {
                flushSprites(renderer, program, numSpritesBatched, screenWidth, screenHeight, currentTextureKey);
                numSpritesBatched = 0;
                currentTextureKey = sprite->textureKey;
            }

            int numFloats = numSpritesBatched * 8;
            int numColors = numSpritesBatched * 16;

            renderer->spriteVertexPositions[numFloats]   = sprite->pos[0].x;
            renderer->spriteVertexPositions[numFloats+1] = sprite->pos[0].y;
            renderer->spriteVertexPositions[numFloats+2] = sprite->pos[1].x;
            renderer->spriteVertexPositions[numFloats+3] = sprite->pos[1].y;
            renderer->spriteVertexPositions[numFloats+4] = sprite->pos[2].x;
            renderer->spriteVertexPositions[numFloats+5] = sprite->pos[2].y;
            renderer->spriteVertexPositions[numFloats+6] = sprite->pos[3].x;
            renderer->spriteVertexPositions[numFloats+7] = sprite->pos[3].y;

            renderer->spriteTextureCoords[numFloats]   = sprite->texCoord[0].x;
            renderer->spriteTextureCoords[numFloats+1] = sprite->texCoord[0].y;
            renderer->spriteTextureCoords[numFloats+2] = sprite->texCoord[1].x;
            renderer->spriteTextureCoords[numFloats+3] = sprite->texCoord[1].y;
            renderer->spriteTextureCoords[numFloats+4] = sprite->texCoord[2].x;
            renderer->spriteTextureCoords[numFloats+5] = sprite->texCoord[2].y;
            renderer->spriteTextureCoords[numFloats+6] = sprite->texCoord[3].x;
            renderer->spriteTextureCoords[numFloats+7] = sprite->texCoord[3].y;

            renderer->spriteColors[numColors]   =  sprite->color[0].r;
            renderer->spriteColors[numColors+1] =  sprite->color[0].g;
            renderer->spriteColors[numColors+2] =  sprite->color[0].b;
            renderer->spriteColors[numColors+3] =  sprite->color[0].a;
            renderer->spriteColors[numColors+4] =  sprite->color[1].r;
            renderer->spriteColors[numColors+5] =  sprite->color[1].g;
            renderer->spriteColors[numColors+6] =  sprite->color[1].b;
            renderer->spriteColors[numColors+7] =  sprite->color[1].a;
            renderer->spriteColors[numColors+8] =  sprite->color[2].r;
            renderer->spriteColors[numColors+9] =  sprite->color[2].g;
            renderer->spriteColors[numColors+10] = sprite->color[2].b;
            renderer->spriteColors[numColors+11] = sprite->color[2].a;
            renderer->spriteColors[numColors+12] = sprite->color[3].r;
            renderer->spriteColors[numColors+13] = sprite->color[3].g;
            renderer->spriteColors[numColors+14] = sprite->color[3].b;
            renderer->spriteColors[numColors+15] = sprite->color[3].a;

            ++numSpritesBatched;
        }
        if (numSpritesBatched > 0) {
            flushSprites(renderer, program, numSpritesBatched, screenWidth, screenHeight, currentTextureKey);
        }
    }
}

void drawLines (openGL_renderer *renderer, GLuint program, render_command_lines *lineCommand) {
    glBindBuffer(GL_ARRAY_BUFFER, renderer->debugPositionBuffer);
    glBufferData(GL_ARRAY_BUFFER, lineCommand->numLines * sizeof(line), lineCommand->lines, GL_DYNAMIC_DRAW);

    GLint positionLocation = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, FALSE, 0, 0);

    GLint viewMatrixLocation = glGetUniformLocation(program, "viewMatrix");
    glUniformMatrix4fv(viewMatrixLocation, 1, true, renderer->viewMatrix.m);

    GLint projMatrixLocation = glGetUniformLocation(program, "projMatrix");
    glUniformMatrix4fv(projMatrixLocation, 1, true, renderer->projMatrix.m);

    glDrawArrays(GL_LINES, 0, lineCommand->numLines * 2);
}

void renderBackgroundVisualization (openGL_renderer *renderer, float screenWidth, float screenHeight, 
                                    render_command_background_visualization *visualizationCommand) 
{
    GLuint program = renderer->shaders[SHADER_TYPE_BACKGROUND_VISUALIZATION].program;
    glUseProgram(program);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->fullScreenQuadBuffer);

    GLint positionLocation = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, FALSE, 0, 0);

    GLint screenWidthLocation = glGetUniformLocation(program, "screenWidth");
    glUniform1f(screenWidthLocation, screenWidth);

    GLint screenHeightLocation = glGetUniformLocation(program, "screenHeight");
    glUniform1f(screenHeightLocation, screenHeight);

    GLint tLocation = glGetUniformLocation(program, "t");
    glUniform1f(tLocation, visualizationCommand->t);

    // triangles
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void renderFrame (renderer_memory *memory, render_command_list *renderCommands) {
    openGL_renderer *renderer = (openGL_renderer *)memory->memory;

    glViewport(0, 0, renderCommands->windowWidth, renderCommands->windowHeight);
    //glEnable(GL_DEPTH_TEST); // off for 2d stuff
    //glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.7f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    unsigned int renderCommandOffset = 0;
    while (renderCommandOffset < renderCommands->memory.size) {
        render_command_header *header = 
            (render_command_header *)((char *)renderCommands->memory.base + 
                                     renderCommandOffset);
        renderCommandOffset += sizeof(render_command_header);
        switch(header->type) {
            default:
                // error
                assert(false);
                break;
            case RENDER_COMMAND_SPRITE: 
            {
                // TODO(ebuchholz): have a way to not have to set the program for command
                //    GLuint program = renderer->shaders[SHADER_TYPE_SPRITE].program;
                //    glUseProgram(program);

                //    render_command_sprite *spriteCommand = 
                //        (render_command_sprite *)((char *)renderCommands->memory.base + 
                //                                renderCommandOffset);
                //    drawSprite(renderer, program, spriteCommand, (float)renderCommands->windowWidth, (float)renderCommands->windowHeight);
                renderCommandOffset += sizeof(render_command_sprite);
            } break;
            case RENDER_COMMAND_SPRITE_LIST: 
            {
                // TODO(ebuchholz): have a way to not have to set the program for command
                GLuint program = renderer->shaders[SHADER_TYPE_SPRITE].program;
                glUseProgram(program);

                render_command_sprite_list *spriteListCommand = 
                    (render_command_sprite_list *)((char *)renderCommands->memory.base + renderCommandOffset);
                drawSpriteList(renderer, program, spriteListCommand, (float)renderCommands->windowWidth, (float)renderCommands->windowHeight);
                renderCommandOffset += sizeof(render_command_sprite_list);
                renderCommandOffset += spriteListCommand->numSprites * sizeof(render_sprite);
            } break;
            case RENDER_COMMAND_MODEL: 
            {
                // TODO(ebuchholz): have a way to not have to set the program for command
                GLuint program = renderer->shaders[SHADER_TYPE_DEFAULT].program;
                glUseProgram(program);

                render_command_model *modelCommand = 
                    (render_command_model *)((char *)renderCommands->memory.base + 
                                            renderCommandOffset);
                drawModel(renderer, program, modelCommand);
                renderCommandOffset += sizeof(render_command_model);
            } break;
            case RENDER_COMMAND_LINES: 
            {
                GLuint program = renderer->shaders[SHADER_TYPE_LINES].program;
                glUseProgram(program);

                render_command_lines *lineCommand = 
                    (render_command_lines *)((char *)renderCommands->memory.base + 
                                            renderCommandOffset);
                drawLines(renderer, program, lineCommand);
                renderCommandOffset += sizeof(render_command_lines);
                renderCommandOffset += sizeof(line) * lineCommand->numLines; // also account for the line data
            } break;
            case RENDER_COMMAND_SET_CAMERA: 
            {
                render_command_set_camera *setCameraCommand = 
                    (render_command_set_camera *)((char *)renderCommands->memory.base + 
                                                  renderCommandOffset);
                setCamera(renderer, setCameraCommand);
                renderCommandOffset += sizeof(render_command_set_camera);
            } break;
            case RENDER_COMMAND_BACKGROUND_VISUALIZATION:
            {
                render_command_background_visualization *visualizationCommand = 
                    (render_command_background_visualization *)((char *)renderCommands->memory.base + 
                                                                renderCommandOffset);
                renderBackgroundVisualization(renderer, (float)renderCommands->windowWidth, (float)renderCommands->windowHeight, visualizationCommand);
                renderCommandOffset += sizeof(render_command_background_visualization);
            } break;
        }
    }

    SwapBuffers(renderer->deviceContext);
    //glFinish();
}
