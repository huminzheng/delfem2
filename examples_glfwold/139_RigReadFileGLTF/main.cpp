/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <vector>
#include <set>
#include "delfem2/rig_geo3.h"
#include "delfem2/tinygltf/io_gltf.h"
// ------------------
#include <GLFW/glfw3.h>
#include "delfem2/opengl/funcs_glold.h"
#include "delfem2/opengl/rigv3_glold.h"
#include "delfem2/opengl/glfw/viewer_glfw.h"

namespace dfm2 = delfem2;

// ---------------------------------

int main(int argc,char* argv[])
{
  std::vector<double> aXYZ0;
  std::vector<unsigned int> aTri;
  std::vector<double> aRigWeight;
  std::vector<unsigned int> aRigJoint;
  std::vector<double> aXYZ;
  std::vector<dfm2::CRigBone> aBone;
  {
//    std::string path_gltf = std::string(PATH_INPUT_DIR)+"/Duck.glb";
//    std::string path_glb = std::string(PATH_INPUT_DIR)+"/Monster.glb";
    
    //      std::string path_gltf = std::string(PATH_INPUT_DIR)+"/RiggedSimple.glb";
    //    std::string path_gltf = std::string(PATH_INPUT_DIR)+"/RiggedFigure.glb";
    std::string path_glb = std::string(PATH_INPUT_DIR)+"/CesiumMan.glb";
    dfm2::CGLTF gltf;
    gltf.Read(path_glb);
    gltf.Print();
    gltf.GetMeshInfo(aXYZ0, aTri, aRigWeight, aRigJoint, 0,0);
    gltf.GetBone(aBone, 0);
  }
  
  UpdateBoneRotTrans(aBone);
  aXYZ = aXYZ0;
  dfm2::Skinning_LBS_LocalWeight(aXYZ.data(),
                                 aXYZ0.data(), aXYZ0.size()/3,
                                 aTri.data(), aTri.size()/3,
                                 aBone, aRigWeight.data(), aRigJoint.data());

  // --------------
  // opengl starts here
  delfem2::opengl::CViewer_GLFW viewer;
  viewer.Init_oldGL();
  viewer.nav.camera.view_height = 2.0;
  viewer.nav.camera.camera_rot_mode = delfem2::CCamera<double>::CAMERA_ROT_MODE::TBALL;
  delfem2::opengl::setSomeLighting();
  
  while(!glfwWindowShouldClose(viewer.window)){
    // --------------------
    viewer.DrawBegin_oldGL();
    ::glEnable(GL_LIGHTING);
    delfem2::opengl::DrawMeshTri3D_FaceNorm(aXYZ.data(), aTri.data(), aTri.size()/3);
    delfem2::opengl::DrawAxis(1);    
    
    ::glDisable(GL_DEPTH_TEST);
    delfem2::opengl::DrawBone(aBone,
                              -1, -1,
                              0.01, 1.0);
    ::glEnable(GL_DEPTH_TEST);
    viewer.SwapBuffers();
    glfwPollEvents();
  }
  glfwDestroyWindow(viewer.window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}






/*
 int ibone_selected = -1;
 int ielem_bone_selected = -1;
 double rad_bone_sphere = 0.01;
 double rad_rot_hndlr = 1.0;
 
 bool is_animation = false;
 
 void myGlutMotion( int x, int y )
 {
 nav.glutMotion(x, y);
 if( nav.imodifier != 0 ) return;
 ////
 if( ibone_selected>=0 && ibone_selected<aBone.size() ){
 nav.SetGL_Camera();
 float mMV[16]; glGetFloatv(GL_MODELVIEW_MATRIX, mMV);
 float mPj[16]; glGetFloatv(GL_PROJECTION_MATRIX, mPj);
 CVector2 sp1(nav.mouse_x, nav.mouse_y);
 CVector2 sp0(nav.mouse_x-nav.dx, nav.mouse_y-nav.dy);
 CRigBone& bone = aBone[ibone_selected];
 DragHandlerRot_Mat4(bone.rot,
 ielem_bone_selected, sp0, sp1, bone.Mat,
 mMV, mPj);
 UpdateBoneRotTrans(aBone);
 UpdateRigSkin(aXYZ.data(),
 aXYZ0.data(), aXYZ.size()/3,
 aTri.data(), aTri.size()/3,
 aBone, aRigWeight.data(), aRigJoint.data());
 }
 ////
 ::glutPostRedisplay();
 }
 
 void myGlutMouse(int button, int state, int x, int y)
 {
 nav.glutMouse(button, state, x, y);
 /////
 nav.SetGL_Camera();
 float mMV[16]; glGetFloatv(GL_MODELVIEW_MATRIX, mMV);
 float mPj[16]; glGetFloatv(GL_PROJECTION_MATRIX, mPj);
 CVector3 src = screenUnProjection(CVector3(nav.mouse_x,nav.mouse_y,0.0), mMV, mPj);
 CVector3 dir = screenDepthDirection(src,mMV,mPj);
 if( state == GLUT_DOWN ){
 const double wh = 1.0/mPj[5];
 std::cout << wh << std::endl;
 PickBone(ibone_selected, ielem_bone_selected,
 aBone,
 src,dir,
 rad_rot_hndlr,
 wh*0.05);
 std::cout << ibone_selected << std::endl;
 }
 /////
 ::glutPostRedisplay();
 }
 */

