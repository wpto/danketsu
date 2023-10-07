#ifndef RENDERER_H
#define RENDERER_H

#include "texture.h"

#define MAX_TRIANGLES 2048
#define MAX_VERTICES MAX_TRIANGLES * 3

typedef struct RenderVertex {
  f32 pos[2];
  f32 color[4];
  f32 uv[2];
  f32 texIndex;
} RenderVertex;

typedef struct Renderer {
  // The required OpenGL objects
  u32 vao;
  u32 vbo;
  u32 shader;

  f32 _pj[16];
  m4 projection;

  // Tightly packed triangle data. This is a cpu side mirror of the buffer
  RenderVertex triangleBuffer[MAX_VERTICES];
  u32 triangleLen;

  // Texture stuff
  u32 texture[8];
  u32 textureLen;
} Renderer;

void RenderInit(Renderer *r);
void printShaderLog(u32 shader);
void printProgramLog(u32 program);
void RenderFree(Renderer *r);
void RenderBeginFrame(Renderer *r);
void RenderEndFrame(Renderer *r);

void RenderPushTriangle(Renderer *r, v2 a, v2 b, v2 c, v4 aColor, v4 bColor,
                        v4 cColor, v2 aUV, v2 bUV, v2 cUV, u32 texture);

#include "renderer.cpp"
#endif