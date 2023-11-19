
struct rect2
{
  v2 Min;
  v2 Max;
};

struct rect3i
{
  v3i Min;
  v3i Max;
};


link_internal rect2
InvertedInfinityRectangle()
{
  rect2 Result = {
    .Min = V2(f32_MAX),
    .Max = V2(-f32_MAX),
  };
  return Result;
}

link_internal v2
BottomLeft(rect2 Rect)
{
  v2 Result = V2(Rect.Min.x, Rect.Max.y);
  return Result;
}

link_internal v2
TopRight(rect2 Rect)
{
  v2 Result = V2(Rect.Max.x, Rect.Min.y);
  return Result;
}


poof(
  func gen_rect_helpers(rect, vector)
  {
    link_internal rect.name
    RectMinMax((vector.name) Min, vector.name Max)
    {
      rect.name Result = { .Min = Min, .Max = Max };
      return Result;
    }

    link_internal rect.name
    RectMinDim((vector.name) Min, vector.name Dim)
    {
      rect.name Result = { Min, Min + Dim };
      return Result;
    }

    link_internal b32
    IsInside((vector.name) P, rect.name Rect)
    {
      b32 Result = (P >= Rect.Min && P < Rect.Max);
      return Result;
    }
  }
)


poof(gen_rect_helpers(rect2, v2))
#include <generated/gen_rect_helpers_rect2_v2.h>
/* poof(gen_rect_helpers(rect2i, v2i)) */

/* poof(gen_rect_helpers(rect3, v3)) */
poof(gen_rect_helpers(rect3i, v3i))
#include <generated/gen_rect_helpers_rect3i_v3i.h>


/* link_internal rect3i */
/* Rect3i(v3i Min, v3i Max) */
/* { */
/* } */

link_internal rect3i
Rect3iMinDim(v3i Min, v3i Dim)
{
  rect3i Result = {Min, Min+Dim};
  return Result;
}

link_internal rect3i
Rect3iMinMax(v3i Min, v3i Max)
{
  rect3i Result = {Min, Max};
  return Result;
}

struct aabb
{
  v3 Center;
  v3 Radius;

  aabb(v3 Center_in, v3 Radius_in)
  {
    this->Center = Center_in;
    this->Radius = Radius_in;
  }

  aabb(world_position Center_in, world_position Radius_in)
  {
    this->Center = V3(Center_in);
    this->Radius = V3(Radius_in);
  }

  aabb() { Clear(this); }
};



struct sphere
{
  v3 P;
  r32 Radius;
};

link_internal sphere
Sphere(v3 P, r32 Radius)
{
  sphere Result = { .P = P, .Radius = Radius };
  return Result;
}

v3 GetMax(aabb *Box)
{
  v3 Result = Box->Center + Box->Radius;
  return Result;
}

v3 GetMin(aabb *Box)
{
  v3 Result = Box->Center - Box->Radius;
  return Result;
}

link_internal rect3i
Rect3i(aabb AABB)
{
  v3 Min = GetMin(&AABB);
  v3 Max = GetMax(&AABB);

  rect3i Result = RectMinMax(V3i(Min), V3i(Max));
  return Result;
}

link_internal v3
ClipPToAABB(aabb *AABB, v3 P)
{
  v3 AABBMin = GetMin(AABB);
  v3 AABBMax = GetMax(AABB);

  v3 Result = {};
  Result.x = Max(AABBMin.x, Min(P.x, AABBMax.x));
  Result.y = Max(AABBMin.y, Min(P.y, AABBMax.y));
  Result.z = Max(AABBMin.z, Min(P.z, AABBMax.z));
  return Result;
}

link_internal b32
Intersect(aabb *AABB, sphere *S)
{
  v3 ClippedSphereCenter = ClipPToAABB(AABB, S->P);
  r32 DistSq = (ClippedSphereCenter.x - S->P.x)*(ClippedSphereCenter.x - S->P.x) +
               (ClippedSphereCenter.y - S->P.y)*(ClippedSphereCenter.y - S->P.y) +
               (ClippedSphereCenter.z - S->P.z)*(ClippedSphereCenter.z - S->P.z);

  b32 Result = DistSq < Square(S->Radius);
  return Result;
}

inline b32
Intersect(aabb *First, aabb *Second)
{
  b32 Result = True;

  Result &= (Abs(First->Center.x - Second->Center.x) < (First->Radius.x + Second->Radius.x));
  Result &= (Abs(First->Center.y - Second->Center.y) < (First->Radius.y + Second->Radius.y));
  Result &= (Abs(First->Center.z - Second->Center.z) < (First->Radius.z + Second->Radius.z));

  return Result;
}

link_internal aabb
AABBMinMax(v3i Min, v3i Max)
{
  v3 Radius = V3(Max - Min)/2.0f;
  v3 Center = V3(Min + Radius);
  aabb Result(Center, Radius);
  return Result;
}

link_internal aabb
AABBMinMax(v3 Min, v3 Max)
{
  v3 Radius = (Max - Min)/2.0f;
  v3 Center = Min + Radius;
  aabb Result(Center, Radius);
  return Result;
}

// TODO(Jesse): Delete this
link_internal aabb
MinMaxAABB(v3 Min, v3 Max)
{
  return AABBMinMax(Min, Max);
}

link_internal aabb
AABBCenterDim(v3 Center, v3 Dim)
{
  v3 Radius = Dim/2.f;
  aabb Result(Center, Radius);
  return Result;
}

link_internal aabb
AABBMinRad(v3 Min, v3 Radius)
{
  v3 Center = Min + Radius;
  aabb Result(Center, Radius);
  return Result;
}

link_internal aabb
AABBMinDim(v3 Min, v3i Dim)
{
  v3 Radius = V3(Dim)/2.f;
  v3 Center = Min + Radius;
  aabb Result(Center, Radius);
  return Result;
}

link_internal aabb
AABBMinDim(v3i Min, v3i Dim)
{
  v3 Radius = V3(Dim)/2.f;
  v3 Center = V3(Min + Radius);
  aabb Result(Center, Radius);
  return Result;
}

link_internal aabb
AABBMinDim(v3 Min, v3 Dim)
{
  v3 Radius = Dim/2.f;
  v3 Center = Min + Radius;
  aabb Result(Center, Radius);
  return Result;
}

inline rect3i
Union(rect3i *First, rect3i *Second)
{
  v3i ResultMin = Max(First->Min, Second->Min);
  v3i ResultMax = Min(First->Max, Second->Max);
  rect3i Result = Rect3iMinMax(ResultMin, ResultMax);

  return Result;
}

inline aabb
Union(aabb *First, aabb *Second)
{
  v3 FirstMin = GetMin(First);
  v3 SecondMin = GetMin(Second);

  v3 FirstMax = GetMax(First);
  v3 SecondMax = GetMax(Second);

  v3 ResultMin = Max(FirstMin, SecondMin);
  v3 ResultMax = Min(FirstMax, SecondMax);
  aabb Result = AABBMinMax(ResultMin, ResultMax);

  return Result;
}

link_internal b32
Contains(rect3i Rect, v3i P)
{
  b32 Result = (P >= Rect.Min && P < Rect.Max);
  return Result;
}

link_internal b32
Contains(aabb AABB, v3 P)
{
  v3 Min = AABB.Center-AABB.Radius;
  v3 Max = AABB.Center+AABB.Radius;

  b32 Result = (P >= Min && P < Max);
  return Result;
}

link_internal b32
IsInside(aabb AABB, v3 P)
{
  b32 Result = Contains(AABB, P);
  return Result;
}

link_internal rect2
operator+(rect2 R1, v2 P)
{
  rect2 Result = {
    .Min = R1.Min + P,
    .Max = R1.Max + P,
  };
  return Result;
}

link_internal rect2
operator+(rect2 R1, rect2 R2)
{
  rect2 Result;
  Result.Min = R1.Min + R2.Min;
  Result.Max = R1.Max + R2.Max;
  return Result;
}

link_internal void
operator+=(rect2 &R1, rect2 R2)
{
  R1.Min += R2.Min;
  R1.Max += R2.Max;
}

link_internal void
operator+=(rect2 &R1, v2 P)
{
  R1.Min += P;
  R1.Max += P;
}

link_internal rect3i
operator-(rect3i R1, v3i P)
{
  rect3i Result = {
    .Min = R1.Min - P,
    .Max = R1.Max - P,
  };
  return Result;
}

link_internal rect2
operator-(rect2 R1, v2 P)
{
  rect2 Result = {
    .Min = R1.Min - P,
    .Max = R1.Max - P,
  };
  return Result;
}

link_internal rect2
operator-(rect2 R1, rect2 R2)
{
  rect2 Result = {
    .Min = R1.Min - R2.Min,
    .Max = R1.Max - R2.Max,
  };
  return Result;
}

link_internal aabb
operator+(aabb AABB, v3 V)
{
  aabb Result = AABB;
  Result.Radius = AABB.Radius + V;
  return Result;
}

link_internal v3
HalfDim( v3 P1 )
{
  v3 Result = P1 / 2;
  return Result;
}

link_internal b32
IsInsideRect(rect2 Rect, v2 P)
{
  b32 Result = (P >= Rect.Min && P < Rect.Max);
  return Result;
}

link_internal v3
GetMin(aabb Rect)
{
  v3 Result = Rect.Center - Rect.Radius;
  return Result;
}

link_internal v3
GetMax(aabb Rect)
{
  v3 Result = Rect.Center + Rect.Radius;
  return Result;
}

v2 GetRadius(rect2 Rect)
{
  v2 Result = Rect.Max - Rect.Min;
  return Result;
}

r32 Area(rect2 Rect)
{
  v2 Radius = GetRadius(Rect);
  r32 Result = (Radius.x * Radius.y) * 4.f;
  return Result;
}

link_internal v3i
GetDim(rect3i Rect)
{
  v3i Dim = Rect.Max - Rect.Min;
  return Dim;
}

link_internal v2
GetDim(rect2 Rect)
{
  v2 Dim = Rect.Max - Rect.Min;
  return Dim;
}

link_internal v3
GetDim(aabb Rect)
{
  v3 Dim = Rect.Radius * 2.f;
  return Dim;
}

inline s32
Volume(rect3i Rect)
{
  v3i Dim = Max(V3i(0), GetDim(Rect));
  s32 Result = Volume(Dim);
  return Result;
}

inline s32
Volume(aabb Rect)
{
  v3 Dim = Rect.Radius*2.f;
  s32 Result = Volume(Dim);
  return Result;
}


