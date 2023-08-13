struct ray
{
  v3 Origin;
  v3 Dir;
};

ray Ray(v3 Origin, v3 Dir)
{
  ray Result;
  Result.Origin = Origin;
  Result.Dir = Dir;
  return Result;
}

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

// TODO(Jesse, id: 92, tags: speed, aabb): This could be SIMD optimized ?
//
// NOTE(Jesse): Result is tmin value along the ray .. ie closest intersection
// to origin of the ray.  f32_MAX means no intersection
link_internal f32
Intersect(aabb AABB, ray *Ray)
{
  v3 min = AABB.Center - AABB.Radius;
  v3 max = AABB.Center + AABB.Radius;

  r32 tmin = (min.x - Ray->Origin.x) / Ray->Dir.x;
  r32 tmax = (max.x - Ray->Origin.x) / Ray->Dir.x;

  if (tmin > tmax)
  {
    r32 temp = tmin;
    tmin = tmax;
    tmax = temp;
  }

  r32 tymin = (min.y - Ray->Origin.y) / Ray->Dir.y;
  r32 tymax = (max.y - Ray->Origin.y) / Ray->Dir.y;

  if (tymin > tymax)
  {
    r32 temp = tymin;
    tymin = tymax;
    tymax = temp;
  }

  if ((tmin > tymax) || (tymin > tmax))
  return f32_MAX;

  if (tymin > tmin)
  tmin = tymin;

  if (tymax < tmax)
  tmax = tymax;

  r32 tzmin = (min.z - Ray->Origin.z) / Ray->Dir.z;
  r32 tzmax = (max.z - Ray->Origin.z) / Ray->Dir.z;

  if (tzmin > tzmax)
  {
    r32 temp = tzmin;
    tzmin = tzmax;
    tzmax = temp;
  }

  if ((tmin > tzmax) || (tzmin > tmax))
  return f32_MAX;

  if (tzmin > tmin)
  tmin = tzmin;

  if (tzmax < tmax)
  tmax = tzmax;

  return tmin;
}

// TODO(Jesse): Remove this?
link_internal f32
Intersect(aabb AABB, ray Ray)
{
  f32 Result = Intersect(AABB, &Ray);
  return Result;
}

