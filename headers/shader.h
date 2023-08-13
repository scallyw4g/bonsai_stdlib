
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

