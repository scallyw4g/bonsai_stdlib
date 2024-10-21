
poof(
  func shader_magic(shader_struct)
  {
    link_internal void
    Initialize(shader_struct.name.to_capital_case)( shader_struct.name *Struct shader_struct.map(member) { member.has_tag(uniform)?  {  , member.type member.is_pointer?{*}member.name } })
    {
      shader_struct.has_tag(vert_source_file)?
      {
        shader_struct.has_tag(frag_source_file)?
        {
          Struct->Program = LoadShaders(CSz((shader_struct.tag_value(vert_source_file))), CSz((shader_struct.tag_value(frag_source_file))));

          u32 UniformIndex = 0;

          shader_struct.map(member)
          {
            member.has_tag(uniform)?
            {
              Struct->member.name = member.name;
              Struct->Uniforms[UniformIndex] = ShaderUniform(&Struct->Program, member.is_pointer?{}{&}Struct->member.name, "member.name");
              ++UniformIndex;
            }
          }

          if (UniformIndex != shader_struct.member(1, (Uniforms) { Uniforms.array }) )
          {
            Error("Shader ((shader_struct.name)) had an incorrect number of uniform slots!");
          }
        }
        {
          poof_error { Poof func shader_magic requires tag @frag_source_file }
        }
      }
      {
        poof_error { Poof func shader_magic requires tag @vert_source_file }
      }

      RegisterShaderForHotReload(GetStdlib(), &Struct->Program);
    }

    link_internal void
    UseShader( shader_struct.name *Struct )
    {
      GL.UseProgram(Struct->Program.ID);

      s32 TextureUnit = 0;
      s32 UniformIndex = 0;
      shader_struct.map(member)
      {
        member.has_tag(uniform)?
        {
          BindUnifromById(Struct->Uniforms+UniformIndex, &TextureUnit);
          ++UniformIndex;
        }
      }

      if (UniformIndex != shader_struct.member(1, (Uniforms) { Uniforms.array }) )
      {
        Error("Shader ((shader_struct.name)) had an incorrect number of uniform slots!");
      }
    }
  }
)


// NOTE(Jesse): These are the basic types you could imagine passing to a shader
// There are extended types in the engine 
enum shader_uniform_type
{
  ShaderUniform_Undefined,
  ShaderUniform_M4,
  ShaderUniform_V2,
  ShaderUniform_V3,
  ShaderUniform_S32,
  ShaderUniform_U32,
  ShaderUniform_R32,
  ShaderUniform_Texture,

  // NOTE(Jesse): These are the start of the engine uniforms.  Using these
  // outside of an engine context results in an assertion.
  ShaderUniform_Light,
  ShaderUniform_Camera,
};


struct light;
struct camera;

// TODO(Jesse, id: 83, tags: metaprogramming, immediate): d_union-ify this
// TODO(Jesse): Convert all shaders to use poof func shader_magic and remove
// the next pointer.
struct shader_uniform
{
  shader_uniform_type Type;
  union {
    texture *Texture;
    m4 *M4;
    v2 *V2;
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

  cs VertexSourceFilename;
  cs FragSourceFilename;

  s64 VertexTimeModifiedWhenLoaded;
  s64 FragmentTimeModifiedWhenLoaded;
};

typedef shader* shader_ptr;

poof(block_array_h(shader_ptr, {64}, {}))
#include <generated/block_array_h_shader_ptr_688853971_0.h>

enum shader_language_setting
{
  ShaderLanguageSetting_330core, // default
  ShaderLanguageSetting_310es,   // web
};

