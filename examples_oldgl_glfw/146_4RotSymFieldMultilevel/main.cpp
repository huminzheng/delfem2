/*
* Copyright (c) 2019 Nobuyuki Umetani
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

/**
 * @brief implementation of 4 rotatoinal symetry field
 * @details implementation is based on "Wenzel Jakob, Marco Tarini, Daniele Panozzo, and Olga Sorkine-Hornung. Instant field-aligned meshes. Siggraph Asia 2015"
 */

#include "delfem2/opengl/glfw/viewer_glfw.h"
#include "delfem2/opengl/old/funcs.h"
#include "delfem2/opengl/old/mshuni.h"
#include "delfem2/opengl/old/v3q.h"
#include "delfem2/mshio.h"
#include "delfem2/mshmisc.h"
#include "delfem2/points.h"
#include "delfem2/mshuni.h"
#include "delfem2/vec3.h"
#include "delfem2/4rotsym.h"
#include "delfem2/clusterpoints.h"
#include <GLFW/glfw3.h>
#include <cstdlib>

namespace dfm2 = delfem2;

// ---------------

void Draw(
    const std::vector<double>& aXYZ,
    const std::vector<double>& aNorm,
    const std::vector<double>& aOdir,
    const std::vector<unsigned int>& aTri)
{
  ::glEnable(GL_LIGHTING);
  dfm2::opengl::DrawMeshTri3D_FaceNorm(aXYZ.data(), aTri.data(), aTri.size()/3);
  ::glDisable(GL_LIGHTING);
  double len = 0.03;
  ::glLineWidth(3);
  unsigned int np = aXYZ.size()/3;
  for(unsigned int ip=0;ip<np;++ip){
    const dfm2::CVec3d p = dfm2::CVec3d(aXYZ.data()+ip*3);
    const dfm2::CVec3d n = dfm2::CVec3d(aNorm.data()+ip*3).Normalize();
    const dfm2::CVec3d o = dfm2::CVec3d(aOdir.data()+ip*3).Normalize();
    const dfm2::CVec3d q = dfm2::Cross(n,o);
    ::glBegin(GL_LINES);
//    ::glColor3d(0,0,0);
//    dfm2::opengl::myGlVertex(p);
//    dfm2::opengl::myGlVertex(p+len*n);
    ::glColor3d(0,0,1);
    dfm2::opengl::myGlVertex(p-len*o);
    dfm2::opengl::myGlVertex(p);
    ::glColor3d(1,0,0);
    dfm2::opengl::myGlVertex(p);
    dfm2::opengl::myGlVertex(p+len*o);
    dfm2::opengl::myGlVertex(p-len*q);
    dfm2::opengl::myGlVertex(p+len*q);
    ::glEnd();
  }
}

int main()
{
  class CClusterData
  {
  public:
    std::vector<double> aXYZ; // center position of the cluster
    std::vector<double> aArea; // area of the cluster
    std::vector<double> aNorm; // normal of the cluster
    std::vector<double> aOdir; // director vector
    std::vector<unsigned int> psup_ind, psup; // connectivity of the cluster
    std::vector<unsigned int> map_fine2this; // index of cluster for mesh points.
    // below: data for visualization
//    std::vector<float> aColor; // color of the cluster
//    std::vector<unsigned int> map0c; // index of cluster for mesh points.
  };

  std::vector<CClusterData> aLayer;

  std::vector<unsigned int> aTri;
  { // load level 0
    aLayer.resize(1);
    delfem2::Read_Ply(std::string(PATH_INPUT_DIR) + "/bunny_1k.ply",
                      aLayer[0].aXYZ, aTri);
    delfem2::Normalize_Points3(aLayer[0].aXYZ);
    aLayer[0].aNorm.resize(aLayer[0].aXYZ.size());
    dfm2::Normal_MeshTri3D(aLayer[0].aNorm.data(),
                           aLayer[0].aXYZ.data(), aLayer[0].aXYZ.size() / 3,
                           aTri.data(), aTri.size() / 3);
    {
      const double minCoords[3] = {-1, -1, -1};
      const double maxCoords[3] = {+1, +1, +1};
      aLayer[0].aOdir.resize(aLayer[0].aXYZ.size());
      dfm2::Points_RandomUniform(aLayer[0].aOdir.data(),
                                 aLayer[0].aXYZ.size() / 3, 3, minCoords, maxCoords);
      dfm2::TangentVector_Points3(aLayer[0].aOdir,
                            aLayer[0].aNorm);
    }
    aLayer[0].aArea.resize(aLayer[0].aXYZ.size() / 3);
    dfm2::MassPoint_Tri3D(aLayer[0].aArea.data(),
                          1.0,
                          aLayer[0].aXYZ.data(), aLayer[0].aXYZ.size() / 3,
                          aTri.data(), aTri.size() / 3);
    dfm2::JArray_PSuP_MeshElem(aLayer[0].psup_ind, aLayer[0].psup,
                               aTri.data(), aTri.size()/3, 3,
                               aLayer[0].aXYZ.size()/3);
  }
  for(unsigned int itr=0;itr<8;++itr) {
    aLayer.resize(aLayer.size() + 1);
    const CClusterData &pd0 = aLayer[itr];
    CClusterData &pd1 = aLayer[itr + 1];
    dfm2::BinaryClustering_Points3d(
        pd1.aXYZ, pd1.aArea, pd1.aNorm, pd1.map_fine2this,
        pd0.aXYZ, pd0.aArea, pd0.aNorm, pd0.psup_ind, pd0.psup);
    dfm2::Clustering_Psup(pd1.psup_ind, pd1.psup,
                          pd1.aXYZ.size() / 3,
                          pd0.aXYZ.size() / 3,
                          pd1.map_fine2this.data(), pd0.psup_ind.data(), pd0.psup.data());
    /*
    const unsigned int np0 = aLayer[0].aXYZ.size()/3;
    pd1.map0c.resize(np0,UINT_MAX);
    for(unsigned int ip=0;ip<np0;++ip){
      unsigned int ic0 = pd0.map0c[ip];
      assert( ic0 < map01.size() );
      pd1.map0c[ip] = map01[ic0];
      assert( pd1.map0c[ip] < pd1.aXYZ.size()/3 );
    }
    */
  }

  const unsigned int nlayer = aLayer.size();
  for(int ilayer=(int)aLayer.size()-1;ilayer>=0;--ilayer){
    if( ilayer == (int)nlayer-1 ){
      const double minCoords[3] = {-1, -1, -1};
      const double maxCoords[3] = {+1, +1, +1};
      aLayer[ilayer].aOdir.resize(aLayer[ilayer].aXYZ.size());
      dfm2::Points_RandomUniform(aLayer[ilayer].aOdir.data(),
                                 aLayer[ilayer].aXYZ.size() / 3, 3, minCoords, maxCoords);
      dfm2::TangentVector_Points3(aLayer[ilayer].aOdir,
          aLayer[ilayer].aNorm);
    }
    else{
      unsigned int np0 = aLayer[ilayer].aXYZ.size()/3; // this
      assert( aLayer[ilayer+1].map_fine2this.size() == np0 );
      unsigned int np1 = aLayer[ilayer+1].aXYZ.size()/3; // corse
      aLayer[ilayer].aOdir.resize(np0*3);
      for(unsigned int ip0=0;ip0<np0;++ip0){
        unsigned int ip1 = aLayer[ilayer+1].map_fine2this[ip0];
        assert( ip1 < np1 );
        aLayer[ilayer].aOdir[ip0*3+0] = aLayer[ilayer+1].aOdir[ip1*3+0];
        aLayer[ilayer].aOdir[ip0*3+1] = aLayer[ilayer+1].aOdir[ip1*3+1];
        aLayer[ilayer].aOdir[ip0*3+2] = aLayer[ilayer+1].aOdir[ip1*3+2];
      }
    }
    dfm2::Smooth4RotSym(aLayer[ilayer].aOdir,
                        aLayer[ilayer].aNorm, aLayer[ilayer].psup_ind, aLayer[ilayer].psup);
  }



  // ------------------
  delfem2::opengl::CViewer_GLFW viewer;
  viewer.Init_oldGL();
  viewer.camera.camera_rot_mode = dfm2::CCam3_OnAxisZplusLookOrigin<double>::CAMERA_ROT_MODE::TBALL;
  dfm2::opengl::setSomeLighting();
  unsigned int iframe = 0;
  while (true)
  {
    /*
    if( iframe == 0 ){
//        dfm2::InitializeTangentField(aLayer[0].aOdir,aLayer[0].aNorm);
    }
    if( iframe > 30 ){
//      dfm2::Smooth4RotSym(aLayer[0].aOdir,
//                          aLayer[0].aNorm, aLayer[0].psup_ind, aLayer[0].psup);
    }
     */
    // --------------------

    viewer.DrawBegin_oldGL();
    Draw(aLayer[0].aXYZ,aLayer[0].aNorm,aLayer[0].aOdir,aTri);
/*
    ::glColor3d(0,0,0);
    ::glLineWidth(2);
    dfm2::opengl::DrawMeshTri3D_Edge(aLayer[0].aXYZ.data(),aLayer[0].aXYZ.size()/3,aTri.data(),aTri.size()/3);
    dfm2::opengl::DrawMeshTri3D_FaceNorm(aLayer[0].aXYZ.data(),aTri.data(),aTri.size()/3);
    */
    glfwSwapBuffers(viewer.window);
    glfwPollEvents();
    iframe = (iframe+1)%60;
    if( glfwWindowShouldClose(viewer.window) ){ goto EXIT; }
  }
EXIT:
  glfwDestroyWindow(viewer.window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
