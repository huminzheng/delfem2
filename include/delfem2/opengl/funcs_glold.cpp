/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cstdlib>
#include <cassert>
#include <math.h>
#include <vector>
#include <climits>

#if defined(__APPLE__) && defined(__MACH__) // Mac
  #include <OpenGL/gl.h>
#elif defined(__MINGW32__) // probably I'm using Qt and don't want to use GLUT
  #include <GL/gl.h>
#elif defined(_WIN32) // windows
  #include <windows.h>
  #include <GL/gl.h>
#else // linux
  #include <GL/gl.h>
#endif
#include "delfem2/opengl/funcs_glold.h"

// -----------------------------------------------------------

namespace delfem2{
namespace opengl{
namespace funcs_glold{

const int noelElemFace_Hex[8][4] = { // this numbering is corresponds to VTK_HEX
    { 0, 4, 7, 3 }, // -x
    { 1, 2, 6, 5 }, // +x
    { 0, 1, 5, 4 }, // -y
    { 3, 7, 6, 2 }, // +y
    { 0, 3, 2, 1 }, // -z
    { 4, 5, 6, 7 }  // +z
};

DFM2_INLINE void UnitNormalAreaTri3D
 (double n[3], double& a,
  const double v1[3], const double v2[3], const double v3[3])
{
  n[0] = ( v2[1] - v1[1] )*( v3[2] - v1[2] ) - ( v3[1] - v1[1] )*( v2[2] - v1[2] );
  n[1] = ( v2[2] - v1[2] )*( v3[0] - v1[0] ) - ( v3[2] - v1[2] )*( v2[0] - v1[0] );
  n[2] = ( v2[0] - v1[0] )*( v3[1] - v1[1] ) - ( v3[0] - v1[0] )*( v2[1] - v1[1] );
  a = sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2])*0.5;
  const double invlen = 0.5/a;
  n[0]*=invlen;  n[1]*=invlen;  n[2]*=invlen;
}

DFM2_INLINE void Cross3D
 (double r[3], const double v1[3], const double v2[3]){
  r[0] = v1[1]*v2[2] - v2[1]*v1[2];
  r[1] = v1[2]*v2[0] - v2[2]*v1[0];
  r[2] = v1[0]*v2[1] - v2[0]*v1[1];
}

DFM2_INLINE double Length3D(const double v[3]){
  return sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
}

DFM2_INLINE double SquareLength3D(const double v[3]){
  return v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
}

DFM2_INLINE void GetVertical2Vector3D
(const double vec_n[3],
 double vec_x[3], double vec_y[3])
{
  const double vec_s[3] = {0,1,0};
  Cross3D(vec_x,vec_s,vec_n);
  const double len = Length3D(vec_x);
  if( len < 1.0e-10 ){
    const double vec_t[3] = {1,0,0};
    Cross3D(vec_x,vec_t,vec_n);  // z????
    Cross3D(vec_y,vec_n,vec_x);  // x????
  }
  else{
    const double invlen = 1.0/len;
    vec_x[0] *= invlen;
    vec_x[1] *= invlen;
    vec_x[2] *= invlen;
    Cross3D(vec_y,vec_n,vec_x);
  }
}

DFM2_INLINE void myGlVertex3d
 (unsigned int ixyz,
  const std::vector<double>& aXYZ )
{
  ::glVertex3d(aXYZ[ixyz*3+0], aXYZ[ixyz*3+1], aXYZ[ixyz*3+2] );
}

DFM2_INLINE void myGlVertex2d
 (unsigned int ixy,
  const std::vector<double>& aXY )
{
  ::glVertex2d(aXY[ixy*2+0], aXY[ixy*2+1] );
}

DFM2_INLINE void myGlNorm3d
 (unsigned int ixyz,
  const std::vector<double>& aNorm )
{
  ::glNormal3d(aNorm[ixyz*3+0], aNorm[ixyz*3+1], aNorm[ixyz*3+2] );
}

DFM2_INLINE void DrawSingleTri3D_FaceNorm
 (const double* aXYZ,
  const unsigned int* aIndXYZ,
  const double* pUV)
{
  const unsigned int i0 = aIndXYZ[0]; //assert( i0>=0&&i0<(int)aXYZ.size()/3 );
  const unsigned int i1 = aIndXYZ[1]; //assert( i1>=0&&i1<(int)aXYZ.size()/3 );
  const unsigned int i2 = aIndXYZ[2]; //assert( i2>=0&&i2<(int)aXYZ.size()/3 );
  if( i0 == UINT_MAX ){
    assert(i1==UINT_MAX); assert(i2==UINT_MAX);
    return;
  }
  const double p0[3] = {aXYZ[i0*3+0], aXYZ[i0*3+1], aXYZ[i0*3+2]};
  const double p1[3] = {aXYZ[i1*3+0], aXYZ[i1*3+1], aXYZ[i1*3+2]};
  const double p2[3] = {aXYZ[i2*3+0], aXYZ[i2*3+1], aXYZ[i2*3+2]};
  double un[3], area; UnitNormalAreaTri3D(un,area, p0,p1,p2);
  ::glNormal3dv(un);
  if( pUV != nullptr ){ ::glTexCoord2d(pUV[0],pUV[1]); }
  ::glVertex3dv(p0);
  if( pUV != nullptr ){ ::glTexCoord2d(pUV[2],pUV[3]); }
  ::glVertex3dv(p1);
  if( pUV != nullptr ){ ::glTexCoord2d(pUV[4],pUV[5]); }
  ::glVertex3dv(p2);
}

DFM2_INLINE void DrawSingleQuad3D_FaceNorm
 (const double* aXYZ,
  const unsigned int* aIndXYZ,
  const double* pUV)
{
  const unsigned int i0 = aIndXYZ[0]; //assert( i0 >= 0 && i0 < (int)aXYZ.size()/3 );
  const unsigned int i1 = aIndXYZ[1]; //assert( i1 >= 0 && i1 < (int)aXYZ.size()/3 );
  const unsigned int i2 = aIndXYZ[2]; //assert( i2 >= 0 && i2 < (int)aXYZ.size()/3 );
  const unsigned int i3 = aIndXYZ[3]; //assert( i3 >= 0 && i3 < (int)aXYZ.size()/3 );
  if( i0 == UINT_MAX ){
    assert(i1==UINT_MAX && i2==UINT_MAX && i3==UINT_MAX);
    return;
  }
  const double p0[3] = {aXYZ[i0*3+0], aXYZ[i0*3+1], aXYZ[i0*3+2]};
  const double p1[3] = {aXYZ[i1*3+0], aXYZ[i1*3+1], aXYZ[i1*3+2]};
  const double p2[3] = {aXYZ[i2*3+0], aXYZ[i2*3+1], aXYZ[i2*3+2]};
  const double p3[3] = {aXYZ[i3*3+0], aXYZ[i3*3+1], aXYZ[i3*3+2]};
  {
    double un0[3], area; UnitNormalAreaTri3D(un0,area,  p0, p1, p3);
    if( pUV != nullptr ){ ::glTexCoord2d(pUV[0],pUV[1]); }
    ::glNormal3dv(un0);
    ::glVertex3dv(p0);
  }
  {
    double un1[3], area; UnitNormalAreaTri3D(un1,area,  p0, p1, p2);
    if( pUV != nullptr ){ ::glTexCoord2d(pUV[2],pUV[3]); }
    ::glNormal3dv(un1);
    ::glVertex3dv(p1);
  }
  {
    double un2[3], area; UnitNormalAreaTri3D(un2,area,  p1, p2, p3);
    if( pUV != nullptr ){ ::glTexCoord2d(pUV[4],pUV[5]); }
    ::glNormal3dv(un2);
    ::glVertex3dv(p2);
  }
  {
    double un3[3], area; UnitNormalAreaTri3D(un3,area,  p2, p3, p0);
    if( pUV != nullptr ){ ::glTexCoord2d(pUV[6],pUV[7]); }
    ::glNormal3dv(un3);
    ::glVertex3dv(p3);
  }
}

DFM2_INLINE void Draw_SurfaceMeshEdge
 (unsigned int nXYZ, const double* paXYZ,
  unsigned int nTri, const unsigned int* paTri)
{
  ::glEnableClientState(GL_VERTEX_ARRAY);
  ::glVertexPointer(3 , GL_DOUBLE , 0 , paXYZ);
  ::glBegin(GL_LINES);
  for(unsigned int itri=0;itri<nTri;itri++){
    const unsigned int i1 = paTri[itri*3+0];
    const unsigned int i2 = paTri[itri*3+1];
    const unsigned int i3 = paTri[itri*3+2];
    glArrayElement(i1);
    glArrayElement(i2);
    glArrayElement(i2);
    glArrayElement(i3);
    glArrayElement(i3);
    glArrayElement(i1);
  }
  ::glEnd();
  ::glDisableClientState(GL_VERTEX_ARRAY);
}

DFM2_INLINE void Draw_SurfaceMeshFace
 (unsigned int nXYZ, const double* paXYZ,
  unsigned int nTri, const unsigned int* paTri)
{
  ::glEnableClientState(GL_VERTEX_ARRAY);
  ::glVertexPointer(3 , GL_DOUBLE , 0 , paXYZ);
  ::glDrawElements(GL_TRIANGLES , nTri*3 , GL_UNSIGNED_INT , paTri);
  ::glDisableClientState(GL_VERTEX_ARRAY);
 /*
   /////
   ::glColor3d(1,1,1);
   ::glBegin(GL_TRIANGLES);
   for(unsigned int itri=0;itri<nTri;itri++){
   const unsigned int i1 = paTri[itri*3+0];
   const unsigned int i2 = paTri[itri*3+1];
   const unsigned int i3 = paTri[itri*3+2];
   ::glVertex3dv(paXYZ+i1*3);
   ::glVertex3dv(paXYZ+i2*3);
   ::glVertex3dv(paXYZ+i3*3);
   }
   ::glEnd();
   ::glColor3d(0,0,0);
   ::glBegin(GL_LINES);
   for(unsigned int itri=0;itri<nTri;itri++){
   const unsigned int i1 = paTri[itri*3+0];
   const unsigned int i2 = paTri[itri*3+1];
   const unsigned int i3 = paTri[itri*3+2];
   ::glVertex3dv(paXYZ+i1*3);
   ::glVertex3dv(paXYZ+i2*3);
   ::glVertex3dv(paXYZ+i2*3);
   ::glVertex3dv(paXYZ+i3*3);
   ::glVertex3dv(paXYZ+i3*3);
   ::glVertex3dv(paXYZ+i1*3);
   }
   ::glEnd();
   */
}

DFM2_INLINE void drawLoop2d
 (const std::vector<double>& vec)
{
  ::glBegin(GL_LINES);
  const unsigned int nvec = (int)vec.size()/2;
  for(unsigned int ivec=0;ivec<nvec;ivec++){
    unsigned int jvec = ivec+1; if( jvec >= nvec ){ jvec -= nvec; }
    myGlVertex2d(ivec,vec);
    myGlVertex2d(jvec,vec);
  }
  ::glEnd();
    ////
  ::glBegin(GL_POINTS);
  for(unsigned int ivec=0;ivec<nvec;ivec++){
    myGlVertex2d(ivec,vec);
  }
  ::glEnd();
}

DFM2_INLINE void DrawMeshTriMap3D_Edge
 (const std::vector<double>& aXYZ,
  const std::vector<int>& aTri,
  const std::vector<int>& map)
{
  GLboolean is_lighting = glIsEnabled(GL_LIGHTING);
  const int nTri = (int)aTri.size()/3;
  ::glDisable(GL_LIGHTING);
  ::glBegin(GL_LINES);
  ::glColor3d(0, 0, 0);
  for (int itri = 0; itri<nTri; ++itri){
    const int j1 = aTri[itri*3+0];
    const int j2 = aTri[itri*3+1];
    const int j3 = aTri[itri*3+2];
    if( j1 == -1 ){
      assert(j2==-1); assert(j3==-1);
      continue;
    }
    const int i1 = map[j1];
    const int i2 = map[j2];
    const int i3 = map[j3];
    assert(i1>=0 && i1<(int)aXYZ.size()/3);
    assert(i2>=0 && i2<(int)aXYZ.size()/3);
    assert(i3>=0 && i3<(int)aXYZ.size()/3);
    double p1[3] = { aXYZ[i1*3+0], aXYZ[i1*3+1], aXYZ[i1*3+2] };
    double p2[3] = { aXYZ[i2*3+0], aXYZ[i2*3+1], aXYZ[i2*3+2] };
    double p3[3] = { aXYZ[i3*3+0], aXYZ[i3*3+1], aXYZ[i3*3+2] };
    ::glVertex3dv(p1); ::glVertex3dv(p2);
    ::glVertex3dv(p2); ::glVertex3dv(p3);
    ::glVertex3dv(p3); ::glVertex3dv(p1);
  }
  ::glEnd();
  
  if (is_lighting){ ::glEnable(GL_LIGHTING); }
}


DFM2_INLINE void ShadowMatrix(float m[16], const float plane[4], float lpos[3])
{
  float dot = plane[0]*lpos[0] + plane[1]*lpos[1] + plane[2]*lpos[2] + plane[3];
  for(int j=0; j<4;++j){
    for(int i=0; i<4; ++i){
      m[j*4+i] = - plane[j]*lpos[i];
      if (i == j){ m[j*4+i] += dot; }
    }
  }
}


DFM2_INLINE void DrawMeshTri3DPart_FaceNorm
 (const std::vector<double>& aXYZ,
  const std::vector<unsigned int>& aTri,
  const std::vector<int>& aIndTri)
{
  ::glBegin(GL_TRIANGLES);
  for(int itri : aIndTri){
    assert( itri>=0&&itri<(int)aTri.size()/3 );
    DrawSingleTri3D_FaceNorm(aXYZ.data(), aTri.data()+itri*3,0);
  }
  ::glEnd();
}

DFM2_INLINE void DrawMeshTri3D_FaceNorm_Flg
 (const std::vector<double>& aXYZ,
  const std::vector<unsigned int>& aTri,
  int iflg,
  const std::vector<int>& aFlgTri)
{
  const int nTri = (int)aTri.size()/3;
  //  const int nXYZ = (int)aXYZ.size()/3;
  ::glBegin(GL_TRIANGLES);
  for(int itri=0;itri<nTri;++itri){
    const int iflg0 = aFlgTri[itri];
    if( iflg0 != iflg ) continue;
    DrawSingleTri3D_FaceNorm(aXYZ.data(), aTri.data()+itri*3,0);
  }
  ::glEnd();
}


DFM2_INLINE void DrawMeshTri3D_FaceEdge
 (const std::vector<double>& aXYZ,
  const std::vector<int>& aTri)
{
  const std::size_t nTri = aTri.size()/3;
  ::glBegin(GL_TRIANGLES);
  for(unsigned int itri=0;itri<nTri;itri++){
    const int i1 = aTri[itri*3+0];
    const int i2 = aTri[itri*3+1];
    const int i3 = aTri[itri*3+2];
    myGlVertex3d(i1,aXYZ);
    myGlVertex3d(i2,aXYZ);
    myGlVertex3d(i3,aXYZ);
  }
  ::glEnd();
  // ------------------------------------
  ::glColor3d(0,0,0);
  ::glBegin(GL_LINES);
  for(unsigned int itri=0;itri<nTri;itri++){
    const unsigned int i1 = aTri[itri*3+0];
    const unsigned int i2 = aTri[itri*3+1];
    const unsigned int i3 = aTri[itri*3+2];
    myGlVertex3d(i1,aXYZ);
    myGlVertex3d(i2,aXYZ);
    myGlVertex3d(i2,aXYZ);
    myGlVertex3d(i3,aXYZ);
    myGlVertex3d(i3,aXYZ);
    myGlVertex3d(i1,aXYZ);
  }
  ::glEnd();
}

}
}
}


// end functions that do not expose
// =====================================================
//

DFM2_INLINE void delfem2::opengl::DrawAxis(double s)
{
  GLboolean is_lighting = ::glIsEnabled(GL_LIGHTING);
  ::glDisable(GL_LIGHTING);
  ::glBegin(GL_LINES);
  ::glColor3d(1,0,0);
  ::glVertex3d(0,0,0);
  ::glVertex3d(s,0,0);
  ::glColor3d(0,1,0);
  ::glVertex3d(0,0,0);
  ::glVertex3d(0,s,0);
  ::glColor3d(0,0,1);
  ::glVertex3d(0,0,0);
  ::glVertex3d(0,0,s);
  ::glEnd();
  if( is_lighting ){ ::glEnable(GL_LIGHTING); }
}

DFM2_INLINE void delfem2::opengl::CAxisXYZ::Draw() const{
  glLineWidth(line_width);
  DrawAxis(len);
}


DFM2_INLINE void delfem2::opengl::DrawSphere
(int nla, int nlo)
{
  if( nla <= 1 || nlo <= 2 ){ return; }
  const double pi = 3.1415926535;
  double dla = 2.0*pi/nla;
  double dlo = pi/nlo;
  ::glBegin(GL_QUADS);
  for(int ila=0;ila<nla;ila++){
    for(int ilo=0;ilo<nlo;ilo++){
      double rla0 = (ila+0)*dla;
      double rla1 = (ila+1)*dla;
      double rlo0 = (ilo+0)*dlo;
      double rlo1 = (ilo+1)*dlo;
      double p0[3] = { cos(rla0)*cos(rlo0), cos(rla0)*sin(rlo0), sin(rla0) };
      double p1[3] = { cos(rla0)*cos(rlo1), cos(rla0)*sin(rlo1), sin(rla0) };
      double p2[3] = { cos(rla1)*cos(rlo1), cos(rla1)*sin(rlo1), sin(rla1) };
      double p3[3] = { cos(rla1)*cos(rlo0), cos(rla1)*sin(rlo0), sin(rla1) };
      ::glVertex3dv(p0);
      ::glVertex3dv(p1);
      ::glVertex3dv(p2);
      ::glVertex3dv(p3);
    }
  }
  ::glEnd();
}

DFM2_INLINE void delfem2::opengl::DrawSphereAt
 (int nla, int nlo, double rad, double x, double y, double z)
{
  ::glTranslated(+x,+y,+z);
  ::glScaled(rad, rad, rad);
  DrawSphere(nla,nlo);
  ::glScaled(1.0/rad, 1.0/rad, 1.0/rad);
  ::glTranslated(-x,-y,-z);
}

DFM2_INLINE void delfem2::opengl::DrawSphere_Edge
(double radius_)
{
  const bool is_lighting = ::glIsEnabled(GL_LIGHTING);
  const bool is_texture = ::glIsEnabled(GL_TEXTURE_2D);
  ::glDisable(GL_LIGHTING);
  ::glDisable(GL_TEXTURE_2D);
  
  const unsigned int nlg = 32;
  const unsigned int nlt = 18;
  const double rlg = 6.28/nlg;  // longtitude
  //  const double rlt = 6.28/nlt;  // latitude
  //  const double divlg = 6.28/ndiv_lg;
  //  const double divlt = 6.28/ndiv_lt;
  const unsigned int ndiv = 32;
  const double rdiv = 6.28/ndiv;
  for(unsigned int ilg=0;ilg<nlg;ilg++){
    ::glBegin(GL_LINE_LOOP);
    for(unsigned int idiv=0;idiv<ndiv;idiv++){
      ::glVertex3d(radius_*cos(idiv*rdiv)*cos(ilg*rlg),
                   radius_*cos(idiv*rdiv)*sin(ilg*rlg),
                   radius_*sin(idiv*rdiv) );
    }
    ::glEnd();
  }
  for(unsigned int ilt=0;ilt<nlt;ilt++){
    const double d = ((double)ilt/nlt-0.5)*radius_*2.0;
    const double r0 = sqrt(radius_*radius_-d*d);
    ::glBegin(GL_LINE_LOOP);
    for(unsigned int idiv=0;idiv<ndiv;idiv++){
      ::glVertex3d(r0*cos(idiv*rdiv),
                   r0*sin(idiv*rdiv),
                   d);
    }
    ::glEnd();
  }
  //   ::glutWireSphere(radius_,32,32);
  if(is_lighting){ glEnable(GL_LIGHTING); }
  if(is_texture ){ glEnable(GL_TEXTURE_2D); }
}


// -----------------------------------------------------------

DFM2_INLINE void delfem2::opengl::DrawTorus_Edge
(double radius_, double radius_tube_)
{
  const bool is_lighting = ::glIsEnabled(GL_LIGHTING);
  const bool is_texture = ::glIsEnabled(GL_TEXTURE_2D);
  ::glDisable(GL_LIGHTING);
  ::glDisable(GL_TEXTURE_2D);
  const unsigned int nlg = 32;
  const unsigned int nlt = 18;
  const double rlg = 6.28/nlg;  // longtitude
  const double rlt = 6.28/nlt;  // latitude
  const unsigned int ndiv = 32;
  const double rdiv = 6.28/ndiv;
  for(unsigned int ilg=0;ilg<nlg;ilg++){
    ::glBegin(GL_LINE_LOOP);
    for(unsigned int idiv=0;idiv<ndiv;idiv++){
      ::glVertex3d(( radius_ + radius_tube_*cos(idiv*rdiv) )*sin(ilg*rlg),
                   ( radius_ + radius_tube_*cos(idiv*rdiv) )*cos(ilg*rlg),
                   radius_tube_*sin(idiv*rdiv) );
    }
    ::glEnd();
  }
  for(unsigned int ilt=0;ilt<nlt;ilt++){
    double d  = radius_tube_*sin(ilt*rlt);
    double r0 = radius_tube_*cos(ilt*rlt) + radius_;
    ::glBegin(GL_LINE_LOOP);
    for(unsigned int idiv=0;idiv<ndiv;idiv++){
      ::glVertex3d(r0*cos(idiv*rdiv),
                   r0*sin(idiv*rdiv),
                   d);
    }
    ::glEnd();
  }
  if(is_lighting){ ::glEnable(GL_LIGHTING); }
  if(is_texture) { ::glEnable(GL_TEXTURE_2D); }
}


// -----------------------------------------------

DFM2_INLINE void delfem2::opengl::DrawTorus_Solid
 (double rl, // radius of longtitude
  double rm,
  double scale_tex) // radius of meridian
{
  const unsigned int nl = 32;
  const double dl = M_PI*2.0/nl;  // longtitude
  const unsigned int nm = 32;
  const double dm = M_PI*2.0/nm;
  ::glBegin(GL_QUADS);
  for(unsigned int i=0;i<nl;i++){
    for(unsigned int j=0;j<nm;j++){
      ::glTexCoord2d(scale_tex*(i+0.0)/nl,
                     scale_tex*(j+0.0)/nm);
      ::glNormal3d( cos((j+0)*dm)*sin((i+0)*dl),
                    cos((j+0)*dm)*cos((i+0)*dl),
                    sin((j+0)*dm) );
      ::glVertex3d((rm*cos((j+0)*dm)+rl )*sin((i+0)*dl),
                   (rm*cos((j+0)*dm)+rl )*cos((i+0)*dl),
                    rm*sin((j+0)*dm) );
      // -------
      ::glTexCoord2d(scale_tex*(i+1.0)/nl,
                     scale_tex*(j+0.0)/nm);
      ::glNormal3d( cos((j+0)*dm)*sin((i+1)*dl),
                    cos((j+0)*dm)*cos((i+1)*dl),
                    sin((j+0)*dm) );
      ::glVertex3d((rm*cos((j+0)*dm)+rl )*sin((i+1)*dl),
                   (rm*cos((j+0)*dm)+rl )*cos((i+1)*dl),
                    rm*sin((j+0)*dm) );
      // ------
      ::glTexCoord2d(scale_tex*(i+1.0)/nl,
                     scale_tex*(j+1.0)/nm);
      ::glNormal3d( cos((j+1)*dm)*sin((i+1)*dl),
                    cos((j+1)*dm)*cos((i+1)*dl),
                    sin((j+1)*dm) );
      ::glVertex3d((rm*cos((j+1)*dm)+rl )*sin((i+1)*dl),
                   (rm*cos((j+1)*dm)+rl )*cos((i+1)*dl),
                    rm*sin((j+1)*dm) );
      // ----
      ::glTexCoord2d(scale_tex*(i+0.0)/nl,
                     scale_tex*(j+1.0)/nm);
      ::glNormal3d( cos((j+1)*dm)*sin((i+0)*dl),
                    cos((j+1)*dm)*cos((i+0)*dl),
                    sin((j+1)*dm) );
      ::glVertex3d((rm*cos((j+1)*dm)+rl )*sin((i+0)*dl),
                   (rm*cos((j+1)*dm)+rl )*cos((i+0)*dl),
                    rm*sin((j+1)*dm) );
    }
  }
  ::glEnd();
}

// ---------------------------------------------------

DFM2_INLINE void delfem2::opengl::DrawCylinder_Face
(const double* dir_, double radius_, const double* cent_)
{
  const bool is_texture = ::glIsEnabled(GL_TEXTURE_2D);
  ::glDisable(GL_TEXTURE_2D);
  //
  ::glLineWidth(1);
  ::glColor3d(1,0,0);
  const float brown[3] = {0.7f, 0.20f, 0.15f};
  ::glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, brown);
  ::glMatrixMode(GL_MODELVIEW);
  ::glPushMatrix();
  
  double h[3],v[3];
  {
    double x[3] = { 1,0,0 };
    funcs_glold::Cross3D(h, dir_, x);
    if( funcs_glold::SquareLength3D(h) > 1.0e-5 ){
      double invlenh=1.0/funcs_glold::Length3D(h);
      h[0]*=invlenh;
      h[1]*=invlenh;
      h[2]*=invlenh;
      funcs_glold::Cross3D(v, h, dir_);
    }
    else{
      double y[3] = { 0,1,0 };
      funcs_glold::Cross3D(h, dir_, y);
      double invlenh=1.0/funcs_glold::Length3D(h);
      h[0]*=invlenh;
      h[1]*=invlenh;
      h[2]*=invlenh;
      funcs_glold::Cross3D(v, h, dir_);
    }
  }
  
  const unsigned int nl = 20;
  const unsigned int nr = 32;
  const double rr = 6.28/nr;
  
  for(unsigned int ir=0;ir<nr;ir++){
    double rs0 = +radius_*sin(ir*rr);
    double rc0 = +radius_*cos(ir*rr);
    double rs1 = +radius_*sin(ir*rr+rr);
    double rc1 = +radius_*cos(ir*rr+rr);
    
    double v0[3] = {
      cent_[0]+rs0*h[0]+rc0*v[0],
      cent_[1]+rs0*h[1]+rc0*v[1],
      cent_[2]+rs0*h[2]+rc0*v[2] };
    double v1[3] = {
      cent_[0]+rs1*h[0]+rc1*v[0],
      cent_[1]+rs1*h[1]+rc1*v[1],
      cent_[2]+rs1*h[2]+rc1*v[2] };
    ::glBegin(GL_TRIANGLES);
    ::glNormal3d(+dir_[0],+dir_[1],+dir_[2]);
    ::glVertex3d(cent_[0]+dir_[0], cent_[1]+dir_[1], cent_[2]+dir_[2]);
    ::glVertex3d(dir_[0]+v0[0], dir_[1]+v0[1], dir_[2]+v0[2]);
    ::glVertex3d(dir_[0]+v1[0], dir_[1]+v1[1], dir_[2]+v1[2]);
    /////
    ::glNormal3d(-dir_[0],-dir_[1],-dir_[2]);
    ::glVertex3d(cent_[0]-dir_[0], cent_[1]-dir_[1], cent_[2]-dir_[2]);
    ::glVertex3d(-dir_[0]+v0[0],-dir_[1]+v0[1],-dir_[2]+v0[2]);
    ::glVertex3d(-dir_[0]+v1[0],-dir_[1]+v1[1],-dir_[2]+v1[2]);
    ::glEnd();
    /////
    double s0 = sin(ir*rr);
    double c0 = cos(ir*rr);
    double n0[3] = {
      s0*h[0]+c0*v[0],
      s0*h[1]+c0*v[1],
      s0*h[2]+c0*v[2] };
    /////
    double s1 = sin(ir*rr+rr);
    double c1 = cos(ir*rr+rr);
    double n1[3] = {
      s1*h[0]+c1*v[0],
      s1*h[1]+c1*v[1],
      s1*h[2]+c1*v[2] };
    
    ::glBegin(GL_QUADS);
    for(unsigned int il=0;il<nl;il++){
      double l0 = -1+(2.0/nl)*il;
      double l1 = -1+(2.0/nl)*(il+1);
      ::glNormal3d(+n0[0],+n0[1],+n0[2]);
      ::glVertex3d(l0*dir_[0]+v0[0], l0*dir_[1]+v0[1], l0*dir_[2]+v0[2]);
      ::glVertex3d(l1*dir_[0]+v0[0], l1*dir_[1]+v0[1], l1*dir_[2]+v0[2]);
      ::glNormal3d(+n1[0],+n1[1],+n1[2]);
      ::glVertex3d(l1*dir_[0]+v1[0], l1*dir_[1]+v1[1], l1*dir_[2]+v1[2]);
      ::glVertex3d(l0*dir_[0]+v1[0], l0*dir_[1]+v1[1], l0*dir_[2]+v1[2]);
    }
    ::glEnd();
  }
  ::glPopMatrix();
  ////
  if(is_texture ){ glEnable(GL_TEXTURE_2D); }
}

DFM2_INLINE void delfem2::opengl::DrawCylinder_Edge
(const double* dir_, double radius_, const double* cent_)
{
  const bool is_lighting = ::glIsEnabled(GL_LIGHTING);
  const bool is_texture = ::glIsEnabled(GL_TEXTURE_2D);
  ::glDisable(GL_LIGHTING);
  ::glDisable(GL_TEXTURE_2D);
  ////
  ::glLineWidth(1);
  ::glColor3d(1,0,0);
  ::glMatrixMode(GL_MODELVIEW);
  ::glPushMatrix();
  
  double h[3],v[3];
  {
    double x[3] = { 1,0,0 };
    funcs_glold::Cross3D(h, dir_, x);
    if( funcs_glold::SquareLength3D(h) > 1.0e-5 ){
      double invlenh=1.0/funcs_glold::Length3D(h);
      h[0]*=invlenh;
      h[1]*=invlenh;
      h[2]*=invlenh;
      funcs_glold::Cross3D(v, h, dir_);
    }
    else{
      double y[3] = { 0,1,0 };
      funcs_glold::Cross3D(h, dir_, y);
      double invlenh=1.0/funcs_glold::Length3D(h);
      h[0]*=invlenh;
      h[1]*=invlenh;
      h[2]*=invlenh;
      funcs_glold::Cross3D(v, h, dir_);
    }
  }
  
  const unsigned int nr = 32;
  const double rr = 6.28/nr;
  ::glBegin(GL_LINES);
  for(unsigned int ir=0;ir<nr;ir++){
    double rs = +radius_*sin(ir*rr);
    double rc = +radius_*cos(ir*rr);
    ::glVertex3d(cent_[0]+dir_[0],
                 cent_[1]+dir_[1],
                 cent_[2]+dir_[2]);
    ::glVertex3d(cent_[0]+dir_[0]+rs*h[0]+rc*v[0],
                 cent_[1]+dir_[1]+rs*h[1]+rc*v[1],
                 cent_[2]+dir_[2]+rs*h[2]+rc*v[2]);
    ::glVertex3d(cent_[0]-dir_[0],
                 cent_[1]-dir_[1],
                 cent_[2]-dir_[2]);
    ::glVertex3d(cent_[0]-dir_[0]+rs*h[0]+rc*v[0],
                 cent_[1]-dir_[1]+rs*h[1]+rc*v[1],
                 cent_[2]-dir_[2]+rs*h[2]+rc*v[2]);
    ::glVertex3d(cent_[0]+dir_[0]+rs*h[0]+rc*v[0],
                 cent_[1]+dir_[1]+rs*h[1]+rc*v[1],
                 cent_[2]+dir_[2]+rs*h[2]+rc*v[2]);
    ::glVertex3d(cent_[0]-dir_[0]+rs*h[0]+rc*v[0],
                 cent_[1]-dir_[1]+rs*h[1]+rc*v[1],
                 cent_[2]-dir_[2]+rs*h[2]+rc*v[2]);
  }
  ::glEnd();
  ::glPopMatrix();
  //
  if(is_lighting){ glEnable(GL_LIGHTING); }
  if(is_texture ){ glEnable(GL_TEXTURE_2D); }
}

DFM2_INLINE void delfem2::opengl::DrawPlane_Edge
(const double* origin_, const double* normal_)
{
  const bool is_lighting = ::glIsEnabled(GL_LIGHTING);
  ::glDisable(GL_LIGHTING);
  ::glLineWidth(1);
  ::glColor3d(1,0,0);
  double d1[3],d2[3];
  funcs_glold::GetVertical2Vector3D(normal_,d1,d2);
  ::glBegin(GL_LINES);
  for(int i=-10;i<11;i++){
    for(int j=-10;j<11;j++){
      ::glVertex3d(origin_[0]+d1[0]*0.08*(i+0)+d2[0]*0.08*(j+0),
                   origin_[1]+d1[1]*0.08*(i+0)+d2[1]*0.08*(j+0),
                   origin_[2]+d1[2]*0.08*(i+0)+d2[2]*0.08*(j+0));
      ::glVertex3d(origin_[0]+d1[0]*0.08*(i+1)+d2[0]*0.08*(j+0),
                   origin_[1]+d1[1]*0.08*(i+1)+d2[1]*0.08*(j+0),
                   origin_[2]+d1[2]*0.08*(i+1)+d2[2]*0.08*(j+0));
      
      ::glVertex3d(origin_[0]+d1[0]*0.08*(i+0)+d2[0]*0.08*(j+0),
                   origin_[1]+d1[1]*0.08*(i+0)+d2[1]*0.08*(j+0),
                   origin_[2]+d1[2]*0.08*(i+0)+d2[2]*0.08*(j+0));
      ::glVertex3d(origin_[0]+d1[0]*0.08*(i+0)+d2[0]*0.08*(j+1),
                   origin_[1]+d1[1]*0.08*(i+0)+d2[1]*0.08*(j+1),
                   origin_[2]+d1[2]*0.08*(i+0)+d2[2]*0.08*(j+1));
    }
  }
  ::glEnd();
  /*  const double para[3] = { normal_[1], -normal_[0], 0 };
   ::glColor3d(0,0,0);
   ::glBegin(GL_QUADS);
   ::glVertex3d(origin_[0]+para[0]-normal_[0], origin_[1]+para[1]-normal_[1], origin_[2]+para[2]-normal_[2] );
   ::glVertex3d(origin_[0]-para[0]-normal_[0], origin_[1]-para[1]-normal_[1], origin_[2]-para[2]-normal_[2] );
   ::glVertex3d(origin_[0]-para[0], origin_[1]-para[1], origin_[2]-para[2] );
   ::glVertex3d(origin_[0]+para[0], origin_[1]+para[1], origin_[2]+para[2] );
   ::glEnd();*/
  if(is_lighting){ glEnable(GL_LIGHTING); }
}


// -------------------------------------

DFM2_INLINE void delfem2::opengl::setSomeLighting()
{
  glEnable(GL_LIGHTING);
  //  glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, 1.0);
  glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 0.0);
//  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  {
    glEnable(GL_LIGHT0);
    GLfloat light0_Kd[]  = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat light0_Specular[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat light0_Pos[4] = {0.25f, 1.0f, +1.25f, 0.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_Kd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_Specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_Pos);
  }
  {
    glEnable(GL_LIGHT1);
    GLfloat light1_Kd[]  = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat light1_Pos[4] = {0.00f, 0.0f, +1.00f, 0.0f};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_Kd);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_Pos);
  }
}

DFM2_INLINE void delfem2::opengl::setSomeLighting2()
{
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1.0);
  { // initialize light parameter
    GLfloat light0_Kd[]   = {0.9f, 0.3f, 0.3f, 1.0f};
    GLfloat light0_Pos[4] = {+0.5f, -0.5f, +1.0f, 0.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0_Kd);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_Pos);
    ////
    GLfloat light1_Kd[]   = {0.3f, 0.3f, 0.9f, 1.0f};
    GLfloat light1_Pos[4] = {-0.5f, +0.5f, +1.0f, 0.0f};
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  light1_Kd);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_Pos);
  }
}

DFM2_INLINE void delfem2::opengl::setSomeLighting3()
{
  glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 0.0);
  {
    glEnable(GL_LIGHT0);
    GLfloat light0_Kd[]  = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat light0_Specular[4] = {0.5f, 0.5f, .5f, 1.0f};
    GLfloat light0_Pos[4] = {0.25f, 1.0f, +1.25f, 0.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_Kd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_Specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_Pos);
  }
  {
    glEnable(GL_LIGHT1);
    GLfloat light1_Kd[]  = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat light1_Pos[4] = {-1.00f, 0.0f, +1.00f, 0.0f};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_Kd);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_Pos);
  }
}


DFM2_INLINE void delfem2::opengl::drawFloorShadow(void (*DrawObject)(), float yfloor, float wfloor)
{
  GLboolean is_lighting = ::glIsEnabled(GL_LIGHTING);
  {
    ::glClearStencil(0);
    { // draw floor (stencil 1)
      glEnable(GL_STENCIL_TEST);
      glStencilFunc( GL_ALWAYS, 1, ~0);
      glStencilOp(GL_KEEP,GL_KEEP ,GL_REPLACE);
      { // floor
        ::glDisable(GL_LIGHTING);
        glColor4f(0.6f, 0.6f, 0.5f, 1.0f);
        ::glBegin(GL_QUADS);
        ::glNormal3d(0,1,0);
        ::glVertex3d(-wfloor,yfloor,-wfloor);
        ::glVertex3d(+wfloor,yfloor,-wfloor);
        ::glVertex3d(+wfloor,yfloor,+wfloor);
        ::glVertex3d(-wfloor,yfloor,+wfloor);
        ::glEnd();
      }
    }
    { // draw stensil
      glColorMask(0,0,0,0);
      glDepthMask(0);
      glEnable(GL_STENCIL_TEST);
      glStencilFunc( GL_EQUAL, 1, ~0);
      glStencilOp(GL_KEEP,GL_KEEP ,GL_INCR);
      glPushMatrix();
      {
        float plane[4] = {0,1,0,-yfloor-0.001f};
        float lpos[4] = {0,5,0,1};
        float m_shadow[16]; funcs_glold::ShadowMatrix(m_shadow, plane, lpos);
        glMultMatrixf(m_shadow);
      }
      DrawObject();
      glPopMatrix();
      glColorMask(1,1,1,1);
      glDepthMask(1);
    }
    { // draw shadow
      glStencilFunc( GL_EQUAL, 2, ~0 );
      glStencilOp(GL_KEEP, GL_KEEP ,GL_KEEP);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      ::glDisable(GL_DEPTH_TEST);
      ::glDisable(GL_LIGHTING);
      { // draw shadow on floor
        ::glBegin(GL_QUADS);
        glColor4f(0.1f, 0.1f, 0.1f, 0.5f);
        ::glNormal3d(0,1,0);
        ::glVertex3d(-wfloor,yfloor,-wfloor);
        ::glVertex3d(+wfloor,yfloor,-wfloor);
        ::glVertex3d(+wfloor,yfloor,+wfloor);
        ::glVertex3d(-wfloor,yfloor,+wfloor);
        ::glEnd();
      }
      glEnable(GL_DEPTH_TEST);
      glDisable(GL_BLEND);
      glDisable(GL_STENCIL_TEST);
    }
  }
  if( is_lighting ){ ::glEnable(GL_LIGHTING); }
}

// --------------------------------------------------------

static int Ptn035[]={   /* # */
   2,  12,66, 87,66,
   2,  12,33, 87,33,
   2,  37,91, 37, 8,
   2,  62, 8, 62,91,
  -1
};
static int PtnA[]={   /* A */
   3,   0, 0,  50,100, 100,0,
   2,  25,50,  75, 50,
  -1,
};
static int PtnB[]={   /* B */
  10,   0, 0, 0,100, 75,100, 87,91, 87,58, 75,50, 100,33, 100,8, 87,0, 0,0,
   2,  75,50, 0, 50,
  -1
};
static int PtnC[]={   /* C */
   8,  100,83, 75,100, 25,100, 0,83, 0,16, 25,0, 75,0, 100,16,
  -1,
};
static int PtnD[]={   /* D */
   7,  0,100, 75,100, 100,83, 100,16, 75,0, 0,0, 0,100,
  -1,
};
static int PtnE[]={   /* E */
   4,  100,100,  0,100, 0,0, 100,0,
   2,    0, 50, 87, 50,
  -1,
};
static int PtnF[]={   /* F */
   3,  100,100,  0,100, 0,0,
   2,    0, 50, 75, 50,
  -1,
};
static int PtnG[]={   /* G */
  10,  100,83, 75,100, 25,100, 0,83, 0,16, 25,0, 75,0, 100,16, 100,41, 62,41,
  -1,
};
///////////
static int Ptn3[]={   /* 3 */
  11,  12,83, 37,100, 75,100, 100,83, 100,66, 75,50, 100,33, 100,16, 75,0, 25,0, 0,16,
  -1
};
static int Ptn4[]={   /* 4 */
   3,  37,100, 12,25, 87,25,
   2,  62, 75, 62, 0,
  -1
};
static int Ptn5[]={   /* 5 */
  10,  87,100, 12,100, 12,41, 37,58, 62,58, 87,41, 87,16, 62,0, 37,0, 12,16,
  -1
};
static int Ptn6[]={   /* 6 */
  12,  87,83, 62,100, 25,100, 0,83, 0,16, 25,0, 75,0, 100,16, 100,33, 75,50, 25,50, 0,33,
  -1
};
static int Ptn7[]={   /* 7 */
   5,  12,83, 12,100, 87,100, 50,33, 50,0,
  -1
};
static int Ptn8[]={   /* 8 */
   9,  100,83, 75,100, 25,100,  0,83,  0,66,  25,50,  75,50, 100,66, 100,83,
   8,   25,50,  0, 33,  0, 16, 25, 0, 75, 0, 100,16, 100,33,  75,50,
  -1
};
static int Ptn9[]={   /* 9 */
  12,  0,16, 25,0, 75,0, 100,16, 100,83, 75,100, 25,100, 0,83, 0,58, 25,41, 75,41, 100,58,
  -1
};

// x = ax*[x] + bx
// y = ay*[y] + by
DFM2_INLINE void delfem2::opengl::DrawCharacter
(int* pChr,
 double ax, double bx,
 double ay, double by)
{
  assert(pChr!=0);
  int icur = 0;
  for(;;){
    int np = pChr[icur];
    if( np == -1 ) break;
    ::glBegin(GL_LINE_STRIP);
    for(int ip=0;ip<np;ip++){
      int ix0 = pChr[icur+1+ip*2+0];
      int iy0 = pChr[icur+1+ip*2+1];
      double sx = ix0*ax+bx;
      double sy = iy0*ay+by;
      ::glVertex2d(sx,sy);
    }
    ::glEnd();
    icur += np*2+1;
  }
}

// x = ax*[x] + bx
// y = ay*[y] + by
DFM2_INLINE void delfem2::opengl::DrawCharacter
(char ic,
 double ax, double bx,
 double ay, double by)
{
  int* pChr = 0;
  if( ic == '#'){ pChr = Ptn035; }
  if( ic == 'A'){ pChr = PtnA; }
  if( ic == 'B'){ pChr = PtnB; }
  if( ic == 'C'){ pChr = PtnC; }
  if( ic == 'D'){ pChr = PtnD; }
  if( ic == 'E'){ pChr = PtnE; }
  if( ic == 'F'){ pChr = PtnF; }
  if( ic == 'G'){ pChr = PtnG; }
  if( ic == '3'){ pChr = Ptn3; }
  if( ic == '4'){ pChr = Ptn4; }
  if( ic == '5'){ pChr = Ptn5; }
  if( ic == '6'){ pChr = Ptn6; }
  if( ic == '7'){ pChr = Ptn7; }
  if( ic == '8'){ pChr = Ptn8; }
  if( ic == '9'){ pChr = Ptn9; }  
  assert(pChr!=0);
  int icur = 0;
  for(;;){
    int np = pChr[icur];
    if( np == -1 ) break;
    ::glBegin(GL_LINE_STRIP);
    for(int ip=0;ip<np;ip++){
      int ix0 = pChr[icur+1+ip*2+0];
      int iy0 = pChr[icur+1+ip*2+1];
      double sx = ix0*ax+bx;
      double sy = iy0*ay+by;
      ::glVertex2d(sx,sy);
    }
    ::glEnd();
    icur += np*2+1;
  }
}

// ==================================================================
// Points

DFM2_INLINE void delfem2::opengl::DrawPoints2D_Vectors(
    const double* aXY,
    unsigned int nXY,
    const double* aVal,
    int nstride,
    int noffset,
    double mag)
{
  ::glBegin(GL_LINES);
  for(unsigned int ino=0;ino<nXY;ino++){
    const double vx = aVal[ino*nstride+noffset+0]*mag;
    const double vy = aVal[ino*nstride+noffset+1]*mag;
    const double p0[2] = { aXY[ino*2+0],    aXY[ino*2+1]    };
    const double p1[2] = { aXY[ino*2+0]+vx, aXY[ino*2+1]+vy };
    ::glVertex2dv( p0 );
    ::glVertex2dv( p1 );
  }
  ::glEnd();
}

DFM2_INLINE void delfem2::opengl::DrawPoints2d_Points(const std::vector<double>& aXY)
{
  const unsigned int nxys = aXY.size()/2;
  ::glBegin(GL_POINTS);
  for(unsigned int ino=0;ino<nxys;ino++){
    const double p0[2] = { aXY[ino*2+0], aXY[ino*2+1] };
    ::glVertex2dv( p0 );
  }
  ::glEnd();
}

DFM2_INLINE void delfem2::opengl::DrawPoints3d_Points(const std::vector<double>& aXYZ)
{
  const unsigned int nxyz = aXYZ.size()/3;
  ::glBegin(GL_POINTS);
  for(unsigned int ino=0;ino<nxyz;ino++){
    const double p0[3] = { aXYZ[ino*3+0], aXYZ[ino*3+1], aXYZ[ino*3+2]};
    ::glVertex3dv( p0 );
  }
  ::glEnd();
}

DFM2_INLINE void delfem2::opengl::DrawPoints3d_NormVtx(
    const std::vector<double>& aXYZ,
    const std::vector<double>& aNrm,
    double scale)
{
  const unsigned int np = aXYZ.size()/3;
  ::glBegin(GL_LINES);
  for(unsigned int ip=0;ip<np;ip++){
    const double p0[3] = {
      aXYZ[ip*3+0],
      aXYZ[ip*3+1],
      aXYZ[ip*3+2] };
    const double p1[3] = {
      aXYZ[ip*3+0]+scale*aNrm[ip*3+0],
      aXYZ[ip*3+1]+scale*aNrm[ip*3+1],
      aXYZ[ip*3+2]+scale*aNrm[ip*3+2] };
    ::glVertex3dv( p0 );
    ::glVertex3dv( p1 );
  }
  ::glEnd();
}


// ================================================================
// Axis-aligned box

DFM2_INLINE void delfem2::opengl::DrawBox3_Edge
(const double* p0, // pmin
 const double* p1) // pmax
{
  if( p0[0] > p1[0] ){ return; } // this bounding box is empty
  ::glBegin(GL_LINES);
  ::glVertex3d(p1[0],p0[1],p1[2]); ::glVertex3d(p0[0],p0[1],p1[2]);
  ::glVertex3d(p1[0],p0[1],p0[2]); ::glVertex3d(p0[0],p0[1],p0[2]);
  ::glVertex3d(p1[0],p1[1],p1[2]); ::glVertex3d(p0[0],p1[1],p1[2]);
  ::glVertex3d(p1[0],p1[1],p0[2]); ::glVertex3d(p0[0],p1[1],p0[2]);
  //
  ::glVertex3d(p1[0],p0[1],p0[2]); ::glVertex3d(p1[0],p1[1],p0[2]);
  ::glVertex3d(p0[0],p0[1],p0[2]); ::glVertex3d(p0[0],p1[1],p0[2]);
  ::glVertex3d(p1[0],p0[1],p1[2]); ::glVertex3d(p1[0],p1[1],p1[2]);
  ::glVertex3d(p0[0],p0[1],p1[2]); ::glVertex3d(p0[0],p1[1],p1[2]);
  //
  ::glVertex3d(p1[0],p0[1],p0[2]); ::glVertex3d(p1[0],p0[1],p1[2]);
  ::glVertex3d(p0[0],p0[1],p0[2]); ::glVertex3d(p0[0],p0[1],p1[2]);
  ::glVertex3d(p1[0],p1[1],p0[2]); ::glVertex3d(p1[0],p1[1],p1[2]);
  ::glVertex3d(p0[0],p1[1],p0[2]); ::glVertex3d(p0[0],p1[1],p1[2]);
  ::glEnd();
}

DFM2_INLINE void delfem2::opengl::DrawBox3_Face
 (const double* p0,
  const double* p1)
{
  const double p[8][3] = {
    { p0[0],p0[1],p0[2] },
    { p1[0],p0[1],p0[2] },
    { p0[0],p1[1],p0[2] },
    { p1[0],p1[1],p0[2] },
    { p0[0],p0[1],p1[2] },
    { p1[0],p0[1],p1[2] },
    { p0[0],p1[1],p1[2] },
    { p1[0],p1[1],p1[2] }
  };
  const int elfc[6][4] = { // this numbering is corresponds to VTK_VOX
    { 0, 4, 6, 2 }, // -x
    { 1, 3, 7, 5 }, // +x
    { 0, 1, 5, 4 }, // -y
    { 2, 6, 7, 3 }, // +y
    { 0, 2, 3, 1 }, // -z
    { 4, 5, 7, 6 }  // +z
  };
  const double an[6][3] = {
    {-1, 0, 0},
    {+1, 0, 0},
    { 0,-1, 0},
    { 0,+1, 0},
    { 0, 0,-1},
    { 0, 0,+1}
  };
  ::glBegin(GL_QUADS);
  for(int ifc=0;ifc<6;++ifc){
    ::glNormal3dv(an[ifc]);
    ::glVertex3dv(p[elfc[ifc][0]]);
    ::glVertex3dv(p[elfc[ifc][1]]);
    ::glVertex3dv(p[elfc[ifc][2]]);
    ::glVertex3dv(p[elfc[ifc][3]]);
  }
  ::glEnd();
}


DFM2_INLINE void delfem2::opengl::DrawBox_MinMaxXYZ
 (double x_min, double x_max,
  double y_min, double y_max,
  double z_min, double z_max)
{
  const double min3[3] = {x_min,y_min,z_min};
  const double max3[3] = {x_max,y_max,z_max};
  DrawBox3_Edge(min3, max3);
}

DFM2_INLINE void delfem2::opengl::DrawBox_MinMaxXYZ
 (double aabbMinMaxXYZ[6])
{// show bounding box
  DrawBox_MinMaxXYZ(aabbMinMaxXYZ[0], aabbMinMaxXYZ[1],
                    aabbMinMaxXYZ[2], aabbMinMaxXYZ[3],
                    aabbMinMaxXYZ[4], aabbMinMaxXYZ[5]);
}

DFM2_INLINE void delfem2::opengl::DrawAABB3D_Edge
 (double cx, double cy, double cz, double wx, double wy, double wz)
{
  const double pxyz[3] = {cx-0.5*wx,cy-0.5*wy,cz-0.5*wz};
  const double pXYZ[3] = {cx+0.5*wx,cy+0.5*wy,cz+0.5*wz};
  DrawBox3_Edge(pxyz, pXYZ);
}

DFM2_INLINE void delfem2::opengl::DrawAABB3D_Edge(const double cw[6])
{
  DrawAABB3D_Edge(cw[0], cw[1], cw[2], cw[3],cw[4], cw[5]);
}


// =====================================================
// MeshTri3D

DFM2_INLINE void delfem2::opengl::DrawMeshTri3D_FaceNorm
(const double* paXYZ,
 const unsigned int* paTri,
 unsigned int nTri)
{
  ::glBegin(GL_TRIANGLES);
  for(unsigned int itri=0;itri<nTri;++itri){
    funcs_glold::DrawSingleTri3D_FaceNorm(paXYZ, paTri+itri*3,0);
  }
  ::glEnd();
}

DFM2_INLINE void delfem2::opengl::DrawMeshTri3D_FaceNorm
(const std::vector<double>& aXYZ,
 const std::vector<unsigned int>& aTri)
{
  DrawMeshTri3D_FaceNorm(aXYZ.data(), aTri.data(), aTri.size()/3);
}


DFM2_INLINE void delfem2::opengl::DrawMeshTri3D_FaceNorm_TexVtx
(const std::vector<double>& aXYZ,
 const std::vector<unsigned int>& aTri,
 const std::vector<double>& aTex)
{
  const unsigned int nTri = aTri.size()/3;
  //
  double uv[6];
  ::glBegin(GL_TRIANGLES);
  for(unsigned int itri=0;itri<nTri;++itri){
    const unsigned int ip0 = aTri[itri*3+0];
    const unsigned int ip1 = aTri[itri*3+1];
    const unsigned int ip2 = aTri[itri*3+2];
    uv[0] = aTex[ip0*2+0];
    uv[1] = aTex[ip0*2+1];
    uv[2] = aTex[ip1*2+0];
    uv[3] = aTex[ip1*2+1];
    uv[4] = aTex[ip2*2+0];
    uv[5] = aTex[ip2*2+1];
    funcs_glold::DrawSingleTri3D_FaceNorm(aXYZ.data(), aTri.data()+itri*3,uv);
  }
  ::glEnd();
}

DFM2_INLINE void delfem2::opengl::DrawMeshTri3D_FaceNorm_TexFace
(const std::vector<double>& aXYZ,
 const std::vector<unsigned int>& aTri,
 const std::vector<double>& aTex)
{
  const unsigned int nTri = aTri.size()/3;
  //
  ::glBegin(GL_TRIANGLES);
  for(unsigned int itri=0;itri<nTri;++itri){
    funcs_glold::DrawSingleTri3D_FaceNorm(aXYZ.data(),
                             aTri.data()+itri*3,
                             aTex.data()+itri*6);
  }
  ::glEnd();
}

DFM2_INLINE void delfem2::opengl::DrawMeshElem3D_FaceNorm
(const std::vector<double>& aXYZ,
 const std::vector<unsigned int>& aElemInd,
 const std::vector<unsigned int>& aElem)
{
  assert( !aElemInd.empty() );
  const unsigned int nelem = aElemInd.size()-1;
  for(unsigned int ielem=0;ielem<nelem;++ielem){
    const int ielemind0 = aElemInd[ielem];
    const int ielemind1 = aElemInd[ielem+1];
    if( ielemind1 - ielemind0 == 3 ){
      ::glBegin(GL_TRIANGLES);
      funcs_glold::DrawSingleTri3D_FaceNorm(aXYZ.data(), aElem.data()+ielemind0,0);
      ::glEnd();
    }
    else if(ielemind1-ielemind0 == 4){
      ::glBegin(GL_QUADS);
      funcs_glold::DrawSingleQuad3D_FaceNorm(aXYZ.data(),aElem.data()+ielemind0,0);
      ::glEnd();
    }
  }
}

DFM2_INLINE void delfem2::opengl::DrawMeshElem3D_FaceNorm
(const std::vector<double>& aXYZ,
 const std::vector<unsigned int>& aElemInd,
 const std::vector<unsigned int>& aElem,
 const std::vector<double>& aUV)
{
  assert( !aElemInd.empty() );
  const std::size_t nelem = aElemInd.size()-1;
  for(unsigned int ielem=0;ielem<nelem;++ielem){
    const int ielemind0 = aElemInd[ielem];
    const int ielemind1 = aElemInd[ielem+1];
    if( ielemind1 - ielemind0 == 3 ){
      ::glBegin(GL_TRIANGLES);
      funcs_glold::DrawSingleTri3D_FaceNorm(aXYZ.data(),
                               aElem.data()+ielemind0,
                               aUV.data()+ielemind0*2);
      ::glEnd();
    }
    else if(ielemind1-ielemind0 == 4){
      ::glBegin(GL_QUADS);
      funcs_glold::DrawSingleQuad3D_FaceNorm(aXYZ.data(),
                                aElem.data()+ielemind0,
                                aUV.data()+ielemind0*2);
      ::glEnd();
    }
  }
}

DFM2_INLINE void delfem2::opengl::DrawMeshElemPart3D_FaceNorm_TexPoEl
(const std::vector<double>& aXYZ,
 const std::vector<unsigned int>& aElemInd,
 const std::vector<unsigned int>& aElem,
 const std::vector<int>& aIndElemPart,
 const std::vector<double>& aUV)
{
  const bool isUV = (aUV.size()==aElem.size()*2);
  for(int ielem : aIndElemPart){
    const int ielemind0 = aElemInd[ielem];
    const int ielemind1 = aElemInd[ielem+1];
    const double* pUV = isUV ? aUV.data()+ielemind0*2:0;
    if( ielemind1 - ielemind0 == 3 ){
      ::glBegin(GL_TRIANGLES);
      funcs_glold::DrawSingleTri3D_FaceNorm(aXYZ.data(),
                               aElem.data()+ielemind0,
                               pUV);
      ::glEnd();
    }
    else if(ielemind1-ielemind0 == 4){
      ::glBegin(GL_QUADS);
      funcs_glold::DrawSingleQuad3D_FaceNorm(aXYZ.data(),
                                aElem.data()+ielemind0,
                                pUV);
      ::glEnd();
    }
  }
}

DFM2_INLINE void delfem2::opengl::DrawMeshTri3D_FaceNorm_XYsym
(const std::vector<double>& aXYZ,
 const std::vector<unsigned int>& aTri)
{
  const unsigned int nTri = aTri.size()/3;
  //
  ::glBegin(GL_TRIANGLES);
  for(unsigned int itri=0;itri<nTri;++itri){
    const unsigned int i1 = aTri[itri*3+0];
    const unsigned int i2 = aTri[itri*3+1];
    const unsigned int i3 = aTri[itri*3+2];
    assert( i1 < aXYZ.size()/3 );
    assert( i2 < aXYZ.size()/3 );
    assert( i3 < aXYZ.size()/3 );
    double p1[3] = {aXYZ[i1*3+0], aXYZ[i1*3+1], -aXYZ[i1*3+2]};
    double p2[3] = {aXYZ[i2*3+0], aXYZ[i2*3+1], -aXYZ[i2*3+2]};
    double p3[3] = {aXYZ[i3*3+0], aXYZ[i3*3+1], -aXYZ[i3*3+2]};
    double un[3], area;
    funcs_glold::UnitNormalAreaTri3D(un,area, p1,p3,p2);
    ::glNormal3dv(un);
    ::glVertex3dv(p1);
    ::glVertex3dv(p3);
    ::glVertex3dv(p2);
  }
  ::glEnd();
}

DFM2_INLINE void delfem2::opengl::DrawMeshTri3D_FaceNormEdge
(const std::vector<double>& aXYZ,
 const std::vector<unsigned int>& aTri)
{
  GLboolean is_lighting = glIsEnabled(GL_LIGHTING);
  const unsigned int nTri = aTri.size()/3;
  ::glBegin(GL_TRIANGLES);
  for (unsigned int itri=0; itri<nTri; ++itri){
    funcs_glold::DrawSingleTri3D_FaceNorm(aXYZ.data(), aTri.data()+itri*3,0);
  }
  ::glEnd();

  ::glDisable(GL_LIGHTING);
  ::glBegin(GL_LINES);
  ::glColor3d(0, 0, 0);
  for (unsigned int itri = 0; itri<nTri; ++itri){
    const int i1 = aTri[itri*3+0];
    const int i2 = aTri[itri*3+1];
    const int i3 = aTri[itri*3+2];
    if( i1 == -1 ){
      assert(i2==-1); assert(i3==-1);
      continue;
    }
    assert(i1>=0&&i1 < (int)aXYZ.size()/3 );
    assert(i2>=0&&i2 < (int)aXYZ.size()/3 );
    assert(i3>=0&&i3 < (int)aXYZ.size()/3 );
    const double p1[3] = { aXYZ[i1*3+0], aXYZ[i1*3+1], aXYZ[i1*3+2] };
    const double p2[3] = { aXYZ[i2*3+0], aXYZ[i2*3+1], aXYZ[i2*3+2] };
    const double p3[3] = { aXYZ[i3*3+0], aXYZ[i3*3+1], aXYZ[i3*3+2] };
    ::glVertex3dv(p1); ::glVertex3dv(p2);
    ::glVertex3dv(p2); ::glVertex3dv(p3);
    ::glVertex3dv(p3); ::glVertex3dv(p1);
  }
  ::glEnd();

  if (is_lighting){ ::glEnable(GL_LIGHTING); }
}

DFM2_INLINE void delfem2::opengl::DrawMeshTri3D_Edge
(const double* aXYZ, unsigned int nXYZ,
 const unsigned int* aTri, unsigned int nTri)
{
  GLboolean is_lighting = glIsEnabled(GL_LIGHTING);
  //
  ::glDisable(GL_LIGHTING);
  ::glBegin(GL_LINES);
  for (unsigned int itri = 0; itri<nTri; ++itri){
    const unsigned int i1 = aTri[itri*3+0];
    const unsigned int i2 = aTri[itri*3+1];
    const unsigned int i3 = aTri[itri*3+2];
    assert(i1 < nXYZ);
    assert(i2 < nXYZ);
    assert(i3 < nXYZ);
    const double p1[3] = { aXYZ[i1*3+0], aXYZ[i1*3+1], aXYZ[i1*3+2] };
    const double p2[3] = { aXYZ[i2*3+0], aXYZ[i2*3+1], aXYZ[i2*3+2] };
    const double p3[3] = { aXYZ[i3*3+0], aXYZ[i3*3+1], aXYZ[i3*3+2] };
    ::glVertex3dv(p1); ::glVertex3dv(p2);
    ::glVertex3dv(p2); ::glVertex3dv(p3);
    ::glVertex3dv(p3); ::glVertex3dv(p1);
  }
  ::glEnd();
  
  if (is_lighting){ ::glEnable(GL_LIGHTING); }
}

DFM2_INLINE void delfem2::opengl::DrawMeshTri3D_Edge
(const std::vector<double>& aXYZ,
 const std::vector<unsigned int>& aTri)
{
  DrawMeshTri3D_Edge(aXYZ.data(), aXYZ.size()/3,
                     aTri.data(), aTri.size()/3);
}

DFM2_INLINE void delfem2::opengl::DrawMeshTri3D_FaceNorm
(const std::vector<double>& aXYZ,
 const std::vector<unsigned int>& aTri,
 const std::vector<double>& aNorm)
{
  const unsigned int nTri = aTri.size()/3;
  //
  ::glBegin(GL_TRIANGLES);
  for(unsigned int itri=0;itri<nTri;itri++){
    const unsigned int i1 = aTri[itri*3+0];
    const unsigned int i2 = aTri[itri*3+1];
    const unsigned int i3 = aTri[itri*3+2];
    funcs_glold::myGlNorm3d(i1,aNorm);  funcs_glold::myGlVertex3d(i1,aXYZ);
    funcs_glold::myGlNorm3d(i2,aNorm);  funcs_glold::myGlVertex3d(i2,aXYZ);
    funcs_glold::myGlNorm3d(i3,aNorm);  funcs_glold::myGlVertex3d(i3,aXYZ);
  }
  ::glEnd();
}

DFM2_INLINE void delfem2::opengl::DrawMeshTri3D_FaceNorm
(const std::vector<double>& aXYZ,
 const std::vector<unsigned int>& aTriVtx,
 const std::vector<double>& aNorm,
 const std::vector<unsigned int>& aTriNrm)
{
  const unsigned int nTri = aTriVtx.size()/3;
  assert( aTriNrm.size() == nTri*3 );
  ::glBegin(GL_TRIANGLES);
  for(unsigned int itri=0;itri<nTri;itri++){
    const unsigned int iv1 = aTriVtx[itri*3+0];
    const unsigned int iv2 = aTriVtx[itri*3+1];
    const unsigned int iv3 = aTriVtx[itri*3+2];
    const unsigned int in1 = aTriNrm[itri*3+0];
    const unsigned int in2 = aTriNrm[itri*3+1];
    const unsigned int in3 = aTriNrm[itri*3+2];
    const bool bn1 = in1*3<aNorm.size();
    const bool bn2 = in2*3<aNorm.size();
    const bool bn3 = in3*3<aNorm.size();
    const bool bv1 = iv1*3<aXYZ.size();
    const bool bv2 = iv2*3<aXYZ.size();
    const bool bv3 = iv3*3<aXYZ.size();
    const bool bn123 = bn1 && bn2 && bn3;
    const bool bv123 = bv1 && bv2 && bv3;
    if( bn123 && bv123 ){
      funcs_glold::myGlNorm3d(in1, aNorm); funcs_glold::myGlVertex3d(iv1, aXYZ);
      funcs_glold::myGlNorm3d(in2, aNorm); funcs_glold::myGlVertex3d(iv2, aXYZ);
      funcs_glold::myGlNorm3d(in3, aNorm); funcs_glold::myGlVertex3d(iv3, aXYZ);
    }
    else if( bv123 ){
      funcs_glold::myGlVertex3d(iv1, aXYZ);
      funcs_glold::myGlVertex3d(iv2, aXYZ);
      funcs_glold::myGlVertex3d(iv3, aXYZ);
    }
  }
  ::glEnd();
}

// ------------------------------------------------

DFM2_INLINE void delfem2::opengl::DrawMeshTri2D_Face
(const std::vector<unsigned int>& aTri,
 const std::vector<double>& aXY)
{
  const std::size_t ntri = aTri.size()/3;
  //  const int nxys = (int)aXY.size()/2;
  ::glBegin(GL_TRIANGLES);
  for(unsigned int itri=0;itri<ntri;itri++){
    const int i0 = aTri[itri*3+0];
    const int i1 = aTri[itri*3+1];
    const int i2 = aTri[itri*3+2];
    const double p0[2] = { aXY[i0*2+0], aXY[i0*2+1] };
    const double p1[2] = { aXY[i1*2+0], aXY[i1*2+1] };
    const double p2[2] = { aXY[i2*2+0], aXY[i2*2+1] };
    ::glVertex2dv( p0 );
    ::glVertex2dv( p1 );
    ::glVertex2dv( p2 );
  }
  ::glEnd();
}

DFM2_INLINE void delfem2::opengl::DrawMeshTri2D_FaceDisp2D(
    const double* aXY,
    unsigned int nXY,
    const unsigned int* aTri,
    unsigned int nTri,
    const double* aDisp,
    int nstride)
{
  ::glColor3d(1,1,1);
  ::glBegin(GL_TRIANGLES);
  for(unsigned int itri=0;itri<nTri;itri++){
    const unsigned int i0 = aTri[itri*3+0];
    const unsigned int i1 = aTri[itri*3+1];
    const unsigned int i2 = aTri[itri*3+2];
    const double p0[2] = { aXY[i0*2+0]+aDisp[i0*nstride+0], aXY[i0*2+1]+aDisp[i0*nstride+1] };
    const double p1[2] = { aXY[i1*2+0]+aDisp[i1*nstride+0], aXY[i1*2+1]+aDisp[i1*nstride+1] };
    const double p2[2] = { aXY[i2*2+0]+aDisp[i2*nstride+0], aXY[i2*2+1]+aDisp[i2*nstride+1] };
    ::glVertex2dv( p0 );
    ::glVertex2dv( p1 );
    ::glVertex2dv( p2 );
  }
  ::glEnd();
  // --------------------------------------
  ::glDisable(GL_LIGHTING);
  ::glColor3d(0,0,0);
  ::glBegin(GL_LINES);
  for(unsigned int itri=0;itri<nTri;itri++){
    const unsigned int i0 = aTri[itri*3+0];
    const unsigned int i1 = aTri[itri*3+1];
    const unsigned int i2 = aTri[itri*3+2];
    const double p0[2] = { aXY[i0*2+0]+aDisp[i0*nstride+0], aXY[i0*2+1]+aDisp[i0*nstride+1] };
    const double p1[2] = { aXY[i1*2+0]+aDisp[i1*nstride+0], aXY[i1*2+1]+aDisp[i1*nstride+1] };
    const double p2[2] = { aXY[i2*2+0]+aDisp[i2*nstride+0], aXY[i2*2+1]+aDisp[i2*nstride+1] };
    ::glVertex2dv( p0 ); ::glVertex2dv( p1 );
    ::glVertex2dv( p1 ); ::glVertex2dv( p2 );
    ::glVertex2dv( p2 ); ::glVertex2dv( p0 );
  }
  ::glEnd();
}


DFM2_INLINE void delfem2::opengl::DrawMeshTri2D_Edge
(const double* aXY, unsigned int nXY,
 const unsigned int* aTri, unsigned int nTri)
{
  //  const unsigned int nxys = (int)aXY.size()/2;
  ::glColor3d(0,0,0);
  ::glBegin(GL_LINES);
  for(unsigned int itri=0;itri<nTri;itri++){
    const unsigned int ino0 = aTri[itri*3+0];
    const unsigned int ino1 = aTri[itri*3+1];
    const unsigned int ino2 = aTri[itri*3+2];
    ::glVertex2d( aXY[ino0*2+0], aXY[ino0*2+1] );
    ::glVertex2d( aXY[ino1*2+0], aXY[ino1*2+1] );
    ::glVertex2d( aXY[ino1*2+0], aXY[ino1*2+1] );
    ::glVertex2d( aXY[ino2*2+0], aXY[ino2*2+1] );
    ::glVertex2d( aXY[ino2*2+0], aXY[ino2*2+1] );
    ::glVertex2d( aXY[ino0*2+0], aXY[ino0*2+1] );
  }
  ::glEnd();
}

DFM2_INLINE void delfem2::opengl::DrawMeshTri2D_Edge
(const std::vector<unsigned int>& aTri,
 const std::vector<double>& aXY)
{
  DrawMeshTri2D_Edge(aXY.data(), aXY.size()/2,
                     aTri.data(), aTri.size()/3);
}


DFM2_INLINE void delfem2::opengl::DrawMeshTri2D_FaceColor(
    const unsigned int* aTri,
    unsigned int nTri,
    const double* aXY,
    unsigned int nXY,
    const double* aColor)
{
  ::glBegin(GL_TRIANGLES);
  for(unsigned int itri=0;itri<nTri;itri++){
    const unsigned int i0 = aTri[itri*3+0];
    const unsigned int i1 = aTri[itri*3+1];
    const unsigned int i2 = aTri[itri*3+2];
    const double p0[2] = { aXY[i0*2+0], aXY[i0*2+1] };
    const double p1[2] = { aXY[i1*2+0], aXY[i1*2+1] };
    const double p2[2] = { aXY[i2*2+0], aXY[i2*2+1] };
    ::glColor3dv(aColor+i0*3);
    ::glVertex2dv( p0 );
    ::glColor3dv(aColor+i1*3);
    ::glVertex2dv( p1 );
    ::glColor3dv(aColor+i2*3);
    ::glVertex2dv( p2 );
  }
  ::glEnd();
}


// ===============================================================
// MeshQuad

DFM2_INLINE void delfem2::opengl::DrawMeshQuad3D_Edge
(const double* aXYZ, unsigned int nXYZ,
 const unsigned int* aQuad, unsigned int nQuad)
{
  GLboolean is_lighting = glIsEnabled(GL_LIGHTING);
  ////
  ::glDisable(GL_LIGHTING);
  ::glBegin(GL_LINES);
  ::glColor3d(0, 0, 0);
  for (unsigned int iq = 0; iq<nQuad; ++iq){
    const unsigned int i1 = aQuad[iq*4+0];
    const unsigned int i2 = aQuad[iq*4+1];
    const unsigned int i3 = aQuad[iq*4+2];
    const unsigned int i4 = aQuad[iq*4+3];
    assert(i1<nXYZ);
    assert(i2<nXYZ);
    assert(i3<nXYZ);
    assert(i4<nXYZ);
    double p1[3] = { aXYZ[i1*3+0], aXYZ[i1*3+1], aXYZ[i1*3+2] };
    double p2[3] = { aXYZ[i2*3+0], aXYZ[i2*3+1], aXYZ[i2*3+2] };
    double p3[3] = { aXYZ[i3*3+0], aXYZ[i3*3+1], aXYZ[i3*3+2] };
    double p4[3] = { aXYZ[i4*3+0], aXYZ[i4*3+1], aXYZ[i4*3+2] };
    ::glVertex3dv(p1); ::glVertex3dv(p2);
    ::glVertex3dv(p2); ::glVertex3dv(p3);
    ::glVertex3dv(p3); ::glVertex3dv(p4);
    ::glVertex3dv(p4); ::glVertex3dv(p1);
  }
  ::glEnd();
  if (is_lighting){ ::glEnable(GL_LIGHTING); }
}

DFM2_INLINE void delfem2::opengl::DrawMeshQuad3D_Edge
(const std::vector<double>& aXYZ,
 const std::vector<unsigned int>& aQuad)
{
  DrawMeshQuad3D_Edge(aXYZ.data(), aXYZ.size()/3,
                      aQuad.data(), aQuad.size()/4);
}


DFM2_INLINE void delfem2::opengl::DrawMeshQuad3D_FaceNorm
(const double* aXYZ,
 const unsigned int* aQuad, const unsigned int nQuad)
{
  ::glBegin(GL_QUADS);
  for(unsigned int iq=0;iq<nQuad;++iq){
    funcs_glold::DrawSingleQuad3D_FaceNorm(aXYZ, aQuad+iq*4, 0);
  }
  ::glEnd();
}

DFM2_INLINE void delfem2::opengl::DrawMeshQuad3D_FaceNorm
(const std::vector<double>& aXYZ,
 const std::vector<unsigned int>& aQuad)
{
  DrawMeshQuad3D_FaceNorm(aXYZ.data(),aQuad.data(),aQuad.size()/4);
}


DFM2_INLINE void delfem2::opengl::DrawMeshQuad2D_Edge
(const double* aXY, unsigned int nXY,
 const unsigned int* aQuad, unsigned int nQuad)
{
  GLboolean is_lighting = glIsEnabled(GL_LIGHTING);
  // ---------------------
  ::glDisable(GL_LIGHTING);
  ::glBegin(GL_LINES);
  ::glColor3d(0, 0, 0);
  for (unsigned int iq = 0; iq<nQuad; ++iq){
    const unsigned int i1 = aQuad[iq*4+0];
    const unsigned int i2 = aQuad[iq*4+1];
    const unsigned int i3 = aQuad[iq*4+2];
    const unsigned int i4 = aQuad[iq*4+3];
    assert(i1<nXY);
    assert(i2<nXY);
    assert(i3<nXY);
    assert(i4<nXY);
    double p1[2] = { aXY[i1*2+0], aXY[i1*2+1] };
    double p2[2] = { aXY[i2*2+0], aXY[i2*2+1] };
    double p3[2] = { aXY[i3*2+0], aXY[i3*2+1] };
    double p4[2] = { aXY[i4*2+0], aXY[i4*2+1] };
    ::glVertex2dv(p1); ::glVertex2dv(p2);
    ::glVertex2dv(p2); ::glVertex2dv(p3);
    ::glVertex2dv(p3); ::glVertex2dv(p4);
    ::glVertex2dv(p4); ::glVertex2dv(p1);
  }
  ::glEnd();
  if (is_lighting){ ::glEnable(GL_LIGHTING); }
}

DFM2_INLINE void delfem2::opengl::DrawMeshQuad2D_Edge
(const std::vector<double>& aXY,
 const std::vector<unsigned int>& aQuad)
{
  DrawMeshQuad2D_Edge(aXY.data(), aXY.size()/2,
                      aQuad.data(), aQuad.size()/4);
}

// ----------------------------------------------------------------------------

DFM2_INLINE void delfem2::opengl::DrawMeshTet3DSurface_FaceNorm
(const std::vector<double>& aXYZ,
 const std::vector<unsigned int>& aTet,
 const std::vector<unsigned int>& aTetFace)
{
  const unsigned int noelTetFace[4][3] = {
    { 1, 2, 3 },
    { 0, 3, 2 },
    { 0, 1, 3 },
    { 0, 2, 1 } };
  //  const int nTri = (int)aTri.size()/3;
  // const int nXYZ = (int)aXYZ.size()/3;
  /////
  ::glBegin(GL_TRIANGLES);
  for(unsigned int itf=0;itf<aTetFace.size()/2;++itf){
    unsigned int itet = aTetFace[itf*2+0];
    unsigned int iface = aTetFace[itf*2+1];
    const int i1 = aTet[itet*4+noelTetFace[iface][0]];
    const int i2 = aTet[itet*4+noelTetFace[iface][1]];
    const int i3 = aTet[itet*4+noelTetFace[iface][2]];
    if( i1 == -1 ){
      assert(i2==-1); assert(i3==-1);
      continue;
    }
    assert( i1 >= 0 && i1 < (int)aXYZ.size()/3 );
    assert( i2 >= 0 && i2 < (int)aXYZ.size()/3 );
    assert( i3 >= 0 && i3 < (int)aXYZ.size()/3 );
    double p1[3] = {aXYZ[i1*3+0], aXYZ[i1*3+1], aXYZ[i1*3+2]};
    double p2[3] = {aXYZ[i2*3+0], aXYZ[i2*3+1], aXYZ[i2*3+2]};
    double p3[3] = {aXYZ[i3*3+0], aXYZ[i3*3+1], aXYZ[i3*3+2]};
    double un[3], area;
    funcs_glold::UnitNormalAreaTri3D(un,area, p1,p2,p3);
    ::glNormal3dv(un);
    funcs_glold::myGlVertex3d(i1,aXYZ);
    funcs_glold::myGlVertex3d(i2,aXYZ);
    funcs_glold::myGlVertex3d(i3,aXYZ);
  }
  ::glEnd();
}

DFM2_INLINE void delfem2::opengl::DrawMeshTet3DSurface_Edge
(const std::vector<double>& aXYZ,
 const std::vector<unsigned int>& aTet,
 const std::vector<unsigned int>& aTetFace)
{
  const unsigned int noelTetFace[4][3] = {
    { 1, 2, 3 },
    { 0, 3, 2 },
    { 0, 1, 3 },
    { 0, 2, 1 } };
  
  GLboolean is_lighting = glIsEnabled(GL_LIGHTING);
  ::glDisable(GL_LIGHTING);
  ::glBegin(GL_LINES);
  ::glColor3d(0, 0, 0);
  for (unsigned int itf=0; itf<aTetFace.size()/2;++itf){
    int itet = aTetFace[itf*2+0];
    int iface = aTetFace[itf*2+1];
    const int i1 = aTet[itet*4+noelTetFace[iface][0]];
    const int i2 = aTet[itet*4+noelTetFace[iface][1]];
    const int i3 = aTet[itet*4+noelTetFace[iface][2]];
    if( i1 == -1 ){
      assert(i2==-1); assert(i3==-1);
      continue;
    }
    assert(i1>=0&&i1 < (int)aXYZ.size()/3);
    assert(i2>=0&&i2 < (int)aXYZ.size()/3);
    assert(i3>=0&&i3 < (int)aXYZ.size()/3);
    double p1[3] = { aXYZ[i1*3+0], aXYZ[i1*3+1], aXYZ[i1*3+2] };
    double p2[3] = { aXYZ[i2*3+0], aXYZ[i2*3+1], aXYZ[i2*3+2] };
    double p3[3] = { aXYZ[i3*3+0], aXYZ[i3*3+1], aXYZ[i3*3+2] };
    ::glVertex3dv(p1); ::glVertex3dv(p2);
    ::glVertex3dv(p2); ::glVertex3dv(p3);
    ::glVertex3dv(p3); ::glVertex3dv(p1);
  }
  ::glEnd();
  
  if (is_lighting){ ::glEnable(GL_LIGHTING); }
}

DFM2_INLINE void delfem2::opengl::DrawMeshTet3D_Edge(
    const double* aXYZ,
    unsigned int nXYZ,
    const unsigned int* aTet,
    unsigned int nTet)
{
  for (unsigned int itet = 0; itet<nTet; itet++){
    const unsigned int i0 = aTet[itet*4+0];
    const unsigned int i1 = aTet[itet*4+1];
    const unsigned int i2 = aTet[itet*4+2];
    const unsigned int i3 = aTet[itet*4+3];
    const double p0[3] = { aXYZ[i0*3+0], aXYZ[i0*3+1], aXYZ[i0*3+2] };
    const double p1[3] = { aXYZ[i1*3+0], aXYZ[i1*3+1], aXYZ[i1*3+2] };
    const double p2[3] = { aXYZ[i2*3+0], aXYZ[i2*3+1], aXYZ[i2*3+2] };
    const double p3[3] = { aXYZ[i3*3+0], aXYZ[i3*3+1], aXYZ[i3*3+2] };
    //::glColor3d(0, 0, 0);
    ::glBegin(GL_LINES);
    ::glVertex3dv(p0); ::glVertex3dv(p1);
    ::glVertex3dv(p0); ::glVertex3dv(p2);
    ::glVertex3dv(p0); ::glVertex3dv(p3);
    ::glVertex3dv(p1); ::glVertex3dv(p2);
    ::glVertex3dv(p1); ::glVertex3dv(p3);
    ::glVertex3dv(p2); ::glVertex3dv(p3);
    ::glEnd();
  }
}

DFM2_INLINE void delfem2::opengl::DrawMeshLine3D_Edge
(const double* aXYZ,
 unsigned int nXYZ,
 const unsigned int* aLine,
 unsigned int nLine)
{
  for (unsigned int il = 0; il<nLine; il++){
    const unsigned int i0 = aLine[il*2+0];
    const unsigned int i1 = aLine[il*2+1];
    const double p0[3] = { aXYZ[i0*3+0], aXYZ[i0*3+1], aXYZ[i0*3+2] };
    const double p1[3] = { aXYZ[i1*3+0], aXYZ[i1*3+1], aXYZ[i1*3+2] };
    //::glColor3d(0, 0, 0);
    ::glBegin(GL_LINES);
    ::glVertex3dv(p0); ::glVertex3dv(p1);
    ::glEnd();
  }
}

DFM2_INLINE void delfem2::opengl::DrawMeshTet3D_EdgeDisp(
    const double* aXYZ,
    const unsigned int* aTet,
    unsigned int nTet,
    const double* aDisp,
    double s0)
{
  for (unsigned int itet = 0; itet<nTet; itet++){
    const unsigned int i0 = aTet[itet*4+0];
    const unsigned int i1 = aTet[itet*4+1];
    const unsigned int i2 = aTet[itet*4+2];
    const unsigned int i3 = aTet[itet*4+3];
    const double p0[3] = { aXYZ[i0*3+0], aXYZ[i0*3+1], aXYZ[i0*3+2] };
    const double p1[3] = { aXYZ[i1*3+0], aXYZ[i1*3+1], aXYZ[i1*3+2] };
    const double p2[3] = { aXYZ[i2*3+0], aXYZ[i2*3+1], aXYZ[i2*3+2] };
    const double p3[3] = { aXYZ[i3*3+0], aXYZ[i3*3+1], aXYZ[i3*3+2] };
    double q0[3] = { p0[0]+s0*aDisp[i0*3+0], p0[1]+s0*aDisp[i0*3+1], p0[2]+s0*aDisp[i0*3+2] };
    double q1[3] = { p1[0]+s0*aDisp[i1*3+0], p1[1]+s0*aDisp[i1*3+1], p1[2]+s0*aDisp[i1*3+2] };
    double q2[3] = { p2[0]+s0*aDisp[i2*3+0], p2[1]+s0*aDisp[i2*3+1], p2[2]+s0*aDisp[i2*3+2] };
    double q3[3] = { p3[0]+s0*aDisp[i3*3+0], p3[1]+s0*aDisp[i3*3+1], p3[2]+s0*aDisp[i3*3+2] };
    ::glBegin(GL_LINES);
    ::glVertex3dv(q0); ::glVertex3dv(q1);
    ::glVertex3dv(q0); ::glVertex3dv(q2);
    ::glVertex3dv(q0); ::glVertex3dv(q3);
    ::glVertex3dv(q1); ::glVertex3dv(q2);
    ::glVertex3dv(q1); ::glVertex3dv(q3);
    ::glVertex3dv(q2); ::glVertex3dv(q3);
    ::glEnd();
  }
}

DFM2_INLINE void delfem2::opengl::DrawMeshTet3D_FaceNorm
(const double* aXYZ,
 const unsigned int* aTet,
 unsigned int nTet)
{
  for (unsigned  itet = 0; itet<nTet; itet++){
    const unsigned int i0 = aTet[itet*4+0];
    const unsigned int i1 = aTet[itet*4+1];
    const unsigned int i2 = aTet[itet*4+2];
    const unsigned int i3 = aTet[itet*4+3];
    const double p0[3] = { aXYZ[i0*3+0], aXYZ[i0*3+1], aXYZ[i0*3+2] };
    const double p1[3] = { aXYZ[i1*3+0], aXYZ[i1*3+1], aXYZ[i1*3+2] };
    const double p2[3] = { aXYZ[i2*3+0], aXYZ[i2*3+1], aXYZ[i2*3+2] };
    const double p3[3] = { aXYZ[i3*3+0], aXYZ[i3*3+1], aXYZ[i3*3+2] };
    double un0[3], a0; funcs_glold::UnitNormalAreaTri3D(un0,a0, p1,p2,p3);
    double un1[3], a1; funcs_glold::UnitNormalAreaTri3D(un1,a1, p2,p0,p3);
    double un2[3], a2; funcs_glold::UnitNormalAreaTri3D(un2,a2, p3,p0,p1);
    double un3[3], a3; funcs_glold::UnitNormalAreaTri3D(un3,a3, p0,p2,p1);
    //    ::glColor3d(0, 0, 0);
    ::glBegin(GL_TRIANGLES);
    ::glNormal3dv(un0); ::glVertex3dv(p1); ::glVertex3dv(p2); ::glVertex3dv(p3);
    ::glNormal3dv(un1); ::glVertex3dv(p2); ::glVertex3dv(p3); ::glVertex3dv(p0);
    ::glNormal3dv(un2); ::glVertex3dv(p3); ::glVertex3dv(p0); ::glVertex3dv(p1);
    ::glNormal3dv(un3); ::glVertex3dv(p0); ::glVertex3dv(p1); ::glVertex3dv(p2);
    ::glEnd();
  }
}

/*
void opengl::DrawMeshTet3D_FaceNormDisp
 (const double* aXYZ, int nXYZ,
  const unsigned int* aTet, int nTet,
  const double* aDisp)
{
  for (int itet = 0; itet<nTet; itet++){
    const int i0 = aTet[itet*4+0];
    const int i1 = aTet[itet*4+1];
    const int i2 = aTet[itet*4+2];
    const int i3 = aTet[itet*4+3];
    const double p0[3] = { aXYZ[i0*3+0]+aDisp[i0*3+0], aXYZ[i0*3+1]+aDisp[i0*3+1], aXYZ[i0*3+2]+aDisp[i0*3+2] };
    const double p1[3] = { aXYZ[i1*3+0]+aDisp[i1*3+0], aXYZ[i1*3+1]+aDisp[i1*3+1], aXYZ[i1*3+2]+aDisp[i1*3+2] };
    const double p2[3] = { aXYZ[i2*3+0]+aDisp[i2*3+0], aXYZ[i2*3+1]+aDisp[i2*3+1], aXYZ[i2*3+2]+aDisp[i2*3+2] };
    const double p3[3] = { aXYZ[i3*3+0]+aDisp[i3*3+0], aXYZ[i3*3+1]+aDisp[i3*3+1], aXYZ[i3*3+2]+aDisp[i3*3+2] };
    double un0[3], a0; UnitNormalAreaTri3D(un0,a0, p1,p2,p3);
    double un1[3], a1; UnitNormalAreaTri3D(un1,a1, p2,p0,p3);
    double un2[3], a2; UnitNormalAreaTri3D(un2,a2, p3,p0,p1);
    double un3[3], a3; UnitNormalAreaTri3D(un3,a3, p0,p2,p1);
      //    ::glColor3d(0, 0, 0);
    ::glBegin(GL_TRIANGLES);
    ::glNormal3dv(un0); ::glVertex3dv(p1); ::glVertex3dv(p2); ::glVertex3dv(p3);
    ::glNormal3dv(un1); ::glVertex3dv(p2); ::glVertex3dv(p3); ::glVertex3dv(p0);
    ::glNormal3dv(un2); ::glVertex3dv(p3); ::glVertex3dv(p0); ::glVertex3dv(p1);
    ::glNormal3dv(un3); ::glVertex3dv(p0); ::glVertex3dv(p1); ::glVertex3dv(p2);
    ::glEnd();
  }
}
 */

// ---------------------------------------------------------

DFM2_INLINE void delfem2::opengl::DrawMeshHex3D_FaceNorm(
    const double* aXYZ,
    const unsigned int* aHex,
    unsigned int nHex)
{
  namespace lcl = ::delfem2::opengl::funcs_glold;
  ::glBegin(GL_TRIANGLES);
  for (unsigned int ihex = 0; ihex<nHex; ihex++){
    const unsigned int i0 = aHex[ihex*8+0];
    const unsigned int i1 = aHex[ihex*8+1];
    const unsigned int i2 = aHex[ihex*8+2];
    const unsigned int i3 = aHex[ihex*8+3];
    const unsigned int i4 = aHex[ihex*8+4];
    const unsigned int i5 = aHex[ihex*8+5];
    const unsigned int i6 = aHex[ihex*8+6];
    const unsigned int i7 = aHex[ihex*8+7];
    const double p0[3] = { aXYZ[i0*3+0], aXYZ[i0*3+1], aXYZ[i0*3+2] };
    const double p1[3] = { aXYZ[i1*3+0], aXYZ[i1*3+1], aXYZ[i1*3+2] };
    const double p2[3] = { aXYZ[i2*3+0], aXYZ[i2*3+1], aXYZ[i2*3+2] };
    const double p3[3] = { aXYZ[i3*3+0], aXYZ[i3*3+1], aXYZ[i3*3+2] };
    const double p4[3] = { aXYZ[i4*3+0], aXYZ[i4*3+1], aXYZ[i4*3+2] };
    const double p5[3] = { aXYZ[i5*3+0], aXYZ[i5*3+1], aXYZ[i5*3+2] };
    const double p6[3] = { aXYZ[i6*3+0], aXYZ[i6*3+1], aXYZ[i6*3+2] };
    const double p7[3] = { aXYZ[i7*3+0], aXYZ[i7*3+1], aXYZ[i7*3+2] };
    const double* aP[8] = {p0,p1,p2,p3,p4,p5,p6,p7};
    for(int iface=0;iface<6;++iface){
      const double* q0 = aP[ lcl::noelElemFace_Hex[iface][0] ];
      const double* q1 = aP[ lcl::noelElemFace_Hex[iface][1] ];
      const double* q2 = aP[ lcl::noelElemFace_Hex[iface][2] ];
      const double* q3 = aP[ lcl::noelElemFace_Hex[iface][3] ];
      double un0[3], a0; funcs_glold::UnitNormalAreaTri3D(un0,a0, q0,q1,q2);
      ::glNormal3dv(un0); ::glVertex3dv(q0); ::glVertex3dv(q1); ::glVertex3dv(q2);
      double un1[3], a1; funcs_glold::UnitNormalAreaTri3D(un1,a1, q0,q2,q3);
      ::glNormal3dv(un1); ::glVertex3dv(q0); ::glVertex3dv(q2); ::glVertex3dv(q3);
    }
  }
  ::glEnd();
}

DFM2_INLINE void delfem2::opengl::DrawHex3D_FaceNormDisp
(const std::vector<double>& aXYZ,
 const std::vector<int>& aHex,
 const std::vector<double>& aDisp)
{
  namespace lcl = ::delfem2::opengl::funcs_glold;
  ::glBegin(GL_TRIANGLES);
  for (unsigned int ihex = 0; ihex<aHex.size()/8; ihex++){
    const int i0 = aHex[ihex*8+0];
    const int i1 = aHex[ihex*8+1];
    const int i2 = aHex[ihex*8+2];
    const int i3 = aHex[ihex*8+3];
    const int i4 = aHex[ihex*8+4];
    const int i5 = aHex[ihex*8+5];
    const int i6 = aHex[ihex*8+6];
    const int i7 = aHex[ihex*8+7];
    const double p0[3] = { aXYZ[i0*3+0], aXYZ[i0*3+1], aXYZ[i0*3+2] };
    const double p1[3] = { aXYZ[i1*3+0], aXYZ[i1*3+1], aXYZ[i1*3+2] };
    const double p2[3] = { aXYZ[i2*3+0], aXYZ[i2*3+1], aXYZ[i2*3+2] };
    const double p3[3] = { aXYZ[i3*3+0], aXYZ[i3*3+1], aXYZ[i3*3+2] };
    const double p4[3] = { aXYZ[i4*3+0], aXYZ[i4*3+1], aXYZ[i4*3+2] };
    const double p5[3] = { aXYZ[i5*3+0], aXYZ[i5*3+1], aXYZ[i5*3+2] };
    const double p6[3] = { aXYZ[i6*3+0], aXYZ[i6*3+1], aXYZ[i6*3+2] };
    const double p7[3] = { aXYZ[i7*3+0], aXYZ[i7*3+1], aXYZ[i7*3+2] };
    ////
    const double r0[3] = { p0[0]+aDisp[i0*3+0], p0[1]+aDisp[i0*3+1], p0[2]+aDisp[i0*3+2] };
    const double r1[3] = { p1[0]+aDisp[i1*3+0], p1[1]+aDisp[i1*3+1], p1[2]+aDisp[i1*3+2] };
    const double r2[3] = { p2[0]+aDisp[i2*3+0], p2[1]+aDisp[i2*3+1], p2[2]+aDisp[i2*3+2] };
    const double r3[3] = { p3[0]+aDisp[i3*3+0], p3[1]+aDisp[i3*3+1], p3[2]+aDisp[i3*3+2] };
    const double r4[3] = { p4[0]+aDisp[i4*3+0], p4[1]+aDisp[i4*3+1], p4[2]+aDisp[i4*3+2] };
    const double r5[3] = { p5[0]+aDisp[i5*3+0], p5[1]+aDisp[i5*3+1], p5[2]+aDisp[i5*3+2] };
    const double r6[3] = { p6[0]+aDisp[i6*3+0], p6[1]+aDisp[i6*3+1], p6[2]+aDisp[i6*3+2] };
    const double r7[3] = { p7[0]+aDisp[i7*3+0], p7[1]+aDisp[i7*3+1], p7[2]+aDisp[i7*3+2] };
    const double* aR[8] = {r0,r1,r2,r3,r4,r5,r6,r7};
    for(int iface=0;iface<6;++iface){
      const double* q0 = aR[ lcl::noelElemFace_Hex[iface][0] ];
      const double* q1 = aR[ lcl::noelElemFace_Hex[iface][1] ];
      const double* q2 = aR[ lcl::noelElemFace_Hex[iface][2] ];
      const double* q3 = aR[ lcl::noelElemFace_Hex[iface][3] ];
      double un0[3], a0; funcs_glold::UnitNormalAreaTri3D(un0,a0, q0,q1,q2);
      ::glNormal3dv(un0); ::glVertex3dv(q0); ::glVertex3dv(q1); ::glVertex3dv(q2);
      double un1[3], a1; funcs_glold::UnitNormalAreaTri3D(un1,a1, q0,q2,q3);
      ::glNormal3dv(un1); ::glVertex3dv(q0); ::glVertex3dv(q2); ::glVertex3dv(q3);
    }
  }
  ::glEnd();
}

DFM2_INLINE void delfem2::opengl::DrawMeshHex3D_Edge(
    const double* aXYZ,
    const unsigned int nXYZ,
    const unsigned int* aHex,
    const unsigned int nHex)
{
  const int noelEdge_Hex[12][2] = {
    {0,1},{3,2},{4,5},{7,6},
    {0,3},{1,2},{4,7},{5,6},
    {0,4},{1,5},{3,7},{2,6} };
  ::glBegin(GL_LINES);
  for (unsigned int ihex = 0; ihex<nHex; ihex++){
    const unsigned int i0 = aHex[ihex*8+0];
    const unsigned int i1 = aHex[ihex*8+1];
    const unsigned int i2 = aHex[ihex*8+2];
    const unsigned int i3 = aHex[ihex*8+3];
    const unsigned int i4 = aHex[ihex*8+4];
    const unsigned int i5 = aHex[ihex*8+5];
    const unsigned int i6 = aHex[ihex*8+6];
    const unsigned int i7 = aHex[ihex*8+7];
    const double p0[3] = { aXYZ[i0*3+0], aXYZ[i0*3+1], aXYZ[i0*3+2] };
    const double p1[3] = { aXYZ[i1*3+0], aXYZ[i1*3+1], aXYZ[i1*3+2] };
    const double p2[3] = { aXYZ[i2*3+0], aXYZ[i2*3+1], aXYZ[i2*3+2] };
    const double p3[3] = { aXYZ[i3*3+0], aXYZ[i3*3+1], aXYZ[i3*3+2] };
    const double p4[3] = { aXYZ[i4*3+0], aXYZ[i4*3+1], aXYZ[i4*3+2] };
    const double p5[3] = { aXYZ[i5*3+0], aXYZ[i5*3+1], aXYZ[i5*3+2] };
    const double p6[3] = { aXYZ[i6*3+0], aXYZ[i6*3+1], aXYZ[i6*3+2] };
    const double p7[3] = { aXYZ[i7*3+0], aXYZ[i7*3+1], aXYZ[i7*3+2] };
    const double* aP[8] = {p0,p1,p2,p3,p4,p5,p6,p7};
    for(auto iedge : noelEdge_Hex){
      const double* q0 = aP[ iedge[0] ];
      const double* q1 = aP[ iedge[1] ];
      ::glVertex3dv(q0);
      ::glVertex3dv(q1);
    }
  }
  ::glEnd();
}

DFM2_INLINE void delfem2::opengl::DrawMeshTet3D_FaceNormDisp(
    const double* aXYZ,
    const unsigned int nXYZ,
    const unsigned int* aTet,
    const unsigned int nTet,
    const double* aDisp)
{
  for (unsigned int itet = 0; itet<nTet; itet++){
    const unsigned int i0 = aTet[itet*4+0];
    const unsigned int i1 = aTet[itet*4+1];
    const unsigned int i2 = aTet[itet*4+2];
    const unsigned int i3 = aTet[itet*4+3];
    const double p0[3] = { aXYZ[i0*3+0]+aDisp[i0*3+0], aXYZ[i0*3+1]+aDisp[i0*3+1], aXYZ[i0*3+2]+aDisp[i0*3+2] };
    const double p1[3] = { aXYZ[i1*3+0]+aDisp[i1*3+0], aXYZ[i1*3+1]+aDisp[i1*3+1], aXYZ[i1*3+2]+aDisp[i1*3+2] };
    const double p2[3] = { aXYZ[i2*3+0]+aDisp[i2*3+0], aXYZ[i2*3+1]+aDisp[i2*3+1], aXYZ[i2*3+2]+aDisp[i2*3+2] };
    const double p3[3] = { aXYZ[i3*3+0]+aDisp[i3*3+0], aXYZ[i3*3+1]+aDisp[i3*3+1], aXYZ[i3*3+2]+aDisp[i3*3+2] };
    double un0[3], a0; funcs_glold::UnitNormalAreaTri3D(un0,a0, p1,p2,p3);
    double un1[3], a1; funcs_glold::UnitNormalAreaTri3D(un1,a1, p2,p0,p3);
    double un2[3], a2; funcs_glold::UnitNormalAreaTri3D(un2,a2, p3,p0,p1);
    double un3[3], a3; funcs_glold::UnitNormalAreaTri3D(un3,a3, p0,p2,p1);
    ::glBegin(GL_TRIANGLES);
    ::glNormal3dv(un0); ::glVertex3dv(p1); ::glVertex3dv(p2); ::glVertex3dv(p3);
    ::glNormal3dv(un1); ::glVertex3dv(p2); ::glVertex3dv(p3); ::glVertex3dv(p0);
    ::glNormal3dv(un2); ::glVertex3dv(p3); ::glVertex3dv(p0); ::glVertex3dv(p1);
    ::glNormal3dv(un3); ::glVertex3dv(p0); ::glVertex3dv(p1); ::glVertex3dv(p2);
    ::glEnd();
  }
}

// ----------------------------------------------------------

DFM2_INLINE void delfem2::opengl::showdepth()
{
  GLint view[4]; glGetIntegerv(GL_VIEWPORT, view); // get viewport size
  GLubyte* buffer = (GLubyte *)malloc(view[2] * view[3]); // get buffer with view port size
  if (!buffer) { return; }
  
  glFinish(); // wait for finishing display
  
  // read depth buffer
  glReadPixels(view[0], view[1], view[2], view[3],
               GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, buffer);
  // write to color buffer
  glDrawPixels(view[2], view[3], GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer);
  
  free(buffer);
}

// --------------------------

DFM2_INLINE void delfem2::opengl::getPosOnScreen_Camera2D
(double& x, double& y,
 int i, int j)
{
  int viewport[8];
  glGetIntegerv(GL_VIEWPORT, viewport);
  double hw = (double)viewport[2]*0.5; // half width
  double hh = (double)viewport[3]*0.5; // half height
  double asp = hw/hh;
  x = (i-hw)/hw*asp;
  y = (hh-j)/hh;
}

DFM2_INLINE void delfem2::opengl::setGL_Camera2D()
{
  int viewport[8];
  glGetIntegerv(GL_VIEWPORT, viewport);
  auto w = (double)viewport[2];
  auto h = (double)viewport[3];
  double asp = w/h;
  ::glMatrixMode(GL_PROJECTION);
  ::glLoadIdentity();
  ::glOrtho(-asp*2, +asp*2, -2, +2, -10, +10);
  ::glMatrixMode(GL_MODELVIEW);
  ::glLoadIdentity();
}
