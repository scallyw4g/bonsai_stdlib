

#if 0
v2 hash( v2 x )   // this hash is not production ready, please
{                 // replace this by something better
    const v2 k = V2( 0.3183099f, 0.3678794f );
    x = x*k + V2(k.y, k.x);
    return -1.0f + 2.0f*Fract( 16.0f * k*Fract( x.x*x.y*(x.x+x.y)) );
}
#endif
// https://iquilezles.org/articles/gradientnoise/
float IQ_QuinticGradientNoise( v3 x )
{
    // grid
    v3 p = Floor(x);
    v3 w = Fract(x);

    // quintic interpolant
    v3 u = w*w*w*(w*(w*6.0-15.0)+10.0);

    // gradients
    v3 ga = hash_v3( p+V3(0.0f,0.0f,0.0f) );
    v3 gb = hash_v3( p+V3(1.0f,0.0f,0.0f) );
    v3 gc = hash_v3( p+V3(0.0f,1.0f,0.0f) );
    v3 gd = hash_v3( p+V3(1.0f,1.0f,0.0f) );
    v3 ge = hash_v3( p+V3(0.0f,0.0f,1.0f) );
    v3 gf = hash_v3( p+V3(1.0f,0.0f,1.0f) );
    v3 gg = hash_v3( p+V3(0.0f,1.0f,1.0f) );
    v3 gh = hash_v3( p+V3(1.0f,1.0f,1.0f) );

    // projections
    float va = Dot( ga, w-V3(0.0f,0.0f,0.0f) );
    float vb = Dot( gb, w-V3(1.0f,0.0f,0.0f) );
    float vc = Dot( gc, w-V3(0.0f,1.0f,0.0f) );
    float vd = Dot( gd, w-V3(1.0f,1.0f,0.0f) );
    float ve = Dot( ge, w-V3(0.0f,0.0f,1.0f) );
    float vf = Dot( gf, w-V3(1.0f,0.0f,1.0f) );
    float vg = Dot( gg, w-V3(0.0f,1.0f,1.0f) );
    float vh = Dot( gh, w-V3(1.0f,1.0f,1.0f) );

    // interpolation
    return va +
           u.x*(vb-va) +
           u.y*(vc-va) +
           u.z*(ve-va) +
           u.x*u.y*(va-vb-vc+vd) +
           u.y*u.z*(va-vc-ve+vg) +
           u.z*u.x*(va-vb-ve+vf) +
           u.x*u.y*u.z*(-va+vb+vc-vd+ve-vf-vg+vh);
}

v4 ValueNoise_InigoQuilez( v3 x )
{
  v3 p = Floor(x);
  v3 w = Fract(x);

  v3 u = w*w*w*(w*(w*6.0-15.0)+10.0);
  v3 du = 30.0*w*w*(w*(w-2.0)+1.0);

  float a = hashf( p+V3(0,0,0) );
  float b = hashf( p+V3(1,0,0) );
  float c = hashf( p+V3(0,1,0) );
  float d = hashf( p+V3(1,1,0) );
  float e = hashf( p+V3(0,0,1) );
  float f = hashf( p+V3(1,0,1) );
  float g = hashf( p+V3(0,1,1) );
  float h = hashf( p+V3(1,1,1) );

  float k0 =   a;
  float k1 =   b - a;
  float k2 =   c - a;
  float k3 =   e - a;
  float k4 =   a - b - c + d;
  float k5 =   a - c - e + g;
  float k6 =   a - b - e + f;
  float k7 = - a + b + c - d + e - f - g + h;

  return V4( -1.0f + 2.0f * (k0 + k1*u.x + k2*u.y + k3*u.z + k4*u.x*u.y + k5*u.y*u.z + k6*u.z*u.x + k7*u.x*u.y*u.z),
              2.0f* du * V3( k1 + k4*u.y + k6*u.z + k7*u.y*u.z,
                             k2 + k5*u.z + k4*u.x + k7*u.z*u.x,
                             k3 + k6*u.x + k5*u.y + k7*u.x*u.y ) );
}


//---------------------------------------------------------------
// value noise, and its analytical derivatives

link_internal f32
IQ_ValueNoise_AnalyticNormals(f32 xin, f32 yin, f32 zin, v3 *Normal)
{
  v3 x = V3(xin, yin, zin);

  v3 p = Floor(x);
  v3 w = Fract(x);
  v3 u = w*w*(3.0-2.0*w);
  v3 du = 6.0*w*(1.0-w);

  float n = p.x + p.y*157.0f + 113.0f*p.z;

  float a = hash_f32(n+  0.f);
  float b = hash_f32(n+  1.f);
  float c = hash_f32(n+157.f);
  float d = hash_f32(n+158.f);
  float e = hash_f32(n+113.f);
  float f = hash_f32(n+114.f);
  float g = hash_f32(n+270.f);
  float h = hash_f32(n+271.f);

  float k0 =   a;
  float k1 =   b - a;
  float k2 =   c - a;
  float k3 =   e - a;
  float k4 =   a - b - c + d;
  float k5 =   a - c - e + g;
  float k6 =   a - b - e + f;
  float k7 = - a + b + c - d + e - f - g + h;

  v3 uzyx = V3(u.z, u.y, u.x);
  v3 uzxy = V3(u.z, u.x, u.y);
  v3 uyzx = V3(u.y, u.z, u.x);

  *Normal = du * (V3(k1,k2,k3) + uyzx*V3(k4,k5,k6) + uzxy*V3(k6,k4,k5) + k7*uyzx*uzxy);

  f32 Result = k0 + k1*u.x + k2*u.y + k3*u.z + k4*u.x*u.y + k5*u.y*u.z + k6*u.z*u.x + k7*u.x*u.y*u.z;
  return Result;
}


// Based on code found here:
// https://www.ronja-tutorials.com/post/028-voronoi-noise/
//
//
// NOTE(Jesse): Squareness == 0 (not square), Squareness == 1 (very square)
link_internal f32
VoronoiNoise3D(v3 Basis, r32 Squreness = 0.f, r32 MaskChance = 0.f)
{
  v3 baseCell = Floor(Basis);

  random_series BaseCellEntropy = RandomSeriesFromV3(baseCell);
  if (RandomUnilateral(&BaseCellEntropy) > MaskChance)
  {
    return 0;
  }
  else
  {
    v3 CellOffsets[27];

    // first pass to find the closest cell
    //
    f32 minDistToCellSq = 100;
    v3 toClosestCell;
    v3 closestCell;
    s32 CellIndex = 0;
    for( s32 x1 = -1;
             x1 <= 1;
           ++x1 )
    {
      for(s32 y1 = -1;
              y1 <= 1;
            ++y1 )
      {
        for( s32 z1 = -1;
                 z1 <= 1;
               ++z1 )
        {
          v3 cell = baseCell + V3(x1, y1, z1);
          v3 offset = Clamp01(RandomV3FromV3(cell) - Squreness);
          CellOffsets[CellIndex++] = offset;

          v3 cellPosition = cell + offset;
          v3 toCell = cellPosition - Basis;
          f32 distToCellSq = LengthSq(toCell);
          if(distToCellSq < minDistToCellSq)
          {
            minDistToCellSq = distToCellSq;
            closestCell = cell;
            toClosestCell = toCell;
          }
        }
      }
    }
    Assert(CellIndex == 27);

    // TODO(Jesse): This seems like you'd just want to do it in-line in the first
    // loop ..?
    //
    // second pass to find the distance to the closest edge
    //
    f32 minEdgeDistance = 10;
    CellIndex = 0;
    for( s32 x2 = -1;
             x2 <= 1;
           ++x2 )
    {
      for(s32 y2 = -1;
              y2 <= 1;
            ++y2 )
      {
        for( s32 z2 = -1;
                 z2 <= 1;
               ++z2 )
        {
          v3 cell = baseCell + V3(x2, y2, z2);
          v3 offset = CellOffsets[CellIndex++];

          v3 cellPosition = cell + offset;
          v3 toCell = cellPosition - Basis;

          v3 diffToClosestCell = Abs(closestCell - cell);
          b32 isClosestCell = diffToClosestCell.x + diffToClosestCell.y + diffToClosestCell.z < 0.1f;
          if(isClosestCell == False)
          {
            v3 toCenter = (toClosestCell + toCell) * 0.5;
            v3 cellDifference = Normalize(toCell - toClosestCell);
            f32 edgeDistance = Dot(toCenter, cellDifference);
            minEdgeDistance = Min(minEdgeDistance, edgeDistance);
          }
        }
      }
    }
    Assert(CellIndex == 27);
    return minEdgeDistance;
  }

  /* f32 random = rand3dTo1d(closestCell); */
  /* return V3(minDistToCellSq, random, minEdgeDistance); */
  /* return SquareRoot(minDistToCellSq); */
}

// NOTE(Jesse): MaskChance == 0.f (never mask) MaskChance == 1.f (always mask)
link_internal void
VoronoiNoise3D_8x_Masked(f32 *Results, r32 BaseMask, r32 *MaskChances, f32 *xMapped, v2 yzMapped, r32 Squreness = 0.f)
{
  RangeIterator(Index, 8)
  {
    v3 Basis = V3(xMapped[Index], yzMapped.E[0], yzMapped.E[1]);
    v3 baseCell = Floor(Basis);
    /* v3 baseCell = Floor(Basis) + V3(Index, 0, 0); */

    random_series BaseCellEntropy = RandomSeriesFromV3(baseCell);
    if (RandomUnilateral(&BaseCellEntropy) > BaseMask+MaskChances[Index])
    {
      Results[Index] = 0.f;
    }
    else
    {
      v3 CellOffsets[27];

      // first pass to find the closest cell
      //
      f32 minDistToCellSq = 100;
      v3 toClosestCell;
      v3 closestCell;
      s32 CellIndex = 0;
      for( s32 x1 = -1;
               x1 <= 1;
             ++x1 )
      {
        for(s32 y1 = -1;
                y1 <= 1;
              ++y1 )
        {
          for( s32 z1 = -1;
                   z1 <= 1;
                 ++z1 )
          {
            v3 cell = baseCell + V3(x1, y1, z1);

            random_series CellEntropy = RandomSeriesFromV3(cell);

            /* RandomUnilateral(&CellEntropy); */
            /* if (RandomUnilateral(&CellEntropy) > MaskChance) */
            {
              v3 offset = Clamp01(RandomV3(&CellEntropy) - Squreness);
              CellOffsets[CellIndex++] = offset;

              v3 cellPosition = cell + offset;
              v3 toCell = cellPosition - Basis;
              f32 distToCellSq = LengthSq(toCell);
              if(distToCellSq < minDistToCellSq)
              {
                minDistToCellSq = distToCellSq;
                closestCell = cell;
                toClosestCell = toCell;
              }
            }
            /* else */
            {
              /* CellOffsets[CellIndex++].x = f32_MAX; */
            }
          }
        }
      }
      Assert(CellIndex == 27);

      // TODO(Jesse): This seems like you'd just want to do it in-line in the first
      // loop ..?
      //
      // second pass to find the distance to the closest edge
      //
      f32 minEdgeDistance = f32_MAX;
      CellIndex = 0;
      for( s32 x2 = -1;
               x2 <= 1;
             ++x2 )
      {
        for(s32 y2 = -1;
                y2 <= 1;
              ++y2 )
        {
          for( s32 z2 = -1;
                   z2 <= 1;
                 ++z2 )
          {
            v3 cell = baseCell + V3(x2, y2, z2);
            v3 offset = CellOffsets[CellIndex++];
            /* if (offset.x != f32_MAX) */
            {
              v3 cellPosition = cell + offset;
              v3 toCell = cellPosition - Basis;

              v3 diffToClosestCell = Abs(closestCell - cell);
              b32 isClosestCell = diffToClosestCell.x + diffToClosestCell.y + diffToClosestCell.z < 0.1f;
              if(isClosestCell == False)
              {
                v3 toCenter = (toClosestCell + toCell) * 0.5;
                v3 cellDifference = Normalize(toCell - toClosestCell);
                f32 edgeDistance = Dot(toCenter, cellDifference);
                minEdgeDistance = Min(minEdgeDistance, edgeDistance);
              }
            }
          }
        }
      }
      Assert(CellIndex == 27);

      /* f32 random = rand3dTo1d(closestCell); */
      /* return V3(minDistToCellSq, random, minEdgeDistance); */
      /* return minEdgeDistance; */
      /* return SquareRoot(minDistToCellSq); */

      if (minEdgeDistance == f32_MAX) { minEdgeDistance = 0.f; }
      Results[Index] = minEdgeDistance;
    }
  }
}
link_internal void
VoronoiNoise3D_8x(f32 *Results, f32 *xMapped, v2 yzMapped, r32 Squreness = 0.f, r32 MaskChance = 0.9f)
{
  RangeIterator(Index, 8)
  {
    v3 Basis = V3(xMapped[Index], yzMapped.E[0], yzMapped.E[1]);
    v3 baseCell = Floor(Basis);
    /* v3 baseCell = Floor(Basis) + V3(Index, 0, 0); */

    random_series BaseCellEntropy = RandomSeriesFromV3(baseCell);
    if (RandomUnilateral(&BaseCellEntropy) > MaskChance)
    {
      Results[Index] = 0.f;
    }
    else
    {
      v3 CellOffsets[27];

      // first pass to find the closest cell
      //
      f32 minDistToCellSq = 100;
      v3 toClosestCell;
      v3 closestCell;
      s32 CellIndex = 0;
      for( s32 x1 = -1;
               x1 <= 1;
             ++x1 )
      {
        for(s32 y1 = -1;
                y1 <= 1;
              ++y1 )
        {
          for( s32 z1 = -1;
                   z1 <= 1;
                 ++z1 )
          {
            v3 cell = baseCell + V3(x1, y1, z1);

            random_series CellEntropy = RandomSeriesFromV3(cell);

            /* RandomUnilateral(&CellEntropy); */
            /* if (RandomUnilateral(&CellEntropy) > MaskChance) */
            {
              v3 offset = Clamp01(RandomV3(&CellEntropy) - Squreness);
              CellOffsets[CellIndex++] = offset;

              v3 cellPosition = cell + offset;
              v3 toCell = cellPosition - Basis;
              f32 distToCellSq = LengthSq(toCell);
              if(distToCellSq < minDistToCellSq)
              {
                minDistToCellSq = distToCellSq;
                closestCell = cell;
                toClosestCell = toCell;
              }
            }
            /* else */
            {
              /* CellOffsets[CellIndex++].x = f32_MAX; */
            }
          }
        }
      }
      Assert(CellIndex == 27);

      // TODO(Jesse): This seems like you'd just want to do it in-line in the first
      // loop ..?
      //
      // second pass to find the distance to the closest edge
      //
      f32 minEdgeDistance = f32_MAX;
      CellIndex = 0;
      for( s32 x2 = -1;
               x2 <= 1;
             ++x2 )
      {
        for(s32 y2 = -1;
                y2 <= 1;
              ++y2 )
        {
          for( s32 z2 = -1;
                   z2 <= 1;
                 ++z2 )
          {
            v3 cell = baseCell + V3(x2, y2, z2);
            v3 offset = CellOffsets[CellIndex++];
            /* if (offset.x != f32_MAX) */
            {
              v3 cellPosition = cell + offset;
              v3 toCell = cellPosition - Basis;

              v3 diffToClosestCell = Abs(closestCell - cell);
              b32 isClosestCell = diffToClosestCell.x + diffToClosestCell.y + diffToClosestCell.z < 0.1f;
              if(isClosestCell == False)
              {
                v3 toCenter = (toClosestCell + toCell) * 0.5;
                v3 cellDifference = Normalize(toCell - toClosestCell);
                f32 edgeDistance = Dot(toCenter, cellDifference);
                minEdgeDistance = Min(minEdgeDistance, edgeDistance);
              }
            }
          }
        }
      }
      Assert(CellIndex == 27);

      /* f32 random = rand3dTo1d(closestCell); */
      /* return V3(minDistToCellSq, random, minEdgeDistance); */
      /* return minEdgeDistance; */
      /* return SquareRoot(minDistToCellSq); */

      if (minEdgeDistance == f32_MAX) { minEdgeDistance = 0.f; }
      Results[Index] = minEdgeDistance;
    }
  }
}
