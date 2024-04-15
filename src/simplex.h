#if 0

v3 mod289(v3 x)
{
    return x - Floor(x / 289.0f) * 289.0f;
}

v4 mod289(v4 x)
{
    return x - Floor(x / 289.0f) * 289.0f;
}

v4 permute(v4 x)
{
    return mod289((x * 34.0f + 1.0f) * x);
}

v4 taylorInvSqrt(v4 r)
{
    return 1.79284291400159f - r * 0.85373472095314f;
}

v4 snoise(v3 v)
{
  const v2 C = V2(1.f / 6.f, 1.f / 3.f);

  // First corner
  v3 i  = Floor(v + Dot(v, V3(C.y)));
  v3 x0 = v   - i + Dot(i, V3(C.x));

  // Other corners
  v3 g = step(x0.yzx, x0.xyz);
  v3 l = 1.0 - g;
  v3 i1 = min(g.xyz, l.zxy);
  v3 i2 = max(g.xyz, l.zxy);

  v3 x1 = x0 - i1 + C.x;
  v3 x2 = x0 - i2 + C.y;
  v3 x3 = x0 - 0.5;

  // Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  v4 p =
    permute(permute(permute(i.z + v4(0.0, i1.z, i2.z, 1.0))
                          + i.y + v4(0.0, i1.y, i2.y, 1.0))
                          + i.x + v4(0.0, i1.x, i2.x, 1.0));

  // Gradients: 7x7 points over a square, mapped onto an octahedron.
  // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  v4 j = p - 49.0 * Floor(p / 49.0);  // mod(p,7*7)

  v4 x_ = Floor(j / 7.0);
  v4 y_ = Floor(j - 7.0 * x_); 

  v4 x = (x_ * 2.0 + 0.5) / 7.0 - 1.0;
  v4 y = (y_ * 2.0 + 0.5) / 7.0 - 1.0;

  v4 h = 1.0 - abs(x) - abs(y);

  v4 b0 = v4(x.xy, y.xy);
  v4 b1 = v4(x.zw, y.zw);

  v4 s0 = Floor(b0) * 2.0 + 1.0;
  v4 s1 = Floor(b1) * 2.0 + 1.0;
  v4 sh = -step(h, v4(0.0));

  v4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
  v4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

  v3 g0 = v3(a0.xy, h.x);
  v3 g1 = v3(a0.zw, h.y);
  v3 g2 = v3(a1.xy, h.z);
  v3 g3 = v3(a1.zw, h.w);

  // Normalize gradients
  v4 norm = taylorInvSqrt(v4(dot(g0, g0), dot(g1, g1), dot(g2, g2), dot(g3, g3)));
  g0 *= norm.x;
  g1 *= norm.y;
  g2 *= norm.z;
  g3 *= norm.w;

  // Compute noise and gradient at P
  v4 m = max(0.6 - v4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
  v4 m2 = m * m;
  v4 m3 = m2 * m;
  v4 m4 = m2 * m2;
  v3 grad =
    -6.0 * m3.x * x0 * dot(x0, g0) + m4.x * g0 +
    -6.0 * m3.y * x1 * dot(x1, g1) + m4.y * g1 +
    -6.0 * m3.z * x2 * dot(x2, g2) + m4.z * g2 +
    -6.0 * m3.w * x3 * dot(x3, g3) + m4.w * g3;
  v4 px = v4(dot(x0, g0), dot(x1, g1), dot(x2, g2), dot(x3, g3));
  return 42.0 * v4(grad, dot(m4, px));
}


//===============================================================================================
//===============================================================================================
//===============================================================================================
//===============================================================================================
//===============================================================================================

void mainImage( out v4 fragColor, in v2 fragCoord )
{
  v2 p = (-iResolution.xy + 2.0*fragCoord) / iResolution.y;

     // camera movement	.
  float an = 0.1*iTime;
  v3 ro = v3( 2.5*cos(an), 1.0, 2.5*sin(an) );
    v3 ta = v3( 0.0, 1.0, 0.0 );
    // camera matrix
    v3 ww = normalize( ta - ro );
    v3 uu = normalize( cross(ww,v3(0.0,1.0,0.0) ) );
    v3 vv = normalize( cross(uu,ww));
  // create view ray
  v3 rd = normalize( p.x*uu + p.y*vv + 1.5*ww );

    // sphere center	
  v3 sc = v3(0.0,1.0,0.0);

    // raytrace
  float tmin = 10000.0;
  v3  nor = v3(0.0);
  float occ = 1.0;
  v3  pos = v3(0.0);
  
  // raytrace-plane
  float h = (0.0-ro.y)/rd.y;
  if( h>0.0 ) 
  { 
    tmin = h; 
    nor = v3(0.0,1.0,0.0); 
    pos = ro + h*rd;
    v3 di = sc - pos;
    float l = length(di);
    occ = 1.0 - dot(nor,di/l)*1.0*1.0/(l*l); 
  }

  // raytrace-sphere
  v3  ce = ro - sc;
  float b = dot( rd, ce );
  float c = dot( ce, ce ) - 1.0;
  h = b*b - c;
  if( h>0.0 )
  {
    h = -b - SquareRoot(h);
    if( h<tmin ) 
    { 
      tmin=h; 
      nor = normalize(ro+h*rd-sc); 
      occ = 0.5 + 0.5*nor.y;
    }
  }

    // shading/lighting	
  v3 col = v3(0.9);
  if( tmin<100.0 )
  {
      pos = ro + tmin*rd;
        
        //animate Floor plane
    if(pos.y < 0.01)pos.y = iTime * 0.05;
        v4 n = snoise( 8.0*pos );
        col = 0.5 + ((p.x>0.0)?n.xyz * 0.2:n.www * 0.5);
    
        //fog
    col = mix( col, v3(0.9), 1.0-exp( -0.003*tmin*tmin ) );
  }
  
  
  fragColor = v4( col, 1.0 );
}
#endif
