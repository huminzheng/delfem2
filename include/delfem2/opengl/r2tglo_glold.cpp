/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "glad/glad.h" // gl3.0+
#if defined(__APPLE__) && defined(__MACH__) // Mac
  #include <OpenGL/gl.h>
#elif defined(_WIN32) // windows
  #include <windows.h>
  #include <GL/gl.h>
#else
  #include <GL/gl.h>
#endif
#include "delfem2/opengl/funcs_glold.h"
#include "delfem2/opengl/v3q_glold.h"
#include "delfem2/opengl/r2tglo_glold.h"
#include "delfem2/vec3.h"

// ------------------


// --------------------------------------------

void delfem2::opengl::CRender2Tex_DrawOldGL::SetPointColor(double r, double g, double b){
  colorPoint[0] = r;
  colorPoint[1] = g;
  colorPoint[2] = b;
}

void delfem2::opengl::CRender2Tex_DrawOldGL::InitGL() {
  CRender2Tex::InitGL();
  if( aRGBA_8ui.size() == nResX*nResY*4 ){
    ::glBindTexture(GL_TEXTURE_2D, id_tex_color);
      // define size and format of level 0
    ::glTexImage2D(GL_TEXTURE_2D, 0,
        GL_RGBA, nResX, nResY, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, aRGBA_8ui.data());
  }
}


void delfem2::opengl::CRender2Tex_DrawOldGL::SetView(){
  ::glMatrixMode(GL_MODELVIEW);
  ::glLoadIdentity();
  ::glMultMatrixd(this->mMV);
  ::glMatrixMode(GL_PROJECTION);
  ::glLoadIdentity();
  ::glMultMatrixd(this->mP);
  ::glMatrixMode(GL_MODELVIEW);
}


void delfem2::opengl::CRender2Tex_DrawOldGL::Start()
{
  CRender2Tex::Start();
  this->SetView();
}

void delfem2::opengl::CRender2Tex_DrawOldGL::GetDepth()
{
  CRender2Tex::ExtractFromTexture_Depth(aZ);
}

void delfem2::opengl::CRender2Tex_DrawOldGL::GetColor()
{
  if( is_rgba_8ui ){
    CRender2Tex::ExtractFromTexture_RGBA8UI(aRGBA_8ui);
  }
  else{
    CRender2Tex::ExtractFromTexture_RGBA32F(aRGBA_32f);
  }
}

void delfem2::opengl::CRender2Tex_DrawOldGL::Draw_Texture() const {
  double mMVP[16]; MatMat4(mMVP, this->mMV,this->mP);
  double mMVPinv[16]; Inverse_Mat4(mMVPinv, mMVP);
  ::glMatrixMode(GL_MODELVIEW);
  ::glPushMatrix();
  ::glMultMatrixd(mMVPinv);
  //
  ::glEnable(GL_TEXTURE_2D);
  ::glDisable(GL_LIGHTING);
  ::glColor3d(1,1,1);
  ::glBegin(GL_QUADS);
  ::glTexCoord2d(0.0, 0.0); ::glVertex3d(-1,-1,-1);
  ::glTexCoord2d(1.0, 0.0); ::glVertex3d(+1,-1,-1);
  ::glTexCoord2d(1.0, 1.0); ::glVertex3d(+1,+1,-1);
  ::glTexCoord2d(0.0, 1.0); ::glVertex3d(-1,+1,-1);
  ::glEnd();
  ::glBindTexture(GL_TEXTURE_2D, 0);
  ::glDisable(GL_TEXTURE_2D);
  //
  ::glPopMatrix();
}

void delfem2::opengl::CRender2Tex_DrawOldGL::Draw() const {

  ::glPointSize(this->pointSize);
  this->Draw_Point();
  // -----------
  ::glLineWidth(3);
  this->Draw_Axis();
  // ----------
  ::glLineWidth(1);
  ::glColor3d(0,0,0);
  this->Draw_BoundingBox();
  // -----------
  if( id_tex_color > 0 && this->isDrawTex ){
    ::glBindTexture(GL_TEXTURE_2D, id_tex_color);
    this->Draw_Texture();
  }

}

void delfem2::opengl::CRender2Tex_DrawOldGL::Draw_Axis() const
{
  double mMVP[16]; MatMat4(mMVP, this->mMV,this->mP);
  double mMVPinv[16]; Inverse_Mat4(mMVPinv, mMVP);
  ::glMatrixMode(GL_MODELVIEW);
  ::glPushMatrix();
  ::glMultMatrixd(mMVPinv);
  ::glTranslated(-1.01,-1.01,-1.01);
  delfem2::opengl::DrawAxis(draw_len_axis);
  ::glPopMatrix();
}

void delfem2::opengl::CRender2Tex_DrawOldGL::Draw_BoundingBox() const
{
  double mMVP[16]; MatMat4(mMVP, this->mMV,this->mP);
  double mMVPinv[16]; Inverse_Mat4(mMVPinv, mMVP);
  ::glMatrixMode(GL_MODELVIEW);
  ::glPushMatrix();
  ::glMultMatrixd(mMVPinv);
  ::glLineWidth(3);
  ::glDisable(GL_LIGHTING);
  const double pmin[3] = {-1., -1., -1.};
  const double pmax[3] = {+1., +1., +1.};
  ::delfem2::opengl::DrawBox3_Edge(pmin, pmax);
  ::glMatrixMode(GL_MODELVIEW);
  ::glPopMatrix();
}

void delfem2::opengl::CRender2Tex_DrawOldGL::Draw_Point() const
{
  double mMVP[16]; MatMat4(mMVP, this->mMV,this->mP);
  double mMVPinv[16]; Inverse_Mat4(mMVPinv, mMVP);
  //
  ::glMatrixMode(GL_MODELVIEW);
  ::glPushMatrix();
  ::glMultMatrixd(mMVPinv);
  // --
  ::glDisable(GL_LIGHTING);
  if( aZ.size() != nResX*nResY ){
    glPopMatrix();
    return;
  }
  if( colorPoint.size() == 3 ){ ::glColor3dv(colorPoint.data()); }
  if( colorPoint.size() == 4 ){ ::glColor4dv(colorPoint.data()); }
  ::glBegin(GL_POINTS);
  for(unsigned int iy=0;iy<nResY;++iy){
    for(unsigned int ix=0;ix<nResX;++ix){
      const double x0 = -1+2.0/nResX*ix;
      const double y0 = -1+2.0/nResY*iy;
      const double z0 = 2.0*aZ[iy*nResX+ix]-1;
      if( z0 > 0.9 && isDrawOnlyHitPoints ){ continue; } // ray is shooted from -1 to +1
      ::glVertex3d(x0,y0,z0);
    }
  }
  ::glEnd();
  // --
  glPopMatrix();
}

void delfem2::opengl::CRender2Tex_DrawOldGL::getGPos(
    double* p,
    int ix,
    int iy) const
{
  /*
  const CVec3d& dx = x_axis;
  const CVec3d& dz = z_axis;
  const CVec3d& dy = Cross(dz,dx);
  const double lz = aZ[iy*nResX+ix];
  const double lx = (ix+0.5)*lengrid;
  const double ly = (iy+0.5)*lengrid;
  CVec3d vp =  CVec3d(origin) + lx*dx + ly*dy + lz*dz;
  vp.CopyTo(p);
   */
}

void delfem2::opengl::CRender2Tex_DrawOldGL::BoundingBox3(
    double* pmin,
    double* pmax) const
{
  /*
  if( aZ.size() != nResX*nResY ){ return; }
  for(unsigned int ix=0;ix<nResX;++ix){
    for(unsigned int iy=0;iy<nResY;++iy){
      CVec3d vp;
      {
        const CVec3d& dx = x_axis;
        const CVec3d& dz = z_axis;
        const CVec3d& dy = Cross(dz,dx);
        double lz = aZ[iy*nResX+ix];
        double lx = (ix+0.5)*lengrid;
        double ly = (iy+0.5)*lengrid;
        vp = lx*dx+ly*dy+lz*dz + CVec3d(origin);
        if( -lz > z_range*0.99 ) continue;
      }
      // ----------
      if( pmin[0] > pmax[0] ){
        pmin[0] = pmax[0] = vp.x();
        pmin[1] = pmax[1] = vp.y();
        pmin[2] = pmax[2] = vp.z();
        continue;
      }
      const double x0 = vp.x();
      if( x0 < pmin[0] ){ pmin[0] = x0; }
      if( x0 > pmax[0] ){ pmax[0] = x0; }
      const double y0 = vp.y();
      if( y0 < pmin[1] ){ pmin[1] = y0; }
      if( y0 > pmax[1] ){ pmax[1] = y0; }
      const double z0 = vp.z();
      if( z0 < pmin[2] ){ pmin[2] = z0; }
      if( z0 > pmax[2] ){ pmax[2] = z0; }
    }
  }
   */
}


// --------------------------------------------------------

void delfem2::opengl::CRender2Tex_DrawOldGL_BOX::Initialize(
    unsigned int nresX_,
    unsigned int nresY_,
    unsigned int nresZ_,
    double elen_)
{
  this->lengrid = elen_;
  //
  aSampler.resize(0);
  {
    aSampler.resize(aSampler.size() + 1);
    auto &smplr = aSampler[aSampler.size() - 1];
    smplr.SetTextureProperty(nresY_, nresZ_, true); // +x
    ::delfem2::Mat4_OrthongoalProjection_AffineTrans(
        smplr.mMV, smplr.mP,
        CVec3d(+0.5 * elen_ * nresX_, -0.5 * elen_ * nresY_, -0.5 * elen_ * nresZ_).p,
        CVec3d(+1, 0, 0).p,
        CVec3d(0, +1, 0).p,
        nresY_, nresZ_, elen_, elen_ * nresX_);
    smplr.SetPointColor(1.0, 0.0, 0.0);
  }


  {
    aSampler.resize(aSampler.size() + 1);
    auto &smplr = aSampler[aSampler.size() - 1];
    smplr.SetTextureProperty(nresY_, nresZ_, true); // -x
    ::delfem2::Mat4_OrthongoalProjection_AffineTrans(
        smplr.mMV, smplr.mP,
        CVec3d(-0.5 * elen_ * nresX_, -0.5 * elen_ * nresY_, +0.5 * elen_ * nresZ_).p,
        CVec3d(-1, 0, 0).p,
        CVec3d(0, +1, 0).p,
        nresY_, nresZ_, elen_, elen_ * nresX_);
    smplr.SetPointColor(1.0, 0.5, 0.5);
  }
  //
  {
    aSampler.resize(aSampler.size() + 1);
    auto &smplr = aSampler[aSampler.size() - 1];
    smplr.SetTextureProperty(nresX_, nresZ_, true); // +y
    ::delfem2::Mat4_OrthongoalProjection_AffineTrans(
        smplr.mMV, smplr.mP,
        CVec3d(-0.5 * elen_ * nresX_, +0.5 * elen_ * nresY_, +0.5 * elen_ * nresZ_).p,
        CVec3d(0, +1, 0).p,
        CVec3d(1, +0, 0).p,
        nresX_, nresZ_, elen_, elen_ * nresY_);
    smplr.SetPointColor(0.0, 1.0, 0.0);
  }
  {
    aSampler.resize(aSampler.size() + 1);
    auto &smplr = aSampler[aSampler.size() - 1];
    smplr.SetTextureProperty(nresX_, nresZ_, true); // -y
    ::delfem2::Mat4_OrthongoalProjection_AffineTrans(
        smplr.mMV, smplr.mP,
        CVec3d(-0.5 * elen_ * nresX_, -0.5 * elen_ * nresY_, -0.5 * elen_ * nresZ_).p,
        CVec3d(0, -1, 0).p,
        CVec3d(1, +0, 0).p,
        nresX_, nresZ_, elen_, elen_ * nresY_);
    smplr.SetPointColor(0.5, 1.0, 0.5);
  }
  {
    aSampler.resize(aSampler.size()+1);
    auto& smplr = aSampler[aSampler.size()-1];
    smplr.SetTextureProperty(nresX_, nresY_, true);
    ::delfem2::Mat4_OrthongoalProjection_AffineTrans(
        smplr.mMV, smplr.mP,
        CVec3d(-0.5 * elen_ * nresX_, -0.5 * elen_ * nresY_, +0.5 * elen_ * nresZ_).p,
        CVec3d(0, 0, +1).p,
        CVec3d(1, 0, 0).p,
        nresX_, nresY_, elen_, elen_ * nresZ_);
    smplr.SetPointColor(0.0, 0.0, 1.0);
  }
  {
    aSampler.resize(aSampler.size()+1);
    auto& smplr = aSampler[aSampler.size()-1];
    smplr.SetTextureProperty(nresX_, nresY_, true);
    ::delfem2::Mat4_OrthongoalProjection_AffineTrans(
        smplr.mMV, smplr.mP,
        CVec3d(-0.5 * elen_ * nresX_, +0.5 * elen_ * nresY_, -0.5 * elen_ * nresZ_).p,
        CVec3d(0, 0, -1).p,
        CVec3d(1, 0, 0).p,
        nresX_, nresY_, elen_, elen_ * nresZ_);
    smplr.SetPointColor(0.5, 0.5, 1.0);
  }

  // ------------------------
  for(auto& smplr : aSampler){
    smplr.draw_len_axis = 0.2;
    smplr.isDrawTex = false;
    smplr.isDrawOnlyHitPoints = true;
  }

}


void delfem2::opengl::CarveVoxelByDepth(
    std::vector<int>& aVal,
    const CRender2Tex_DrawOldGL_BOX& sampler_box)
{
  const unsigned int nx = sampler_box.nDivX();
  const unsigned int ny = sampler_box.nDivY();
  const unsigned int nz = sampler_box.nDivZ();
  const double el = sampler_box.edgeLen();
  // ------
  aVal.assign(nz*ny*nx,1);
  for(unsigned int iy=0;iy<ny;++iy){
    for(unsigned int iz=0;iz<nz;++iz){
      double d0 = sampler_box.aSampler[0].aZ[iz*ny+iy];
      const unsigned int nd = d0*nx;
      for(unsigned int id=0;id<nd;id++){
        const unsigned int ix0 = nx-id-1;
        const unsigned int iy0 = iy;
        const unsigned int iz0 = iz;
        aVal[iz0*ny*nx+iy0*nx+ix0] = 0;
      }
    }
  }
  for(unsigned int iy=0;iy<ny;++iy){
    for(unsigned int iz=0;iz<nz;++iz){
      double d0 = sampler_box.aSampler[1].aZ[iz*ny+iy];
      const unsigned int nd = d0*nx;
      for(unsigned int id=0;id<nd;id++){
        const unsigned int ix0 = id;
        const unsigned int iy0 = iy;
        const unsigned int iz0 = nz-1-iz;
        aVal[iz0*ny*nx+iy0*nx+ix0] = 0;
      }
    }
  }

  for(unsigned int ix=0;ix<nx;++ix){
    for(unsigned int iz=0;iz<nz;++iz){
      double d0 = sampler_box.aSampler[2].aZ[iz*nx+ix];
      const unsigned int nd = d0*ny;
      for(unsigned int id=0;id<nd;id++){
        const unsigned int ix0 = ix;
        const unsigned int iy0 = ny-1-id;
        const unsigned int iz0 = nz-1-iz;
        aVal[iz0*ny*nx+iy0*nx+ix0] = 0;
      }
    }
  }
  for(unsigned int ix=0;ix<nx;++ix){
    for(unsigned int iz=0;iz<nz;++iz){
      double d0 = sampler_box.aSampler[3].aZ[iz*nx+ix];
      const unsigned int nd = d0*ny;
      for(unsigned int id=0;id<nd;id++){
        const unsigned int ix0 = ix;
        const unsigned int iy0 = id;
        const unsigned int iz0 = iz;
        aVal[iz0*ny*nx+iy0*nx+ix0] = 0;
      }
    }
  }
  for(unsigned int ix=0;ix<nx;++ix){
    for(unsigned int iy=0;iy<ny;++iy){
      double d0 = sampler_box.aSampler[4].aZ[iy*nx+ix];
      const unsigned int nd = d0*nz;
      for(unsigned int id=0;id<nd;id++){
        const unsigned int ix0 = ix;
        const unsigned int iy0 = iy;
        const unsigned int iz0 = nz-1-id;
        aVal[iz0*ny*nx+iy0*nx+ix0] = 0;
      }
    }
  }
  for(unsigned int ix=0;ix<nx;++ix){
    for(unsigned int iy=0;iy<ny;++iy){
      double d0 = sampler_box.aSampler[5].aZ[iy*nx+ix];
      const unsigned int nd = d0*nz;
      for(unsigned int id=0;id<nd;id++){
        const unsigned int ix0 = ix;
        const unsigned int iy0 = ny-1-iy;
        const unsigned int iz0 = id;
        aVal[iz0*ny*nx+iy0*nx+ix0] = 0;
      }
    }
  }
}


bool delfem2::opengl::GetProjectedPoint(
    CVec3d& p0,
    CVec3d& n0,
    const CVec3d& ps,
    const CRender2Tex_DrawOldGL& smplr)
{
  double mMVPG[16]; smplr.GetMVPG(mMVPG);
  double mMVPGinv[16]; Inverse_Mat4(mMVPGinv, mMVPG);
  double pg[3]; Vec3_Vec3Mat4_AffineProjection(pg,ps.p,mMVPG);
  const unsigned int nx = smplr.nResX;
  const unsigned int ny = smplr.nResY;
  /*
  double p1[3]; Vec3_Vec3Mat4_AffineProjection(p1,ps.p,smplr.mMV);
  double p2[3]; Vec3_Vec3Mat4_AffineProjection(p2,p1,smplr.mP);
  p2[0] = (p2[0] + 1.0)*0.5*nx - 0.5;
  p2[1] = (p2[1] + 1.0)*0.5*ny - 0.5;
   */
  const int ix0 = (int)floor(pg[0]);
  const int iy0 = (int)floor(pg[1]);
  const int ix1 = ix0+1;
  const int iy1 = iy0+1;
  if( ix0 < 0 && ix0 >= (int)nx ){ return false; }
  if( ix1 < 0 && ix1 >= (int)nx ){ return false; }
  if( iy0 < 0 && iy0 >= (int)ny ){ return false; }
  if( iy1 < 0 && iy1 >= (int)ny ){ return false; }
  if( smplr.aZ[iy0*nx+ix0] > 0.99 ) return false;
  if( smplr.aZ[iy0*nx+ix1] > 0.99 ) return false;
  if( smplr.aZ[iy1*nx+ix0] > 0.99 ) return false;
  if( smplr.aZ[iy1*nx+ix1] > 0.99 ) return false;
  const CVec3d p00(ix0,iy0,smplr.aZ[iy0*nx+ix0]);
  const CVec3d p01(ix0,iy1,smplr.aZ[iy1*nx+ix0]);
  const CVec3d p10(ix1,iy0,smplr.aZ[iy0*nx+ix1]);
  const CVec3d p11(ix1,iy1,smplr.aZ[iy1*nx+ix1]);
  const double rx = pg[0]-ix0;
  const double ry = pg[1]-iy0;
  CVec3d p3 = (1-rx)*(1-ry)*p00 + rx*(1-ry)*p10 + (1-rx)*ry*p01 + rx*ry*p11;
  CVec3d dpx = (ry-1)*p00 + (1-ry)*p10 - ry*p01 + ry*p11;
  CVec3d dpy = (rx-1)*p00 - rx*p10 + (1-rx)*p01 + rx*p11;
  CVec3d n3 = Cross(dpx,dpy);
  //
  /*
  std::cout << p3 << std::endl;
  p3.p[0] = (p3.p[0]+0.5)*2.0/nx;
  p3.p[1] = (p3.p[1]+0.5)*2.0/ny;
  p3.p[2] *= 2.0;
  p3 -= CVec3d(1,1,1);
   */
  Vec3_Vec3Mat4_AffineProjection(p0.p, p3.p,mMVPGinv);
  CVec3d p4; Vec3_Vec3Mat4_AffineProjection(p4.p, (p3+n3).p,mMVPGinv);
  n0 = (p4-p0).Normalize();
  return true;
}
