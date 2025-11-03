#define INVALID_SHADER_UNIFORM (-1)
#define INVALID_SHADER (u32)(-1)


poof(
  func shader_magic(shader_struct, type_poof_symbol async_tag)
  {
    link_internal b32
    poof((async_tag))
    Initialize(shader_struct.name.to_capital_case)
    (
      shader_struct.name *Element
      shader_struct.map(member)
      {
          member.has_tag(uniform)?  {, member.type member.is_pointer?{*}member.name}
      }
    )
    {
      shader_struct.has_tag(vert_source_file)?
      {
        shader_struct.has_tag(frag_source_file)?
        {
          b32 Result = CompileShaderPair(&Element->Program, CSz((shader_struct.tag_value(vert_source_file))), CSz((shader_struct.tag_value(frag_source_file))));

          if (Result)
          {
            Element->Program.Uniforms = ShaderUniformBuffer(Element->Uniforms, ArrayCount(Element->Uniforms));

            u32 UniformIndex = 0;

            shader_struct.map(member)
            {
              member.tags(tag)
              {
                tag.is_named(uniform)?
                {
                  Element->member.name = member.name;
                  InitShaderUniform(
                      &Element->Program,
                       UniformIndex++,

                        /// Insert the value of the uniform tag, otherwise use
                        /// a default
                        tag.value?
                        {
                          tag.value(1),
                          tag.value(0),
                          tag.value(2)
                        }
                        {
                          member.is_pointer?{}{&}Element->member.name,
                          "member.name"
                           member.has_tag(array_length)? {, Cast(u16, member.tag_value(array_length))}
                        }
                      );

                }
              }
            }

            u32 Expected = shader_struct.member(1, (Uniforms) { Uniforms.array });
            if (UniformIndex != Expected )
            {
              Error("Shader ((shader_struct.name)) had an incorrect number of uniform slots! Expected (%d), Got (%d)", Expected, UniformIndex);
            }
          }
        }
        {
          poof_error { Poof func shader_magic requires tag @frag_source_file }
        }
      }
      {
        poof_error { Poof func shader_magic requires tag @vert_source_file }
      }

      AssertNoGlErrors;
      return Result;
    }

    link_internal void
    UseRenderPass_(shader_struct.name)
    ( shader_struct.name *Element )
    {
      TIMED_FUNCTION();
      if (Element->Program.ID != INVALID_SHADER)
      {
        {
          TIMED_NAMED_BLOCK(UseProgram);
          GetGL()->UseProgram(Element->Program.ID);
        }

        {
          TIMED_NAMED_BLOCK(BindUniforms);
          s32 TextureUnit = 0;
          s32 UniformIndex = 0;
          shader_struct.map(member)
          {
            member.tags(tag)
            {
              tag.is_named(uniform)?
              {
                  {
                    shader_uniform *Uniform = Element->Uniforms+UniformIndex;
                    BindUniformById(Uniform, &TextureUnit);
                    ++UniformIndex;
                    AssertNoGlErrors;
                  }
              }
            }
          }

          if (UniformIndex != shader_struct.member(1, (Uniforms) { Uniforms.array }) )
          {
            Error("Shader ((shader_struct.name)) had an incorrect number of uniform slots!");
          }
        }
      }
      else
      {
        SoftError("Attempted to bind uncompiled Shader ((shader_struct.tag_value(vert_source_file))) | ((shader_struct.tag_value(frag_source_file)))");
      }

      AssertNoGlErrors;
    }

    // NOTE(Jesse): This is for binding when passing a custom RP through the UI 
    link_internal void
    UseRenderPass_(shader_struct.name)( void *Element )
    {
      UseRenderPass_(shader_struct.name)( Cast((shader_struct.name) *, Element) );
    }

    link_internal void
    UseRenderPass( shader_struct.name *Element )
    {
      UseRenderPass_(shader_struct.name)(Element);
    }

    // TODO(Jesse): Remove in favor of UseRenderPass
    link_internal void
    UseShader( shader_struct.name *Element )
    {
      UseRenderPass_(shader_struct.name)(Element);
    }
  }
)


// NOTE(Jesse): These are the basic types you could imagine passing to a shader
// There are extended types in the engine 
enum shader_uniform_type
#ifndef POOF_PREPROCESSOR
: u16
#endif
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
struct shader_uniform
{
  shader_uniform_type Type;
  u32 *Count; // Optional.  If set to null, Count is assumed to be 1

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

  s32 ID = INVALID_SHADER_UNIFORM;
  const char *Name; poof(@ui_construct_as(CS))
};

poof(buffer_h(shader_uniform, u32))
#include <generated/buffer_h_struct_u32.h>

struct shader
{
  u32 ID = INVALID_SHADER;
  shader_uniform_buffer Uniforms;

  cs VertexSourceFilename;
  cs FragSourceFilename;

  s64 VertexTimeModifiedWhenLoaded;
  s64 FragmentTimeModifiedWhenLoaded;

  b32 HotReloaded;
};

typedef shader* shader_ptr;

poof(block_array_h(shader_ptr, {64}, {}))
#include <generated/block_array_h_shader_ptr_688853972_0.h>

enum shader_language_setting
{
  ShaderLanguageSetting_330core, // default
  ShaderLanguageSetting_310es,   // web
};

