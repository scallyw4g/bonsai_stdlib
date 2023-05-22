#define IterateOver(Container, ElementPtrName, IndexVariableName) \
  auto *ElementPtrName = GetPtr(Container, 0); \
  for (umm IndexVariableName = 0; IndexVariableName < AtElements(Container); ++IndexVariableName, ElementPtrName = GetPtr((Container), (umm)IndexVariableName))

/* #define IterateOverTotal(Container, ElementPtrName, IndexVariableName) \ */
/*   auto *ElementPtrName = GetPtr(Container, 0); \ */
/*   for (umm IndexVariableName = 0; IndexVariableName < TotalElements(Container); ++IndexVariableName, ElementPtrName = GetPtr((Container), (umm)IndexVariableName)) */

#define RangeIterator( ArgName, RangeMax) \
  for (s32 ArgName = 0; ArgName < RangeMax; ++ArgName)

#define CursorIterator( ArgName, Cursor ) \
  for (umm ArgName = 0; ArgName < CurrentCount(Cursor); ++ArgName)

#define BufferIterator( Stack, ArgName ) CursorIterator(ArgName, Stack)

#define StackIterator( ArgName, Stack ) CursorIterator(ArgName, Stack)

#define DimIterator( xArg, yArg, zArg, Dim) \
  for (s32 zArg = 0; zArg < Dim.z; ++zArg)  \
  for (s32 yArg = 0; yArg < Dim.y; ++yArg)  \
  for (s32 xArg = 0; xArg < Dim.x; ++xArg)

#define MinDimIterator( xArg, yArg, zArg, Min, Dim)  \
  for (s32 zArg = Min.z; zArg < Min.z+Dim.z; ++zArg) \
  for (s32 yArg = Min.y; yArg < Min.y+Dim.y; ++yArg) \
  for (s32 xArg = Min.x; xArg < Min.x+Dim.x; ++xArg)

#define MinMaxIterator( xArg, yArg, zArg, Min, Max)  \
  for (s32 zArg = Min.z; zArg < Max.z; ++zArg)       \
  for (s32 yArg = Min.y; yArg < Max.y; ++yArg)       \
  for (s32 xArg = Min.x; xArg < Max.x; ++xArg)




#define TryCast(T, Ptr) ((Ptr)->Type == type_##T ? (&(Ptr)->T) : 0)
#define SafeCast(T, Ptr) (T*)(&(Ptr)->T); Assert((Ptr)->Type == type_##T)

#define SafeAccess(T, Ptr) SafeCast(T, Ptr)
#define SafeAccessPtr(T, Ptr) ((Ptr)->T); Assert((Ptr)->Type == type_##T)

#define ITERATE_OVER_AS(prefix, value_ptr)        \
  for (auto prefix##Iter = Iterator((value_ptr)); \
      IsValid(&prefix##Iter);                     \
      Advance(&prefix##Iter))

#define ITERATE_OVER(value_ptr)           \
  for (auto Iter = Iterator((value_ptr)); \
      IsValid(&Iter);                     \
      Advance(&Iter))

#define GET_ELEMENT(I) (&(I).At->Element)

#define SetBitfield(type, Dest, Value) do { \
  Assert( ((Dest)&(Value)) == 0 );          \
  (Dest) = (type)((Dest) | (Value));        \
} while (false)

#define UnsetBitfield(type, Dest, Value) do { \
  Assert( (Dest) & (Value) );                 \
  (Dest) = (type)((Dest) & ~(Value));         \
} while (false)

#define ToggleBitfieldValue(Dest, Value) \
      (Dest) = (Dest) & (Value) ?  ((u32)(Dest) & ~(u32)(Value)) : ((u32)(Dest) | (u32)(Value))


