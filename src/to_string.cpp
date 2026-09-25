
poof(
  func to_string_primitive(type_poof_symbol type_list)
  {
    type_list.map(type)
    {
      link_internal cs
      ToString((type.name) *Element)
      {
        cs Result = CS(*Element);
        return Result;
      }
    }
  }
)

poof(
  func to_string_vector(type_poof_symbol type_list)
  {
    type_list.map(type)
    {
      link_internal cs
      ToString((type.name) *Element)
      {
        type.member(0, (E)
        {
          cs Result = FSz("E.map_array(value) {%S }", E.map_array(index).sep(,) { ToString(&Element->E.name[index]) });
          return Result;
        })
      }
    }
  }
)

poof(
  func to_string(type)
  {
    link_internal cs
    ToString((type.name) *Element)
    {
      cs Result = FSz("type.map(m) {(m.name)(%S) }", type.map(m).sep(,)
        {
          ToString( m.is_pointer?{}{ m.is_enum?{}{&} }Element->m.name)
        });
      return Result;
    }
  }
)

link_internal cs
ToString(cs *Element)
{
  return *Element;
}

poof(to_string_primitive({s8 u8 s16 u16 s32 u32 s64 u64 f32 f64}))
#include <generated/to_string_primitive$103038030$n4NiNpYh.h>

poof(to_string_vector({v2 v2i}))
#include <generated/to_string_vector$240271411$rJYvB8Q9.h>

poof(string_and_value_tables(texture_storage_format))
#include <generated/string_and_value_tables$texture_storage_format$TaJkLisr.h>

poof(to_string(texture))
#include <generated/to_string$texture$QackrOQm.h>
