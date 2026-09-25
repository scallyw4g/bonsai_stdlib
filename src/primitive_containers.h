u32
ChrisWellonsIntegerHash_lowbias32(u32 x)
{
  u32 Result = x;
  Result = Result ^ (Result >> 16);
  Result = Result ^ 0x7feb352d;
  Result = Result ^ (Result >> 15);
  Result = Result * 0x846ca68b;
  Result = Result ^ (Result >> 16);
  return Result;
}


poof(block_array_h(u32, {8}, {}))
#include <generated/block_array_h$u32.688856411.0$kfpVd8BG.h>


struct u32_hashtable_wrapper
{
  u32 N;
};

link_inline u32 Hash(u32 *E) { return ChrisWellonsIntegerHash_lowbias32(*E); }
link_inline u32 Hash(u64 *E) { NotImplemented; return 0; }

link_inline u32
Hash(u32_hashtable_wrapper *E) { return Hash(&E->N); }

poof(maybe(u32))
#include <generated/maybe$u32$n9UUsJqF.h>
/* poof(are_equal(u32)) */
/* #include <generated/are_equal_u32_hashtable_wrapper.h> */

poof(hashtable_struct(u32))
#include <generated/hashtable_struct$u32$ObifyhsQ.h>





poof(block_array_h(base_ptr_relative_edit, {8}, {u64 BasePtr;}))
#include <generated/block_array_h$base_ptr_relative_edit.688856411.594525846$xhvgf1kt.h>


poof(
  func change_records_for_int_types(type_poof_symbol Types)
  {
    Types.map(t)
    {
      link_internal void
      MaybePushChangeRecord( base_ptr_relative_edit_block_array *ChangeRecords, t.name StartingValue, t.name *ValuePtr )
      {
        MaybePushChangeRecord(ChangeRecords, PrimitiveType_(t.name), Cast(u64, StartingValue), Cast(u64, ValuePtr));
      }

      link_internal void
      MaybePushChangeRecord( base_ptr_relative_edit_block_array *ChangeRecords, t.name *ValuePtr )
      {
        MaybePushChangeRecord(ChangeRecords, *ValuePtr, ValuePtr);
      }
    }
  }
)

poof(
  func change_records_for_float_types(type_poof_symbol Types)
  {
    Types.map(t)
    {
      link_internal void
      MaybePushChangeRecord( base_ptr_relative_edit_block_array *ChangeRecords, t.name StartingValue, t.name *ValuePtr )
      {
        r64 Tmp = Cast(r64, StartingValue);
        MaybePushChangeRecord(ChangeRecords, PrimitiveType_(t.name), ReinterpretCast(u64, Tmp), Cast(u64, ValuePtr));
      }

      link_internal void
      MaybePushChangeRecord( base_ptr_relative_edit_block_array *ChangeRecords, t.name *ValuePtr )
      {
        MaybePushChangeRecord(ChangeRecords, *ValuePtr, ValuePtr);
      }
    }
  }
)

link_internal void
MaybePushChangeRecord( base_ptr_relative_edit_block_array *ChangeRecords, primitive_type Datatype, u64 PrevValue, u64 ValuePtr );

poof(change_records_for_int_types({s64 u64 s32 u32 s16 u16 s8 u8 b8}))
#include <generated/change_records_for_int_types$986235458$fuqprd4a.h>

poof(change_records_for_float_types({r64 r32}))
#include <generated/change_records_for_float_types$638347018$SsBMhVcX.h>

