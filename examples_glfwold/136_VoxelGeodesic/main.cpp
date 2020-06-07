/*
 * Copyright (c) 2020 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <random>
#include "delfem2/mshio.h"
#include "delfem2/mshmisc.h"
#include "delfem2/gridvoxel.h"
//
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "delfem2/opengl/color_glold.h"
#include "delfem2/opengl/r2tglo_glold.h"
#include "delfem2/opengl/funcs_glold.h"
#include "delfem2/opengl/glfw/viewer_glfw.h"

namespace dfm2 = delfem2;

// ------------------------------------------------------

void Draw_CGrid3
(const dfm2::CGrid3<int>& grid,
 const std::vector<double>& aDist,
 const std::vector<std::pair<double, dfm2::CColor> >& colorMap,
 const unsigned int ivox0)
{
  { // set-up transformation
    const dfm2::CMat4d& am = grid.am;
    dfm2::CMat4d amt = am.Transpose();
    ::glMatrixMode(GL_MODELVIEW);
    ::glPushMatrix();
    ::glMultMatrixd(amt.mat);
  }
  // -----------
  /*
  {
    ::glDisable(GL_LIGHTING);
    const double x0 = (double)grid.ndivx;
    const double y0 = (double)grid.ndivy;
    const double z0 = (double)grid.ndivz;
    double p[2][3] = {
      { 0, 0, 0},
      {x0, 0, 0},
    };
    ::glBegin(GL_LINES);
    ::glVertex3dv(p[0]);
    ::glVertex3dv(p[1]);
    ::glEnd();
  }
   */
  {
    ::glEnable(GL_LIGHTING);
    ::glEnable(GL_NORMALIZE);
    const unsigned int nx = grid.ndivx;
    const unsigned int ny = grid.ndivy;
    const unsigned int nz = grid.ndivz;
    for(unsigned int iz=0;iz<nz;++iz){
      for(unsigned int iy=0;iy<ny;++iy){
        for(unsigned int ix=0;ix<nx;++ix){
          if( grid.aVal[iz*ny*nx+iy*nx+ix] == 0 ){ continue; }
          const double val = aDist[iz*ny*nx+iy*nx+ix];
          dfm2::CColor c = dfm2::getColor(val, colorMap);
          dfm2::opengl::myGlMaterialDiffuse(c);
          const double pmin[3] = {(double)ix,(double)iy,(double)iz};
          const double pmax[3] = {(double)ix+1.,(double)iy+1.,(double)iz+1.};
          dfm2::opengl::DrawBox3_Face(pmin,pmax);
        }
      }
    }
  }
  
  {
    ::glDisable(GL_LIGHTING);
    ::glColor3d(1,0,0);
    const unsigned int nx = grid.ndivx;
    const unsigned int ny = grid.ndivy;
//    const unsigned int nz = grid.ndivz;
    const int iz0 = ivox0/(ny*nx);
    const int iy0 = (ivox0-iz0*ny*nx)/nx;
    const int ix0 = ivox0-iz0*ny*nx-iy0*nx;
    dfm2::opengl::DrawSphereAt(32, 32, 5.0, ix0+0.5, iy0+0.5, iz0+0.5);
  }
  
  // --------
  // end transformation
  ::glPopMatrix();
}


// ------------------------------------------------------

int main(int argc,char* argv[])
{
  std::vector<double> aXYZ;
  std::vector<unsigned int> aTri;

  dfm2::Read_Obj(
      std::string(PATH_INPUT_DIR)+"/bunny_1k.obj",
      aXYZ,aTri);
  dfm2::Normalize_Points3(aXYZ,4.0);
  // ---------------------------------------

  dfm2::opengl::CRender2Tex_DrawOldGL_BOX sampler_box;
  sampler_box.Initialize(128, 128, 128, 0.04);

  // ---------------------------------------
  dfm2::opengl::CViewer_GLFW viewer;
  viewer.Init_oldGL();
  viewer.nav.camera.view_height = 3.0;
  viewer.nav.camera.camera_rot_mode = dfm2::CCamera<double>::CAMERA_ROT_MODE::TBALL;
//  viewer.nav.camera.Rot_Camera(+0.2, -0.2);
  if(!gladLoadGL()) {     // glad: load all OpenGL function pointers
    printf("Something went wrong in loading OpenGL functions!\n");
    exit(-1);
  }
  dfm2::opengl::setSomeLighting();
  ::glEnable(GL_DEPTH_TEST);
  // ------------
  for(auto& smplr: sampler_box.aSampler){
    smplr.InitGL(); // move the sampled image to a texture
    smplr.Start();
    ::glClearColor(1.0, 1.0, 1.0, 1.0 );
    ::glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    ::glEnable(GL_DEPTH_TEST);
    ::glDisable(GL_BLEND);
    ::glEnable(GL_LIGHTING);
    dfm2::opengl::DrawMeshTri3D_FaceNorm(aXYZ,aTri);
    smplr.End();
    smplr.GetDepth();
  }
  
  dfm2::CGrid3<int> grid;
  {
    const unsigned int nx = sampler_box.nDivX();
    const unsigned int ny = sampler_box.nDivY();
    const unsigned int nz = sampler_box.nDivZ();
    const double el = sampler_box.edgeLen();
    grid.am.SetScale(el, el, el);
    grid.am.mat[ 3] = -(nx*el*0.5);
    grid.am.mat[ 7] = -(ny*el*0.5);
    grid.am.mat[11] = -(nz*el*0.5);
    grid.Initialize(nx, ny, nz, 1);
  }
  CarveVoxelByDepth(grid.aVal,
                    sampler_box);
  dfm2::Grid3Voxel_Dilation(grid);
  
  std::random_device rd;
  std::mt19937 reng(rd());
  std::uniform_int_distribution<unsigned int> dist(0,grid.aVal.size()-1);
     
  std::vector<std::pair<double, dfm2::CColor> > colorMap;
  {
    double el = sampler_box.edgeLen();
    const unsigned int nx = sampler_box.nDivX();
    const unsigned int ny = sampler_box.nDivY();
    const unsigned int nz = sampler_box.nDivZ();
    const double len = el*sqrt(nx*nx + ny*ny + nz*nz);
    dfm2::ColorMap_BlueCyanGreenYellowRed(colorMap, 0, len);
  }
  // -------
  std::vector<double> aDist;
  while (true)
  {
    unsigned int ivox0 = UINT_MAX;
    for(int itr=0;itr<100;++itr){
      ivox0 = dist(reng);
      if( grid.aVal[ivox0] == 1 ){ break; }
    }
    if( ivox0 == UINT_MAX ){
      glfwDestroyWindow(viewer.window);
      glfwTerminate();
      exit(EXIT_SUCCESS);
    }
    VoxelGeodesic(aDist,
                  ivox0, sampler_box.edgeLen(), grid);
    for(int iframe=0;iframe<10;++iframe){
      viewer.DrawBegin_oldGL();
      Draw_CGrid3(grid,aDist,colorMap,ivox0);
      viewer.DrawEnd_oldGL();
      if( glfwWindowShouldClose(viewer.window) ){ goto EXIT; }
    }
  }
EXIT:
  glfwDestroyWindow(viewer.window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}


