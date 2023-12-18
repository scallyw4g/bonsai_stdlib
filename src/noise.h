
v3 Floor(v3 v)
{
  v.x = Floorf(v.x);
  v.y = Floorf(v.y);
  v.z = Floorf(v.z);
  return v;
}

f32 Fract(f32 v)
{
  double intval;
  f32 Result = f32(modf((double)v, &intval));
  return Result;
}

v2 Fract(v2 v)
{
  double intval;
  v.x = f32(modf((double)v.x, &intval));
  v.y = f32(modf((double)v.y, &intval));
  return v;
}

v3 Fract(v3 v)
{
  double intval;
  v.x = f32(modf((double)v.x, &intval));
  v.y = f32(modf((double)v.y, &intval));
  v.z = f32(modf((double)v.z, &intval));
  return v;
}

v3 Sin(v3 v)
{
  v.x = f32(Sin(v.x));
  v.y = f32(Sin(v.y));
  v.z = f32(Sin(v.z));
  return v;
}


#if 0
v2 hash( v2 x )   // this hash is not production ready, please
{                 // replace this by something better
    const v2 k = V2( 0.3183099f, 0.3678794f );
    x = x*k + V2(k.y, k.x);
    return -1.0f + 2.0f*Fract( 16.0f * k*Fract( x.x*x.y*(x.x+x.y)) );
}
#endif

#if 1
v3 hash( v3 p )      // this hash is not production ready, please
{                    // replace this by something better
  p = V3( Dot(p, V3(127.1f,311.7f, 74.7f)),
          Dot(p, V3(269.5f,183.3f,246.1f)),
          Dot(p, V3(113.5f,271.9f,124.6f)));
  return -1.0 + 2.0*Fract(Sin(p)*43758.5453123f);
}
#endif

float hash( float n ) { return Fract(Sin(n)*753.5453123f); }

float hashf(v3 p)  // replace this by something better
{
    p  = 50.0*Fract( p*0.3183099f + V3(0.71f,0.113f,0.419f));
    return -1.0f+2.0f*Fract( p.x*p.y*p.z*(p.x+p.y+p.z) );
}

// https://iquilezles.org/articles/gradientnoise/
float IQ_QuinticGradientNoise( v3 x )
{
    // grid
    v3 p = Floor(x);
    v3 w = Fract(x);

    // quintic interpolant
    v3 u = w*w*w*(w*(w*6.0-15.0)+10.0);

    // gradients
    v3 ga = hash( p+V3(0.0f,0.0f,0.0f) );
    v3 gb = hash( p+V3(1.0f,0.0f,0.0f) );
    v3 gc = hash( p+V3(0.0f,1.0f,0.0f) );
    v3 gd = hash( p+V3(1.0f,1.0f,0.0f) );
    v3 ge = hash( p+V3(0.0f,0.0f,1.0f) );
    v3 gf = hash( p+V3(1.0f,0.0f,1.0f) );
    v3 gg = hash( p+V3(0.0f,1.0f,1.0f) );
    v3 gh = hash( p+V3(1.0f,1.0f,1.0f) );

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

  float a = hash(n+  0.f);
  float b = hash(n+  1.f);
  float c = hash(n+157.f);
  float d = hash(n+158.f);
  float e = hash(n+113.f);
  float f = hash(n+114.f);
  float g = hash(n+270.f);
  float h = hash(n+271.f);

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


