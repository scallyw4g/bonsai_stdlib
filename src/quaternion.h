union Quaternion
{
  float E[4];

  struct { r32 x; r32 y; r32 z; r32 w; };
  struct { v3 xyz; r32 _ignored; };

  Quaternion(void)
  {
    this->x = 0.f;
    this->y = 0.f;
    this->z = 0.f;
    this->w = 1.f;
  }

  Quaternion( v3 Vec, r32 W )
  {
    this->x = Vec.x;
    this->y = Vec.y;
    this->z = Vec.z;
    this->w = W;
  }

  Quaternion( r32 X, r32 Y, r32 Z, r32 W )
  {
    this->x = X;
    this->y = Y;
    this->z = Z;
    this->w = W;
  }
};

Quaternion
operator*(Quaternion A, Quaternion B)
{
  Quaternion Result = {};

  Result.w = A.w*B.w - A.x*B.x - A.y*B.y - A.z*B.z;
  Result.x = A.w*B.x + A.x*B.w + A.y*B.z - A.z*B.y;
  Result.y = A.w*B.y + A.y*B.w + A.z*B.x - A.x*B.z;
  Result.z = A.w*B.z + A.z*B.w + A.x*B.y - A.y*B.x;

  return Result;
}

inline r32
GetTheta(v3 P1, v3 P2)
{
  r32 DotP1P2 = Dot(P1,P2);

  r32 LP1 = Length(P1);
  r32 LP2 = Length(P2);


  r32 Theta = 0.f;
  if (LP1 == 0.f || LP2 == 0.f)
  {
    // Fundamentally, if either point is 0, getting a theta mesurement between
    // that and another point is undefined, so we just return 0
  }
  else
  {
    Assert(LP1 != 0.0f);
    Assert(LP2 != 0.0f);
    r32 cosTheta = ClampBilateral(DotP1P2 / (LP1*LP2));
    Theta = (r32)ArcCos( cosTheta );
  }

  return Theta;
}

// NOTE(Jesse): Euler angles in radians
// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
link_internal Quaternion
FromEuler(v3 Euler)
{
  /* Euler = Normalize(Euler); */
  Quaternion Result;

  f32 cr = Cos(Euler.x * 0.5f);
  f32 sr = Sin(Euler.x * 0.5f);
  f32 cp = Cos(Euler.y * 0.5f);
  f32 sp = Sin(Euler.y * 0.5f);
  f32 cy = Cos(Euler.z * 0.5f);
  f32 sy = Sin(Euler.z * 0.5f);

  Result.w = cr * cp * cy + sr * sp * sy;
  Result.x = sr * cp * cy - cr * sp * sy;
  Result.y = cr * sp * cy + sr * cp * sy;
  Result.z = cr * cp * sy - sr * sp * cy;

  return Result;
}

inline Quaternion
RotatePoint(v3 P1, v3 P2)
{
  P1 = Normalize(P1);
  P2 = Normalize(P2);
  v3 Axis = Normalize(Cross(P1, P2));

  r32 Theta = GetTheta(P1, P2);

  Quaternion Result = Quaternion((Axis*(r32)Sin(Theta/2.0f)), (r32)Cos(Theta/2.0f));

  if (Length(Result.xyz) == 0)  // The resulting rotation was inconsequential
    Result = Quaternion();

  return Result;
}

inline Quaternion
Conjugate( Quaternion q )
{
  Quaternion Result = {-q.x, -q.y, -q.z, q.w};
  return Result;
}


inline v3_u8
Rotate(v3_u8 P, Quaternion Rotation)
{
  // TODO(Jesse): make sure this works
  NotImplemented;

  v3 Result = ((Rotation * Quaternion(V3(P),0)) * Conjugate(Rotation)).xyz;
  return V3U8(Result);
}

inline v3
Rotate(v3 P, Quaternion Rotation)
{
  v3 Result = ((Rotation * Quaternion(P,0)) * Conjugate(Rotation)).xyz;
  return Result;
}

inline line
Rotate(line Line, Quaternion Rotation)
{
  line Result;

  Result.MinP = Rotate(Line.MinP, Rotation);
  Result.MaxP = Rotate(Line.MaxP, Rotation);

  return Result;
}

inline v3
RandomRotation(random_series *Entropy)
{
  v3 Result = RandomV3(Entropy);
  return Result;
}

inline Quaternion
RandomQuaternion(random_series *Entropy)
{
  v3 RotP = {};
  RotP.x = Sin(RandomUnilateral(Entropy));
  RotP.y = Sin(RandomUnilateral(Entropy));
  RotP.z = Sin(RandomUnilateral(Entropy));

  Quaternion Result = RotatePoint(V3(0.f, -1.f, 0.f), RotP);
  return Result;
}


link_internal void
Normalize( Quaternion *Q )
{
  f32 LenSq = Q->w*Q->w + Q->x*Q->x + Q->y*Q->y + Q->z*Q->z;
  f32 Len = SquareRoot(LenSq);

  Q->x /= Len;
  Q->y /= Len;
  Q->z /= Len;
  Q->w /= Len;
}

link_internal Quaternion
Normalize( Quaternion Q )
{
  f32 LenSq = Q.w*Q.w + Q.x*Q.x + Q.y*Q.y + Q.z*Q.z;
  f32 Len = SquareRoot(LenSq);

  r32 x = Q.x/Len;
  r32 y = Q.y/Len;
  r32 z = Q.z/Len;
  r32 w = Q.w/Len;

  return Quaternion(x, y, z, w);
}

// NOTE(Jesse): This is mostly for testing purposes as a sanity-check when I
// was implementing quaternions.  This (AFAIK) method suffers from gimbal lock
// and should be avoided in favor of using a matrix computed from a quaternion.
//
// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
//
link_internal m4
RotateTransform( v3 Euler )
{
  r32 psi   = Euler.x;
  r32 theta = Euler.y;
  r32 phi   = Euler.z;

  r32 cpsi   = Cos(psi);
  r32 ctheta = Cos(theta);
  r32 cphi   = Cos(phi);

  r32 spsi   = Sin(psi);
  r32 stheta = Sin(theta);
  r32 sphi   = Sin(phi);

  m4 Result;

  r32 m00 = ctheta*cphi;
  r32 m01 = -cpsi*sphi + spsi*stheta*cphi;
  r32 m02 = spsi*sphi + cpsi*stheta*cphi;
  r32 m03 = 0.f;

  r32 m10 = ctheta*sphi;
  r32 m11 = cpsi*cphi + spsi*stheta*sphi;
  r32 m12 = -spsi*cphi + cpsi*stheta*sphi;
  r32 m13 = 0.f;

  r32 m20 = -stheta;
  r32 m21 = spsi*ctheta;
  r32 m22 = cpsi*ctheta;
  r32 m23 = 0.f;

  r32 m30 = 0.f;
  r32 m31 = 0.f;
  r32 m32 = 0.f;
  r32 m33 = 1.f;


  // Column-major
  Result.E[0] = V4(m00, m10, m20, m30); // column 0
  Result.E[1] = V4(m01, m11, m21, m31); // column 1
  Result.E[2] = V4(m02, m12, m22, m32); // column 2
  Result.E[3] = V4(m03, m13, m23, m33); // column 3

  return Result;
}

// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
//
link_internal m4
RotateTransform( Quaternion v )
{
  Normalize(&v);

  r32 x = v.x;
  r32 y = v.y;
  r32 z = v.z;
  r32 w = v.w;

  m4 Result;

  r32 m00 = 1.f - 2.f*y*y - 2.f*z*z;
  r32 m01 = 2.f*x*y - 2.f*w*z;
  r32 m02 = 2.f*x*z + 2.f*w*y;
  r32 m03 = 0.f;

  r32 m10 = 2.f*x*y + 2.f*w*z;
  r32 m11 = 1.f - 2.f*x*x - 2.f*z*z;
  r32 m12 = 2.f*y*z - 2.f*w*x;
  r32 m13 = 0.f;

  r32 m20 = 2.f*x*z - 2.f*w*y;
  r32 m21 = 2.f*y*z + 2.f*w*x;
  r32 m22 = 1.f - 2.f*x*x - 2.f*y*y;
  r32 m23 = 0.f;

  r32 m30 = 0.f;
  r32 m31 = 0.f;
  r32 m32 = 0.f;
  r32 m33 = 1.f;


  // Column-major
  Result.E[0] = V4(m00, m10, m20, m30); // column 0
  Result.E[1] = V4(m01, m11, m21, m31); // column 1
  Result.E[2] = V4(m02, m12, m22, m32); // column 2
  Result.E[3] = V4(m03, m13, m23, m33); // column 3

  return Result;
}
