
// NOTE(Jesse): These are the basic types you could imagine passing to a shader
// There are extended types in the engine 
enum shader_uniform_type
{
  ShaderUniform_Undefined,
  ShaderUniform_M4,
  ShaderUniform_V3,
  ShaderUniform_S32,
  ShaderUniform_U32,
  ShaderUniform_R32,
  ShaderUniform_Texture,

  // NOTE(Jesse): These are the start of the engine uniforms.  Using these
  // outside of an engine context results in an assertion.
  EngineShaderUniform_Light,
  EngineShaderUniform_Camera,
};


struct light;
struct camera;

// TODO(Jesse, id: 83, tags: metaprogramming, immediate): d_union-ify this
struct shader_uniform
{
  shader_uniform_type Type;
  union {
    texture *Texture;
    m4 *M4;
    v3 *V3;
    s32 *S32;
    u32 *U32;
    r32 *R32;
    camera *Camera;
    light *Light;
    void *Data;
  };

  s32 ID;
  const char *Name;
  shader_uniform *Next;
};

struct shader
{
  u32 ID;
  shader_uniform *FirstUniform;
};
