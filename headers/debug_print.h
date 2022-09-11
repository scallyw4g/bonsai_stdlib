bonsai_function void
DebugPrint(r64 E, u32 Depth = 0)
{
  DebugChars("%*s%f", Depth, "", E);
}

bonsai_function void
DebugPrint(s64 E, u32 Depth = 0)
{
  DebugChars("%*s%ld", Depth, "", E);
}

bonsai_function void
DebugPrint(u64 E, u32 Depth = 0)
{
  DebugChars("%*s%lu", Depth, "", E);
}

bonsai_function void
DebugPrint(r32 E, u32 Depth = 0)
{
  DebugChars("%*s%.2f", Depth, "", E);
}

bonsai_function void
DebugPrint(s32 E, u32 Depth = 0)
{
  DebugChars("%*s%d", Depth, "", E);
}

bonsai_function void
DebugPrint(u32 E, u32 Depth = 0)
{
  DebugChars("%*s%u", Depth, "", E);
}

bonsai_function void
DebugPrint(volatile void* E, u32 Depth = 0)
{
  DebugChars("%*s%p", Depth, "", E);
}

bonsai_function void
DebugPrint(void* E, u32 Depth = 0)
{
  DebugChars("%*s%p", Depth, "", E);
}

bonsai_function void
DebugPrint(__m128 E, u32 Depth = 0)
{
  DebugChars("%*s%f %f %f %f", Depth, "", E[0], E[1], E[2], E[3]);
}

bonsai_function void
DebugPrint(counted_string E, u32 Depth = 0)
{
  if (StringsMatch(E, CSz("\n")))
  {
    DebugChars("%*s'\\n'", Depth, "", E);
  }
  else
  {
    DebugChars("%*s'%S'", Depth, "", E);
  }
}

bonsai_function void
DebugPrint(const char *E, u32 Depth = 0)
{
  DebugChars("%*s%s", Depth, "", E);
}

// NOTE(Jesse): For the purposes of DebugPrint, counted_string needs to be implemented by hand
poof( named_list(project_primitives) { counted_string })

// TODO(Jesse id: 185, tags: bug): These contain function
// definitions and we currently don't have a way of handling functions when
// iterating over stuff.  Should be pretty easy to add one of these days
poof( named_list(buggy_datatypes) { opengl debug_timed_function debug_state })

// NOTE(Jesse): d_unions (for the moment) have a different implementation of DebugPrint
poof( named_list(d_unions) { ast_node datatype declaration} )

poof(dunion_debug_print_prototype(ast_node))
#include <debug_print/debug_print_prototype_ast_node.h>

poof(dunion_debug_print_prototype(datatype))
#include <debug_print/debug_print_prototype_datatype.h>

poof(dunion_debug_print_prototype(declaration))
#include <debug_print/debug_print_prototype_declaration.h>

poof(
  for_datatypes(all).exclude(project_primitives buggy_datatypes d_unions)

    func (StructDef)
    {
      bonsai_function void DebugPrint( StructDef.name *S, u32 Depth = 0);
      bonsai_function void DebugPrint( StructDef.name  S, u32 Depth = 0);
    }

    func (EnumDef)
    {
      bonsai_function void DebugPrint( EnumDef.name EnumValue, u32 Depth = 0);
    }
)
#include <debug_print/for_all_datatypes_debug_print_prototypes.h>

poof(
  for_datatypes(all)
    .exclude(project_primitives buggy_datatypes d_unions)

  func (StructDef)
  {
    bonsai_function void DebugPrint( StructDef.name RuntimeStruct, u32 Depth)
    {
      if (Depth == 0)
      {
        DebugPrint("StructDef.name {\n", Depth);
      }

      StructDef.map_members (Member)
      {
        Member.is_defined?
        {
          Member.name?
          {
            Member.is_compound?
            {
              DebugPrint("Member.type Member.name {\n", Depth+2);
              DebugPrint(RuntimeStruct.(Member.name), Depth+4);
              DebugPrint("}\n", Depth+2);
            }
            {
              DebugPrint("Member.type Member.name =", Depth+2);
              DebugPrint(RuntimeStruct.(Member.name), 1);
              DebugPrint(";\n");
            }
          }
          {
            // NOTE(Jesse): an anonymous struct or union
            DebugPrint("Member.type Member.name\n", Depth+2);
          }
        }
        {
          // NOTE(Jesse): found no definition for this type.. probably from stdlib
          DebugPrint("undefined((Member.type) (Member.name))\n", Depth+2);
        }
      }

      if (Depth == 0)
      {
        DebugPrint("}\n", Depth);
      }
    }

    bonsai_function void DebugPrint( StructDef.name *RuntimePtr, u32 Depth)
    {
      if (RuntimePtr) { DebugPrint(*RuntimePtr, Depth); }
      else { DebugPrint("ptr(0)\n", Depth); }
    }
  }

  func (TEnum)
  {
    bonsai_function void DebugPrint( TEnum.name RuntimeValue, u32 Depth)
    {
      switch (RuntimeValue)
      {
        TEnum.map_values (TValue)
        {
          case TValue.name:
          {
            DebugPrint("TValue.name", Depth);
          } break;
        }
      }
    }
  }

)
#include <debug_print/for_all_datatypes_debug_print_functions.h>




poof(dunion_debug_print(ast_node))
#include <debug_print/debug_print_ast_node.h>

poof(dunion_debug_print(datatype))
#include <debug_print/debug_print_datatype.h>

poof(dunion_debug_print(declaration))
#include <debug_print/debug_print_declaration.h>

