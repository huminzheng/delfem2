/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cstdlib>
#include <cmath>
#include <stack>
#include "delfem2/vec3.h"

#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

// =====================================
// below: unexposed 

namespace delfem2 {
namespace vec3 {

DFM2_INLINE bool MyIsnan(double x) { return x!=x; }

// evaluate cubic function
template <typename REAL>
DFM2_INLINE REAL EvaluateCubic(
    REAL x,
    REAL k0, REAL k1, REAL k2, REAL k3) // coefficient of cubic function
{
  return k0 + k1*x + k2*x*x + k3*x*x*x;
}
#ifndef DFM2_HEADER_ONLY
template float EvaluateCubic(float r2, float k0, float k1, float k2, float k3);
template double EvaluateCubic(double r2, double k0, double k1, double k2, double k3);
#endif


// find root of cubic function using bisection method
DFM2_INLINE double FindRootCubic_Bisect(
    double r0, double r1,
    double v0, double v1,
    double k0, double k1, double k2, double k3)
{
  assert( v0*v1 <= 0 );
  if( v0*v1 == 0 ){
    if( v0 == 0 ){ return r0; }
    else{ return r1; }
  }
  for(unsigned int itr=0;itr<15;itr++){
    const double r2 = 0.5*(r0+r1);
    const double v2 = EvaluateCubic(r2, k0,k1,k2,k3);
    if( v2 == 0 ){ return r2; }
    if( v0*v2 < 0 ){ r1 = r2; v1 = v2; }
    else{            r0 = r2; v0 = v2; }
  }
  return 0.5*(r0+r1);
}


// there is another impelemntation in quat.h so this is "static function"
// transform vector with quaternion
template <typename REAL>
DFM2_INLINE void MyQuatVec
 (REAL vo[],
  const REAL q[],
  const REAL vi[])
{
  REAL x2 = q[1] * q[1] * 2.0;
  REAL y2 = q[2] * q[2] * 2.0;
  REAL z2 = q[3] * q[3] * 2.0;
  REAL xy = q[1] * q[2] * 2.0;
  REAL yz = q[2] * q[3] * 2.0;
  REAL zx = q[3] * q[1] * 2.0;
  REAL xw = q[1] * q[0] * 2.0;
  REAL yw = q[2] * q[0] * 2.0;
  REAL zw = q[3] * q[0] * 2.0;
  vo[0] = (1.0 - y2 - z2)*vi[0] + (xy - zw      )*vi[1] + (zx + yw      )*vi[2];
  vo[1] = (xy + zw      )*vi[0] + (1.0 - z2 - x2)*vi[1] + (yz - xw      )*vi[2];
  vo[2] = (zx - yw      )*vi[0] + (yz + xw      )*vi[1] + (1.0 - x2 - y2)*vi[2];
}
#ifndef DFM2_HEADER_ONLY
template void MyQuatVec(float vo[], const float q[], const float vi[]);
template void MyQuatVec(double vo[], const double q[], const double vi[]);
#endif

// ----------------------

template <typename REAL>
DFM2_INLINE void MyMat4Vec3
 (REAL vo[3],
  const REAL M[16], const REAL vi[3])
{
  vo[0] = M[0*4+0]*vi[0] + M[0*4+1]*vi[1] + M[0*4+2]*vi[2];
  vo[1] = M[1*4+0]*vi[0] + M[1*4+1]*vi[1] + M[1*4+2]*vi[2];
  vo[2] = M[2*4+0]*vi[0] + M[2*4+1]*vi[1] + M[2*4+2]*vi[2];
}
#ifndef DFM2_HEADER_ONLY
template void MyMat4Vec3(float vo[3],
    const float M[16], const float vi[3]);
template void MyMat4Vec3(double vo[3],
    const double M[16], const double vi[3]);
#endif

// ----------------------

// there is formal implementation in quat.cpp so this is static to avoid dumplicated
template <typename REAL>
DFM2_INLINE void MyQuatConjVec(
    REAL vo[3],
    const REAL q[4],
    const REAL vi[3])
{
  REAL x2 = q[1] * q[1] * 2.0;
  REAL y2 = q[2] * q[2] * 2.0;
  REAL z2 = q[3] * q[3] * 2.0;
  REAL xy = q[1] * q[2] * 2.0;
  REAL yz = q[2] * q[3] * 2.0;
  REAL zx = q[3] * q[1] * 2.0;
  REAL xw = q[1] * q[0] * 2.0;
  REAL yw = q[2] * q[0] * 2.0;
  REAL zw = q[3] * q[0] * 2.0;
  vo[0] = (1.0 - y2 - z2)*vi[0] + (xy + zw      )*vi[1] + (zx - yw      )*vi[2];
  vo[1] = (xy - zw      )*vi[0] + (1.0 - z2 - x2)*vi[1] + (yz + xw      )*vi[2];
  vo[2] = (zx + yw      )*vi[0] + (yz - xw      )*vi[1] + (1.0 - x2 - y2)*vi[2];
//  vo[0] = (1.0 - y2 - z2)*vi[0] + (xy - zw      )*vi[1] + (zx + yw      )*vi[2];
//  vo[1] = (xy + zw      )*vi[0] + (1.0 - z2 - x2)*vi[1] + (yz - xw      )*vi[2];
//  vo[2] = (zx - yw      )*vi[0] + (yz + xw      )*vi[1] + (1.0 - x2 - y2)*vi[2];
}
#ifndef DFM2_HEADER_ONLY
template void MyQuatConjVec(float vo[3], const float q[4], const float vi[3]);
template void MyQuatConjVec(double vo[3], const double q[4], const double vi[3]);
#endif

// --------------------------

template <typename REAL>
DFM2_INLINE void MyInverse_Mat3
    (REAL Ainv[9],
     const REAL A[9])
{
  const REAL det =
      + A[0]*A[4]*A[8] + A[3]*A[7]*A[2] + A[6]*A[1]*A[5]
      - A[0]*A[7]*A[5] - A[6]*A[4]*A[2] - A[3]*A[1]*A[8];
  const REAL inv_det = 1.0/det;
  Ainv[0] = inv_det*(A[4]*A[8]-A[5]*A[7]);
  Ainv[1] = inv_det*(A[2]*A[7]-A[1]*A[8]);
  Ainv[2] = inv_det*(A[1]*A[5]-A[2]*A[4]);
  Ainv[3] = inv_det*(A[5]*A[6]-A[3]*A[8]);
  Ainv[4] = inv_det*(A[0]*A[8]-A[2]*A[6]);
  Ainv[5] = inv_det*(A[2]*A[3]-A[0]*A[5]);
  Ainv[6] = inv_det*(A[3]*A[7]-A[4]*A[6]);
  Ainv[7] = inv_det*(A[1]*A[6]-A[0]*A[7]);
  Ainv[8] = inv_det*(A[0]*A[4]-A[1]*A[3]);
}

template <typename T>
DFM2_INLINE void MyMatVec3
    (T y[3],
     const T m[9], const T x[3])
{
  y[0] = m[0]*x[0] + m[1]*x[1] + m[2]*x[2];
  y[1] = m[3]*x[0] + m[4]*x[1] + m[5]*x[2];
  y[2] = m[6]*x[0] + m[7]*x[1] + m[8]*x[2];
}

}
}

// ===========================================================

namespace delfem2 { // template specialization need to be done in the namespace
  
template <>
DFM2_INLINE double Distance3
 (const double p0[3],
  const double p1[3])
{
  return sqrt( (p1[0]-p0[0])*(p1[0]-p0[0]) + (p1[1]-p0[1])*(p1[1]-p0[1]) + (p1[2]-p0[2])*(p1[2]-p0[2]) );
}

template <>
DFM2_INLINE float Distance3
 (const float p0[3],
  const float p1[3])
{
  return sqrtf( (p1[0]-p0[0])*(p1[0]-p0[0]) + (p1[1]-p0[1])*(p1[1]-p0[1]) + (p1[2]-p0[2])*(p1[2]-p0[2]) );
}
  
}

// -------------------------

namespace delfem2 { // template specialization need to be done in the namespace

template <>
DFM2_INLINE double Length3(const double v[3]){
  return sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
}

template <>
DFM2_INLINE float Length3(const float v[3]){
  return sqrtf( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
}
  
}

// ---------------------------------

template <typename T>
DFM2_INLINE T delfem2::Dot3(const T a[3], const T b[3]){
  return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}
#ifndef DFM2_HEADER_ONLY
template float delfem2::Dot3(const float a[3], const float b[3]);
template double delfem2::Dot3(const double a[3], const double b[3]);
#endif


// ---------------------------------

template <typename REAL>
DFM2_INLINE void delfem2::Add3(
    REAL vo[3],
    const REAL vi[3])
{
  vo[0] += vi[0];
  vo[1] += vi[1];
  vo[2] += vi[2];
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::Add3(float vo[3], const float vi[3]);
template void delfem2::Add3(double vo[3], const double vi[3]);
#endif

// ---------------------------------

template <typename T>
T delfem2::Volume_Tet3
 (const T v1[3],
  const T v2[3],
  const T v3[3],
  const T v4[3])
{
  return
  ((v2[0]-v1[0])*((v3[1]-v1[1])*(v4[2]-v1[2])-(v4[1]-v1[1])*(v3[2]-v1[2]))
   -(v2[1]-v1[1])*((v3[0]-v1[0])*(v4[2]-v1[2])-(v4[0]-v1[0])*(v3[2]-v1[2]))
   +(v2[2]-v1[2])*((v3[0]-v1[0])*(v4[1]-v1[1])-(v4[0]-v1[0])*(v3[1]-v1[1]))
   ) * 0.16666666666666666666666666666667;
}
#ifndef DFM2_HEADER_ONLY
template float delfem2::Volume_Tet3(const float v1[3],
                                    const float v2[3],
                                    const float v3[3],
                                    const float v4[3]);
template double delfem2::Volume_Tet3(const double v1[3],
                                     const double v2[3],
                                     const double v3[3],
                                     const double v4[3]);
#endif

// ---------------------------------

template <typename T>
void delfem2::Cross3(T r[3], const T v1[3], const T v2[3]){
  r[0] = v1[1]*v2[2] - v2[1]*v1[2];
  r[1] = v1[2]*v2[0] - v2[2]*v1[0];
  r[2] = v1[0]*v2[1] - v2[0]*v1[1];
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::Cross3(float r[3], const float v1[3], const float v2[3]);
template void delfem2::Cross3(double r[3], const double v1[3], const double v2[3]);
#endif


// ---------------------------------

template <typename T>
DFM2_INLINE void delfem2::Normalize3(T v[3])
{
  double len = sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
  v[0] /= len;
  v[1] /= len;
  v[2] /= len;
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::Normalize3(float v[3]);
template void delfem2::Normalize3(double v[3]);
#endif

// ---------------------------------

template <typename T>
T delfem2::Area_Tri3(const T v1[3], const T v2[3], const T v3[3]){
  const T n[3] = {
    ( v2[1] - v1[1] )*( v3[2] - v1[2] ) - ( v3[1] - v1[1] )*( v2[2] - v1[2] ),
    ( v2[2] - v1[2] )*( v3[0] - v1[0] ) - ( v3[2] - v1[2] )*( v2[0] - v1[0] ),
    ( v2[0] - v1[0] )*( v3[1] - v1[1] ) - ( v3[0] - v1[0] )*( v2[1] - v1[1] ) };
  return 0.5*Length3(n);
}
#ifndef DFM2_HEADER_ONLY
template float delfem2::Area_Tri3(const float v1[3], const float v2[3], const float v3[3]);
template double delfem2::Area_Tri3(const double v1[3], const double v2[3], const double v3[3]);
#endif

// ----------------------------------

template <typename T>
DFM2_INLINE T delfem2::SquareDistance3(const T p0[3], const T p1[3]){
  return (p1[0]-p0[0])*(p1[0]-p0[0]) + (p1[1]-p0[1])*(p1[1]-p0[1]) + (p1[2]-p0[2])*(p1[2]-p0[2]);
}
#ifndef DFM2_HEADER_ONLY
template float delfem2::SquareDistance3(const float p0[3], const float p1[3]);
template double delfem2::SquareDistance3(const double p0[3], const double p1[3]);
#endif

// ------------------------------------

template <typename T>
DFM2_INLINE T delfem2::SquareLength3(const T v[3]){
  return v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
}
#ifndef DFM2_HEADER_ONLY
template float delfem2::SquareLength3(const float v[3]);
template double delfem2::SquareLength3(const double v[3]);
#endif

// ------------------------------------

template <typename T>
T delfem2::ScalarTripleProduct3(const T a[], const T b[], const T c[])
{
  return a[0]*(b[1]*c[2] - b[2]*c[1])
  +a[1]*(b[2]*c[0] - b[0]*c[2])
  +a[2]*(b[0]*c[1] - b[1]*c[0]);
}
#ifndef DFM2_HEADER_ONLY
template float delfem2::ScalarTripleProduct3(const float a[], const float b[], const float c[]);
template double delfem2::ScalarTripleProduct3(const double a[], const double b[], const double c[]);
#endif

// -----------------------------------------

template <typename REAL>
void delfem2::NormalTri3(
    REAL n[3],
    const REAL v1[3],
    const REAL v2[3],
    const REAL v3[3])
{
  n[0] = ( v2[1] - v1[1] )*( v3[2] - v1[2] ) - ( v3[1] - v1[1] )*( v2[2] - v1[2] );
  n[1] = ( v2[2] - v1[2] )*( v3[0] - v1[0] ) - ( v3[2] - v1[2] )*( v2[0] - v1[0] );
  n[2] = ( v2[0] - v1[0] )*( v3[1] - v1[1] ) - ( v3[0] - v1[0] )*( v2[1] - v1[1] );
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::NormalTri3(float n[3], const float v1[3], const float v2[3], const float v3[3]);
template void delfem2::NormalTri3(double n[3], const double v1[3], const double v2[3], const double v3[3]);
#endif

// ------------------------------------------

template <typename REAL>
void delfem2::UnitNormalAreaTri3(
    REAL n[3],
    REAL& a,
    const REAL v1[3], const REAL v2[3], const REAL v3[3])
{
  NormalTri3(n,
             v1, v2, v3);
  a = Length3(n)*0.5;//sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2])*0.5;
  const double invlen = 0.5/a;
  n[0]*=invlen;  n[1]*=invlen;  n[2]*=invlen;
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::UnitNormalAreaTri3(float n[3], float& a,
    const float v1[3], const float v2[3], const float v3[3]);
template void delfem2::UnitNormalAreaTri3(double n[3], double& a,
    const double v1[3], const double v2[3], const double v3[3]);
#endif

// ------------------------------------------

DFM2_INLINE void delfem2::GetNearest_LineSegPoint3D
(double pn[3],
 const double p[3], // point
 const double s[3], // source
 const double e[3]) // end
{
  const double d[3] = { e[0]-s[0], e[1]-s[1], e[2]-s[2] };
  double t = 0.5;
  if ( Dot3(d, d)>1.0e-20){
    const double ps[3] = { s[0]-p[0], s[1]-p[1], s[2]-p[2] };
    double a = Dot3(d, d);
    double b = Dot3(d, ps);
    t = -b/a;
    if (t<0) t = 0;
    if (t>1) t = 1;
  }
  pn[0] = s[0]+t*d[0];
  pn[1] = s[1]+t*d[1];
  pn[2] = s[2]+t*d[2];
}


DFM2_INLINE void delfem2::GetNearest_TrianglePoint3D
(double pn[3],
 double& r0, double& r1,
 const double ps[3], // origin point
 const double q0[3],
 const double q1[3],
 const double q2[3])
{
  double area, n012[3]; UnitNormalAreaTri3(n012, area, q0, q1, q2);
  const double pe[3] = { ps[0]+n012[0], ps[1]+n012[1], ps[2]+n012[2] };
  const double v012 = Volume_Tet3(ps, q0, q1, q2);
  if (fabs(v012) > 1.0e-10){
    const double sign = (v012 > 0) ? +1 : -1;
    const double v0 = Volume_Tet3(ps, q1, q2, pe)*sign;
    const double v1 = Volume_Tet3(ps, q2, q0, pe)*sign;
    const double v2 = Volume_Tet3(ps, q0, q1, pe)*sign;
    assert(fabs(v0+v1+v2) > 1.0e-10);
    double inv_v012 = 1.0/(v0+v1+v2);
    r0 = v0*inv_v012;
    r1 = v1*inv_v012;
    const double r2 = (1.0-r0-r1);
    const double tol = 1.0e-4;
    if (r0 > -tol && r1 > -tol && r2 > -tol){
      pn[0] = q0[0]*r0+q1[0]*r1+q2[0]*r2;
      pn[1] = q0[1]*r0+q1[1]*r1+q2[1]*r2;
      pn[2] = q0[2]*r0+q1[2]*r1+q2[2]*r2;
      return;
    }
  }
  double r12[3]; GetNearest_LineSegPoint3D(r12, ps, q1, q2);
  double r20[3]; GetNearest_LineSegPoint3D(r20, ps, q2, q0);
  double r01[3]; GetNearest_LineSegPoint3D(r01, ps, q0, q1);
  const double d12 = Distance3(r12, ps);
  const double d20 = Distance3(r20, ps);
  const double d01 = Distance3(r01, ps);
  if (d12 < d20){
    if (d12 < d01){ // 12 is the smallest
      pn[0] = r12[0];
      pn[1] = r12[1];
      pn[2] = r12[2];
      r0 = 0;
      r1 = Distance3(pn,q2)/Distance3(q1,q2);
      return;
    }
  }
  else{
    if (d20 < d01){ // d20 is the smallest
      pn[0] = r20[0];
      pn[1] = r20[1];
      pn[2] = r20[2];
      r0 = Distance3(pn,q2)/Distance3(q0,q2);
      r1 = 0;
      return;
    }
  }
  pn[0] = r01[0];
  pn[1] = r01[1];
  pn[2] = r01[2];
  r0 = Distance3(pn,q1)/Distance3(q0,q1);
  r1 = 1-r0;
}


DFM2_INLINE void delfem2::GetVertical2Vector3D
(const double vec_n[3],
 double vec_x[3],
 double vec_y[3])
{
  const double vec_s[3] = {0,1,0};
  Cross3(vec_x,vec_s,vec_n);
  const double len = Length3(vec_x);
  if( len < 1.0e-10 ){
    const double vec_t[3] = {1,0,0};
    Cross3(vec_x,vec_t,vec_n);  // z????
    Cross3(vec_y,vec_n,vec_x);  // x????
  }
  else{
    const double invlen = 1.0/len;
    vec_x[0] *= invlen;
    vec_x[1] *= invlen;
    vec_x[2] *= invlen;
    Cross3(vec_y,vec_n,vec_x);
  }
}

// ---------------------------------------------------------------------------

template <typename T>
T delfem2::Dot(const CVec3<T>& arg1, const CVec3<T>& arg2)
{
  return Dot3(arg1.p,arg2.p);
}
#ifndef DFM2_HEADER_ONLY
template float delfem2::Dot(const CVec3f& arg1, const CVec3f& arg2);
template double delfem2::Dot(const CVec3d& arg1, const CVec3d& arg2);
#endif
 
// ----------------------------

// cross product
template <typename T>
delfem2::CVec3<T> delfem2::Cross(const CVec3<T>& arg1, const CVec3<T>& arg2)
{
  CVec3<T> temp;
  Cross3(temp.p, arg1.p, arg2.p);
  return temp;
}
#ifndef DFM2_HEADER_ONLY
template delfem2::CVec3f delfem2::Cross(const CVec3f& arg1, const CVec3f& arg2);
template delfem2::CVec3d delfem2::Cross(const CVec3d& arg1, const CVec3d& arg2);
#endif

// ----------------------------

template <typename REAL>
void delfem2::AverageTwo3(
    REAL po[3],
    const REAL p0[3], const REAL p1[3])
{
  po[0] = (p0[0]+p1[0])*0.5;
  po[1] = (p0[1]+p1[1])*0.5;
  po[2] = (p0[2]+p1[2])*0.5;
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::AverageTwo3(float po[3], const float p0[3], const float p1[3]);
template void delfem2::AverageTwo3(double po[3], const double p0[3], const double p1[3]);
#endif

// ----------------------------

template <typename REAL>
void delfem2::AverageFour3(
    REAL po[3],
    const REAL p0[3], const REAL p1[3], const REAL p2[3], const REAL p3[3])
{
  po[0] = (p0[0]+p1[0]+p2[0]+p3[0])*0.25;
  po[1] = (p0[1]+p1[1]+p2[1]+p3[1])*0.25;
  po[2] = (p0[2]+p1[2]+p2[2]+p3[2])*0.25;
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::AverageFour3(float po[3],
    const float p0[3], const float p1[3], const float p2[3], const float p3[3]);
template void delfem2::AverageFour3(double po[3],
    const double p0[3], const double p1[3], const double p2[3], const double p3[3]);
#endif



// above: without CVec3
// --------------------------------------------------------------------------------------------
// below: with CVec3

namespace delfem2 {
  
/*
template <typename T>
CVec3<T> Cross(const CVec3<T>& arg1, const CVec3<T>& arg2)
{
  CVec3<T> temp;
  Cross3(temp.p, arg1.p, arg2.p);
  return temp;
}
template CVec3<double> Cross(const CVec3<double>& arg1, const CVec3<double>& arg2);
 */
  
// -----------------------------

//! add
template <typename T>
CVec3<T> operator+ (const CVec3<T>& lhs, const CVec3<T>& rhs){
  CVec3<T> temp = lhs;
  temp += rhs;
  return temp;
}
#ifndef DFM2_HEADER_ONLY
template CVec3f operator+ (const CVec3f& lhs, const CVec3f& rhs);
template CVec3d operator+ (const CVec3d& lhs, const CVec3d& rhs);
#endif
  
// ------------------

//! subtract
template <typename T>
CVec3<T> operator - (const CVec3<T>& lhs, const CVec3<T>& rhs){
  CVec3<T> temp = lhs;
  temp -= rhs;
  return temp;
}
#ifndef DFM2_HEADER_ONLY
template CVec3f operator - (const CVec3f& lhs, const CVec3f& rhs);
template CVec3d operator - (const CVec3d& lhs, const CVec3d& rhs);
#endif

  
// ------------------

//! scale
template <typename T>
CVec3<T> operator* (T d, const CVec3<T>& rhs){
  CVec3<T> temp = rhs;
  temp *= d;
  return temp;
}
#ifndef DFM2_HEADER_ONLY
template CVec3f operator* (float d, const CVec3f& rhs);
template CVec3d operator* (double d, const CVec3d& rhs);
#endif

//! scale
template <typename T>
CVec3<T> operator* (const CVec3<T>& vec, T d){
  CVec3<T> temp = vec;
  temp *= d;
  return temp;
}
#ifndef DFM2_HEADER_ONLY
template CVec3f operator* (const CVec3f& vec, float d);
template CVec3d operator* (const CVec3d& vec, double d);
#endif
  
// -----------------------

//! mult
template <typename T>
T operator* (const CVec3<T>& lhs, const CVec3<T>& rhs){
  return Dot(lhs,rhs);
}
#ifndef DFM2_HEADER_ONLY
template float operator* (const CVec3f& lhs, const CVec3f& rhs);
template double operator* (const CVec3d& lhs, const CVec3d& rhs);
#endif


//! divide by real number
template <typename T>
CVec3<T> operator/ (const CVec3<T>& vec, T d){
  CVec3<T> temp = vec;
  temp /= d;
  return temp;
}
#ifndef DFM2_HEADER_ONLY
template CVec3f operator/ (const CVec3f& vec, float d);
template CVec3d operator/ (const CVec3d& vec, double d);
#endif
  
// ----------------

//! mult
template <typename T>
CVec3<T> operator^ (const CVec3<T>& lhs, const CVec3<T>& rhs){
  return Cross(lhs,rhs);
}
#ifndef DFM2_HEADER_ONLY
template CVec3f operator^ (const CVec3f& lhs, const CVec3f& rhs);
template CVec3d operator^ (const CVec3d& lhs, const CVec3d& rhs);
#endif
  
// ------------------

template <typename T>
std::ostream &operator<<(std::ostream &output, const CVec3<T>& v)
{
  output.setf(std::ios::scientific);
  output << v.p[0] << " " << v.p[1] << " " << v.p[2];
  return output;
}
#ifndef DFM2_HEADER_ONLY
template std::ostream &operator<<(std::ostream &output, const CVec3d& v);
template std::ostream &operator<<(std::ostream &output, const CVec3f& v);
#endif
  
// ---------------------

template <typename T>
std::istream &operator>>(std::istream &input, CVec3<T>& v)
{
  input >> v.p[0] >> v.p[1] >> v.p[2];
  return input;
}
#ifndef DFM2_HEADER_ONLY
template std::istream &operator>>(std::istream &input, CVec3d& v);
template std::istream &operator>>(std::istream &input, CVec3f& v);
#endif
  
// ----------------------

template <typename T>
std::ostream &operator<<(std::ostream &output, const std::vector<CVec3<T>>& aV){
  output<<aV.size()<<std::endl;
  for (int iv = 0; iv<(int)aV.size(); ++iv){
    output<<"  "<<aV[iv]<<std::endl;
  }
  return output;
}

template <typename T>
std::istream &operator>>(std::istream &input, std::vector<CVec3<T>>& aV){
  int nV;  input>>nV; aV.resize(nV);
  for (int iv = 0; iv<nV; iv++){
    input>>aV[iv];
  }
  return input;
}
  
} // end namespace delfem2
  
// ------------------------------------------------------------------------------------------------------------------------------

template <typename T>
delfem2::CVec3<T> delfem2::Mat3Vec(const T mat[9], const CVec3<T>& v){
  CVec3<T> u;
  vec3::MyMatVec3(u.p, mat, v.p);
  return u;
}
#ifndef DFM2_HEADER_ONLY
template delfem2::CVec3<float> delfem2::Mat3Vec(
    const float mat[9], const CVec3<float>& v);
template delfem2::CVec3<double> delfem2::Mat3Vec(
    const double mat[9], const CVec3<double>& v);
#endif
  
// -------------------------

template <typename T>
delfem2::CVec3<T> delfem2::Mat4Vec(const T mat[16], const CVec3<T>& v)
{
  CVec3<T> u;
  vec3::MyMat4Vec3(u.p, mat, v.p);
  return u;
}
#ifndef DFM2_HEADER_ONLY
template delfem2::CVec3f delfem2::Mat4Vec(const float mat[16], const CVec3f& v);
template delfem2::CVec3d delfem2::Mat4Vec(const double mat[16], const CVec3d& v);
#endif
  
// ------------------------

template <typename T>
DFM2_INLINE delfem2::CVec3<T> delfem2::QuatVec
(const T quat[4],
 const CVec3<T>& v0)
{
  T v1a[3]; vec3::MyQuatVec(v1a,quat,v0.p);
  return CVec3<T>(v1a[0],v1a[1],v1a[2]);
}
#ifndef DFM2_HEADER_ONLY
template delfem2::CVec3f delfem2::QuatVec (const float quat[4], const CVec3f& v0);
template delfem2::CVec3d delfem2::QuatVec (const double quat[4], const CVec3d& v0);
#endif
  
// ----------------------------

template <typename REAL>
delfem2::CVec3<REAL> delfem2::QuatConjVec
(const REAL quat[4],
 const CVec3<REAL>& v0)
{
  REAL v1a[3];
  vec3::MyQuatConjVec(v1a,
                quat,v0.p);
  return CVec3<REAL>(v1a[0],v1a[1],v1a[2]);
}
#ifndef DFM2_HEADER_ONLY
template delfem2::CVec3f delfem2::QuatConjVec(const float quat[4], const CVec3f& v0);
template delfem2::CVec3d delfem2::QuatConjVec(const double quat[4], const CVec3d& v0);
#endif


// ------------------------------------------------------------

template <typename T>
double delfem2::ScalarTripleProduct
(const CVec3<T>& a,
 const CVec3<T>& b,
 const CVec3<T>& c)
{
//  return a.x*(b.y*c.z - b.z*c.y) + a.y*(b.z*c.x - b.x*c.z) + a.z*(b.x*c.y - b.y*c.x);
  return a.p[0]*(b.p[1]*c.p[2] - b.p[2]*c.p[1]) + a.p[1]*(b.p[2]*c.p[0] - b.p[0]*c.p[2]) + a.p[2]*(b.p[0]*c.p[1] - b.p[1]*c.p[0]);
}

// --------------------------

namespace delfem2 {
  
template <typename T>
bool operator== (const CVec3<T>& lhs, const CVec3<T>& rhs){
  if( fabs(lhs.p[0] - rhs.p[0]) < NEARLY_ZERO
     && fabs(lhs.p[1] - rhs.p[1]) < NEARLY_ZERO
     && fabs(lhs.p[2] - rhs.p[2]) < NEARLY_ZERO ){ return true; }
  else{ return false; }
}

template <typename T>
bool operator!= (const CVec3<T>& lhs, const CVec3<T>& rhs){
  return !(lhs == rhs);
}
  
} // namespace delfem2

  
// ----------------------------
  
template <typename T>
void delfem2::CVec3<T>::SetNormalizedVector()
{
  double invmag = 1.0/Length();
  p[0] *= invmag;
  p[1] *= invmag;
  p[2] *= invmag;
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::CVec3<float>::SetNormalizedVector();
template void delfem2::CVec3<double>::SetNormalizedVector();
#endif
  
// ----------------------------

template <typename T>
void delfem2::CVec3<T>::SetZero()
{
  p[0] = 0.0;
  p[1] = 0.0;
  p[2] = 0.0;
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::CVec3<float>::SetZero();
template void delfem2::CVec3<double>::SetZero();
#endif

  
// -----------------------
/*
template <typename T, typename DIST>
void delfem2::CVec3<T>::SetRandom(DIST dist)
{
  p[0] = dist();
  p[1] = dist();
  p[2] = dist();
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::CVec3<float>::SetRandom();
template void delfem2::CVec3<double>::SetRandom();
#endif
 */
  
// --------------------------------

//! Hight of a tetrahedra
template <typename T>
double delfem2::Height
(const CVec3<T>& v1,
 const CVec3<T>& v2,
 const CVec3<T>& v3,
 const CVec3<T>& v4)
{
  T n[3]; NormalTri3(n,
                     v1.p,v2.p,v3.p);
  Normalize3(n);
  return (v4.p[0]-v1.p[0])*n[0]+(v4.p[1]-v1.p[1])*n[1]+(v4.p[2]-v1.p[2])*n[2];
}
#ifndef DFM2_HEADER_ONLY
template double delfem2::Height(const CVec3f& v1,
                                const CVec3f& v2,
                                const CVec3f& v3,
                                const CVec3f& v4);
template double delfem2::Height(const CVec3d& v1,
                                const CVec3d& v2,
                                const CVec3d& v3,
                                const CVec3d& v4);
#endif

// -------------------------------------------------------------------------

template <typename T>
void delfem2::GetVertical2Vector
(const CVec3<T>& vec_n,
 CVec3<T>& vec_x,
 CVec3<T>& vec_y)
{
  vec_x = Cross(CVec3<T>(0,1,0),vec_n);
  const double len = vec_x.Length();
  if( len < 1.0e-10 ){
    vec_x = Cross(CVec3<T>(1,0,0),vec_n);  // z????
    vec_x.SetNormalizedVector();
    vec_y = Cross(vec_n,vec_x);  // x????
  }
  else{
    const double invlen = 1.0/len;
    vec_x *= invlen;
    vec_y = Cross(vec_n,vec_x);
  }
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::GetVertical2Vector(const CVec3f& vec_n,
                                          CVec3f& vec_x,
                                          CVec3f& vec_y);
template void delfem2::GetVertical2Vector(const CVec3d& vec_n,
                                          CVec3d& vec_x,
                                          CVec3d& vec_y);
#endif

// --------------------
    
template <typename REAL>
bool delfem2::IntersectRay_Tri3(
    REAL& r0, REAL& r1,
    const CVec3<REAL>& org, const CVec3<REAL>& dir,
    const CVec3<REAL>& p0,  const CVec3<REAL>& p1, const CVec3<REAL>& p2,
    REAL eps)
{
  const REAL v0 = Volume_Tet(p1, p2, org, org+dir);
  const REAL v1 = Volume_Tet(p2, p0, org, org+dir);
  const REAL v2 = Volume_Tet(p0, p1, org, org+dir);
  const REAL vt = v0+v1+v2;
  r0 = v0/vt;
  r1 = v1/vt;
  const REAL r2 = v2/vt;
  return (r0 >= -eps && r1 >= -eps && r2 >= -eps);
}
#ifndef DFM2_HEADER_ONLY
template bool delfem2::IntersectRay_Tri3(double& r0, double& r1,
    const CVec3d& org, const CVec3d& dir,
    const CVec3d& p0,  const CVec3d& p1, const CVec3d& p2,
    double eps);
template bool delfem2::IntersectRay_Tri3(float& r0, float& r1,
    const CVec3f& org, const CVec3f& dir,
    const CVec3f& p0,  const CVec3f& p1, const CVec3f& p2,
    float eps);
#endif

// --------------------------------------------------------

template <typename T>
delfem2::CVec3<T> delfem2::nearest_Line_Point
(const CVec3<T>& p, // point
 const CVec3<T>& s, // source
 const CVec3<T>& d) // direction
{
  assert( Dot(d,d) > 1.0e-20 );
  const CVec3<T> ps = s-p;
  T a = Dot(d,d);
  T b = Dot(d,s-p);
  T t = -b/a;
  return s+t*d;
}
#ifndef DFM2_HEADER_ONLY
template delfem2::CVec3d delfem2::nearest_Line_Point(
    const CVec3d& p, // point
    const CVec3d& s, // source
    const CVec3d& d); // direction
template delfem2::CVec3f delfem2::nearest_Line_Point(
    const CVec3f& p, // point
    const CVec3f& s, // source
    const CVec3f& d); // direction
#endif

// -------------------------------------------------------

template <typename T>
delfem2::CVec3<T> delfem2::nearest_Line_Point
(double& t,
 const CVec3<T>& p, // point
 const CVec3<T>& s, // source
 const CVec3<T>& d) // direction
{
  if( Dot(d,d) < 1.0e-20 ){
    t = 0;
    return s;
  }
  const CVec3<T> ps = s-p;
  double a = Dot(d,d);
  double b = Dot(d,s-p);
  t = -b/a;
  return s+t*d;
}

template <typename T>
delfem2::CVec3<T> delfem2::nearest_Origin_LineSeg
(const CVec3<T>& s, // start
 const CVec3<T>& e) // end
{
  CVec3<T> d = e-s;
  double a = Dot(d,d);
  if( a < 1.0e-20 ){ return (s+e)*0.5; }
  double b = Dot(d,s);
  double t = -b/a;
  if( t < 0 ) t = 0;
  if( t > 1 ) t = 1;
  return s+t*d;
}

// r0==0 -> p0==org
// r0==1 -> p1==org
template <typename T>
delfem2::CVec3<T> delfem2::nearest_Origin_LineSeg
(double& r0,
 const CVec3<T>& p0, // start
 const CVec3<T>& p1) // end
{
  CVec3<T> d = p1-p0;
  double a = Dot(d,d);
  if( a < 1.0e-20 ){
    r0=0.5;
    return (p0+p1)*0.5;
  }
  double b = Dot(d,p0);
  r0 = -b/a;
  if( r0 < 0 ) r0 = 0;
  if( r0 > 1 ) r0 = 1;
  return (1.0-r0)*p0+r0*p1;
}

template <typename T>
delfem2::CVec3<T> delfem2::nearest_LineSeg_Point
(const CVec3<T>& p, // point
 const CVec3<T>& s, // start
 const CVec3<T>& e) // end
{
  CVec3<T> d = e-s;
  if( Dot(d,d) < 1.0e-20 ){
    return (s+e)*0.5;
  }
  const CVec3<T> ps = s-p;
  double a = Dot(d,d);
  double b = Dot(d,s-p);
  double t = -b/a;
  if( t < 0 ) t = 0;
  if( t > 1 ) t = 1;
  return s+t*d;
}

template <typename T>
delfem2::CVec3<T> delfem2::nearest_LineSeg_Point
(double& t,
 const CVec3<T>& p, // point
 const CVec3<T>& s, // source
 const CVec3<T>& e) // end
{
  CVec3<T> d = e-s;
  if( Dot(d,d) < 1.0e-20 ){
    t = 0.5;
    return (1-t)*s + t*e;
  }
  const CVec3<T> ps = s-p;
  double a = Dot(d,d);
  double b = Dot(d,s-p);
  t = -b/a;
  if( t < 0 ) t = 0;
  if( t > 1 ) t = 1;
  return s+t*d;
}

// ---------------------------------------------

template <typename T>
void delfem2::nearest_LineSeg_Line
 (CVec3<T>& a, CVec3<T>& b,
  const CVec3<T>& ps, const CVec3<T>& pe,
  const CVec3<T>& pb_, const CVec3<T>& vb)
{
  T D0, Dta0, Dtb0;
  CVec3<T> Da0, Db0;
  nearest_Line_Line(D0, Da0, Db0, Dta0, Dtb0,
                    ps, pe-ps,
                    pb_, vb);
  if( abs(D0) < 1.0e-10 ){ // pararell
    a = (ps+pe)*(T)0.5;
    b = ::delfem2::nearest_Line_Point(a, pb_, vb);
    return;
  }
  T ta = Dta0/D0;
  if( ta > 0 && ta < 1 ){ // nearst point is inside the segment
    a = Da0/D0;
    b = Db0/D0;
    return;
  }
  //
  CVec3<T> p1 = nearest_Line_Point(ps,  pb_, vb);
  CVec3<T> p2 = nearest_Line_Point(pe,  pb_, vb);
  T Dist1 = (p1-ps).Length();
  T Dist2 = (p2-pe).Length();
  if( Dist1 < Dist2 ){
    a = ps;
    b = p1;
    return;
  }
  a = pe;
  b = p2;
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::nearest_LineSeg_Line
 (CVec3f& a, CVec3f& b,
 const CVec3f& ps, const CVec3f& pe,
  const CVec3f& pb_, const CVec3f& vb);
template void delfem2::nearest_LineSeg_Line
(CVec3d& a, CVec3d& b,
 const CVec3d& ps, const CVec3d& pe,
 const CVec3d& pb_, const CVec3d& vb);
#endif

// ---------------------------------------------

DFM2_INLINE double delfem2::Nearest_LineSeg_LineSeg_CCD_Iteration(
    double p[3],
    const CVec3d& p0s,
    const CVec3d& p0e,
    const CVec3d& p1s,
    const CVec3d& p1e,
    const CVec3d& q0s,
    const CVec3d& q0e,
    const CVec3d& q1s,
    const CVec3d& q1e,
    unsigned int nitr )
{
  CVec3d v0;
  for(unsigned int itr=0;itr<nitr;++itr) {
    const double s0 = p[0], t0 = p[1], u0 = p[2];
    v0 =
        + ((1 - s0) * (1 - u0)) * p0s + ((1 - s0) * u0) * p0e + (s0 * (1 - u0)) * p1s + (s0 * u0) * p1e
        - ((1 - t0) * (1 - u0)) * q0s - ((1 - t0) * u0) * q0e - (t0 * (1 - u0)) * q1s - (t0 * u0) * q1e;
//    std::cout << "   " << itr << " " << v0.Length() << "  " << p[0] << " " << p[1] << " " << p[2] << std::endl;
    const CVec3d ds = -(1 - u0) * p0s - u0 * p0e + (1 - u0) * p1s + u0 * p1e;
    const CVec3d dt = +(1 - u0) * q0s + u0 * q0e - (1 - u0) * q1s - u0 * q1e;
    const CVec3d du =
        - (1 - s0) * p0s + (1 - s0) * p0e - s0 * p1s + s0 * p1e
        + (1 - t0) * q0s - (1 - t0) * q0e + t0 * q1s - t0 * q1e;
    const CVec3d dsu = + p0s - p0e - p1s + p1e;
    const CVec3d dtu = - q0s + q0e + q1s - q1e;
    double R[3] = { v0*ds, v0*dt, v0*du };
    double A[9] = {
        ds*ds,        ds*dt,        ds*du+v0*dsu,
        dt*ds,        dt*dt,        dt*du*v0*dtu,
        du*ds+v0*dsu, du*dt+v0*dtu, du*du };
    {
      double eps = (A[0] + A[4] + A[8]) * 1.0e-10 + 1.0e-20;
      A[0] += eps;
      A[4] += eps;
      A[8] += eps;
    }
    double Ainv[9];
    vec3::MyInverse_Mat3(Ainv, A);
    double D[3];
    vec3::MyMatVec3(D, Ainv, R);
    p[0] -= D[0];
    p[1] -= D[1];
    p[2] -= D[2];
    if (p[0] < 0) { p[0] = 0.0; } else if (p[0] > 1) { p[0] = 1.0; }
    if (p[1] < 0) { p[1] = 0.0; } else if (p[1] > 1) { p[1] = 1.0; }
    if (p[2] < 0) { p[2] = 0.0; } else if (p[2] > 1) { p[2] = 1.0; }
  }
  return v0.Length();
}

// ---------------------------------------------

template <typename T>
void delfem2::nearest_Line_Line
(T& D, CVec3<T>& Da, CVec3<T>& Db,
 const CVec3<T>& pa_, const CVec3<T>& va,
 const CVec3<T>& pb_, const CVec3<T>& vb)
{
  T xaa = va*va;
  T xab = vb*va;
  T xbb = vb*vb;
  D = (xaa*xbb-xab*xab);
  T xac = va*(pb_-pa_);
  T xbc = vb*(pb_-pa_);
  T da = xbb*xac-xab*xbc;
  T db = xab*xac-xaa*xbc;
  Da = D*pa_+da*va;
  Db = D*pb_+db*vb;
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::nearest_Line_Line
 (float& D, CVec3f& Da, CVec3f& Db,
  const CVec3f& pa_, const CVec3f& va,
  const CVec3f& pb_, const CVec3f& vb);
template void delfem2::nearest_Line_Line
 (double& D, CVec3d& Da, CVec3d& Db,
  const CVec3d& pa_, const CVec3d& va,
  const CVec3d& pb_, const CVec3d& vb);
#endif

// ---------------------------------------------

template <typename T>
void delfem2::nearest_Line_Line
(T& D, CVec3<T>& Da, CVec3<T>& Db,
 T& Dta, T& Dtb,
 //
 const CVec3<T>& pa_, const CVec3<T>& va,
 const CVec3<T>& pb_, const CVec3<T>& vb)
{
  T xaa = va*va;
  T xab = vb*va;
  T xbb = vb*vb;
  D = (xaa*xbb-xab*xab);
  T xac = va*(pb_-pa_);
  T xbc = vb*(pb_-pa_);
  Dta = xbb*xac-xab*xbc;
  Dtb = xab*xac-xaa*xbc;
  Da = D*pa_+Dta*va;
  Db = D*pb_+Dtb*vb;
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::nearest_Line_Line
 (float& D, CVec3f& Da, CVec3f& Db,
  float& Dta, float& Dtb,
  const CVec3f& pa_, const CVec3f& va,
  const CVec3f& pb_, const CVec3f& vb);
template void delfem2::nearest_Line_Line
(double& D, CVec3d& Da, CVec3d& Db,
 double& Dta, double& Dtb,
 const CVec3d& pa_, const CVec3d& va,
 const CVec3d& pb_, const CVec3d& vb);
#endif

// ------------------------------------------

template <typename T>
delfem2::CVec3<T> delfem2::nearest_Plane_Point
(const CVec3<T>& p, // point
 const CVec3<T>& o, // origin
 const CVec3<T>& n) // normal
{
  const CVec3<T> n0  = n.Normalize();
  return p + ((o-p)*n0)*n0;
}

template <typename T>
delfem2::CVec3<T> delfem2::Nearest_Orgin_PlaneTri
(double& r0,
 double& r1,
 const CVec3<T>& q0,
 const CVec3<T>& q1,
 const CVec3<T>& q2)
{
  const CVec3<T> n1 = ((q1-q0)^(q2-q0)).Normalize();
  const double v0 = Volume_OrgTet(q1, q2, n1);
  const double v1 = Volume_OrgTet(q2, q0, n1);
  const double v2 = Volume_OrgTet(q0, q1, n1);
  assert( fabs(v0+v1+v2) > 1.0e-10 );
  double vt_inv = 1.0/(v0+v1+v2);
  double r2;
  r0 = v0*vt_inv;
  r1 = v1*vt_inv;
  r2 = v2*vt_inv;
  return q0*r0 + q1*r1 + q2*r2;
}

template <typename T>
delfem2::CVec3<T> delfem2::Nearest_Origin_Tri
(double& r0,
 double& r1,
 const CVec3<T>& q0,
 const CVec3<T>& q1,
 const CVec3<T>& q2)
{
  { // check on triangle
    CVec3<T> p012 = Nearest_Orgin_PlaneTri(r0,r1, q0,q1,q2);
    if( r0>0 && r1>0 && (1-r0-r1)>0 ){ return p012; }
  }
  CVec3<T> p_min = q0;
  double d_min = q0.Length();
  r0=1; r1=0;
  {
    double s2;
    CVec3<T> p12 = nearest_Origin_LineSeg(s2, q1, q2);
    const double d12 = p12.Length();
    if(d12<d_min){ d_min=d12; p_min=p12; r1=1-s2; r0=0; }
  }
  {
    double s0;
    CVec3<T> p20 = nearest_Origin_LineSeg(s0, q2, q0);
    const double d20 = p20.Length();
    if(d20<d_min){ d_min=d20; p_min=p20; r1=0; r0=s0; }
  }
  {
    double s1;
    CVec3<T> p01 = nearest_Origin_LineSeg(s1, q0, q1);
    const double d01 = p01.Length();
    if(d01<d_min){ d_min=d01; p_min=p01; r0=1-s1; r1=s1; }
  }
  return p_min;
}
#ifndef DFM2_HEADER_ONLY
template delfem2::CVec3d delfem2::Nearest_Origin_Tri
  (double& r0, double& r1,
   const CVec3d& q0, const CVec3d& q1, const CVec3d& q2);
#endif
  
// -------------------------------------------

template <typename T>
delfem2::CVec3<T> delfem2::nearst_Origin_Quad
(double& s0,
 double& s1,
 const CVec3<T>& q0,
 const CVec3<T>& q1,
 const CVec3<T>& q2,
 const CVec3<T>& q3)
{
  double dist_min = -1;
  CVec3<T> q_min;
  for(int ip=0;ip<5;++ip){
    double t0=0, t1=0;
    if(      ip == 0 ){ t0=0.0; t1=0.0; }
    else if( ip == 1 ){ t0=1.0; t1=0.0; }
    else if( ip == 2 ){ t0=1.0; t1=1.0; }
    else if( ip == 3 ){ t0=0.0; t1=1.0; }
    else if( ip == 4 ){ t0=0.5; t1=0.5; }
    CVec3<T> q;
    for(int itr=0;itr<4;++itr){
      CVec3<T> pq = (1-t0)*(1-t1)*q0 + t0*(1-t1)*q1 + t0*t1*q2 + (1-t0)*t1*q3;
      CVec3<T> dqt0 = -(1-t1)*q0 + (1-t1)*q1 + t1*q2 - t1*q3;
      CVec3<T> dqt1 = -(1-t0)*q0 - t0*q1 + t0*q2 + (1-t0)*q3;
      CVec3<T> ddqt0t1 = q0 - q1 + q2 - q3;
      double f0 = -dqt0*pq;
      double f1 = -dqt1*pq;
      double A00 = dqt0*dqt0;
      double A11 = dqt1*dqt1;
      double A01 = dqt1*dqt0 + ddqt0t1*pq;
      double det = A00*A11-A01*A01;
      double detinv = 1.0/det;
      double B00 = +A11*detinv;
      double B11 = +A00*detinv;
      double B01 = -A01*detinv;
      double d0 = B00*f0 + B01*f1;
      double d1 = B01*f0 + B11*f1;
      t0 += d0;
      t1 += d1;
    }
    double tol = 1.0e-4;
    if( t0 > -tol && t0 < 1.0+tol && t1 > -tol && t1 < 1.0+tol ){
      double d0 = q.Length();
      if( dist_min < 0 || d0 < dist_min ){
        dist_min = d0;
        s0 = t0;
        s1 = t1;
        q_min = q;
      }
    }
  }
  if( dist_min > 0 ){ return q_min; }
  //
  const CVec3<T> q01 = nearest_Origin_LineSeg(q0,q1);
  const double d01  = q01.Length();
  if( dist_min < 0 || d01 < dist_min ){
    dist_min = d01;
    s0 = Distance(q01,q0)/Distance(q0,q1);
    s1 = 0.0;
    q_min = q01;
  }
  //
  CVec3<T> q12 = nearest_Origin_LineSeg(q1,q2);
  const double d12  = q12.Length();
  if( dist_min < 0 || d12 < dist_min ){
    dist_min = d12;
    s0 = 1.0;
    s1 = Distance(q12,q1)/Distance(q1,q2);
    q_min = q12;
  }
  //
  CVec3<T> q23 = nearest_Origin_LineSeg(q2,q3);
  const double d23  = q23.Length();
  if( dist_min < 0 || d23 < dist_min ){
    dist_min = d23;
    s0 = Distance(q23,q3)/Distance(q2,q3);
    s1 = 1.0;
    q_min = q23;
  }
  //
  CVec3<T> q30 = nearest_Origin_LineSeg(q3,q0);
  const double d30  = q30.Length();
  if( dist_min < 0 || d30 < dist_min ){
    dist_min = d30;
    s0 = 0.0;
    s1 = Distance(q30,q0)/Distance(q3,q0);
    q_min = q30;
  }
  return q_min;
}

/*
CVector3 nearest_Origin_Tet
(double& r0, double& r1, double& r2,
 const CVector3& q0,
 const CVector3& q1,
 const CVector3& q2,
 const CVector3& q3)
{
  CVector3 p_min = q0;
  {
    bool res = barycentricCoord_Origin_Tet(r0, r1, r2, q0, q1, q2, q3);
    p_min = r0*q0 + r1*q1 + r2*q2 + (1-r0-r1-r2)*q3;
    if( r0>0 && r1>0 && r2>0 && (1-r0-r1-r2)>0 ){ return p_min; }
  }
  ////////////////////////
  double r3;
  { // face123
    r0 = 0;
    p_min = nearest_Orgin_PlaneTri(r1,r2,r3, q1,q2,q3);
    if( r1>0 && r2>0 && r3>0 ){ return p_min; }
  }
  { // face230
    r1 = 0;
    p_min = nearest_Orgin_PlaneTri(r2,r3,r0, q2,q3,q0);
    if( r2>0 && r3>0 && r0>0 ){ return p_min; }
  }
  { // face301
    r2 = 0;
    p_min = nearest_Orgin_PlaneTri(r3,r0,r1, q3,q0,q1);
    if( r3>0 && r0>0 && r1>0 ){ return p_min; }
  }
  { // face012
    r3 = 0;
    p_min = nearest_Orgin_PlaneTri(r0,r1,r2, q0,q1,q2);
    if( r0>0 && r1>0 && r2>0 ){ return p_min; }
  }
  ////////////////////////
  double d_min = q0.Length();
  double s0,s1,s2,s3;
  { // edge01
    CVector3 p01 = nearest_Origin_LineSeg(s0,s1,q0,q1);
    double d01 = p01.Length();
    if( d01<d_min ){ d_min=d01; p_min=p01; r0=s0; r1=s1; r2=0; r3=0; }
  }
  { // edge02
    CVector3 p02 = nearest_Origin_LineSeg(s0,s2,q0,q2);
    double d02 = p02.Length();
    if( d02<d_min ){ d_min=d02; p_min=p02; r0=s0; r1=0; r2=s2; r3=0; }
  }
  { // edge03
    CVector3 p03 = nearest_Origin_LineSeg(s0,s3,q0,q3);
    double d03 = p03.Length();
    if( d03<d_min ){ d_min=d03; p_min=p03; r0=s0; r1=0; r2=0; r3=s3; }
  }
  { // edge12
    CVector3 p12 = nearest_Origin_LineSeg(s1,s2,q1,q2);
    double d12 = p12.Length();
    if( d12<d_min ){ d_min=d12; p_min=p12; r0=0; r1=s1; r2=s2; r3=0; }
  }
  { // edge13
    CVector3 p13 = nearest_Origin_LineSeg(s1,s3,q1,q3);
    double d13 = p13.Length();
    if( d13<d_min ){ d_min=d13; p_min=p13; r0=0; r1=s1; r2=0; r3=s3; }
  }
  { // edge23
    CVector3 p23 = nearest_Origin_LineSeg(s2,s3,q2,q3);
    double d23 = p23.Length();
    if( d23<d_min ){ d_min=d23; p_min=p23; r0=0; r1=0; r2=s2; r3=s3; }
  }
  return p_min;
}
 */

template <typename T>
void delfem2::Nearest_Line_Circle
(CVec3<T>& p0,
 CVec3<T>& q0,
 const CVec3<T>& src,
 const CVec3<T>& dir,
 const CVec3<T>& org, // center of the circle
 const CVec3<T>& normal, // normal of the circle
 T rad)
{
  const int nitr = 4;
  // ---------------------------------------
  CVec3<T> ex,ey; GetVertical2Vector(normal, ex, ey);
  T u0;
  {
    if( fabs(dir*normal)>fabs((org-src)*normal)*1.0e-4 ){
      u0 = ((org-src)*normal)/(dir*normal);
    }
    else{
      u0 = (org-src)*dir/(dir*dir);
    }
  }
  for(int itr=0;itr<nitr;++itr){
    p0 = src+u0*dir;
    double t0 = atan2(ey*(p0-org),ex*(p0-org));
    q0 = (T)(rad*cos(t0))*ex + (T)(rad*sin(t0))*ey + org;
    u0 = (q0-src)*dir/(dir*dir);
  }
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::Nearest_Line_Circle(CVec3f& p0,
                                           CVec3f& q0,
                                           const CVec3f& src,
                                           const CVec3f& dir,
                                           const CVec3f& org, // center of the circle
                                           const CVec3f& normal, // normal of the circle
                                           float rad);
template void delfem2::Nearest_Line_Circle(CVec3d& p0,
                                           CVec3d& q0,
                                           const CVec3d& src,
                                           const CVec3d& dir,
                                           const CVec3d& org, // center of the circle
                                           const CVec3d& normal, // normal of the circle
                                           double rad);
#endif

// -----------------------------------------------------

template <typename T>
bool delfem2::intersection_Plane_Line
(CVec3<T>& p0, double& r0, double& r1, double& r2,
 double eps,
 const CVec3<T>& src, const CVec3<T>& dir,
 const CVec3<T>& q0, const CVec3<T>& q1, const CVec3<T>& q2)
{
  r0 = Volume_Tet(src, src+dir, q1, q2);
  r1 = Volume_Tet(src, src+dir, q2, q0);
  r2 = Volume_Tet(src, src+dir, q0, q1);
  double v012 = (r0+r1+r2);
  double v012_inv = 1.0/v012;
  r0 *= v012_inv;
  r1 *= v012_inv;
  r2 *= v012_inv;
  p0 = r0*q0 + r1*q1 + r2*q2;
  return r0 > eps && r1 > eps && r2 > eps;
}
#ifndef DFM2_HEADER_ONLY
template bool delfem2::intersection_Plane_Line(CVec3d& p0, double& r0, double& r1, double& r2,
                                            double eps,
                                            const CVec3d& src, const CVec3d& dir,
                                            const CVec3d& q0, const CVec3d& q1, const CVec3d& q2);
#endif


template <typename T>
delfem2::CVec3<T> delfem2::intersection_Plane_Line
(const CVec3<T>& o, // one point on plane
 const CVec3<T>& n, // plane normal
 const CVec3<T>& s, // one point on line
 const CVec3<T>& d) // direction of line
{
  double t = ((o-s)*n)/(d*n);
  return s + t*d;
}
#ifndef DFM2_HEADER_ONLY
template delfem2::CVec3d delfem2::intersection_Plane_Line(const CVec3d& o, // one point on plane
                                                    const CVec3d& n, // plane normal
                                                    const CVec3d& s, // one point on line
                                                    const CVec3d& d); // direction of line
#endif
  
template <typename T>
void delfem2::iteration_intersection_Line_Quad
(double& t0, double& t1,
 const CVec3<T>& src,
 const CVec3<T>& u,
 const CVec3<T>& v,
 const CVec3<T>& q0,
 const CVec3<T>& q1,
 const CVec3<T>& q2,
 const CVec3<T>& q3)
{
  CVec3<T> q = (1-t0)*(1-t1)*q0 + t0*(1-t1)*q1 + t0*t1*q2 + (1-t0)*t1*q3;
  CVec3<T> pq = q-src;
  CVec3<T> dqt0 = -(1-t1)*q0 + (1-t1)*q1 + t1*q2 - t1*q3;
  CVec3<T> dqt1 = -(1-t0)*q0 - t0*q1 + t0*q2 + (1-t0)*q3;
  CVec3<T> ddqt0t1 = q0 - q1 + q2 - q3;
  double f0 = -u*pq;
  double f1 = -v*pq;
  double A00 = u*dqt0;
  double A01 = u*dqt1;
  double A10 = v*dqt0;
  double A11 = v*dqt1;
  double det = A00*A11-A01*A10;
  double detinv = 1.0/det;
  double B00 = +A11*detinv;
  double B01 = -A01*detinv;
  double B10 = -A10*detinv;
  double B11 = +A00*detinv;
  double d0 = B00*f0 + B01*f1;
  double d1 = B10*f0 + B11*f1;
  t0 += d0;
  t1 += d1;
}
  
template <typename T>
bool delfem2::intersection_Point_Quad
(CVec3<T>& psec, double& s0, double& s1,
 const CVec3<T>& src, const CVec3<T>& dir,
 const CVec3<T>& q0, const CVec3<T>& q1, const CVec3<T>& q2, const CVec3<T>& q3)
{
  CVec3<T> u,v; GetVertical2Vector(dir, u, v);
  //
  double dist_min = -1;
  CVec3<T> q_min;
  for(int ip=0;ip<5;++ip){
    double t0=0, t1=0;
    if(      ip == 0 ){ t0=0.0; t1=0.0; }
    else if( ip == 1 ){ t0=1.0; t1=0.0; }
    else if( ip == 2 ){ t0=1.0; t1=1.0; }
    else if( ip == 3 ){ t0=0.0; t1=1.0; }
    else if( ip == 4 ){ t0=0.5; t1=0.5; }
    for(int itr=0;itr<4;++itr){
      iteration_intersection_Line_Quad(t0,t1,
                                       src, u, v,
                                       q0,q1,q2,q3);
    }
    CVec3<T> q = (1-t0)*(1-t1)*q0 + t0*(1-t1)*q1 + t0*t1*q2 + (1-t0)*t1*q3;
    double tol = 1.0e-4;
//    std::cout << t0 << " " << t1 << std::endl;
    if( t0 > -tol && t0 < 1.0+tol && t1 > -tol && t1 < 1.0+tol ){
      double d0 = (q-src).Length();
      if( dist_min < 0 || d0 < dist_min ){
        dist_min = d0;
        s0 = t0;
        s1 = t1;
        q_min = q;
      }
    }
  }
//  std::cout << dist_min << std::endl;
  if( dist_min > 0 ){
    psec = q_min;
    return true;
  }
  return false;
}

// ------------------------------------------------------------------------------

template <typename T>
delfem2::CVec3<T> delfem2::positionBarycentricCoord_Pyramid
(double r0,
 double r1,
 double r2,
 const CVec3<T>& p0,
 const CVec3<T>& p1,
 const CVec3<T>& p2,
 const CVec3<T>& p3,
 const CVec3<T>& p4)
{
  return (1.0-r2)*(1.0-r0)*(1.0-r1)*p0
  + (1.0-r2)*r0*(1.0-r1)*p1
  + (1.0-r2)*r0*r1*p2
  + (1.0-r2)*(1.0-r0)*r1*p3
  + r2*p4;
}

template <typename T>
delfem2::CVec3<T> delfem2::positionBarycentricCoord_Wedge
(double r0,
 double r1,
 double r2,
 const CVec3<T>& p0,
 const CVec3<T>& p1,
 const CVec3<T>& p2,
 const CVec3<T>& p3,
 const CVec3<T>& p4,
 const CVec3<T>& p5)
{
  return (1.0-r2)*r0*p0
  + (1.0-r2)*r1*p1
  + (1.0-r2)*(1.0-r0-r1)*p2
  + r2*r0*p3
  + r2*r1*p4
  + r2*(1.0-r0-r1)*p5;
}

template <typename T>
void delfem2::iteration_barycentricCoord_Origin_Solid
(double& r0,
 double& r1,
 double& r2,
 const CVec3<T>& q, // q=positionBarycentricCoord_Wedge
 const CVec3<T>& dpdr0,
 const CVec3<T>& dpdr1,
 const CVec3<T>& dpdr2,
 double damp)
{
  const double cxx = dpdr0.p[0]*dpdr0.p[0] + dpdr1.p[0]*dpdr1.p[0] + dpdr2.p[0]*dpdr2.p[0];
  const double cxy = dpdr0.p[0]*dpdr0.p[1] + dpdr1.p[0]*dpdr1.p[1] + dpdr2.p[0]*dpdr2.p[1];
  const double cxz = dpdr0.p[0]*dpdr0.p[2] + dpdr1.p[0]*dpdr1.p[2] + dpdr2.p[0]*dpdr2.p[2];
  const double cyy = dpdr0.p[1]*dpdr0.p[1] + dpdr1.p[1]*dpdr1.p[1] + dpdr2.p[1]*dpdr2.p[1];
  const double cyz = dpdr0.p[1]*dpdr0.p[2] + dpdr1.p[1]*dpdr1.p[2] + dpdr2.p[1]*dpdr2.p[2];
  const double czz = dpdr0.p[2]*dpdr0.p[2] + dpdr1.p[2]*dpdr1.p[2] + dpdr2.p[2]*dpdr2.p[2];
  double C[9] = {cxx,cxy,cxz, cxy,cyy,cyz, cxz,cyz,czz};
  double Cinv[9]; vec3::MyInverse_Mat3(Cinv, C);
  const CVec3<T> d = damp*Mat3Vec(Cinv,q);
  r0 -= dpdr0*d;
  r1 -= dpdr1*d;
  r2 -= dpdr2*d;
}

template <typename T>
bool delfem2::barycentricCoord_Origin_Tet
(double& r0,
 double& r1,
 double& r2,
 const CVec3<T>& p0,
 const CVec3<T>& p1,
 const CVec3<T>& p2,
 const CVec3<T>& p3)
{
  double v0 = Volume_OrgTet(p1, p2, p3);
  double v1 = Volume_OrgTet(p2, p0, p3);
  double v2 = Volume_OrgTet(p1, p3, p0);
  double v3 = Volume_OrgTet(p1, p0, p2);
  double vt_inv = 1.0/(v0+v1+v2+v3);
  r0 = v0*vt_inv;
  r1 = v1*vt_inv;
  r2 = v2*vt_inv;
  return true;
}

template <typename T>
bool delfem2::barycentricCoord_Origin_Pyramid
(double& r0,
 double& r1,
 double& r2,
 const CVec3<T>& p0,
 const CVec3<T>& p1,
 const CVec3<T>& p2,
 const CVec3<T>& p3,
 const CVec3<T>& p4)
{
  CVec3<T> q = positionBarycentricCoord_Pyramid(r0,r1,r2, p0,p1,p2,p3,p4);
  for(int itr=0;itr<5;++itr){
    const CVec3<T> dpdr0 = -(1.0-r2)*(1.0-r1)*p0 + (1.0-r2)*(1.0-r1)*p1 + (1.0-r2)*r1*p2 - (1.0-r2)*r1*p3;
    const CVec3<T> dpdr1 = -(1.0-r2)*(1.0-r0)*p0 - (1.0-r2)*r0*p1 + (1.0-r2)*r0*p2 + (1.0-r2)*(1.0-r0)*p3;
    const CVec3<T> dpdr2 = -(1.0-r0)*(1.0-r1)*p0 - r0*(1.0-r1)*p1 - r0*r1*p2 - (1.0-r0)*r1*p3 + p4;
    iteration_barycentricCoord_Origin_Solid(r0,r1,r2, q,
                                            dpdr0,dpdr1,dpdr2,
                                            1.0);
    q = positionBarycentricCoord_Pyramid(r0,r1,r2, p0,p1,p2,p3,p4);
  }
  return true;
}

template <typename T>
bool delfem2::barycentricCoord_Origin_Wedge
(double& r0,
 double& r1,
 double& r2,
 const CVec3<T>& p0,
 const CVec3<T>& p1,
 const CVec3<T>& p2,
 const CVec3<T>& p3,
 const CVec3<T>& p4,
 const CVec3<T>& p5)
{
  CVec3<T> q = positionBarycentricCoord_Wedge(r0,r1,r2, p0,p1,p2,p3,p4,p5);
  for(int itr=0;itr<5;++itr){
    const CVec3<T> dpdr0 = (1.0-r2)*(p0-p2)+r2*(p3-p5);
    const CVec3<T> dpdr1 = (1.0-r2)*(p1-p2)+r2*(p4-p5);
    const CVec3<T> dpdr2 = r0*(p3-p0)+r1*(p4-p1)+(1.0-r0-r1)*(p5-p2);
    iteration_barycentricCoord_Origin_Solid(r0,r1,r2, q,
                                            dpdr0,dpdr1,dpdr2,
                                            1.0);
    q = positionBarycentricCoord_Wedge(r0,r1,r2, p0,p1,p2,p3,p4,p5);
  }
  return true;
}

template <typename T>
bool delfem2::IsInside_Orgin_BoundingBoxPoint6
(const CVec3<T>& p0,
 const CVec3<T>& p1,
 const CVec3<T>& p2,
 const CVec3<T>& p3,
 const CVec3<T>& p4,
 const CVec3<T>& p5)
{
  if( p0.p[0]>0 && p1.p[0]>0 && p2.p[0]>0 && p3.p[0]>0 && p4.p[0]>0 && p5.p[0]>0 ){ return false; }
  if( p0.p[0]<0 && p1.p[0]<0 && p2.p[0]<0 && p3.p[0]<0 && p4.p[0]<0 && p5.p[0]<0 ){ return false; }
  if( p0.p[1]>0 && p1.p[1]>0 && p2.p[1]>0 && p3.p[1]>0 && p4.p[1]>0 && p5.p[1]>0 ){ return false; }
  if( p0.p[1]<0 && p1.p[1]<0 && p2.p[1]<0 && p3.p[1]<0 && p4.p[1]<0 && p5.p[1]<0 ){ return false; }
  if( p0.p[2]>0 && p1.p[2]>0 && p2.p[2]>0 && p3.p[2]>0 && p4.p[2]>0 && p5.p[2]>0 ){ return false; }
  if( p0.p[2]<0 && p1.p[2]<0 && p2.p[2]<0 && p3.p[2]<0 && p4.p[2]<0 && p5.p[2]<0 ){ return false; }
  return true;
}

template <typename T>
bool delfem2::IsInside_Orgin_BoundingBoxPoint5
(const CVec3<T>& p0,
 const CVec3<T>& p1,
 const CVec3<T>& p2,
 const CVec3<T>& p3,
 const CVec3<T>& p4)
{
  if( p0.p[0]>0 && p1.p[0]>0 && p2.p[0]>0 && p3.p[0]>0 && p4.p[0]>0 ){ return false; }
  if( p0.p[0]<0 && p1.p[0]<0 && p2.p[0]<0 && p3.p[0]<0 && p4.p[0]<0 ){ return false; }
  if( p0.p[1]>0 && p1.p[1]>0 && p2.p[1]>0 && p3.p[1]>0 && p4.p[1]>0 ){ return false; }
  if( p0.p[1]<0 && p1.p[1]<0 && p2.p[1]<0 && p3.p[1]<0 && p4.p[1]<0 ){ return false; }
  if( p0.p[2]>0 && p1.p[2]>0 && p2.p[2]>0 && p3.p[2]>0 && p4.p[2]>0 ){ return false; }
  if( p0.p[2]<0 && p1.p[2]<0 && p2.p[2]<0 && p3.p[2]<0 && p4.p[2]<0 ){ return false; }
  return true;
}

template <typename T>
bool delfem2::IsInside_Orgin_BoundingBoxPoint4
(const CVec3<T>& p0,
 const CVec3<T>& p1,
 const CVec3<T>& p2,
 const CVec3<T>& p3)
{
  if( p0.p[0]>0 && p1.p[0]>0 && p2.p[0]>0 && p3.p[0]>0 ){ return false; }
  if( p0.p[0]<0 && p1.p[0]<0 && p2.p[0]<0 && p3.p[0]<0 ){ return false; }
  if( p0.p[1]>0 && p1.p[1]>0 && p2.p[1]>0 && p3.p[1]>0 ){ return false; }
  if( p0.p[1]<0 && p1.p[1]<0 && p2.p[1]<0 && p3.p[1]<0 ){ return false; }
  if( p0.p[2]>0 && p1.p[2]>0 && p2.p[2]>0 && p3.p[2]>0 ){ return false; }
  if( p0.p[2]<0 && p1.p[2]<0 && p2.p[2]<0 && p3.p[2]<0 ){ return false; }
  return true;
}

// ----------------------------------------------------------------------

// distance VF
template <typename T>
double delfem2::DistanceFaceVertex
(const CVec3<T>& p0, const CVec3<T>& p1, const CVec3<T>& p2,
 const CVec3<T>& p3,
 double& w0, double& w1)
{
  CVec3<T> v20 =p0-p2;
  CVec3<T> v21 =p1-p2;
  double t0 = Dot(v20,v20);
  double t1 = Dot(v21,v21);
  double t2 = Dot(v20,v21);
  double t3 = Dot(v20,p3-p2);
  double t4 = Dot(v21,p3-p2);
  double det = t0*t1-t2*t2;
  double invdet = 1.0/det;
  w0 = (+t1*t3-t2*t4)*invdet;
  w1 = (-t2*t3+t0*t4)*invdet;
  const double w2 = 1-w0-w1;
  CVec3<T> pw = w0*p0 + w1*p1 + w2*p2;
  return (pw-p3).Length();
}
#ifndef DFM2_HEADER_ONLY
template double delfem2::DistanceFaceVertex
  (const CVec3d& p0, const CVec3d& p1,
   const CVec3d& p2, const CVec3d& p3,
   double& w0, double& w1);
#endif

//　distance EE
template <typename T>
double delfem2::DistanceEdgeEdge
(const CVec3<T>& p0, const CVec3<T>& p1,
 const CVec3<T>& q0, const CVec3<T>& q1,
 double& ratio_p, double& ratio_q)
{
  const CVec3<T>& vp =p1-p0;
  const CVec3<T>& vq =q1-q0;
  if( Cross(vp,vq).Length() < 1.0e-10 ){ // handling parallel edge
    CVec3<T> pq0 = p0-q0;
    CVec3<T> nvp = vp; nvp.SetNormalizedVector();
    CVec3<T> vert = pq0 - Dot(pq0,nvp)*nvp;
    double dist = vert.Length();
    double lp0 = Dot(p0,nvp);
    double lp1 = Dot(p1,nvp);
    double lq0 = Dot(q0,nvp);
    double lq1 = Dot(q1,nvp);
    double p_min  = ( lp0 < lp1 ) ? lp0 : lp1;
    double p_max  = ( lp0 > lp1 ) ? lp0 : lp1;
    double q_min  = ( lq0 < lq1 ) ? lq0 : lq1;
    double q_max  = ( lq0 > lq1 ) ? lq0 : lq1;
    double lm;
    if(      p_max < q_min ){ lm = (p_max+q_min)*0.5; }
    else if( q_max < p_min ){ lm = (q_max+p_min)*0.5; }
    else if( p_max < q_max ){ lm = (p_max+q_min)*0.5; }
    else{                     lm = (q_max+p_min)*0.5; }
    ratio_p = (lm-lp0)/(lp1-lp0);
    ratio_q = (lm-lq0)/(lq1-lq0);
    return dist;
  }
  double t0 = Dot(vp,vp);
  double t1 = Dot(vq,vq);
  double t2 = Dot(vp,vq);
  double t3 = Dot(vp,q0-p0);
  double t4 = Dot(vq,q0-p0);
  double det = t0*t1-t2*t2;
  double invdet = 1.0/det;
  ratio_p = (+t1*t3-t2*t4)*invdet;
  ratio_q = (+t2*t3-t0*t4)*invdet;
  CVec3<T> pc = p0 + ratio_p*vp;
  CVec3<T> qc = q0 + ratio_q*vq;
  return (pc-qc).Length();
}
#ifndef DFM2_HEADER_ONLY
template double delfem2::DistanceEdgeEdge
  (const CVec3d& p0, const CVec3d& p1,
   const CVec3d& q0, const CVec3d& q1,
   double& ratio_p, double& ratio_q);
#endif

// EEの距離が所定の距離以下にあるかどうか
template <typename T>
bool delfem2::IsContact_EE_Proximity
(int ino0,
 int ino1,
 int jno0,
 int jno1,
 const CVec3<T>& p0,
 const CVec3<T>& p1,
 const CVec3<T>& q0,
 const CVec3<T>& q1,
 const double delta)
{
  if( ino0 == jno0 || ino0 == jno1 || ino1 == jno0 || ino1 == jno1 ) return false;
  if( q0.p[0]+delta < p0.p[0] && q0.p[0]+delta < p1.p[0] && q1.p[0]+delta < p0.p[0] && q1.p[0]+delta < p1.p[0] ) return false;
  if( q0.p[0]-delta > p0.p[0] && q0.p[0]-delta > p1.p[0] && q1.p[0]-delta > p0.p[0] && q1.p[0]-delta > p1.p[0] ) return false;
  if( q0.p[1]+delta < p0.p[1] && q0.p[1]+delta < p1.p[1] && q1.p[1]+delta < p0.p[1] && q1.p[1]+delta < p1.p[1] ) return false;
  if( q0.p[1]-delta > p0.p[1] && q0.p[1]-delta > p1.p[1] && q1.p[1]-delta > p0.p[1] && q1.p[1]-delta > p1.p[1] ) return false;
  if( q0.p[2]+delta < p0.p[2] && q0.p[2]+delta < p1.p[2] && q1.p[2]+delta < p0.p[2] && q1.p[2]+delta < p1.p[2] ) return false;
  if( q0.p[2]-delta > p0.p[2] && q0.p[2]-delta > p1.p[2] && q1.p[2]-delta > p0.p[2] && q1.p[2]-delta > p1.p[2] ) return false;
  double ratio_p, ratio_q;
  double dist = DistanceEdgeEdge(p0, p1, q0, q1, ratio_p, ratio_q);
  if( dist > delta ) return false;
  if( ratio_p < 0 ) return false;
  if( ratio_p > 1 ) return false;
  if( ratio_q < 0 ) return false;
  if( ratio_q > 1 ) return false;
  const CVec3<T>& pm = (1-ratio_p)*p0 + ratio_p*p1;
  const CVec3<T>& qm = (1-ratio_q)*q0 + ratio_q*q1;
  return (pm - qm).Length() <= delta;
}
#ifndef DFM2_HEADER_ONLY
template bool delfem2::IsContact_EE_Proximity
  (int ino0,
   int ino1,
   int jno0,
   int jno1,
   const CVec3d& p0,
   const CVec3d& p1,
   const CVec3d& q0,
   const CVec3d& q1,
   const double delta);
#endif

// compute time where four points gets coplaner
template <typename T>
bool delfem2::FindCoplanerInterp
(double& r,
 const CVec3<T>& s0, const CVec3<T>& s1, const CVec3<T>& s2, const CVec3<T>& s3,
 const CVec3<T>& e0, const CVec3<T>& e1, const CVec3<T>& e2, const CVec3<T>& e3)
{
  const CVec3<T> x1 = s1-s0;
  const CVec3<T> x2 = s2-s0;
  const CVec3<T> x3 = s3-s0;
  const CVec3<T> v1 = e1-e0-x1;
  const CVec3<T> v2 = e2-e0-x2;
  const CVec3<T> v3 = e3-e0-x3;
  // compute coefficient for cubic function
  const T k0 = ScalarTripleProduct(x3,x1,x2);
  const T k1 = ScalarTripleProduct(v3,x1,x2)+ScalarTripleProduct(x3,v1,x2)+ScalarTripleProduct(x3,x1,v2);
  const T k2 = ScalarTripleProduct(v3,v1,x2)+ScalarTripleProduct(v3,x1,v2)+ScalarTripleProduct(x3,v1,v2);
  const T k3 = ScalarTripleProduct(v3,v1,v2);
  // cubic funciton is f(x) = k0 + k1*x + k2*x^2 + k3*x^3
  const T r0 = +0.0;
  const T r1 = +1.0;
  const T f0 = vec3::EvaluateCubic(r0,k0,k1,k2,k3);
  const T f1 = vec3::EvaluateCubic(r1,k0,k1,k2,k3);
  if( f0*f1 <= 0 ){
    r = vec3::FindRootCubic_Bisect(r0,r1, f0,f1, k0,k1,k2,k3);
    return true;
  }
  if( fabs(k3) > 1.0e-30 ){ // cubic function
    const double det = k2*k2-3*k1*k3; // if det > 0, the cubic function takes extreme value
    if( det < 0 ){ return false; } // monotonus function
    //
    const double r3 = (-k2-sqrt(det))/(3*k3); // smaller extreme value
    if( r3 > 0 && r3 < 1 ){
      const double f3 = vec3::EvaluateCubic(r3, k0,k1,k2,k3);
      if( f3 == 0 ){ r = r3; return true; }
      if(      f0*f3 < 0 ){
        r = vec3::FindRootCubic_Bisect(r0,r3, f0,f3, k0,k1,k2,k3);
        return true;
      }
    }
    const double r4 = (-k2+sqrt(det))/(3*k3); // larger extreme value
    if( r4 > 0 && r4 < 1 ){
      const double f4 = vec3::EvaluateCubic(r4, k0,k1,k2,k3);
      if( f4 == 0 ){ r = r4; return true; }
      if(      f0*f4 < 0 ){
        r = vec3::FindRootCubic_Bisect(r0,r4, f0,f4, k0,k1,k2,k3);
        return true;
      }
    }
    return false;
  }
  //
  if( fabs(k2) > 1.0e-30 ){ // quadric function
    const double r2 = -k1/(2*k2); // extreme valuse
    if( r2 > 0 && r2 < 1 ){
      const double f2 = vec3::EvaluateCubic(r2, k0,k1,k2,k3);
      if(      f0*f2 < 0 ){
        r = vec3::FindRootCubic_Bisect(r0,r2, f0,f2, k0,k1,k2,k3);
        return true;
      }
    }
    return false;
  }
  return false;
}

// CCDのFVで接触する要素を検出
template <typename T>
bool delfem2::IsContact_FV_CCD2
(int ino0,
 int ino1,
 int ino2,
 int ino3,
 const CVec3<T>& p0,
 const CVec3<T>& p1,
 const CVec3<T>& p2,
 const CVec3<T>& p3,
 const CVec3<T>& q0,
 const CVec3<T>& q1,
 const CVec3<T>& q2,
 const CVec3<T>& q3)
{
  { // CSAT
    CVec3<T> n = Cross(p1-p0,p2-p0);
    double t0 = Dot(p0-p3,n);
    double t1 = Dot(q0-q3,n);
    double t2 = Dot(q1-q3,n);
    double t3 = Dot(q2-q3,n);
    if( t0*t1 > 0 && t0*t2 > 0 && t0*t3 > 0 ){ return false; }
  }
  double r0,r1;
  double dist = DistanceFaceVertex(p0, p1, p2, p3, r0,r1);
  {
    double vn0 = (p0-q0).Length();
    double vn1 = (p1-q1).Length();
    double vn2 = (p2-q2).Length();
    double vn3 = (p3-q3).Length();
    double vnt = ( vn0 > vn1 ) ? vn0 : vn1;
    vnt = ( vn2 > vnt ) ? vn2 : vnt;
    double max_app = (vnt+vn3);
    const double r2 = 1-r0-r1;
    if( dist > max_app ) return false;
    if( r0 < 0 || r0 > 1 || r1 < 0 || r1 > 1 || r2 < 0 || r2 > 1 ){
      double dist01 = (nearest_LineSeg_Point(p3, p0, p1)-p3).Length();
      double dist12 = (nearest_LineSeg_Point(p3, p1, p2)-p3).Length();
      double dist20 = (nearest_LineSeg_Point(p3, p2, p0)-p3).Length();
      if( dist01 > max_app && dist12 > max_app && dist20 > max_app ){ return false; }
    }
  }
  double t;
  {
    bool res = FindCoplanerInterp(t,
                                  p0,p1,p2,p3, q0,q1,q2,q3);
    if( !res ) return false;
    assert( t >= 0 && t <= 1 );
  }
  CVec3<T> p0m = (1-t)*p0 + t*q0;
  CVec3<T> p1m = (1-t)*p1 + t*q1;
  CVec3<T> p2m = (1-t)*p2 + t*q2;
  CVec3<T> p3m = (1-t)*p3 + t*q3;
  double w0, w1;
  DistanceFaceVertex(p0m, p1m, p2m, p3m, w0,w1);
  double w2 = 1-w0-w1;
  if( w0 < 0 || w0 > 1 ) return false;
  if( w1 < 0 || w1 > 1 ) return false;
  if( w2 < 0 || w2 > 1 ) return false;
  return true;
}
#ifndef DFM2_HEADER_ONLY
template bool delfem2::IsContact_FV_CCD2
  (int ino0,
   int ino1,
   int ino2,
   int ino3,
   const CVec3d& p0,
   const CVec3d& p1,
   const CVec3d& p2,
   const CVec3d& p3,
   const CVec3d& q0,
   const CVec3d& q1,
   const CVec3d& q2,
   const CVec3d& q3);
#endif
  

template <typename T>
bool delfem2::isIntersectTriPair
(CVec3<T>& P0, CVec3<T>& P1,
 int itri, int jtri,
 const std::vector<unsigned int>& aTri,
 const std::vector<double>& aXYZ)
{
  const int i0 = aTri[itri*3+0];
  const int i1 = aTri[itri*3+1];
  const int i2 = aTri[itri*3+2];
  const int j0 = aTri[jtri*3+0];
  const int j1 = aTri[jtri*3+1];
  const int j2 = aTri[jtri*3+2];
  if( i0 == j0 || i0 == j1 || i0 == j2 ) return false;
  if( i1 == j0 || i1 == j1 || i1 == j2 ) return false;
  if( i2 == j0 || i2 == j1 || i2 == j2 ) return false;
  const CVec3<T> p0(aXYZ[i0*3+0], aXYZ[i0*3+1], aXYZ[i0*3+2]);
  const CVec3<T> p1(aXYZ[i1*3+0], aXYZ[i1*3+1], aXYZ[i1*3+2]);
  const CVec3<T> p2(aXYZ[i2*3+0], aXYZ[i2*3+1], aXYZ[i2*3+2]);
  const CVec3<T> q0(aXYZ[j0*3+0], aXYZ[j0*3+1], aXYZ[j0*3+2]);
  const CVec3<T> q1(aXYZ[j1*3+0], aXYZ[j1*3+1], aXYZ[j1*3+2]);
  const CVec3<T> q2(aXYZ[j2*3+0], aXYZ[j2*3+1], aXYZ[j2*3+2]);
  const CVec3<T> np = Normal(p0,p1,p2);
  const CVec3<T> nq = Normal(q0,q1,q2);
  double dp0 = (p0-q0)*nq;
  double dp1 = (p1-q0)*nq;
  double dp2 = (p2-q0)*nq;
  double dq0 = (q0-p0)*np;
  double dq1 = (q1-p0)*np;
  double dq2 = (q2-p0)*np;
  if( ((dp0>0) == (dp1>0)) && ((dp1>0) == (dp2>0)) ) return false;
  if( ((dq0>0) == (dq1>0)) && ((dq1>0) == (dq2>0)) ) return false;
  const CVec3<T> p01 = (1.0/(dp0-dp1))*(dp0*p1-dp1*p0);
  const CVec3<T> p12 = (1.0/(dp1-dp2))*(dp1*p2-dp2*p1);
  const CVec3<T> p20 = (1.0/(dp2-dp0))*(dp2*p0-dp0*p2);
  const CVec3<T> q01 = (1.0/(dq0-dq1))*(dq0*q1-dq1*q0);
  const CVec3<T> q12 = (1.0/(dq1-dq2))*(dq1*q2-dq2*q1);
  const CVec3<T> q20 = (1.0/(dq2-dq0))*(dq2*q0-dq0*q2);
  const CVec3<T> vz = Cross(np,nq);
  CVec3<T> ps,pe;
  if(      dp0*dp1>0 ){ ps=p20; pe=p12; }
  else if( dp1*dp2>0 ){ ps=p01; pe=p20; }
  else{                 ps=p12; pe=p01; }
  if( ps*vz>pe*vz ){ CVec3<T> pt=ps; ps=pe; pe=pt; }
  double zps = ps*vz;
  double zpe = pe*vz;
  assert( zps<=zpe );
  ////
  CVec3<T> qs,qe;
  if(      dq0*dq1>0 ){ qs=q20; qe=q12; }
  else if( dq1*dq2>0 ){ qs=q01; qe=q20; }
  else{                 qs=q12; qe=q01; }
  if( qs*vz>qe*vz ){ CVec3<T> qt=qs; qs=qe; qe=qt; }
  double zqs = qs*vz;
  double zqe = qe*vz;
  assert( zqs<=zqe );
  //
  if( zps>zqe || zqs>zpe ) return false;
  CVec3<T> P[4];
  int icnt = 0;
  if( zps>zqs && zps<zqe ){ P[icnt]=ps; icnt++; }
  if( zpe>zqs && zpe<zqe ){ P[icnt]=pe; icnt++; }
  if( zqs>zps && zqs<zpe ){ P[icnt]=qs; icnt++; }
  if( zqe>zps && zqe<zpe ){ P[icnt]=qe; icnt++; }
  if( icnt != 2 ) return false;
  P0 = P[0];
  P1 = P[1];
  return true;
}
#ifndef DFM2_HEADER_ONLY
template bool delfem2::isIntersectTriPair(
    CVec3d& P0, CVec3d& P1,
    int itri, int jtri,
    const std::vector<unsigned int>& aTri,
    const std::vector<double>& aXYZ);
#endif


// -------------------------------------------------------------------------


// matrix are column major
template <typename T>
delfem2::CVec3<T> delfem2::mult_GlAffineMatrix
(const float* m,
 const CVec3<T>& p)
{
  CVec3<T> v;
  v.p[0] = m[0*4+0]*p.p[0] + m[1*4+0]*p.p[1] + m[2*4+0]*p.p[2] + m[3*4+0];
  v.p[1] = m[0*4+1]*p.p[0] + m[1*4+1]*p.p[1] + m[2*4+1]*p.p[2] + m[3*4+1];
  v.p[2] = m[0*4+2]*p.p[0] + m[1*4+2]*p.p[1] + m[2*4+2]*p.p[2] + m[3*4+2];
  return v;
}

template <typename T>
delfem2::CVec3<T> delfem2::solve_GlAffineMatrix
(const float* m,
 const CVec3<T>& p)
{
  CVec3<T> v = p - CVec3<T>(m[3*4+0],m[3*4+1],m[3*4+2]);
  double M[9] = {
    m[0*4+0],m[1*4+0],m[2*4+0],
    m[0*4+1],m[1*4+1],m[2*4+1],
    m[0*4+2],m[1*4+2],m[2*4+2] };
  double Minv[9]; vec3::MyInverse_Mat3(Minv, M);
  return Mat3Vec(Minv,v);
//  CMatrix3 Minv = M.Inverse();  
//  return Minv*v;
}

template <typename T>
delfem2::CVec3<T> delfem2::solve_GlAffineMatrixDirection
(const float* m,
 const CVec3<T>& v)
{
  double M[9] = {
    m[0*4+0],m[1*4+0],m[2*4+0],
    m[0*4+1],m[1*4+1],m[2*4+1],
    m[0*4+2],m[1*4+2],m[2*4+2] };
  double Minv[9]; vec3::MyInverse_Mat3(Minv, M);
  return Mat3Vec(Minv,v);
  /*
  CMatrix3 M(m[0*4+0],m[1*4+0],m[2*4+0],
             m[0*4+1],m[1*4+1],m[2*4+1],
             m[0*4+2],m[1*4+2],m[2*4+2]);
   */
  /*
   CMatrix3 M(m[0*4+0], m[0*4+1], m[0*4+2],
   m[1*4+0], m[1*4+1], m[1*4+2],
   m[2*4+0], m[2*4+1], m[2*4+2]);
   */
//  CMatrix3 Minv = M.Inverse();
//  return Minv*v;
}

// ----------------------

template <typename T>
delfem2::CVec3<T> delfem2::screenProjection
(const CVec3<T>& v,
 const float* mMV,
 const float* mPj)
{
  CVec3<T> v0 = mult_GlAffineMatrix(mMV, v );
  CVec3<T> v1 = mult_GlAffineMatrix(mPj, v0);
  float w1 = mPj[11]*(float)v0.p[2] + mPj[15];
  return CVec3<T>(v1.p[0]/w1, v1.p[1]/w1, 0.0);
}
#ifndef DFM2_HEADER_ONLY
template delfem2::CVec3d delfem2::screenProjection (
    const CVec3d& v,
    const float* mMV,
    const float* mPj);
#endif

template <typename T>
delfem2::CVec3<T> delfem2::screenUnProjection
(const CVec3<T>& v,
 const float* mMV, const float* mPj)
{
  float D = mPj[11] + mPj[15]; // z is 1 after model view
  CVec3<T> v0( D*v.p[0], D*v.p[1], 0.0 );
  CVec3<T> v1 = solve_GlAffineMatrix(mPj, v0);
  v1.p[2] = 1;
  CVec3<T> v2 = solve_GlAffineMatrix(mMV, v1);
  return v2;
}
#ifndef DFM2_HEADER_ONLY
template delfem2::CVec3d delfem2::screenUnProjection(
    const CVec3d& v,
    const float* mMV, const float* mPj);
#endif

template <typename T>
delfem2::CVec3<T> delfem2::screenUnProjectionDirection
(const CVec3<T>& v,
 const float* mMV,
 const float* mPj)
{
  CVec3<T> v0 = solve_GlAffineMatrixDirection(mPj, v);
  CVec3<T> v1 = solve_GlAffineMatrixDirection(mMV, v0);
  v1.SetNormalizedVector();
  return v1;
}
#ifndef DFM2_HEADER_ONLY
template delfem2::CVec3d delfem2::screenUnProjectionDirection(
    const CVec3d& v,
    const float* mMV,
    const float* mPj);
#endif
  
// -----------------------

template <typename T>
delfem2::CVec3<T> delfem2::screenDepthDirection
(const CVec3<T>& v,
 const float* mMV,
 const float* mPj)
{
  float Dv = mPj[11] + mPj[15]; // z is 1 after model view
  CVec3<T> v0( Dv*v.p[0], Dv*v.p[1], 0.0 );
  CVec3<T> v1 = solve_GlAffineMatrix(mPj, v0);
  v1.p[2] = 1;
  ////
  float Du = mPj[11]*2.f + mPj[15]; // z is 1 after model view
  CVec3<T> u0( Du*v.p[0], Du*v.p[1], 0.0 );
  CVec3<T> u1 = solve_GlAffineMatrix(mPj, u0);
  u1.p[2] = 2;
  ////
  CVec3<T> v2 = solve_GlAffineMatrixDirection(mMV, (v1-u1) );
  v2.SetNormalizedVector();
  return v2;
}

// ----------------------------------------------------------------------------

//! Volume of a tetrahedra
template <typename T>
T delfem2::Volume_Tet
(const CVec3<T>& v0,
 const CVec3<T>& v1,
 const CVec3<T>& v2,
 const CVec3<T>& v3 )
{
  return delfem2::Volume_Tet3(v0.p, v1.p, v2.p, v3.p);
  /*
  double v = (v1.p[0]-v0.p[0])*( (v2.p[1]-v0.p[1])*(v3.p[2]-v0.p[2]) - (v3.p[1]-v0.p[1])*(v2.p[2]-v0.p[2]) )
           + (v1.p[1]-v0.p[1])*( (v2.p[2]-v0.p[2])*(v3.p[0]-v0.p[0]) - (v3.p[2]-v0.p[2])*(v2.p[0]-v0.p[0]) )
           + (v1.p[2]-v0.p[2])*( (v2.p[0]-v0.p[0])*(v3.p[1]-v0.p[1]) - (v3.p[0]-v0.p[0])*(v2.p[1]-v0.p[1]) );
  return v*0.16666666666666666666666666666667;
   */
}
#ifndef DFM2_HEADER_ONLY
template double delfem2::Volume_Tet(const CVec3d& v0, const CVec3d& v1, const CVec3d& v2, const CVec3d& v3 );
template float delfem2::Volume_Tet(const CVec3f& v0, const CVec3f& v1, const CVec3f& v2, const CVec3f& v3 );
#endif


//! Volume of a tetrahedra v0 is orgin
template <typename T>
double delfem2::Volume_OrgTet
(const CVec3<T>& v1,
 const CVec3<T>& v2,
 const CVec3<T>& v3 )
{
  double v = v1.p[0]*(v2.p[1]*v3.p[2]-v3.p[1]*v2.p[2])
           + v1.p[1]*(v2.p[2]*v3.p[0]-v3.p[2]*v2.p[0])
           + v1.p[2]*(v2.p[0]*v3.p[1]-v3.p[0]*v2.p[1]);
  return v*0.16666666666666666666666666666667;
}

template <typename T>
double delfem2::Volume_Pyramid
(const CVec3<T>& p0,
 const CVec3<T>& p1,
 const CVec3<T>& p2,
 const CVec3<T>& p3,
 const CVec3<T>& p4)
{
  double v0124 = Volume_Tet(p0, p1, p2, p4);
  double v0234 = Volume_Tet(p0, p2, p3, p4);
  double v0134 = Volume_Tet(p0, p1, p3, p4);
  double v2314 = Volume_Tet(p2, p3, p1, p4);
  double v0 = v0124+v0234;
  double v1 = v0134+v2314;
  return (v0+v1)*0.5;
}

template <typename T>
double delfem2::Volume_Wedge
(const CVec3<T>& p0,
 const CVec3<T>& p1,
 const CVec3<T>& p2,
 const CVec3<T>& p3,
 const CVec3<T>& p4,
 const CVec3<T>& p5)
{
  CVec3<T> pm = (p0+p1+p2+p3+p4+p5)/6.0;
  double vm012 = Volume_Tet(pm,p0,p1,p2);
  double vm435 = Volume_Tet(pm,p4,p3,p5);
  double vp0143 = Volume_Pyramid(p0,p1,p4,p3,pm);
  double vp1254 = Volume_Pyramid(p1,p2,p5,p4,pm);
  double vp2035 = Volume_Pyramid(p2,p2,p3,p5,pm);
  return vm012+vm435+vp0143+vp1254+vp2035;
}

// ---------------------------------------------------------------

template <typename T>
double delfem2::SolidAngleTri
(const CVec3<T>& v1,
 const CVec3<T>& v2,
 const CVec3<T>& v3)
{
  double l1 = v1.Length();
  double l2 = v2.Length();
  double l3 = v3.Length();
  double den = (v1^v2)*v3;
  double num = l1*l2*l3+(v1*v2)*l3+(v2*v3)*l1+(v3*v1)*l2;
  double tho = den/num;
  double v = atan(tho);
  if (v<0){ v += 2*M_PI; }
  v *= 2;
  return v;
}
#ifndef DFM2_HEADER_ONLY
template double delfem2::SolidAngleTri(const CVec3d& v1, const CVec3d& v2, const CVec3d& v3);
#endif



// ------------------------------------------------------------------

template <typename T>
void delfem2::Cross
(CVec3<T>& lhs,
 const CVec3<T>& v1,
 const CVec3<T>& v2 )
{
  lhs.p[0] = v1.p[1]*v2.p[2] - v2.p[1]*v1.p[2];
  lhs.p[1] = v1.p[2]*v2.p[0] - v2.p[2]*v1.p[0];
  lhs.p[2] = v1.p[0]*v2.p[1] - v2.p[0]*v1.p[1];
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::Cross(CVec3d& lhs, const CVec3d& v1, const CVec3d& v2 );
#endif
  
// ----------------------

template <typename T>
double delfem2::Area_Tri
(const CVec3<T>& v1,
 const CVec3<T>& v2,
 const CVec3<T>& v3)
{
  double x, y, z;
  x = ( v2.p[1] - v1.p[1] )*( v3.p[2] - v1.p[2] ) - ( v3.p[1] - v1.p[1] )*( v2.p[2] - v1.p[2] );
  y = ( v2.p[2] - v1.p[2] )*( v3.p[0] - v1.p[0] ) - ( v3.p[2] - v1.p[2] )*( v2.p[0] - v1.p[0] );
  z = ( v2.p[0] - v1.p[0] )*( v3.p[1] - v1.p[1] ) - ( v3.p[0] - v1.p[0] )*( v2.p[1] - v1.p[1] );
  return 0.5*sqrt( x*x + y*y + z*z );
}
#ifndef DFM2_HEADER_ONLY
template double delfem2::Area_Tri(const CVec3<double>& v1, const CVec3<double>& v2, const CVec3<double>& v3);
#endif

  

template <typename T>
double delfem2::SquareTriArea(const CVec3<T>& v1, const CVec3<T>& v2, const CVec3<T>& v3)
{
  double dtmp_x = (v2.p[1]-v1.p[1])*(v3.p[2]-v1.p[2])-(v2.p[2]-v1.p[2])*(v3.p[1]-v1.p[1]);
  double dtmp_y = (v2.p[2]-v1.p[2])*(v3.p[0]-v1.p[0])-(v2.p[0]-v1.p[0])*(v3.p[2]-v1.p[2]);
  double dtmp_z = (v2.p[0]-v1.p[0])*(v3.p[1]-v1.p[1])-(v2.p[1]-v1.p[1])*(v3.p[0]-v1.p[0]);
  return (dtmp_x*dtmp_x + dtmp_y*dtmp_y + dtmp_z*dtmp_z)*0.25;
}

template <typename T>
double delfem2::SquareDistance
(const CVec3<T>& ipo0,
 const CVec3<T>& ipo1)
{
  return	( ipo1.p[0] - ipo0.p[0] )*( ipo1.p[0] - ipo0.p[0] ) + ( ipo1.p[1] - ipo0.p[1] )*( ipo1.p[1] - ipo0.p[1] ) + ( ipo1.p[2] - ipo0.p[2] )*( ipo1.p[2] - ipo0.p[2] );
}

template <typename T>
double delfem2::SquareLength(const CVec3<T>& point)
{
  return	point.p[0]*point.p[0] + point.p[1]*point.p[1] + point.p[2]*point.p[2];
}


//! length of vector
template <typename T>
double delfem2::Length(const CVec3<T>& point)
{
  return delfem2::Length3(point.p);
}

//! distance between two points
template <typename T>
double delfem2::Distance
(const CVec3<T>& p0,
 const CVec3<T>& p1)
{
  return delfem2::Distance3(p0.p, p1.p);
}
#ifndef DFM2_HEADER_ONLY
template double delfem2::Distance(const CVec3<double>& p0, const CVec3<double>& p1);
#endif


template <typename T>
double delfem2::SqareLongestEdgeLength
(const CVec3<T>& ipo0,
 const CVec3<T>& ipo1,
 const CVec3<T>& ipo2,
 const CVec3<T>& ipo3 )
{
  double edge1, edge2;
  edge1 = SquareDistance( ipo0, ipo1 );
  edge2 = SquareDistance( ipo0, ipo2 );
  if( edge2 > edge1 ) edge1 = edge2;
  edge2 = SquareDistance( ipo0, ipo3 );
  if( edge2 > edge1 ) edge1 = edge2;
  edge2 = SquareDistance( ipo1, ipo2 );
  if( edge2 > edge1 ) edge1 = edge2;
  edge2 = SquareDistance( ipo1, ipo3 );
  if( edge2 > edge1 ) edge1 = edge2;
  edge2 = SquareDistance( ipo2, ipo3 );
  if( edge2 > edge1 ) edge1 = edge2;
  return edge1;
}

// --------------------------------------

template <typename T>
double delfem2::LongestEdgeLength
(const CVec3<T>& ipo0,
 const CVec3<T>& ipo1,
 const CVec3<T>& ipo2,
 const CVec3<T>& ipo3 )
{
  return sqrt( SqareLongestEdgeLength(ipo0,ipo1,ipo2,ipo3) );
}

// --------------------------------------

template <typename T>
double delfem2::SqareShortestEdgeLength
(const CVec3<T>& ipo0,
 const CVec3<T>& ipo1,
 const CVec3<T>& ipo2,
 const CVec3<T>& ipo3 )
{
  double edge1, edge2;
  edge1 = SquareDistance( ipo0, ipo1 );
  edge2 = SquareDistance( ipo0, ipo2 );
  if( edge2 < edge1 ) edge1 = edge2;
  edge2 = SquareDistance( ipo0, ipo3 );
  if( edge2 < edge1 ) edge1 = edge2;
  edge2 = SquareDistance( ipo1, ipo2 );
  if( edge2 < edge1 ) edge1 = edge2;
  edge2 = SquareDistance( ipo1, ipo3 );
  if( edge2 < edge1 ) edge1 = edge2;
  edge2 = SquareDistance( ipo2, ipo3 );
  if( edge2 < edge1 ) edge1 = edge2;
  return edge1;
}

// -----------------------------------------

template <typename T>
double delfem2::ShortestEdgeLength
(const CVec3<T>& ipo0,
 const CVec3<T>& ipo1,
 const CVec3<T>& ipo2,
 const CVec3<T>& ipo3 )
{
  return sqrt( SqareShortestEdgeLength(ipo0,ipo1,ipo2,ipo3) );
}

// -------------------------------------------

template <typename T>
void delfem2::Normal
(CVec3<T>& vnorm,
 const CVec3<T>& v1,
 const CVec3<T>& v2,
 const CVec3<T>& v3)
{
  vnorm.p[0] = (v2.p[1]-v1.p[1])*(v3.p[2]-v1.p[2])-(v2.p[2]-v1.p[2])*(v3.p[1]-v1.p[1]);
  vnorm.p[1] = (v2.p[2]-v1.p[2])*(v3.p[0]-v1.p[0])-(v2.p[0]-v1.p[0])*(v3.p[2]-v1.p[2]);
  vnorm.p[2] = (v2.p[0]-v1.p[0])*(v3.p[1]-v1.p[1])-(v2.p[1]-v1.p[1])*(v3.p[0]-v1.p[0]);
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::Normal(CVec3d& vnorm, const CVec3d& v1, const CVec3d& v2, const CVec3d& v3);
#endif
  
// -------------------------------------------

template <typename T>
delfem2::CVec3<T> delfem2::Normal
(const CVec3<T>& v1,
const CVec3<T>& v2,
const CVec3<T>& v3)
{
  CVec3<T> vnorm;
  vnorm.p[0] = (v2.p[1]-v1.p[1])*(v3.p[2]-v1.p[2])-(v2.p[2]-v1.p[2])*(v3.p[1]-v1.p[1]);
  vnorm.p[1] = (v2.p[2]-v1.p[2])*(v3.p[0]-v1.p[0])-(v2.p[0]-v1.p[0])*(v3.p[2]-v1.p[2]);
  vnorm.p[2] = (v2.p[0]-v1.p[0])*(v3.p[1]-v1.p[1])-(v2.p[1]-v1.p[1])*(v3.p[0]-v1.p[0]);
  return vnorm;
}
#ifndef DFM2_HEADER_ONLY
template delfem2::CVec3d delfem2::Normal(const CVec3d& v1, const CVec3d& v2, const CVec3d& v3);
#endif


// --------------------------------------------
  
template <typename T>
void delfem2::UnitNormal
(CVec3<T>& vnorm,
 const CVec3<T>& v1,
 const CVec3<T>& v2,
 const CVec3<T>& v3)
{
  vnorm.p[0] = (v2.p[1]-v1.p[1])*(v3.p[2]-v1.p[2])-(v2.p[2]-v1.p[2])*(v3.p[1]-v1.p[1]);
  vnorm.p[1] = (v2.p[2]-v1.p[2])*(v3.p[0]-v1.p[0])-(v2.p[0]-v1.p[0])*(v3.p[2]-v1.p[2]);
  vnorm.p[2] = (v2.p[0]-v1.p[0])*(v3.p[1]-v1.p[1])-(v2.p[1]-v1.p[1])*(v3.p[0]-v1.p[0]);
  const double dtmp1 = 1.0 / Length(vnorm);
  vnorm.p[0] *= dtmp1;
  vnorm.p[1] *= dtmp1;
  vnorm.p[2] *= dtmp1;
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::UnitNormal(CVec3f& vnorm, const CVec3f& v1, const CVec3f& v2, const CVec3f& v3);
template void delfem2::UnitNormal(CVec3d& vnorm, const CVec3d& v1, const CVec3d& v2, const CVec3d& v3);
#endif

// ---------------------------------------

template <typename T>
delfem2::CVec3<T> delfem2::UnitNormal
(const CVec3<T>& v1,
const CVec3<T>& v2,
const CVec3<T>& v3)
{
  CVec3<T> vnorm;
  vnorm.p[0] = (v2.p[1]-v1.p[1])*(v3.p[2]-v1.p[2])-(v2.p[2]-v1.p[2])*(v3.p[1]-v1.p[1]);
  vnorm.p[1] = (v2.p[2]-v1.p[2])*(v3.p[0]-v1.p[0])-(v2.p[0]-v1.p[0])*(v3.p[2]-v1.p[2]);
  vnorm.p[2] = (v2.p[0]-v1.p[0])*(v3.p[1]-v1.p[1])-(v2.p[1]-v1.p[1])*(v3.p[0]-v1.p[0]);
  const double dtmp1 = 1.0/Length(vnorm);
  vnorm.p[0] *= dtmp1;
  vnorm.p[1] *= dtmp1;
  vnorm.p[2] *= dtmp1;
  return vnorm;
}
#ifndef DFM2_HEADER_ONLY
template delfem2::CVec3f delfem2::UnitNormal
 (const CVec3f& v1,
  const CVec3f& v2,
  const CVec3f& v3);
template delfem2::CVec3d delfem2::UnitNormal
(const CVec3d& v1,
 const CVec3d& v2,
 const CVec3d& v3);
#endif

// ---------------------------------------------------

template <typename T>
double delfem2::SquareCircumradius
(const CVec3<T>& ipo0,
 const CVec3<T>& ipo1,
 const CVec3<T>& ipo2,
 const CVec3<T>& ipo3)
{
  double base[3][3] = {
    { ipo1.p[0]-ipo0.p[0], ipo1.p[1]-ipo0.p[1], ipo1.p[2]-ipo0.p[2] },
    { ipo2.p[0]-ipo0.p[0], ipo2.p[1]-ipo0.p[1], ipo2.p[2]-ipo0.p[2] },
    { ipo3.p[0]-ipo0.p[0], ipo3.p[1]-ipo0.p[1], ipo3.p[2]-ipo0.p[2] }
  };
  double s[6] = {
    base[0][0]*base[0][0]+base[0][1]*base[0][1]+base[0][2]*base[0][2],
    base[1][0]*base[1][0]+base[1][1]*base[1][1]+base[1][2]*base[1][2],
    base[2][0]*base[2][0]+base[2][1]*base[2][1]+base[2][2]*base[2][2],
    base[1][0]*base[2][0]+base[1][1]*base[2][1]+base[1][2]*base[2][2],
    base[2][0]*base[0][0]+base[2][1]*base[0][1]+base[2][2]*base[0][2],
    base[0][0]*base[1][0]+base[0][1]*base[1][1]+base[0][2]*base[1][2],
  };
  const double vol = Volume_Tet(ipo0,ipo1,ipo2,ipo3)*6.0;
  if( vol < 1.0e-20 ){ assert(0); }
  const double inv_det = 1.0 / (vol*vol);
  double t[6] = {
    (s[1]*s[2]-s[3]*s[3])*0.5*inv_det,
    (s[2]*s[0]-s[4]*s[4])*0.5*inv_det,
    (s[0]*s[1]-s[5]*s[5])*0.5*inv_det,
    (s[4]*s[5]-s[0]*s[3])*0.5*inv_det,
    (s[5]*s[3]-s[1]*s[4])*0.5*inv_det,
    (s[3]*s[4]-s[2]*s[5])*0.5*inv_det,
  };
  double u[3] = {
    t[0]*s[0]+t[5]*s[1]+t[4]*s[2],
    t[5]*s[0]+t[1]*s[1]+t[3]*s[2],
    t[4]*s[0]+t[3]*s[1]+t[2]*s[2],
  };
  return  0.5*(u[0]*s[0]+u[1]*s[1]+u[2]*s[2]);
  /*
   const double square_radius = 0.5*(u[0]*s[0]+u[1]*s[1]+u[2]*s[2]);
   CVector3 vec1;
   vec1.p[0] = base[0][0]*u[0]+base[1][0]*u[1]+base[2][0]*u[2] + ipo0.p[0];
   vec1.p[1] = base[0][1]*u[0]+base[1][1]*u[1]+base[2][1]*u[2] + ipo0.p[1];
   vec1.p[2] = base[0][2]*u[0]+base[1][2]*u[1]+base[2][2]*u[2] + ipo0.p[2];
   std::cout << square_radius << " ";
   std::cout << SquareLength(vec1,ipo0) << " ";
   std::cout << SquareLength(vec1,ipo1) << " ";
   std::cout << SquareLength(vec1,ipo2) << " ";
   std::cout << SquareLength(vec1,ipo3) << std::endl;;
   return square_radius;
   */
}

template <typename T>
delfem2::CVec3<T> delfem2::CircumCenter
(const CVec3<T>& ipo0,
 const CVec3<T>& ipo1,
 const CVec3<T>& ipo2,
 const CVec3<T>& ipo3)
{
  
  double base[3][3] = {
    { ipo1.p[0]-ipo0.p[0], ipo1.p[1]-ipo0.p[1], ipo1.p[2]-ipo0.p[2] },
    { ipo2.p[0]-ipo0.p[0], ipo2.p[1]-ipo0.p[1], ipo2.p[2]-ipo0.p[2] },
    { ipo3.p[0]-ipo0.p[0], ipo3.p[1]-ipo0.p[1], ipo3.p[2]-ipo0.p[2] }
  };
  double s[6] = {
    base[0][0]*base[0][0]+base[0][1]*base[0][1]+base[0][2]*base[0][2],
    base[1][0]*base[1][0]+base[1][1]*base[1][1]+base[1][2]*base[1][2],
    base[2][0]*base[2][0]+base[2][1]*base[2][1]+base[2][2]*base[2][2],
    base[1][0]*base[2][0]+base[1][1]*base[2][1]+base[1][2]*base[2][2],
    base[2][0]*base[0][0]+base[2][1]*base[0][1]+base[2][2]*base[0][2],
    base[0][0]*base[1][0]+base[0][1]*base[1][1]+base[0][2]*base[1][2],
  };
  const double vol = Volume_Tet(ipo0,ipo1,ipo2,ipo3)*6.0;
  if( vol < 1.0e-20 ){ assert(0); }
  const double inv_det = 1.0 / (vol*vol);
  double t[6] = {
    (s[1]*s[2]-s[3]*s[3])*0.5*inv_det,
    (s[2]*s[0]-s[4]*s[4])*0.5*inv_det,
    (s[0]*s[1]-s[5]*s[5])*0.5*inv_det,
    (s[4]*s[5]-s[0]*s[3])*0.5*inv_det,
    (s[5]*s[3]-s[1]*s[4])*0.5*inv_det,
    (s[3]*s[4]-s[2]*s[5])*0.5*inv_det,
  };
  double u[3] = {
    t[0]*s[0]+t[5]*s[1]+t[4]*s[2],
    t[5]*s[0]+t[1]*s[1]+t[3]*s[2],
    t[4]*s[0]+t[3]*s[1]+t[2]*s[2],
  };
  //    const double square_radius = 0.5*(u[0]*s[0]+u[1]*s[1]+u[2]*s[2]);
  CVec3<T> vec1;
  vec1.p[0] = base[0][0]*u[0]+base[1][0]*u[1]+base[2][0]*u[2] + ipo0.p[0];
  vec1.p[1] = base[0][1]*u[0]+base[1][1]*u[1]+base[2][1]*u[2] + ipo0.p[1];
  vec1.p[2] = base[0][2]*u[0]+base[1][2]*u[1]+base[2][2]*u[2] + ipo0.p[2];
  return vec1;
}
#ifndef DFM2_HEADER_ONLY
template delfem2::CVec3d delfem2::CircumCenter(const CVec3d& ipo0,
                                         const CVec3d& ipo1,
                                         const CVec3d& ipo2,
                                         const CVec3d& ipo3);
#endif
  
  
// ----------------------

template <typename T>
void delfem2::MeanValueCoordinate
(double w[3],
 const CVec3<T>& v0,
 const CVec3<T>& v1,
 const CVec3<T>& v2)
{
  double eps  = 1.0e-5;
  double d0 = v0.Length();
  double d1 = v1.Length();
  double d2 = v2.Length();
  const CVec3<T> u0 = v0/d0;
  const CVec3<T> u1 = v1/d1;
  const CVec3<T> u2 = v2/d2;
  double l0 = (u1-u2).Length();
  double l1 = (u2-u0).Length();
  double l2 = (u0-u1).Length();
  if( l0<eps || l1<eps || l2<eps ){
    w[0] = 0;
    w[1] = 0;
    w[2] = 0;
    return;
  }
  double t0 = 2*asin(l0*0.5);
  double t1 = 2*asin(l1*0.5);
  double t2 = 2*asin(l2*0.5);
  double h = (t0+t1+t2)*0.5;
  double c0 = 2*sin(h)*sin(h-t0)/(sin(t1)*sin(t2))-1;
  double c1 = 2*sin(h)*sin(h-t1)/(sin(t2)*sin(t0))-1;
  double c2 = 2*sin(h)*sin(h-t2)/(sin(t0)*sin(t1))-1;
  double vol012 = ScalarTripleProduct(u0,u1,u2);
  double sign = (vol012 > 0) ? 1 : -1;
  double s0 = sign*sqrt(1.0-c0*c0);
  double s1 = sign*sqrt(1.0-c1*c1);
  double s2 = sign*sqrt(1.0-c2*c2);
  if( vec3::MyIsnan(s0) || vec3::MyIsnan(s1) || vec3::MyIsnan(s2) ){
    w[0] = 0;
    w[1] = 0;
    w[2] = 0;
    return;
  }
  if( fabs(d0*sin(t1)*s2)<eps || fabs(d1*sin(t2)*s0)<eps || fabs(d2*sin(t0)*s1)<eps ){
    w[0] = 0;
    w[1] = 0;
    w[2] = 0;
    return;
  }
  w[0] = (t0-c2*t1-c1*t2)/(d0*sin(t1)*s2);
  w[1] = (t1-c0*t2-c2*t0)/(d1*sin(t2)*s0);
  w[2] = (t2-c1*t0-c0*t1)/(d2*sin(t0)*s1);
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::MeanValueCoordinate(double w[3],
                                        const CVec3d& v0,
                                        const CVec3d& v1,
                                        const CVec3d& v2);
#endif

// --------------------------------------------------

template <typename T>
delfem2::CVec3<T> delfem2::ProjectPointOnTriangle
(const CVec3<T> &p0,
 const CVec3<T> &tri_p1, const CVec3<T> &tri_p2, const CVec3<T> &tri_p3)
{
  CVec3<T> normal = Cross(tri_p2 - tri_p1, tri_p3 - tri_p1);
  double cosAlpha = Dot(p0 - tri_p1, normal) / (Length(p0 - tri_p1) * Length(normal));
  double lenP0ProjectedP0 = Length(tri_p1 - p0) * cosAlpha;
  CVec3<T> p0ProjectedP0 = -1 * lenP0ProjectedP0 * normal / Length(normal);
  
  return p0 + p0ProjectedP0;
}
  
// ----------------------

template <typename T>
bool delfem2::isRayIntersectingTriangle
(const CVec3<T> &line0, const CVec3<T> &line1,
 const CVec3<T> &tri0, const CVec3<T> &tri1, const CVec3<T> &tri2,
 CVec3<T> &intersectionPoint)
{
  CVec3<T> normal = Cross(tri1 - tri0, tri2 - tri0);
  
  // The ray is parallel to the triangle plane
  if (Dot(normal, line1 - line0) == 0)
  {
    return false;
  }
  
  double r = Dot(normal, tri0 - line0) / Dot(normal, line1 - line0);
  
  // The ray does not intersect the triangle plane
  if (r < 0)
  {
    return false;
  }
  
  // Find the intersection point
  intersectionPoint = line0 + r * (line1 - line0);
  
  if (!isPointInsideTriangle(intersectionPoint,
                             tri0, tri1, tri2))
  {
    return false;
  }
  
  return true;
}
#ifndef DFM2_HEADER_ONLY
template bool delfem2::isRayIntersectingTriangle
  (const CVec3d &line0, const CVec3d &line1,
   const CVec3d &tri0, const CVec3d &tri1, const CVec3d &tri2,
   CVec3d &intersectionPoint);
#endif
  
// ----------------------------------------

template <typename T>
bool delfem2::isPointInsideTriangle
(const CVec3<T> &p0,
 const CVec3<T> &tri_p1, const CVec3<T> &tri_p2, const CVec3<T> &tri_p3)
{
  if (isPointSameSide(p0, tri_p1, tri_p2, tri_p3)
      && isPointSameSide(p0, tri_p2, tri_p1, tri_p3)
      && isPointSameSide(p0, tri_p3, tri_p1, tri_p2))
  {
    return true;
  } else {
    return false;
  }
}

template <typename T>
bool delfem2::isPointSameSide
(const CVec3<T> &p0, const CVec3<T> &p1,
 const CVec3<T> &line_p0, const CVec3<T> &line_p1)
{
  CVec3<T> crossProd1 = Cross(line_p1 - line_p0, p0 - line_p0);
  CVec3<T> crossProd2 = Cross(line_p1 - line_p0, p1 - line_p0);
  
  if (Dot(crossProd1, crossProd2) >= 0)
  {
    return true;
  } else {
    return false;
  }
}

// ------------------------------------------------

/**
 * @brief check if Delaunay condition satisfied
 * @details
 * 0 : p3 is inside circum circle on the p0,p1,p2
 * 1 :       on
 * 2 :       outsdie
 */
template <typename T>
int delfem2::DetDelaunay
(const CVec3<T>& p0,
const CVec3<T>& p1,
const CVec3<T>& p2,
const CVec3<T>& p3)
{
  const double area = Area_Tri(p0, p1, p2);
  if (fabs(area) < 1.0e-10){
    return 3;
  }
  const double tmp_val = 1.0/(area*area*16.0);

  const double dtmp0 = SquareDistance(p1, p2);
  const double dtmp1 = SquareDistance(p0, p2);
  const double dtmp2 = SquareDistance(p0, p1);

  const double etmp0 = tmp_val*dtmp0*(dtmp1+dtmp2-dtmp0);
  const double etmp1 = tmp_val*dtmp1*(dtmp0+dtmp2-dtmp1);
  const double etmp2 = tmp_val*dtmp2*(dtmp0+dtmp1-dtmp2);

  const CVec3<T> out_center = etmp0*p0+etmp1*p1+etmp2*p2;

  const double qradius = SquareDistance(out_center, p0);
  const double qdistance = SquareDistance(out_center, p3);

  const double tol = 1.0e-20;
  if (qdistance > qradius*(1.0+tol)){ return 2; }	// outside the circumcircle
  else{
    if (qdistance < qradius*(1.0-tol)){ return 0; }	// inside the circumcircle
    else{ return 1; }	// on the circumcircle
  }
  return 0;
}
#ifndef DFM2_HEADER_ONLY
template int delfem2::DetDelaunay(const CVec3d& p0,
                               const CVec3d& p1,
                               const CVec3d& p2,
                               const CVec3d& p3);
#endif
  
// -------------------------------------------

/**
 * @brief curcumradius of a tetrahedra
 */
template <typename T>
double delfem2::Circumradius
(const CVec3<T>& ipo0,
 const CVec3<T>& ipo1,
 const CVec3<T>& ipo2,
 const CVec3<T>& ipo3)
{
  return sqrt( SquareCircumradius(ipo0,ipo1,ipo2,ipo3) );
}
  
// --------------------------------

template <typename T>
delfem2::CVec3<T> delfem2::RotateVector(const CVec3<T>& vec0, const CVec3<T>& rot )
{
  const double theta = rot.Length();
  if( theta < 1.0e-30 ){
    return vec0;
  }
  CVec3<T> e0 = rot;
  e0.SetNormalizedVector();
  CVec3<T> e2 = delfem2::Cross(e0,vec0);
  if( e2.Length() < 1.0e-30 ){
    return vec0;
  }
  e2.SetNormalizedVector();
  CVec3<T> e1 = delfem2::Cross(e2,e0);
  assert( fabs( e1.Length() - 1 ) < 1.0e-10 );
  //	assert( e2.p[0]*vec_0.p[0] + e2.p[1]*vec_0.p[1] + e2.p[2]*vec_0.p[2] < 1.0e-10 );
  const double dot00 = Dot(vec0,e0);
  const double dot01 = Dot(vec0,e1);
  const double cost = cos(theta);
  const double sint = sin(theta);
  CVec3<T> vec1;
  vec1.p[0] = dot00*e0.p[0] + dot01*cost*e1.p[0] + dot01*sint*e2.p[0];
  vec1.p[1] = dot00*e0.p[1] + dot01*cost*e1.p[1] + dot01*sint*e2.p[1];
  vec1.p[2] = dot00*e0.p[2] + dot01*cost*e1.p[2] + dot01*sint*e2.p[2];
  return vec1;
}

template <typename T>
delfem2::CVec3<T> delfem2::RandVector(){
  CVec3<T> r;
  r.p[0] = (2*(double)rand()/(RAND_MAX+1.0)-1);
  r.p[1] = (2*(double)rand()/(RAND_MAX+1.0)-1);
  r.p[2] = (2*(double)rand()/(RAND_MAX+1.0)-1);
  return r;
}

template <typename T>
delfem2::CVec3<T> delfem2::RandUnitVector(){
  for(int itr=0;itr<100;itr++){
    CVec3<T> r = RandVector<T>();
    double l = r.Length();
    if( (l <= 1 || itr==9) && l > 1.0e-5 ){
      r.SetNormalizedVector();
      return r;
    }
  }
  return CVec3<T>(1,0,0);
}

template <typename T>
delfem2::CVec3<T> delfem2::RandGaussVector()
{
  double a0 = rand()/(RAND_MAX+1.0);
  double a1 = rand()/(RAND_MAX+1.0);
  double a2 = rand()/(RAND_MAX+1.0);
  double a3 = rand()/(RAND_MAX+1.0);
  
  double x = sqrt(-2.0*log(a0))*cos(3.1415*2*a1);
  double y = sqrt(-2.0*log(a0))*sin(3.1415*2*a1);
  double z = sqrt(-2.0*log(a2))*cos(3.1415*2*a3);
  return CVec3<T>(x,y,z);
}



// ----------------------------------------------------------------------------------------
// using <vector> from here

// --------------------------------------------------
// TODO: following should be move to mesh class?

template <typename T>
double delfem2::Area_Tri
(const int iv1, const int iv2, const int iv3,
 const std::vector<CVec3<T>>& aPoint )
{
  return Area_Tri(aPoint[iv1],aPoint[iv2],aPoint[iv3]);
}

template <typename T>
double delfem2::Volume_Tet
(int iv1, int iv2, int iv3, int iv4,
 const std::vector<CVec3<T>>& aPoint)
{
  return Volume_Tet(aPoint[iv1],aPoint[iv2],aPoint[iv3],aPoint[iv4]);
}
#ifndef DFM2_HEADER_ONLY
template double delfem2::Volume_Tet
  (int iv1, int iv2, int iv3, int iv4,
   const std::vector<CVec3d>& aPoint);
#endif

// -------------------------------------

template <typename T>
bool delfem2::IsOut
(int itri, const CVec3<T>& v,
 const std::vector<CVec3<T>>& aXYZ,
 const std::vector<int>& aTri)
{
  int i0 = aTri[itri*3+0];
  int i1 = aTri[itri*3+1];
  int i2 = aTri[itri*3+2];
  const CVec3<T>& v0 = aXYZ[i0];
  const CVec3<T>& v1 = aXYZ[i1];
  const CVec3<T>& v2 = aXYZ[i2];
  CVec3<T> n; Normal(n,v0,v1,v2);
  double dot = Dot(v-v0,n);
  return dot > 0;
}

template <typename T>
void delfem2::ConvexHull
(std::vector<int>& aTri,
 const std::vector<CVec3<T>>& aXYZ)
{
  std::vector<int> aBflg( aXYZ.size(), -1 );
  aTri.reserve(aXYZ.size()*6);
  aTri.resize(4*3);
  aTri[ 0] = 1;  aTri[ 1] = 2;  aTri[ 2] = 3; // 0
  aTri[ 3] = 0;  aTri[ 4] = 3;  aTri[ 5] = 2; // 1
  aTri[ 6] = 0;  aTri[ 7] = 1;  aTri[ 8] = 3; // 2
  aTri[ 9] = 0;  aTri[10] = 2;  aTri[11] = 1; // 3
  std::vector< std::pair<int,int> > aTriSur;
  aTriSur.resize(4*3);
  aTriSur[ 0] = std::make_pair(1,0);
  aTriSur[ 1] = std::make_pair(2,0);
  aTriSur[ 2] = std::make_pair(3,0);
  aTriSur[ 3] = std::make_pair(0,0);
  aTriSur[ 4] = std::make_pair(3,2);
  aTriSur[ 5] = std::make_pair(2,1);
  aTriSur[ 6] = std::make_pair(0,1);
  aTriSur[ 7] = std::make_pair(1,2);
  aTriSur[ 8] = std::make_pair(3,1);
  aTriSur[ 9] = std::make_pair(0,2);
  aTriSur[10] = std::make_pair(2,2);
  aTriSur[11] = std::make_pair(1,1);
  {
    double vol = delfem2::Volume_Tet(0, 1, 2, 3, aXYZ);
    if( vol < 0 ){
      aTri[ 0] = 3;  aTri[ 1] = 2;  aTri[ 2] = 1; // 0
      aTri[ 3] = 2;  aTri[ 4] = 3;  aTri[ 5] = 0; // 1
      aTri[ 6] = 3;  aTri[ 7] = 1;  aTri[ 8] = 0; // 2
      aTri[ 9] = 1;  aTri[10] = 2;  aTri[11] = 0; // 3
      aTriSur[ 0] = std::make_pair(3,2);
      aTriSur[ 1] = std::make_pair(2,2);
      aTriSur[ 2] = std::make_pair(1,2);
      aTriSur[ 3] = std::make_pair(2,1);
      aTriSur[ 4] = std::make_pair(3,0);
      aTriSur[ 5] = std::make_pair(0,2);
      aTriSur[ 6] = std::make_pair(3,1);
      aTriSur[ 7] = std::make_pair(1,0);
      aTriSur[ 8] = std::make_pair(0,1);
      aTriSur[ 9] = std::make_pair(1,1);
      aTriSur[10] = std::make_pair(2,0);
      aTriSur[11] = std::make_pair(0,0);
    }
  }
  const int triEd[3][2] = {
    { 1, 2 },
    { 2, 0 },
    { 0, 1 } };
  for(int iv=4;iv<(int)aXYZ.size();iv++){
    CVec3<T> v = aXYZ[iv];
    int itri_ker = -1;
    for(int itri=0;itri<(int)aTri.size()/3;itri++){
      if( IsOut(itri,v,aXYZ,aTri) ){ itri_ker = itri; break; }
    }
#ifndef NDEBUG
    {
      for(unsigned int itri=0;itri<aTri.size()/3;itri++){
        for(int ied=0;ied<3;ied++){
          int ied1 = triEd[ied][0];
          int ied2 = triEd[ied][1];
          int itri_s = aTriSur[itri*3+ied].first;
          int ied_s0 = aTriSur[itri*3+ied].second;
          assert( aTriSur[itri_s*3+ied_s0].first  == (int)itri );
          assert( aTriSur[itri_s*3+ied_s0].second == ied );
          int ied_s1 = triEd[ied_s0][0];
          int ied_s2 = triEd[ied_s0][1];
          assert( aTri[itri*3+ied1] == aTri[itri_s*3+ied_s2] );
          assert( aTri[itri*3+ied2] == aTri[itri_s*3+ied_s1] );
        }
      }
    }
#endif
    if( itri_ker == -1 ) continue; // inside
    std::vector< std::pair<int,int> > aB;
    std::vector<int> isDelTri( aTri.size()/3, -1 );
    {
      std::vector<int> isLookedEdge( aTri.size(), -1 );
      std::stack< std::pair<int,int> > sBound;
      { // initialize
        sBound.push( aTriSur[itri_ker*3+0] );
        sBound.push( aTriSur[itri_ker*3+1] );
        sBound.push( aTriSur[itri_ker*3+2] );
        isDelTri[itri_ker] = 1;
      }
      for(;;){
        if( sBound.empty() ) break;
        int itri0 = sBound.top().first;
        int ied0  = sBound.top().second;
        sBound.pop();
        if( isLookedEdge[itri0*3+ied0] == 1 ) continue;
        isLookedEdge[itri0*3+ied0] = 1;
        {
          const std::pair<int,int>& s0 = aTriSur[itri0*3+ied0];
          isLookedEdge[s0.first*3+s0.second] = 1;
        }
        isDelTri[itri0] = ( IsOut(itri0,v,aXYZ,aTri) ) ? 1 : 0;
        if( isDelTri[itri0] == 1 ){ // expand this boundary
          int ied1 = triEd[ied0][0];
          int ied2 = triEd[ied0][1];
          const std::pair<int,int>& s1 = aTriSur[itri0*3+ied1];
          const std::pair<int,int>& s2 = aTriSur[itri0*3+ied2];
          assert( aTriSur[s1.first*3+s1.second].first == itri0 );
          assert( aTriSur[s2.first*3+s2.second].first == itri0 );
          sBound.push( s1 );
          sBound.push( s2 );
        }
        else{ // this is a actuall boundary
          aB.emplace_back(itri0,ied0 );
        }
      }
    }
    std::vector<int> aBSur( aB.size()*2, -1);
    {
      for(auto & ib : aB){
        int itri0 = ib.first;
        int itn0  = ib.second;
        int itn1 = triEd[itn0][0];
        int itn2 = triEd[itn0][1];
        int iv1 = aTri[itri0*3+itn1];
        int iv2 = aTri[itri0*3+itn2];
        aBflg[iv1] = -1;
        aBflg[iv2] = -1;
      }
      for(int ib=0;ib<(int)aB.size();ib++){
        int itri0 = aB[ib].first;
        int itn0  = aB[ib].second;
        int itn1 = triEd[itn0][0];
        int itn2 = triEd[itn0][1];
        int iv1 = aTri[itri0*3+itn1];
        int iv2 = aTri[itri0*3+itn2];
        if(      aBflg[iv1] == -2 ){}
        else if( aBflg[iv1] == -1 ){ aBflg[iv1] = ib; }
        else{
          assert( aBflg[iv1] >= 0 );
          int ib0 = aBflg[iv1];
          aBSur[ib *2+1] = ib0;
          aBSur[ib0*2+0] = ib;
          aBflg[iv1] = -2;
        }
        if(      aBflg[iv2] == -2 ){}
        else if( aBflg[iv2] == -1 ){ aBflg[iv2] = ib; }
        else{
          assert( aBflg[iv2] >= 0 );
          int ib0 = aBflg[iv2];
          aBSur[ib *2+0] = ib0;
          aBSur[ib0*2+1] = ib;
          aBflg[iv2] = -2;
        }
      }
    }
#ifndef NDEBUG
    for(std::size_t ib=0;ib<aB.size();ib++){
      for(int inb=0;inb<2;inb++){
        int itri0 = aB[ib].first;
        int itn0  = aB[ib].second;
        int iv1 = aTri[itri0*3+triEd[itn0][0]];
        int iv2 = aTri[itri0*3+triEd[itn0][1]];
        int ib_s0 = aBSur[ib*2+inb];
        int itri_s0 = aB[ib_s0].first;
        int itn_s0  = aB[ib_s0].second;
        int iv_s1 = aTri[itri_s0*3+triEd[itn_s0][0]];
        int iv_s2 = aTri[itri_s0*3+triEd[itn_s0][1]];
        if( inb == 0 ){ assert( iv2 == iv_s1 ); }
        else{           assert( iv1 == iv_s2 ); }
      }
    }
#endif
    std::vector<int> mapOld2New( aTri.size()/3, -1 );
    std::vector<int> aTri1; aTri1.reserve(aTri.size()+60);
    std::vector< std::pair<int,int> > aTriSur1; aTriSur1.reserve(aTriSur.size()+60);
    for(int itri=0;itri<(int)aTri.size()/3;itri++){
      if( isDelTri[itri] ==  1) continue;
      assert( !IsOut(itri,v,aXYZ,aTri) );
      // itri is inside
      mapOld2New[itri] = (int)aTri1.size()/3;
      aTri1.push_back( aTri[itri*3+0] );
      aTri1.push_back( aTri[itri*3+1] );
      aTri1.push_back( aTri[itri*3+2] );
      aTriSur1.emplace_back(-1,0 );
      aTriSur1.emplace_back(-1,0 );
      aTriSur1.emplace_back(-1,0 );
    }
    for(int itri=0;itri<(int)aTri.size()/3;itri++){ // set old relation
      if( isDelTri[itri] ==  1) continue;
      int jtri0 = mapOld2New[itri];
      assert( jtri0 >= 0 && (int)aTri1.size()/3 );
      for(int iet=0;iet<3;iet++){
        int itri_s = aTriSur[itri*3+iet].first;
        if( mapOld2New[itri_s] == -1 ) continue;
        aTriSur1[jtri0*3+iet].first = mapOld2New[itri_s];
        aTriSur1[jtri0*3+iet].second = aTriSur[itri*3+iet].second;
      }
    }
    const int ntri_old = (int)aTri1.size()/3;
    for(int ib=0;ib<(int)aB.size();ib++){
      int itri0 = aB[ib].first;
      int itn0  = aB[ib].second;
      int itn1 = triEd[itn0][0];
      int itn2 = triEd[itn0][1];
      assert( !IsOut(itri0,v,aXYZ,aTri) );
#ifndef NDEBUG
      {
        int itri_s = aTriSur[itri0*3+itn0].first;
        assert( IsOut(itri_s,v,aXYZ,aTri) );
        int ied_s0 = aTriSur[itri0*3+itn0].second;
        assert( aTriSur[itri_s*3+ied_s0].first == itri0 );
        assert( aTriSur[itri_s*3+ied_s0].second == itn0 );
        int ied_s1 = triEd[ied_s0][0];
        int ied_s2 = triEd[ied_s0][1];
        assert( aTri[itri0*3+itn1] == aTri[itri_s*3+ied_s2] );
        assert( aTri[itri0*3+itn2] == aTri[itri_s*3+ied_s1] );
      }
#endif
      assert( isDelTri[itri0] == 0 );
      int jtri0 = mapOld2New[itri0]; assert( jtri0 != -1 );
      int jtri1 = (int)aTri1.size()/3;
      assert( jtri1 == ntri_old + ib );
      aTri1.push_back( iv );
      aTri1.push_back( aTri[itri0*3+itn2] );
      aTri1.push_back( aTri[itri0*3+itn1] );
      aTriSur1[jtri0*3+itn0] = std::make_pair(jtri1,0);
      ////
      int jtri2 = aBSur[ib*2+0] + ntri_old;
      int jtri3 = aBSur[ib*2+1] + ntri_old;
      aTriSur1.emplace_back(jtri0,itn0 );
      aTriSur1.emplace_back(jtri3,2 );
      aTriSur1.emplace_back(jtri2,1 );
    }
    aTri    = aTri1;
    aTriSur = aTriSur1;
  }
}
#ifndef DFM2_HEADER_ONLY
template void delfem2::ConvexHull(
    std::vector<int>& aTri,
    const std::vector<CVec3d>& aXYZ);
#endif

