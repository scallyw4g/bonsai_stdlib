 
struct rect2
{
  v2 Min;
  v2 Max;
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

link_internal rect2
RectMinMax(v2 Min, v2 Max)
{
  rect2 Result = { .Min = Min, .Max = Max };
  return Result;
}

link_internal rect2
RectMinDim(v2 Min, v2 Dim)
{
  rect2 Result = {Min, Min+Dim};
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
MinMaxAABB(v3 Min, v3 Max)
{
  v3 Radius = (Max - Min)/2.0f;
  v3 Center = Min + Radius;
  aabb Result(Center, Radius);
  return Result;
}

link_internal aabb
AABBCenterDim(v3 Center, v3 Dim)
{
  v3 Radius = Dim/2.f;
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


link_internal rect2
operator+(rect2 R1, v2 P)
{
  rect2 Result = {
    .Min = R1.Min + P,
    .Max = R1.Max + P,
  };
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

struct ray
{
  v3 Origin;
  v3 Dir;
};

enum maybe_tag
{
  Maybe_No,
  Maybe_Yes,
};

struct maybe_ray
{
  maybe_tag Tag;
  ray Ray;
};

// TODO(Jesse, id: 92, tags: speed, aabb): This could be optimized significantly
link_internal b32
Intersect(aabb AABB, ray Ray)
{
  v3 min = AABB.Center - AABB.Radius;
  v3 max = AABB.Center + AABB.Radius;

  r32 tmin = (min.x - Ray.Origin.x) / Ray.Dir.x;
  r32 tmax = (max.x - Ray.Origin.x) / Ray.Dir.x;

  if (tmin > tmax)
  {
    r32 temp = tmin;
    tmin = tmax;
    tmax = temp;
  }

  r32 tymin = (min.y - Ray.Origin.y) / Ray.Dir.y;
  r32 tymax = (max.y - Ray.Origin.y) / Ray.Dir.y;

  if (tymin > tymax)
  {
    r32 temp = tymin;
    tymin = tymax;
    tymax = temp;
  }

  if ((tmin > tymax) || (tymin > tmax))
  return false;

  if (tymin > tmin)
  tmin = tymin;

  if (tymax < tmax)
  tmax = tymax;

  r32 tzmin = (min.z - Ray.Origin.z) / Ray.Dir.z;
  r32 tzmax = (max.z - Ray.Origin.z) / Ray.Dir.z;

  if (tzmin > tzmax)
  {
    r32 temp = tzmin;
    tzmin = tzmax;
    tzmax = temp;
  }

  if ((tmin > tzmax) || (tzmin > tmax))
  return false;

  if (tzmin > tmin)
  tmin = tzmin;

  if (tzmax < tmax)
  tmax = tzmax;

  return true;
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

v2
GetDim(rect2 Rect)
{
  v2 Dim = Rect.Max - Rect.Min;
  return Dim;
}
