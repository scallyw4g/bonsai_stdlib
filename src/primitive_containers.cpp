
poof(block_array_c(u32, {8}))
#include <generated/block_array_c_u32_688856411.h>

poof(hashtable_impl(u32));
#include <generated/hashtable_impl_u32.h>

poof(hashtable_get(u32, {u32}, {Value}));
#include <generated/hashtable_get_u32_hashtable_wrapper_688719149_688856424.h>


poof(block_array_c(primitive_value_changed_record, {8}))
#include <generated/block_array_c_Xc85dWkT.h>

link_internal void
MaybePushChangeRecord(
    primitive_value_changed_record_block_array *ChangeRecords,
                               primitive_type  Datatype,
                                          u64  PrevValue,
                                         void *NextValue )
{
  if (ChangeRecords)
  {
    primitive_value_changed_record Record = {};
    Record.Datatype  = SafeTruncateU8(Datatype);
    Record.PrevValue = PrevValue;
    Record.NextValue  = NextValue;
    Push(ChangeRecords, &Record);
  }
}
