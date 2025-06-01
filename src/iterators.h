#define IterateOver(Container, ElementPtrName, IndexVariableName)      \
  auto IndexVariableName = ZerothIndex(Container);                     \
  auto IndexVariableName##_end = AtElements(Container);                \
  for (auto *ElementPtrName = TryGetPtr(Container, IndexVariableName); \
          IndexVariableName < IndexVariableName##_end;                 \
             ElementPtrName = TryGetPtr(Container, ++IndexVariableName))

#define IterateOverReverse(Container, ElementPtrName, IndexVariableName) \
  auto *ElementPtrName = GetPtr(Container, 0); \
  for (s32 IndexVariableName = AtElements(Container)-1; IndexVariableName >= 0; --IndexVariableName, ElementPtrName = GetPtr((Container), umm(IndexVariableName)))

#define IterateOverBackwards(Container, ElementPtrName, IndexVariableName) IterateOverReverse(Container, ElementPtrName, IndexVariableName)


#define LinkedListIter( List, Element ) \
  for (auto Element = (List); Element; Element = Element->Next)

#define RangeIterator_t(type, ArgName, RangeMax ) \
  for (type ArgName = 0; ArgName < RangeMax; ++ArgName)



#define RangeIterator( ArgName, RangeMax ) \
  for (s32 ArgName = 0; ArgName < RangeMax; ++ArgName)

// NOTE(Jesse): Must be signed because there are too many edge cases to detect efficiently with unsigned
#define RangeIteratorReverse(ArgName, RangeMax ) \
  for (s32 ArgName = ((RangeMax)-1); ArgName > -1; --ArgName)



#define RangeIteratorRange_t(t, ArgName, RangeMax, RangeMin ) \
  for (t ArgName = RangeMin; ArgName < RangeMax; ++ArgName)

#define RangeIteratorRange( ArgName, RangeMax, RangeMin ) RangeIteratorRange_t(s32, ArgName, RangeMax, RangeMin )

#define RangeIteratorReverseRange( ArgName, RangeMax, RangeMin ) \
  for (s32 ArgName = ((RangeMax)-1); ArgName >= RangeMin; --ArgName)



#define CursorIterator( ArgName, Cursor ) \
  for (umm ArgName = 0; ArgName < CurrentCount(Cursor); ++ArgName)

#define BufferIterator( Stack, ArgName ) CursorIterator(ArgName, Stack)

#define StackIterator( ArgName, Stack ) CursorIterator(ArgName, Stack)

#define v2iIterator( xArg, yArg, Dim)      \
  for (s32 yArg = 0; yArg < Dim.y; ++yArg) \
  for (s32 xArg = 0; xArg < Dim.x; ++xArg)

#define DimIterator( xArg, yArg, zArg, Dim) \
  for (s32 zArg = 0; zArg < Dim.z; ++zArg)  \
  for (s32 yArg = 0; yArg < Dim.y; ++yArg)  \
  for (s32 xArg = 0; xArg < Dim.x; ++xArg)

#define MinDimIterator( xArg, yArg, zArg, Min, Dim)        \
  for (s32 zArg = (Min).z; zArg < (Min).z+(Dim).z; ++zArg) \
  for (s32 yArg = (Min).y; yArg < (Min).y+(Dim).y; ++yArg) \
  for (s32 xArg = (Min).x; xArg < (Min).x+(Dim).x; ++xArg)

#define MinMaxIterator( xArg, yArg, zArg, Min, Max)  \
  for (s32 zArg = (Min).z; zArg < (Max).z; ++zArg)   \
  for (s32 yArg = (Min).y; yArg < (Max).y; ++yArg)   \
  for (s32 xArg = (Min).x; xArg < (Max).x; ++xArg)

#define Rect3Iterator(xArg, yArg, zArg, Rect) \
  MinMaxIterator(xArg, yArg, zArg, (Rect).Min, (Rect).Max)



#define Cast(T, Ptr) ((T)(Ptr))
/* #define ReinterpretCast(T, Ptr) (T)(void*)(Ptr) */
#define ReinterpretCast(T, Object) (*(T*)(&(Object)))

#define TryCast(T, Ptr) ((Ptr)->Type == type_##T ? (&(Ptr)->T) : 0)
#define SafeCast(T, Ptr) (T*)(&(Ptr)->T); Assert((Ptr)->Type == type_##T)

#define SafeAccess(T, Ptr) SafeCast(T, (Ptr))
#define SafeAccessPtr(T, Ptr) ((Ptr)->T); Assert((Ptr)->Type == type_##T)

#define tswitch(Ptr)  switch ((Ptr)->Type)
#define tmatch(type_name, match_name, var_name) case type_##type_name: type_name *var_name = (type_name*)(&(match_name)->type_name); Assert((match_name)->Type == type_##type_name);


#define ITERATE_OVER_AS(prefix, value_ptr)         \
  for (auto prefix##Iter = Iterator((value_ptr));  \
                           IsValid(&prefix##Iter); \
                           Advance(&prefix##Iter))

#define ITERATE_OVER(value_ptr)           \
  for (auto Iter = Iterator((value_ptr)); \
                   IsValid(&Iter);        \
                   Advance(&Iter))

#define GET_ELEMENT(I) (&(I).At->Element)

// TODO(Jesse): Do we care about this being T/F, or just truthy/falsy
#define BitfieldIsSet(Src, Value) ((Src)&(Value))

#define SetBitfield(type, Dest, Value) do { \
  Assert( ((Dest)&(Value)) == 0 );          \
  (Dest) = (type)((Dest) | (Value));        \
} while (false)

#define UnsetBitfield(type, Dest, Value) do { \
  Assert( (Dest) & (Value) );                 \
  (Dest) = (type)((Dest) & ~(Value));         \
} while (false)

// TODO(Jesse): Should we make this branchless?
#define ToggleBitfieldValue(Dest, Value) \
      (Dest) = (Dest) & (Value) ?  ((u32)(Dest) & ~(u32)(Value)) : ((u32)(Dest) | (u32)(Value))


