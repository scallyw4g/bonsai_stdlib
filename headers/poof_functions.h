// TODO(Jesse): This would be better if we could specifically target the anonymous union
// instead of just doing it for every union we find in the type. Probably it'll never
// be a problem, but probably it will be eventually ..
poof(
  func d_union_constructors(DUnionType)
  {
    DUnionType.map_members(M) {
      M.is_union? {
        M.map_members (ConstructorArgT)
        {
          link_internal DUnionType.name
          DUnionType.name.to_capital_case((ConstructorArgT.name) A)
          {
            DUnionType.name Reuslt = {
              .Type = type_(ConstructorArgT.name),
              .(ConstructorArgT.name) = A
            };
            return Reuslt;
          }
        }
      }
    }
  }
)

poof(
  func gen_constructor(Type)
  {
    link_internal Type.name
    Type.name.to_capital_case( Type.map_members(M).sep(,) { M.type M.name.to_capital_case } )
    {
      Type.name Reuslt = {
        Type.map_members(M).sep(,) {
          .M.name = M.name.to_capital_case
        }
      };
      return Reuslt;
    }
  }
)

poof(
  func gen_hetero_vector_operator(type_datatype t1, type_datatype t2, type_poof_symbol Operator)
  {
    t1.member(0, (E)
    {
      E.is_array?
      {
        inline t1.name
        operator(Operator)( t1.name P1, t2.name P2 )
        {
          t1.name Result = {
          E.map_array(Index)
          {
            .(E.name)[Index] = P1.(E.name)[Index] Operator E.type( P2.(E.name)[Index] ),
          }
          };
          return Result;
        }
      }
    })
  }
)

poof(
  func gen_vector_infix_operator(type_datatype Type, type_poof_symbol Operator)
  {
    Type.member(0, (E)
    {
      E.is_array?
      {
        inline Type.name
        operator(Operator)( Type.name P1, Type.name P2 )
        {
          Type.name Result = {
          E.map_array(Index)
          {
            .(E.name)[Index] = P1.(E.name)[Index] Operator P2.(E.name)[Index],
          }
          };
          return Result;
        }
      }
    })
  }
)

poof(
  func gen_vector_infix_operators(type_datatype Type, type_datatype E, type_poof_symbol Operator)
  {
    inline Type.name
    operator(Operator)( Type.name P1, Type.name P2 )
    {
      Type.name Result = {
      E.map_array(Index)
      {
        .(E.name)[Index] = P1.(E.name)[Index] Operator P2.(E.name)[Index],
      }
      };
      return Result;
    }

    inline Type.name
    operator(Operator)( Type.name P1, E.type Scalar )
    {
      Type.name Result = {
      E.map_array(Index)
      {
        .(E.name)[Index] = P1.(E.name)[Index] Operator Scalar,
      }
      };
      return Result;
    }

    inline Type.name
    operator(Operator)( E.type Scalar, Type.name P1 )
    {
      Type.name Result = {
      E.map_array(Index)
      {
        .(E.name)[Index] = Scalar Operator P1.(E.name)[Index],
      }
      };
      return Result;
    }

  }
)

poof(
  func gen_vector_comparator(type_datatype Type, type_datatype E, type_poof_symbol Comparator)
  {
    inline b32
    operator(Comparator)( Type.name P1, Type.name P2 )
    {
      b32 Result = ( E.map_array(Index).sep(&&) { P1.(E.name)[Index] (Comparator) P2.(E.name)[Index] });
      return Result;
    }
  }
)

poof(
  func gen_vector_mut_infix_operators(type_datatype Type, type_datatype E, type_poof_symbol Operator)
  {
    inline Type.name &
    operator(Operator)( Type.name &P1, Type.name P2 )
    {
      E.map_array(Index) {
        P1.(E.name)[Index] (Operator) P2.(E.name)[Index];
      }
      return P1;
    }

    inline Type.name &
    operator(Operator)( Type.name &P1, E.type Scalar )
    {
      E.map_array(Index) {
        P1.(E.name)[Index] (Operator) Scalar;
      }
      return P1;
    }
  }
)

poof(
  func gen_vector_operators(Type)
  {
    Type.member(0, (E)
    {
      E.is_array?
      {
        gen_vector_comparator(Type, E, {==})

        // NOTE(Jesse): Can't gen != because the condition welding it together
        // is not &&, it's ||
        //
        /* gen_vector_comparator(Type, {!=}) */
        inline b32
        operator!=( Type.name P1, Type.name P2 )
        {
          b32 Result = !(P1 == P2);
          return Result;
        }

        gen_vector_comparator(Type, E, {<})

        gen_vector_comparator(Type, E, {<=})

        gen_vector_comparator(Type, E, {>})

        gen_vector_comparator(Type, E, {>=})

        gen_vector_infix_operators(Type, E, {+})

        gen_vector_infix_operators(Type, E, {-})

        gen_vector_infix_operators(Type, E, {*})

        gen_vector_infix_operators(Type, E, {/})

        gen_vector_mut_infix_operators(Type, E, {+=})

        gen_vector_mut_infix_operators(Type, E, {-=})

        gen_vector_mut_infix_operators(Type, E, {*=})

        gen_vector_mut_infix_operators(Type, E, {/=})
      }
      {
        poof_error { (Type.name).member(0) was not an array.  Got name((E.name)) type((E.type)). }
      }
    })
  }
)

poof(
  func gen_hetero_vector_operators(t1, t2)
  {
    t1.member(0, (m0)
    {
      m0.is_array?
      {
        gen_hetero_vector_operator(t1, t2, {+})
        gen_hetero_vector_operator(t1, t2, {-})
        gen_hetero_vector_operator(t1, t2, {*})
        gen_hetero_vector_operator(t1, t2, {/})
      }
      {
        poof_error { (t1.name).member(0) was not an array.  Got name((m0.name)) t1((m0.t1)). }
      }
    })
  }
)

poof(
  func gen_vector_lerp(vec_t)
  {
    inline vec_t.name
    Lerp(r32 t, vec_t.name P1, vec_t.name P2)
    {
      Assert(t<=1);
      Assert(t>=0);
      vec_t.name Result = (1.0f-t)*P1 + t*P2;
      return Result;
    }
  }
)

/* poof( */
/*   data_func get_vec_base_type(vec_t) */
/*   { */
/*     vec_t.member_named(E) (base_array) -> { */
/*       base_array.is_array? */
/*       { */
/*         return base_array.type */
/*       } */
/*       { */
/*         poof_error { (Type.name).member(0) was not an array.  Got name((base_array.name)) type((base_array.type)). } */
/*       } */
/*     } */
/*   } */
/* ) */

/* poof( */
/*   func gen_vector_area(vec_t) */
/*   { */
/*     vec_t.get_vec_base_type (vec_base_t) */
/*     { */
/*       inline vec_base_t */
/*       Area( vec_t.name Vec ) */
/*       { */
/*         Assert(A.x > 0); */
/*         Assert(A.y > 0); */
/*         vec_base_t Result = A.x * A.y; */
/*         return Result; */
/*       } */
/*     } */
/*   } */
/* ) */

poof(
  func gen_vector_area(vec_t)
  {
    vec_t.member(0, (base_array)
    {
      base_array.is_array?
      {
        inline base_array.type
        Area( vec_t.name Vec )
        {
          Assert(Vec.x > 0);
          Assert(Vec.y > 0);
          base_array.type Result = base_array.map_array(Index).sep(*) { Vec.(base_array.name)[Index] };
          return Result;
        }
      }
      {
        poof_error { (Type.name).member(0) was not an array.  Got name((base_array.name)) type((base_array.type)). }
      }
    })
  }
)

poof(
  func gen_common_vector(vec_t)
  {
    vec_t.member(0, (base_array)
    {
      base_array.is_array?
      {
        inline base_array.type
        LengthSq( vec_t.name Vec )
        {
          base_array.type Result = base_array.map_array(Index).sep(+) { Vec.(base_array.name)[Index]*Vec.(base_array.name)[Index] };
          return Result;
        }

        inline r32
        Length( vec_t.name Vec )
        {
          r32 Result = (r32)sqrt(LengthSq(Vec));
          return Result;
        }

        inline vec_t.name
        Max( vec_t.name A, vec_t.name B )
        {
          vec_t.name Result;
          base_array.map_array(Index)
          {
            Result.(base_array.name)[Index] = Max( A.(base_array.name)[Index], B.(base_array.name)[Index] );
          }
          return Result;
        }

        inline vec_t.name
        Min( vec_t.name A, vec_t.name B )
        {
          vec_t.name Result;
          base_array.map_array(Index)
          {
            Result.(base_array.name)[Index] = Min( A.(base_array.name)[Index], B.(base_array.name)[Index] );
          }
          return Result;
        }

        inline vec_t.name
        Abs( vec_t.name Vec )
        {
          vec_t.name Result;
          base_array.map_array(Index)
          {
            Result.(base_array.name)[Index] = ((base_array.type))Abs( Vec.(base_array.name)[Index] );
          }
          return Result;
        }


        inline vec_t.name
        GetSign( vec_t.name Vec )
        {
          vec_t.name Result;
          base_array.map_array(Index)
          {
            Result.(base_array.name)[Index] = GetSign( Vec.(base_array.name)[Index] );
          }
          return Result;
        }


        inline vec_t.name
        Bilateral( vec_t.name Vec )
        {
          vec_t.name Result;
          base_array.map_array(Index)
          {
            Result.(base_array.name)[Index] = Bilateral( Vec.(base_array.name)[Index] );
          }
          return Result;
        }

        inline vec_t.name
        ClampNegative( vec_t.name V )
        {
          vec_t.name Result = V;
          base_array.map_array(Index)
          {
            if ( V.base_array.name[Index] > base_array.type(0) ) Result.base_array.name[Index] = base_array.type(0);
          }
          return Result;
        }

        inline vec_t.name
        ClampPositive( vec_t.name V )
        {
          vec_t.name Result = V;
          base_array.map_array(Index)
          {
            if ( V.base_array.name[Index] < base_array.type(0) ) Result.base_array.name[Index] = base_array.type(0);
          }
          return Result;
        }


      }
      {
        poof_error { (Type.name).member(0) was not an array.  Got name((base_array.name)) type((base_array.type)). }
      }
    })
  }
)

poof(
  func gen_vector_normalize(vec_t)
  {
    vec_t.member(0, (base_array)
    {
      base_array.is_array?
      {
        inline vec_t.name
        Normalize( vec_t.name Vec, r32 Length)
        {
          if (Length == 0.f) return {};
          vec_t.name Result = Vec/Length;
          return Result;
        }

        inline vec_t.name
        Normalize( vec_t.name Vec )
        {
          vec_t.name Result = Normalize(Vec, Length(Vec));
          return Result;
        }
      }
      {
        poof_error { (Type.name).member(0) was not an array.  Got name((base_array.name)) type((base_array.type)). }
      }
    })
  }
)

poof(
  func index_of(Type)
  {
    link_internal umm
    IndexOf((Type.name)_buffer *Buf, (Type.name) Element)
    {
      umm Result = Buf->Count;

      for (u32 ElementIndex = 0;
          ElementIndex < Buf->Count;
          ++ElementIndex)
      {
        if (AreEqual(Buf->Start[ElementIndex], Element))
        {
          Result = ElementIndex;
          break;
        }
      }

      return Result;
    }
  }
)

poof(
  func are_equal(Type)
  {
    link_internal b32
    AreEqual((Type.name) *Thing1, (Type.name) *Thing2)
    {
      b32 Result = MemoryIsEqual((u8*)Thing1, (u8*)Thing2, sizeof( (Type.name) ) );
      return Result;
    }
  }
)


/* poof( */
/*   func tuple(type_list Types) */
/*   { */
/*     tuple(Types.map (T) { _(T.name) }) */
/*   } */
/* ) */

/* poof( tuple([counted_string, counted_string]) ) */

  // TODO(Jesse): Replace this with stream?  Probably
poof(

  func buffer_builder(Type)
  {
    struct (Type.name)_buffer_builder
    {
      (Type.name)_stream Chunks;
    };

    link_internal void
    Append( (Type.name)_buffer_builder* Builder, (Type.name) E)
    {
      Push(&Builder->Chunks, E);
    }

    struct (Type.name)_buffer
    {
      umm Count;
      (Type.name) *E;
    };

    link_internal (Type.name)_buffer
    (Type.name.to_capital_case)Buffer(umm TotalElements, memory_arena *PermMemory)
    {
      (Type.name)_buffer Result = {};
      Result.Count = TotalElements;
      Result.E = Allocate( (Type.name), PermMemory, TotalElements);
      return Result;
    }

    link_internal (Type.name)_buffer
    Finalize( (Type.name)_buffer_builder *Builder, memory_arena *PermMemory)
    {
      TIMED_FUNCTION();

      u32 TotalElements = 0;

      // TODO(Jesse): Keep track of # of chunks?
      ITERATE_OVER(&Builder->Chunks)
      {
        ++TotalElements;
      }

      auto Result = (Type.name.to_capital_case)Buffer(TotalElements, PermMemory);

      u32 ElementIndex = 0;
      ITERATE_OVER(&Builder->Chunks)
      {
        auto At = GET_ELEMENT(Iter);
        Result.E[ElementIndex] = *At;
        Assert(ElementIndex < Result.Count);
        ++ElementIndex;
      }
      Assert(ElementIndex == Result.Count);

      return Result;
    }
  }

)




poof( func hashtable(Type) { (hashtable_struct(Type)) (hashtable_impl(Type)) })

poof(
  func hashtable_struct(Type)
  {
    struct (Type.name)_linked_list_node
    {
      (Type.name) Element;
      (Type.name)_linked_list_node *Next;
    };

    struct (Type.name)_hashtable
    {
      umm Size;
      (Type.name)_linked_list_node **Elements;
    };
  }
);

poof(
  func hashtable_impl(Type)
  {
    link_internal (Type.name)_linked_list_node *
    Allocate_(Type.name)_linked_list_node(memory_arena *Memory)
    {
      (Type.name)_linked_list_node *Result = Allocate( (Type.name)_linked_list_node, Memory, 1);
      return Result;
    }

    link_internal (Type.name)_hashtable
    Allocate_(Type.name)_hashtable(umm ElementCount, memory_arena *Memory)
    {
      (Type.name)_hashtable Result = {};
      Result.Elements = Allocate( (Type.name)_linked_list_node*, Memory, ElementCount);
      Result.Size = ElementCount;
      return Result;
    }

    link_internal (Type.name)_linked_list_node *
    GetHashBucket(umm HashValue, (Type.name)_hashtable *Table)
    {
      Assert(Table->Size);
      (Type.name)_linked_list_node *Result = Table->Elements[HashValue % Table->Size];
      return Result;
    }

    link_internal Type.name *
    GetFirstAtBucket(umm HashValue, (Type.name)_hashtable *Table)
    {
      (Type.name)_linked_list_node *Bucket = GetHashBucket(HashValue, Table);
      (Type.name) *Result = &Bucket->Element;
      return Result;
    }

    link_internal Type.name *
    Insert((Type.name)_linked_list_node *Node, (Type.name)_hashtable *Table)
    {
      Assert(Table->Size);
      umm HashValue = Hash(&Node->Element) % Table->Size;
      (Type.name)_linked_list_node **Bucket = Table->Elements + HashValue;
      while (*Bucket) Bucket = &(*Bucket)->Next;
      *Bucket = Node;
      return &Bucket[0]->Element;
    }

    link_internal (Type.name)*
    Insert((Type.name) Element, (Type.name)_hashtable *Table, memory_arena *Memory)
    {
      (Type.name)_linked_list_node *Bucket = Allocate_(Type.name)_linked_list_node(Memory);
      Bucket->Element = Element;
      Insert(Bucket, Table);
      return &Bucket->Element;
    }
  }
)

poof(
  func dunion_debug_print_prototype(DUnion)
  {
    link_internal void DebugPrint( (DUnion.type) *Struct, u32 Depth = 0);
    link_internal void DebugPrint( (DUnion.type) Struct, u32 Depth = 0);
  }
)

poof(
  func dunion_debug_print(DUnion)
  {
    link_internal void
    DebugPrint( (DUnion.type) *Struct, u32 Depth)
    {
      if (Struct)
      {
        DebugPrint(*Struct, Depth);
      }
    }

    link_internal void
    DebugPrint( (DUnion.type) Struct, u32 Depth)
    {
      DebugPrint("DUnion.type {\n", Depth);

      switch(Struct.Type)
      {
        DUnion.map_members (M)
        {
          M.is_union?
          {
            M.map_members (UnionMember)
            {
              case type_(UnionMember.type):
              {
                DebugPrint(Struct.(UnionMember.name), Depth+4);
              } break;
            }
          }
        }

        default : { DebugPrint("default while printing ((DUnion.type)) ((DUnion.name)) ", Depth+4); DebugLine("Type(%d)", Struct.Type); } break;
      }
      DebugPrint("}\n", Depth);
    }
  }
)

poof(
  func buffer(Type)
  {
    struct (Type.name)_buffer
    {
      Type.name *Start;
      umm Count;
    };

    link_internal (Type.name)_buffer
    (Type.name.to_capital_case)Buffer(umm ElementCount, memory_arena* Memory)
    {
      (Type.name)_buffer Result = {};

      if (ElementCount)
      {
        Result.Start = Allocate( (Type.name), Memory, ElementCount );
        Result.Count = ElementCount;
      }
      else
      {
        Warn("Attempted to allocate (Type.name)_buffer of 0 length.");
      }

      return Result;
    }
  }
)

poof(
  func generate_cursor(Type)
  {
    struct (Type.name)_cursor
    {
      Type.name *Start;
      // TODO(Jesse)(immediate): For the love of fucksakes change these to indices
      Type.name *At;
      Type.name *End;
    };

    link_internal (Type.name)_cursor
    (Type.name.to_capital_case)Cursor(umm ElementCount, memory_arena* Memory)
    {
      Type.name *Start = ((Type.name)*)PushStruct(Memory, sizeof((Type.name))*ElementCount, 1, 0);
      (Type.name)_cursor Result = {
        .Start = Start,
        .End = Start+ElementCount,
        .At = Start,
      };
      return Result;
    }
  }
)

poof(
  func generate_string_table(EnumType)
  {
    link_internal counted_string
    ToString( (EnumType.name) Type)
    {
      counted_string Result = {};
      switch (Type)
      {
        EnumType.map_values (EnumValue)
        {
          case EnumValue.name: { Result = CSz("EnumValue.name"); } break;
        }
      }
      return Result;
    }
  }
)

poof(
  func generate_value_table(EnumType)
  {
    link_internal (EnumType.name)
    (EnumType.name.to_capital_case)(counted_string S)
    {
      EnumType.name Result = {};

      EnumType.map_values(TEnumV)
      {
        if (StringsMatch(S, CSz("TEnumV.name"))) { return TEnumV.name; }
      }

      return Result;
    }
  }
)



poof(
  func generate_stream_chunk_struct(Type)
  {
    struct (Type.name)_stream_chunk
    {
      (Type.name) Element;
      (Type.name)_stream_chunk* Next;
    };
  }
)

poof(
  func generate_stream_push(Type)
  {
    link_internal (Type.name) *
    Push((Type.name)_stream* Stream, (Type.name) Element)
    {
      if (Stream->Memory == 0)
      {
        Stream->Memory = AllocateArena();
      }

      /* (Type.name)_stream_chunk* NextChunk = AllocateProtection((Type.name)_stream_chunk*), Stream->Memory, 1, False) */
      (Type.name)_stream_chunk* NextChunk = ((Type.name)_stream_chunk*)PushStruct(Stream->Memory, sizeof((Type.name)_stream_chunk), 1, 0);
      NextChunk->Element = Element;

      if (!Stream->FirstChunk)
      {
        Assert(!Stream->LastChunk);
        Stream->FirstChunk = NextChunk;
        Stream->LastChunk = NextChunk;
      }
      else
      {
        Stream->LastChunk->Next = NextChunk;
        Stream->LastChunk = NextChunk;
      }

      Assert(NextChunk->Next == 0);
      Assert(Stream->LastChunk->Next == 0);

      Stream->ChunkCount += 1;

      Type.name *Result = &NextChunk->Element;
      return Result;
    }
  }
)

poof(
  func generate_stream_struct(Type)
  {
    struct (Type.name)_stream
    {
      memory_arena *Memory;
      (Type.name)_stream_chunk* FirstChunk;
      (Type.name)_stream_chunk* LastChunk;
      umm ChunkCount;
    };

    link_internal void
    Deallocate((Type.name)_stream *Stream)
    {
      Stream->LastChunk = 0;
      Stream->FirstChunk = 0;
      VaporizeArena(Stream->Memory);
    }
  }
)

poof(
  func generate_stream_iterator(Type)
  {
    struct (Type.name)_iterator
    {
      (Type.name)_stream* Stream;
      (Type.name)_stream_chunk* At;
    };

    link_internal (Type.name)_iterator
    Iterator((Type.name)_stream* Stream)
    {
      (Type.name)_iterator Iterator = {
        .Stream = Stream,
        .At = Stream->FirstChunk
      };
      return Iterator;
    }

    link_internal b32
    IsValid((Type.name)_iterator* Iter)
    {
      b32 Result = Iter->At != 0;
      return Result;
    }

    link_internal void
    Advance((Type.name)_iterator* Iter)
    {
      Iter->At = Iter->At->Next;
    }

    link_internal b32
    IsLastElement((Type.name)_iterator* Iter)
    {
      b32 Result = Iter->At->Next == 0;
      return Result;
    }

  }
)

poof(
  func generate_stream_getters(InputTypeDef)
  {
    InputTypeDef.map_members (Member)
    {
      link_internal InputTypeDef.type
      GetBy(Member.name)( (Member.Type) Needle, (InputTypeDef.type)_stream *Haystack)
      {
        // TODO : Implement matching!
      }
    }
  }
)

poof(
  func generate_stream_compact(InputTypeDef)
  {
    link_internal (InputTypeDef.name)_buffer
    Compact((InputTypeDef.name)_stream *Stream, memory_arena *PermMemory)
    {
      (InputTypeDef.name)_buffer Result = {};
      if (Stream->ChunkCount)
      {
        Result = (InputTypeDef.name.to_capital_case)Buffer(Stream->ChunkCount, PermMemory);
        /* DebugLine("compact %u", Result.Count); */

        u32 Index = 0;
        ITERATE_OVER(Stream)
        {
          (InputTypeDef.name) *Spot = GET_ELEMENT(Iter);
          Result.Start[Index] = *Spot;

          ++Index;
        }

        Deallocate(Stream);
      }

      return Result;
    }
  }
)


poof(
  func generate_stream(Type)
  {
    (generate_stream_chunk_struct(Type))
    (generate_stream_struct(Type))
    (generate_stream_iterator(Type))
    (generate_stream_push(Type))
  }
)



poof(
  func string_and_value_tables(Def)
  {
    (generate_string_table(Def))
    (generate_value_table(Def))
  }
)

poof(
  func stream_and_cursor(Def)
  {
    (generate_cursor(Def))
    (generate_stream(Def))
  }
)

poof(
  polymorphic_func void DebugPrint( @TypeDef RuntimeValue, u32 Depth)
  {
    DebugPrint("TypeDef.name: ", Depth);
    TypeDef.is_enum?
    {
      TypeDef.map_values (ValueDef)
      {
        DebugPrint("ValueDef.name ValueDef.value", Depth+1);
        DebugPrint(ToString(RuntimeValue), Depth+1);
      }
    }

    TypeDef.is_struct?
    {
      TypeDef.map_members (MemberDef)
      {
        DebugPrint("MemberDef.type MemberDef.name: ", Depth);
        DebugPrint(RuntimeValue.(MemberDef.name), Depth+1);
        DebugPrint("\n");
      }
    }
  }
)

