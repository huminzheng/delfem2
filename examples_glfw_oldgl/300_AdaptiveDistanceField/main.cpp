/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include "delfem2/adf.h"
#include "delfem2/mshmisc.h"
#include "delfem2/mshio.h"
#include "delfem2/primitive.h"
#include "delfem2/bv.h"
//
#include "delfem2/srch_v3bvhmshtopo.h"

// ----------------
#include <GLFW/glfw3.h>
#include "delfem2/opengl/glfw_viewer.hpp"
#include "delfem2/opengl/glold_funcs.h"

namespace dfm2 = delfem2;

// -----------------------

void Draw_Wire(const delfem2::CADF3::CNode& n)
{
  ::glLineWidth(1);
  ::glColor3d(0,0,0);
  ::glBegin(GL_LINES);
  double hw_ = n.hw_;
  double cent_[3] = {n.cent_[0],n.cent_[1],n.cent_[2]};
  ::glVertex3d(cent_[0]-hw_, cent_[1]-hw_, cent_[2]-hw_);
  ::glVertex3d(cent_[0]+hw_, cent_[1]-hw_, cent_[2]-hw_);
  
  ::glVertex3d(cent_[0]+hw_, cent_[1]-hw_, cent_[2]-hw_);
  ::glVertex3d(cent_[0]+hw_, cent_[1]+hw_, cent_[2]-hw_);
  
  ::glVertex3d(cent_[0]+hw_, cent_[1]+hw_, cent_[2]-hw_);
  ::glVertex3d(cent_[0]-hw_, cent_[1]+hw_, cent_[2]-hw_);
  
  ::glVertex3d(cent_[0]-hw_, cent_[1]+hw_, cent_[2]-hw_);
  ::glVertex3d(cent_[0]-hw_, cent_[1]-hw_, cent_[2]-hw_);
    ////
  ::glVertex3d(cent_[0]-hw_, cent_[1]-hw_, cent_[2]+hw_);
  ::glVertex3d(cent_[0]+hw_, cent_[1]-hw_, cent_[2]+hw_);
  
  ::glVertex3d(cent_[0]+hw_, cent_[1]-hw_, cent_[2]+hw_);
  ::glVertex3d(cent_[0]+hw_, cent_[1]+hw_, cent_[2]+hw_);
  
  ::glVertex3d(cent_[0]+hw_, cent_[1]+hw_, cent_[2]+hw_);
  ::glVertex3d(cent_[0]-hw_, cent_[1]+hw_, cent_[2]+hw_);
  
  ::glVertex3d(cent_[0]-hw_, cent_[1]+hw_, cent_[2]+hw_);
  ::glVertex3d(cent_[0]-hw_, cent_[1]-hw_, cent_[2]+hw_);
    ////
  ::glVertex3d(cent_[0]-hw_, cent_[1]-hw_, cent_[2]-hw_);
  ::glVertex3d(cent_[0]-hw_, cent_[1]-hw_, cent_[2]+hw_);
  
  ::glVertex3d(cent_[0]+hw_, cent_[1]-hw_, cent_[2]-hw_);
  ::glVertex3d(cent_[0]+hw_, cent_[1]-hw_, cent_[2]+hw_);
  
  ::glVertex3d(cent_[0]+hw_, cent_[1]+hw_, cent_[2]-hw_);
  ::glVertex3d(cent_[0]+hw_, cent_[1]+hw_, cent_[2]+hw_);
  
  ::glVertex3d(cent_[0]-hw_, cent_[1]+hw_, cent_[2]-hw_);
  ::glVertex3d(cent_[0]-hw_, cent_[1]+hw_, cent_[2]+hw_);
  
  ::glEnd();
}

void DrawThisAndChild_Wire
 (const delfem2::CADF3::CNode& n,
  const std::vector<delfem2::CADF3::CNode>& aNo)
{
    //      std::cout << "ichild " << ichilds_[0] << " " << ichilds_[1] << " " << ichilds_[2] << " " << ichilds_[3] << std::endl;
  if( n.ichilds_[0] == -1 ){
    Draw_Wire(n);
    return;
  }
  DrawThisAndChild_Wire(aNo[n.ichilds_[0]], aNo);
  DrawThisAndChild_Wire(aNo[n.ichilds_[1]], aNo);
  DrawThisAndChild_Wire(aNo[n.ichilds_[2]], aNo);
  DrawThisAndChild_Wire(aNo[n.ichilds_[3]], aNo);
  DrawThisAndChild_Wire(aNo[n.ichilds_[4]], aNo);
  DrawThisAndChild_Wire(aNo[n.ichilds_[5]], aNo);
  DrawThisAndChild_Wire(aNo[n.ichilds_[6]], aNo);
  DrawThisAndChild_Wire(aNo[n.ichilds_[7]], aNo);
}

void Draw(const delfem2::CADF3& adf)
{
    //    std::cout << "ADF" << aNode.size() << std::endl;
  const bool is_lighting = ::glIsEnabled(GL_LIGHTING);
  ::glDisable(GL_LIGHTING);
  if( !adf.aNode.empty() && adf.is_show_cage ){
    DrawThisAndChild_Wire(adf.aNode[0], adf.aNode);
  }
  
  if( adf.nIsoTri_ != 0 ){
    ::glColor3dv(adf.color_);
    ::glEnableClientState(GL_VERTEX_ARRAY);
    ::glVertexPointer(3 , GL_DOUBLE, 0 , adf.aIsoTri_);
    ::glDrawArrays(GL_TRIANGLES, 0 , adf.nIsoTri_*3);
    ::glDisableClientState(GL_VERTEX_ARRAY);
  }
  if( adf.aIsoEdge_ != 0 ){
    ::glColor3d(0,0,0);
    ::glEnableClientState(GL_VERTEX_ARRAY);
    ::glVertexPointer(3 , GL_DOUBLE, 0 , adf.aIsoEdge_);
    ::glDrawArrays(GL_LINES, 0 , adf.nIsoTri_*6);
    ::glDisableClientState(GL_VERTEX_ARRAY);
  }
  if( is_lighting ){ ::glEnable(GL_LIGHTING); }
}


// -----------------------

delfem2::CADF3 adf;
std::vector<unsigned int> aTri;
std::vector<double> aXYZ;

// ---------------

void SetProblem(int iprob)
{
  if( iprob == 0 )
  {
    class CInSphere : public delfem2::CInput_ADF3
    {
    public:
      virtual double sdf(double x, double y, double z) const {
        double n[3];
        return obj.Projection(n,
                              x, y, z);
      }
    public:
      delfem2::CSphere obj;
    };
    CInSphere sphere;
    sphere.obj.radius_ = 0.5;
//    sphere.sdf.GetMesh(aTri, aXYZ, 0.01);
    double bb[6] = { -1, 1, -1, 1, -1,1 };
    adf.SetUp(sphere, bb);
    adf.BuildIsoSurface_MarchingCube();
    adf.BuildMarchingCubeEdge();
  }
  else if( iprob == 1 ){
    class CInTorus : public delfem2::CInput_ADF3
    {
    public:
      double sdf(double x, double y, double z) const override {
        double n[3];
        return obj.Projection(n,
                              x, y, z);
      }
    public:
      delfem2::CTorus obj;
    };
    CInTorus torus;
    torus.obj.radius_ = 0.5;
    torus.obj.radius_tube_ = 0.2;
//    torus.sdf.GetMesh(aTri, aXYZ, 0.01);
    double bb[6] = { -1, 1, -1, 1, -1,1 };
    adf.SetUp(torus, bb);
    adf.BuildIsoSurface_MarchingCube();
    adf.BuildMarchingCubeEdge();
  }
  else if( iprob == 2 ){
    class CMesh : public delfem2::CInput_ADF3
    {
    public:
      double sdf(double x, double y, double z) const override {
        CVector3 n0;
        double sdf0 = obj.SignedDistanceFunction(n0,
                                                 CVector3(x,y,z),
                                                 aXYZ, aTri, aNorm);
        return sdf0;
      }
    public:
      std::vector<double> aNorm;
      dfm2::CBVH_MeshTri3D<dfm2::CBV3D_Sphere> obj;
    };
    CMesh mesh;
    {
      std::cout << PATH_INPUT_DIR << std::endl;
      delfem2::Read_Ply(std::string(PATH_INPUT_DIR)+"/bunny_1k.ply", aXYZ, aTri);
      delfem2::Normalize_Points3D(aXYZ,1.7);
      mesh.obj.Init(aXYZ.data(), aXYZ.size()/3,
                    aTri.data(), aTri.size()/3,
                    0.0);
      mesh.aNorm.resize(aXYZ.size());
      delfem2::Normal_MeshTri3D(mesh.aNorm.data(),
                                aXYZ.data(), aXYZ.size()/3, aTri.data(), aTri.size()/3);
    }
    double bb[6] = { -1, 1, -1, 1, -1,1 };
    adf.SetUp(mesh, bb);
    adf.BuildIsoSurface_MarchingCube();
    adf.BuildMarchingCubeEdge();
  }
}

// ------------------------------------------------

int main(int argc,char* argv[])
{
  delfem2::opengl::CViewer_GLFW viewer;
  viewer.Init_oldGL();
  
  viewer.nav.camera.view_height = 2.0;
  viewer.nav.camera.camera_rot_mode = delfem2::CAMERA_ROT_TBALL;
  
  delfem2::opengl::setSomeLighting();
  
  while(!glfwWindowShouldClose(viewer.window)){
    int iproblem  = 0;
    {
      static int iframe = 0;
      iproblem = iframe/1000;
      if( iframe % 1000  == 0 ){ SetProblem(iproblem); }
      iframe = (iframe+1)%3000;
    }
    // --------------------
    viewer.DrawBegin_oldGL();
    if( iproblem == 0 ){
      adf.SetShowCage(false);
      Draw(adf);
    }
    else if( iproblem == 1 ){
      adf.SetShowCage(true);
      Draw(adf);
    }
    else if( iproblem == 2 ){
//      opengl::DrawMeshTri3D_FaceNorm(aXYZ,aTri);
      Draw(adf);
    }
    viewer.DrawEnd_oldGL();
  }
  glfwDestroyWindow(viewer.window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
