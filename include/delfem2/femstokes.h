/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef DFM2_FEMSTOKES_H
#define DFM2_FEMSTOKES_H

#include "delfem2/dfm2_inline.h"
#include "delfem2/femutil.h"
#include <vector>

namespace delfem2 {


void EMat_Stokes2D_Static_P1(
    double alpha, double g_x, double g_y,
    const double coords[][2],
    const double velo_press[3][3],
    double emat[3][3][3][3],
    double eres[3][3]);

void MakeMat_Stokes2D_Static_P1P1(
    double alpha,
    double g_x,
    double g_y,
    const double coords[][2],
    const double velo[3][2],
    const double press[3],
    double emat_uu[][3][2][2],
    double emat_up[][3][2],
    double emat_pu[][3][2],
    double emat_pp[][3],
    double eres_u[][2],
    double eres_p[3]);

void EMat_Stokes2D_Dynamic_P1(
    double alpha,
    double rho,
    double g_x,
    double g_y,
    const double dt_timestep,
    const double gamma_newmark,
    const double coords[][2],
    const double velo_press[3][3],
    const double acc_apress[3][3],
    double emat[3][3][3][3],
    double eres[3][3]);

void MakeMat_Stokes3D_Static_P1(
    double alpha, double g_x, double g_y, double g_z,
    const double coords[4][3],
    const double velo_press[4][4],
    double emat[4][4][4][4],
    double eres[4][4]);

void MakeMat_Stokes3D_Static_P1P1(
    double alpha, double g_x, double g_y, double g_z,
    const double coords[4][3],
    const double velo[4][3],
    const double press[4],
    double emat_uu[4][4][3][3],
    double emat_up[4][4][3],
    double emat_pu[4][4][3],
    double emat_pp[4][4],
    double eres_u[4][3], double eres_p[4]);

void MakeMat_Stokes3D_Dynamic_P1(
    double alpha, double rho, double g_x, double g_y, double g_z,
    const double dt_timestep, const double gamma_newmark,
    const double coords[4][3],
    const double velo_press[4][4], const double acc_apress[4][4],
    double emat[4][4][4][4],
    double eres[4][4]);

void MakeMat_Stokes3D_Dynamic_Newmark_P1P1(
    double alpha, double rho, double g_x, double g_y, double g_z,
    const double dt_timestep, const double gamma_newmark,
    const double coords[4][3],
    const double velo[4][3],
    const double press[4],
    const double acc[4][3],
    const double apress[4],
    double emat_uu[4][4][3][3], double emat_up[4][4][3], double emat_pu[4][4][3], double emat_pp[4][4],
    double eres_u[4][3], double eres_p[4]);

template <class MAT>
void MergeLinSys_StokesStatic2D(
    MAT& mat_A,
    double* vec_b,
    const double myu,
    const double g_x,
    const double g_y,
    const double* aXY1,
    unsigned int nXY,
    const unsigned int* aTri1,
    unsigned int nTri,
    const double* aVal)
{
  namespace lcl = ::delfem2::femutil;
  const unsigned int np = nXY;
//  const int nDoF = np*3;
  ////
//  mat_A.SetZero();
//  for(int idof=0;idof<nDoF;++idof){ vec_b[idof] = 0.0; }
  std::vector<int> tmp_buffer(np, -1);
  for (unsigned int iel = 0; iel<nTri; ++iel){
    const unsigned int i0 = aTri1[iel*3+0];
    const unsigned int i1 = aTri1[iel*3+1];
    const unsigned int i2 = aTri1[iel*3+2];
    const unsigned int aIP[3] = {i0,i1,i2};
    double coords[3][2]; FetchData(&coords[0][0],3,2,aIP, aXY1);
    double velo_press[3][3]; FetchData(&velo_press[0][0],3,3,aIP, aVal);
    ////
    double eres[3][3];
    double emat[3][3][3][3];
    ////
    EMat_Stokes2D_Static_P1(myu, g_x, g_y, coords, velo_press, emat, eres);
    for (int ino = 0; ino<3; ino++){
      const unsigned int ip = aIP[ino];
      vec_b[ip*3+0] += eres[ino][0];
      vec_b[ip*3+1] += eres[ino][1];
      vec_b[ip*3+2] += eres[ino][2];
    }
    // marge dde
    mat_A.Mearge(3, aIP, 3, aIP, 9, &emat[0][0][0][0], tmp_buffer);
  }
}

template <class MAT>
void MergeLinSys_StokesDynamic2D(
    MAT& mat_A,
    double* vec_b,
    const double myu,
    const double rho,
    const double g_x,
    const double g_y,
    const double dt_timestep,
    const double gamma_newmark,
    const double* aXY1,
    unsigned int nXY,
    const unsigned int* aTri1,
    unsigned int nTri,
    const double* aVal,
    const double* aVelo)
{
  const unsigned int np = nXY;
  std::vector<int> tmp_buffer(np, -1);
  for (unsigned int iel = 0; iel<nTri; ++iel){
    const unsigned int i0 = aTri1[iel*3+0];
    const unsigned int i1 = aTri1[iel*3+1];
    const unsigned int i2 = aTri1[iel*3+2];
    const unsigned int aIP[3] = {i0,i1,i2};
    double coords[3][2]; FetchData(&coords[0][0],3,2,aIP, aXY1);
    double velo_press[3][3]; FetchData(&velo_press[0][0],3,3,aIP, aVal);
    double acc_apress[3][3]; FetchData(&acc_apress[0][0],3,3,aIP, aVelo);
    //
    double eres[3][3];
    double emat[3][3][3][3];
    //
    EMat_Stokes2D_Dynamic_P1(myu, rho,  g_x, g_y,
                                dt_timestep, gamma_newmark,
                                coords, velo_press, acc_apress,
                                emat, eres);
    for (int ino = 0; ino<3; ino++){
      const unsigned int ip = aIP[ino];
      vec_b[ip*3+0] += eres[ino][0];
      vec_b[ip*3+1] += eres[ino][1];
      vec_b[ip*3+2] += eres[ino][2];
    }
    mat_A.Mearge(3, aIP, 3, aIP, 9, &emat[0][0][0][0], tmp_buffer);
  }
}

template <class MAT>
void MergeLinSys_Stokes3D_Static(
    MAT& mat_A,
    std::vector<double>& vec_b,
    const double myu,
    const double rho,
    const double g_x,
    const double g_y,
    const double g_z,
    const std::vector<double>& aXYZ,
    const std::vector<unsigned int>& aTet,
    const std::vector<double>& aVal,
    const std::vector<double>& aVelo)
{
  const int np = (int)aXYZ.size()/3;
  const int nDoF = np*4;
  //
  mat_A.SetZero();
  vec_b.assign(nDoF, 0.0);
  std::vector<int> tmp_buffer(np, -1);
  for (int itet = 0; itet<(int)aTet.size()/4; ++itet){
    const unsigned int i0 = aTet[itet*4+0];
    const unsigned int i1 = aTet[itet*4+1];
    const unsigned int i2 = aTet[itet*4+2];
    const unsigned int i3 = aTet[itet*4+3];
    const unsigned int aIP[4] = {i0,i1,i2,i3};
    double coords[4][3]; FetchData(&coords[0][0],4,3,aIP, aXYZ.data());
    double velo_press[4][4]; FetchData(&velo_press[0][0],4,4,aIP, aVal.data());
    double eres[4][4];
    double emat[4][4][4][4];
    MakeMat_Stokes3D_Static_P1(myu, g_x, g_y, g_z,
                               coords, velo_press,
                               emat, eres);
    for (int ino = 0; ino<4; ino++){
      const unsigned int ip = aIP[ino];
      vec_b[ip*4+0] += eres[ino][0];
      vec_b[ip*4+1] += eres[ino][1];
      vec_b[ip*4+2] += eres[ino][2];
      vec_b[ip*4+3] += eres[ino][3];
    }
    // marge dde
    mat_A.Mearge(4, aIP, 4, aIP,16, &emat[0][0][0][0], tmp_buffer);
  }
}

template <class MAT>
void MergeLinSys_Stokes3D_Dynamic(
    MAT& mat_A,
    std::vector<double>& vec_b,
    const double myu,
    const double rho,
    const double g_x,
    const double g_y,
    const double g_z,
    const double dt_timestep,
    const double gamma_newmark,
    const std::vector<double>& aXYZ,
    const std::vector<unsigned int>& aTet,
    const std::vector<double>& aVal,
    const std::vector<double>& aVelo)
{
  const int np = (int)aXYZ.size()/3;
  const int nDoF = np*4;
  //
  mat_A.SetZero();
  vec_b.assign(nDoF, 0.0);
  std::vector<int> tmp_buffer(np, -1);
  for (int iel = 0; iel<(int)aTet.size()/4; ++iel){
    const unsigned int i0 = aTet[iel*4+0];
    const unsigned int i1 = aTet[iel*4+1];
    const unsigned int i2 = aTet[iel*4+2];
    const unsigned int i3 = aTet[iel*4+3];
    const unsigned int aIP[4] = {i0,i1,i2,i3};
    double coords[4][3]; FetchData(&coords[0][0],4,3,aIP, aXYZ.data());
    double velo_press[4][4]; FetchData(&velo_press[0][0],4,4,aIP, aVal.data());
    double acc_apress[4][4]; FetchData(&acc_apress[0][0],4,4,aIP, aVelo.data());
    ////
    double eres[4][4];
    double emat[4][4][4][4];
    MakeMat_Stokes3D_Dynamic_P1(myu, rho,  g_x, g_y,g_z,
                                dt_timestep, gamma_newmark,
                                coords, velo_press, acc_apress,
                                emat, eres);
    for (int ino = 0; ino<4; ino++){
      const unsigned int ip = aIP[ino];
      vec_b[ip*4+0] += eres[ino][0];
      vec_b[ip*4+1] += eres[ino][1];
      vec_b[ip*4+2] += eres[ino][2];
      vec_b[ip*4+3] += eres[ino][3];
    }
    mat_A.Mearge(4, aIP, 4, aIP,16, &emat[0][0][0][0], tmp_buffer);
  }
}

} // namespace delfem2

#ifdef DFM2_HEADER_ONLY
#  include "delfem2/femstokes.cpp"
#endif
  
#endif /* fem_ematrix_h */