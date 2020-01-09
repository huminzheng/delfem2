#include <stdio.h>
#include <vector>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#if defined(__APPLE__) && defined(__MACH__)
  #include <OpenGL/gl.h>
#elif defined(__MINGW32__) // probably I'm using Qt and don't want to use GLUT
  #include <GL/gl.h>
#elif defined(_WIN32) // windows
  #include <windows.h>
  #include <GL/gl.h>
#else
  #include <GL/gl.h>
#endif

#include "delfem2/opengl/gl_tex.h"

namespace py = pybind11;
namespace dfm2 = delfem2;

dfm2::opengl::CTexture GetTextureFromNumpy(const py::array_t<unsigned char>& a){
  assert(a.ndim()==3);
  assert(a.shape()[2] == 3);
  const int h = a.shape()[0];
  const int w = a.shape()[1];
  dfm2::opengl::CTexture tex(w,h,a.data(),"bgr");
  return tex;
}

void init_texture(py::module &m) {
  py::class_<dfm2::opengl::CTexture>(m, "Texture")
      .def(py::init<>())
      .def_readonly("width", &dfm2::opengl::CTexture::w)
      .def_readonly("height", &dfm2::opengl::CTexture::h)
      .def("draw", &dfm2::opengl::CTexture::Draw)
      .def("init_gl", &dfm2::opengl::CTexture::LoadTex)
      .def("minmax_xyz", &dfm2::opengl::CTexture::MinMaxXYZ)
      .def("set_minmax_xy", &dfm2::opengl::CTexture::SetMinMaxXY);


  m.def("get_texture", &GetTextureFromNumpy);
}
