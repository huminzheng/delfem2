/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cstdlib>
#include <cstdio>
#include <cassert>

#include <GLFW/glfw3.h>
#include "delfem2/opengl/glfw/viewer_glfw.h"

// ---------------

static delfem2::opengl::CViewer_GLFW* pViewer = 0;

// ---------------

static void callback_error(int error, const char* description)
{
  fputs(description, stderr);
}

static void glfw_callback_error(int error, const char* description)
{
  fputs(description, stderr);
}

static void glfw_callback_key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
    glfwSetWindowShouldClose(window, GL_TRUE);
  }
  if(      action == GLFW_PRESS ){   pViewer->key_press(key,mods);   }
  else if( action == GLFW_RELEASE ){ pViewer->key_release(key,mods); }
}

static void glfw_callback_resize(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);
}

static void glfw_callback_mouse_button(GLFWwindow* window, int button, int action, int mods)
{
  assert( pViewer != 0 );
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  const float asp = width / (float) height;
  {
    ::delfem2::CNav3& nav = pViewer->nav;
    nav.imodifier = mods;
    double x, y;  glfwGetCursorPos (window, &x,&y);
    nav.mouse_x = (2.0*x-width)/width;
    nav.mouse_y = (height-2.0*y)/height;
    if( action == 0 ){
      nav.ibutton = -1;
    }
    else if( action == 1 ){ // mouse down
      nav.ibutton = button;
      nav.mouse_x_down = nav.mouse_x;
      nav.mouse_y_down = nav.mouse_y;
    }
  }
  if( action == GLFW_PRESS ){
    float src[3], dir[3];
    float mMVP[16];
    {
      float mMV[16], mP[16];
      pViewer->camera.Mat4_MVP_OpenGL(mMV, mP, asp);
      ::delfem2::MatMat4(mMVP, mMV,mP);
    }
    pViewer->nav.MouseRay(src, dir, asp, mMVP);
    pViewer->mouse_press(src,dir);
  }
}

static void glfw_callback_cursor_position(GLFWwindow* window, double xpos, double ypos)
{
  assert( pViewer != 0 );
//  pViewer->nav.Motion(window,xpos,ypos);
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  const float asp = width / (float) height;
  {
    ::delfem2::CNav3& nav = pViewer->nav;
    ::delfem2::CCam3_OnAxisZplusLookOrigin<double>& camera = pViewer->camera;
    const double mov_end_x = (2.0*xpos-width)/width;
    const double mov_end_y = (height-2.0*ypos)/height;
    nav.dx = mov_end_x - nav.mouse_x;
    nav.dy = mov_end_y - nav.mouse_y;
    if( nav.ibutton == -1 ){
    }
    else{
      if(      nav.imodifier == 4  ){
        camera.Rot_Camera(nav.dx, nav.dy);
      }
      else if( nav.imodifier == 1 ){
        camera.Pan_Camera(nav.dx, nav.dy);
      }
    }
    nav.mouse_x = mov_end_x;
    nav.mouse_y = mov_end_y;
  }
  if( pViewer->nav.ibutton == 0 ){
    float src0[3], src1[3], dir0[3], dir1[3];
    float mMVP[16];
    {
      float mMV[16], mP[16];
      pViewer->camera.Mat4_MVP_OpenGL(mMV, mP, asp);
      ::delfem2::MatMat4(mMVP, mMV,mP);
    }
    pViewer->nav.RayMouseMove(src0, src1, dir0, dir1, asp,mMVP);
    pViewer->mouse_drag(src0,src1,dir0);
  }
}

static void glfw_callback_scroll(GLFWwindow* window, double xoffset, double yoffset)
{
  assert( pViewer != nullptr );
  pViewer->camera.scale *= pow(1.01,yoffset);
}


void delfem2::opengl::CViewer_GLFW::Init_oldGL()
{
  pViewer = this;
  // -----
  glfwSetErrorCallback(glfw_callback_error);
  if (!glfwInit()){
    exit(EXIT_FAILURE);
  }
  // shader version #120
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  // -------
  window = glfwCreateWindow(width, height, "Simple example", nullptr, nullptr);
  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, glfw_callback_resize);
  glfwSetKeyCallback(            window, glfw_callback_key);
  glfwSetMouseButtonCallback(    window, glfw_callback_mouse_button);
  glfwSetCursorPosCallback(      window, glfw_callback_cursor_position);
  glfwSetScrollCallback(         window, glfw_callback_scroll);
}


void delfem2::opengl::CViewer_GLFW::Init_newGL()
{
  pViewer = this;
  // -----
  glfwSetErrorCallback(callback_error);
  if (!glfwInit()){
    exit(EXIT_FAILURE);
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif
  
  /*
   // Decide GL+GLSL versions
   #if __APPLE__
   // GL 3.2 + GLSL 150
   const char *glsl_version = "#version 150";
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);       // Required on Mac
   #else
   // GL 3.0 + GLSL 130
   const char *glsl_version = "#version 130";
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
   //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
   //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
   #endif
   */
  
    // glfw window creation
    // --------------------
  this->window = glfwCreateWindow(width,
                                  height,
                                  "LearnOpenGL",
                                  nullptr,
                                  nullptr);
  if (this->window == nullptr)
  {
    glfwTerminate();
  }
  
  glfwMakeContextCurrent(this->window);
  glfwSetFramebufferSizeCallback(this->window, glfw_callback_resize);
  glfwSetKeyCallback(            this->window, glfw_callback_key);
  glfwSetMouseButtonCallback(    this->window, glfw_callback_mouse_button);
  glfwSetCursorPosCallback(      this->window, glfw_callback_cursor_position);
  glfwSetScrollCallback(         this->window, glfw_callback_scroll);
}


void delfem2::opengl::CViewer_GLFW::DrawBegin_oldGL() const
{
  ::glfwMakeContextCurrent(window);
  //::glClearColor(0.8f, 1.0f, 1.0f, 1.0f);
  ::glClearColor(
      this->bgcolor[0],
      this->bgcolor[1],
      this->bgcolor[2],
      this->bgcolor[3]);
  ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ::glEnable(GL_DEPTH_TEST);
  ::glDepthFunc(GL_LESS);
  ::glEnable(GL_POLYGON_OFFSET_FILL );
  ::glPolygonOffset( 1.1f, 4.0f );

  // glnew will skip compilling following section
#ifdef GL_PROJECTION

  { // make sure that the stack is clear
    int n0; ::glGetIntegerv(GL_MODELVIEW_STACK_DEPTH,&n0);
    int n1; ::glGetIntegerv(GL_PROJECTION_STACK_DEPTH,&n1);
    assert( n0 == 1 && n1 == 1 );
  }

  float mMV[16], mP[16];
  {
    int width0, height0;
    glfwGetFramebufferSize(window, &width0, &height0);
    float asp = width0 / (float) height0;
    camera.Mat4_MVP_OpenGL(mMV,mP, asp);
  }

  ::glEnable(GL_NORMALIZE); // GL_NORMALIZE is not defiend on the modern OpenGLae
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMultMatrixf(mP);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glMultMatrixf(mMV);
#endif
  
}

void delfem2::opengl::CViewer_GLFW::SwapBuffers() const
{
  glfwSwapBuffers(this->window);
}

void delfem2::opengl::CViewer_GLFW::ExitIfClosed() const
{
  if ( !glfwWindowShouldClose(this->window) ) { return; }
  glfwDestroyWindow(this->window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

