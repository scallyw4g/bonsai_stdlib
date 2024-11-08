
struct sort_key_string
{
  u64 Index;
  cs Value;

  b32 operator<(sort_key_string &Cmp) { return (CompareStrings(&this->Value, &Cmp.Value) < 0); }
  b32 operator>(sort_key_string &Cmp) { return (CompareStrings(&this->Value, &Cmp.Value) > 0); }
};

struct sort_key_f64
{
  u64 Index;
  r64 Value;

  b32 operator<(sort_key_f64 &Cmp) { return this->Value < Cmp.Value; }
  b32 operator>(sort_key_f64 &Cmp) { return this->Value > Cmp.Value; }
};

struct sort_key_f32
{
  u64 Index;
  f32 Value;

  b32 operator<(sort_key_f32 &Cmp) { return this->Value < Cmp.Value; }
  b32 operator>(sort_key_f32 &Cmp) { return this->Value > Cmp.Value; }
};

struct sort_key
{
  u64 Index;
  u64 Value;

  b32 operator<(sort_key &Cmp) { return this->Value < Cmp.Value; }
  b32 operator>(sort_key &Cmp) { return this->Value > Cmp.Value; }
};

poof(
  func bubble_sort(type_poof_symbol sort_key_type_list)
  {
    sort_key_type_list.map(sort_key_t)
    {
      link_internal void
      BubbleSort((sort_key_t.name) *Keys, u32 Count)
      {
        for (u32 Ignored = 0;
            Ignored < Count;
            ++Ignored)
        {
          b32 Sorted = True;

          for (u32 Inner = 0;
              Inner < (Count-1);
              ++Inner)
          {
            auto *KeyA = Keys+Inner;
            auto *KeyB = Keys+Inner+1;

            if (*KeyA < *KeyB)
            {
              auto Temp = *KeyA;
              *KeyA = *KeyB;
              *KeyB = Temp;
              Sorted = False;
            }
          }

          if (Sorted) break;
        }

        return;
      }

      link_internal void
      BubbleSort_descending((sort_key_t.name) *Keys, u32 Count)
      {
        for (u32 Ignored = 0;
            Ignored < Count;
            ++Ignored)
        {
          b32 Sorted = True;

          for (u32 Inner = 0;
              Inner < (Count-1);
              ++Inner)
          {
            auto *KeyA = Keys+Inner;
            auto *KeyB = Keys+Inner+1;

            if (*KeyA > *KeyB)
            {
              auto Temp = *KeyA;
              *KeyA = *KeyB;
              *KeyB = Temp;
              Sorted = False;
            }
          }

          if (Sorted) break;
        }

        return;
      }
    }
  }
)

poof(bubble_sort({sort_key sort_key_f64 sort_key_f32 sort_key_string}))
#include <generated/bubble_sort_686708022.h>

link_internal void
LexicograpicSort(sort_key_string *SortKeys, u32 SortKeyCount)
{
  BubbleSort(SortKeys, SortKeyCount);
}

