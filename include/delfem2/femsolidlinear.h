/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**
 * @file make element matrix (EMAT) and merge it to the global matri for linear solid equation
 *
 * (2020/12/26) TODO: use template to generalize the merge functions
 * (2020/12/25) created. separated from "femem3" and "femem2"
 */

#ifndef DFM2_FEMSOLIDLINEAR_H
#define DFM2_FEMSOLIDLINEAR_H

#include "delfem2/dfm2_inline.h"
#include "delfem2/femutil.h"
#include <vector>
#include <cassert>

namespace delfem2 {

DFM2_INLINE void ddW_SolidLinear_Tet3D(
    double* eKmat,
    double lambda, double myu,
    double vol, double dldx[4][3],
    bool is_add,
    unsigned int nstride = 3);

void EMat_SolidLinear2_QuadOrth_GaussInt(
    double emat[4][4][2][2],
    double lx,
    double ly,
    double myu,
    double lambda,
    unsigned int ngauss);

DFM2_INLINE void EMat_SolidLinear_Static_Tet(
    double emat[4][4][3][3],
    double eres[4][3],
    const double myu,
    const double lambda,
    const double coords[4][3],
    const double disp[4][3],
    bool is_add);

DFM2_INLINE void stress_LinearSolid_TetP2 (
  double stress[3][3],
  const double l0,
  const double l1,
  const double l2,
  const double l3,
  const double vol,
  const double lambda,
  const double myu,
  const double g_x,
  const double g_y,
  const double g_z,
  const double dldx[4][3],
  const double disp[10][3]);

void MakeMat_LinearSolid3D_Static_Q1(const double myu, const double lambda,
    const double rho,
    const double g_x,
    const double g_y,
    const double g_z,
    const double coords[8][3],
    const double disp[8][3],
    //
    double emat[8][8][3][3],
    double eres[8][3]);

DFM2_INLINE void matRes_LinearSolid_TetP2(
    double emat[10][10][3][3],
    double eres[10][3],
    const double vol,
    const double lambda,
    const double myu,
    const double g_x,
    const double g_y,
    const double g_z,
    const double rho,
    const double dldx[4][3],
    const double disp[10][3]);

void EMat_SolidLinear_NewmarkBeta_MeshTet3D(
    double eres[4][3],
    double emat[4][4][3][3],
    const double myu, const double lambda,
    const double rho, const double g_x, const double g_y, const double g_z,
    const double dt, const double gamma_newmark,  const double beta_newmark,
    const double disp[4][3], const double velo[4][3], const double acc[4][3],
    const double coords[4][3],
    bool is_initial);

DFM2_INLINE void EMat_SolidDynamicLinear_Tri2D(
    double eres[3][2],
    double emat[3][3][2][2],
    const double myu,
    const double lambda,
    const double rho,
    const double g_x,
    const double g_y,
    const double dt_timestep,
    const double gamma_newmark,
    const double beta_newmark,
    const double disp[3][2],
    const double velo[3][2],
    const double acc[3][2],
    const double coords[3][2],
    bool is_initial);

DFM2_INLINE void EMat_SolidStaticLinear_Tri2D(
    double eres[3][2],
    double emat[3][3][2][2],
    const double myu,
    const double lambda,
    const double rho,
    const double g_x,
    const double g_y,
    const double disp[3][2],
    const double coords[3][2]);



// ------------------------------------

template <class MAT>
void MergeLinSys_SolidLinear_Static_MeshTri2D(
    MAT& mat_A,
    double* vec_b,
    const double myu,
    const double lambda,
    const double rho,
    const double g_x,
    const double g_y,
    const double* aXY1,
    unsigned int nXY,
    const unsigned int* aTri1,
    unsigned int nTri,
    const double* aVal)
{
  const unsigned int np = nXY;
  std::vector<unsigned int> tmp_buffer(np, -1);
  for(unsigned int iel=0; iel<nTri; ++iel){
    const unsigned int aIP[3] = {aTri1[iel*3+0], aTri1[iel*3+1], aTri1[iel*3+2]};
    double coords[3][2]; FetchData<3,2>(coords,aIP, aXY1);
    double disps[3][2]; FetchData<3,2>(disps,aIP, aVal);
    //
    double eres[3][2], emat[3][3][2][2];
    EMat_SolidStaticLinear_Tri2D(
        eres,emat,
        myu, lambda, rho, g_x, g_y,
        disps, coords);
    for (int ino = 0; ino<3; ino++){
      const unsigned int ip = aIP[ino];
      vec_b[ip*2+0] += eres[ino][0];
      vec_b[ip*2+1] += eres[ino][1];
    }
//    mat_A.Mearge(3, aIP, 3, aIP, 4, &emat[0][0][0][0], tmp_buffer);
    Merge<3,3,2,2,double>(mat_A,aIP,aIP,emat,tmp_buffer);
  }
}


template <class MAT>
void MergeLinSys_SolidLinear_Static_MeshTet3D(
    MAT& mat_A,
    double* vec_b,
    const double myu,
    const double lambda,
    const double rho,
    const double *g,
    const double* aXYZ,
    unsigned int nXYZ,
    const unsigned int* aTet,
    unsigned int nTet,
    const double* aDisp)
{
  const unsigned int np = nXYZ;
  std::vector<unsigned int> tmp_buffer(np, UINT_MAX);
  for (unsigned int iel = 0; iel<nTet; ++iel){
    const unsigned int i0 = aTet[iel*4+0];
    const unsigned int i1 = aTet[iel*4+1];
    const unsigned int i2 = aTet[iel*4+2];
    const unsigned int i3 = aTet[iel*4+3];
    const unsigned int aIP[4] = { i0, i1, i2, i3 };
    double P[4][3];  FetchData<4,3>(P, aIP, aXYZ);
    double disps[4][3]; FetchData<4,3>(disps, aIP, aDisp);
    //
    double emat[4][4][3][3];
    for(int i=0;i<144;++i){ (&emat[0][0][0][0])[i] = 0.0; } // zero-clear
    double eres[4][3];
    {
      const double vol = femutil::TetVolume3D(P[0],P[1],P[2],P[3]);
      for(auto & ere : eres){
        ere[0] = vol*rho*g[0]*0.25;
        ere[1] = vol*rho*g[1]*0.25;
        ere[2] = vol*rho*g[2]*0.25;
      }
    }
    EMat_SolidLinear_Static_Tet(
        emat,eres,
        myu, lambda,
        P, disps,
        true); // additive
    for (int ino = 0; ino<4; ino++){
      const unsigned int ip = aIP[ino];
      vec_b[ip*3+0] += eres[ino][0];
      vec_b[ip*3+1] += eres[ino][1];
      vec_b[ip*3+2] += eres[ino][2];
    }
//    mat_A.Mearge(4, aIP, 4, aIP, 9, &emat[0][0][0][0], tmp_buffer);
    Merge<4,4,3,3,double>(mat_A,aIP,aIP,emat,tmp_buffer);
  }
}

template <class MAT>
void MergeLinSys_LinearSolid3D_Static_Q1(
    MAT& mat_A,
    std::vector<double>& vec_b,
    const double myu,
    const double lambda,
    const double rho,
    const double g_x,
    const double g_y,
    const double g_z,
    const std::vector<double>& aXYZ,
    const std::vector<int>& aHex,
    const std::vector<double>& aVal)
{
  const int np = (int)aXYZ.size()/3;
  const int nDoF = np*3;
  //
  mat_A.SetZero();
  vec_b.assign(nDoF, 0.0);
  std::vector<unsigned int> tmp_buffer(np, UINT_MAX);
  for (int iel = 0; iel<(int)aHex.size()/8; ++iel){
    const unsigned int i0 = aHex[iel*8+0];
    const unsigned int i1 = aHex[iel*8+1];
    const unsigned int i2 = aHex[iel*8+2];
    const unsigned int i3 = aHex[iel*8+3];
    const unsigned int i4 = aHex[iel*8+4];
    const unsigned int i5 = aHex[iel*8+5];
    const unsigned int i6 = aHex[iel*8+6];
    const unsigned int i7 = aHex[iel*8+7];
    const unsigned int aIP[8] = { i0, i1, i2, i3, i4, i5, i6, i7 };
    double coords[8][3]; FetchData<8,3>(coords, aIP, aXYZ.data());
    double disps[8][3]; FetchData<8,3>(disps, aIP, aVal.data());
    //
    double eres[8][3];
    double emat[8][8][3][3];
    MakeMat_LinearSolid3D_Static_Q1(myu, lambda,
                                    rho, g_x, g_y, g_z,
                                    coords, disps,
                                    emat,eres);
    for (int ino = 0; ino<8; ino++){
      const unsigned int ip = aIP[ino];
      vec_b[ip*3+0] += eres[ino][0];
      vec_b[ip*3+1] += eres[ino][1];
      vec_b[ip*3+2] += eres[ino][2];
    }
    // marge dde
//    mat_A.Mearge(8, aIP, 8, aIP, 9, &emat[0][0][0][0], tmp_buffer);
    Merge<8,8,3,3,double>(mat_A,aIP,aIP,emat,tmp_buffer);
  }
}


// -----------------------------

template <class MAT>
void MergeLinSys_SolidLinear_NewmarkBeta_MeshTri2D(
    MAT& mat_A,
    double* vec_b,
    const double myu,
    const double lambda,
    const double rho,
    const double g_x,
    const double g_y,
    const double dt_timestep,
    const double gamma_newmark,
    const double beta_newmark,
    const double* aXY1,
    unsigned int nXY,
    const unsigned int* aTri1,
    unsigned int nTri,
    const double* aVal,
    const double* aVelo,
    const double* aAcc)
{
  const unsigned int np = nXY;
  std::vector<unsigned int> tmp_buffer(np, UINT_MAX);
  for (unsigned int iel = 0; iel<nTri; ++iel){
    const unsigned int i0 = aTri1[iel*3+0];
    const unsigned int i1 = aTri1[iel*3+1];
    const unsigned int i2 = aTri1[iel*3+2];
    const unsigned int aIP[3] = {i0,i1,i2};
    double coords[3][2]; FetchData<3,2>(coords,aIP, aXY1);
    double disps[3][2]; FetchData<3,2>(disps,aIP, aVal);
    double velos[3][2]; FetchData<3,2>(velos,aIP, aVelo);
    double accs[3][2]; FetchData<3,2>(accs,aIP, aAcc);
    //
    double eres[3][2];
    double emat[3][3][2][2];
    EMat_SolidDynamicLinear_Tri2D(
        eres,emat,
        myu, lambda,
        rho, g_x, g_y,
        dt_timestep, gamma_newmark, beta_newmark,
        disps, velos, accs, coords,
        true);
    for (int ino = 0; ino<3; ino++){
      const unsigned int ip = aIP[ino];
      vec_b[ip*2+0] += eres[ino][0];
      vec_b[ip*2+1] += eres[ino][1];
    }
    // marge dde
//    mat_A.Mearge(3, aIP, 3, aIP, 4, &emat[0][0][0][0], tmp_buffer);
    Merge<3,3,2,2,double>(mat_A,aIP,aIP,emat,tmp_buffer);
  }
}

template <class MAT>
void MergeLinSys_SolidLinear_NewmarkBeta_MeshTet3D(
    MAT& mat_A,
    double* vec_b,
    const double myu,
    const double lambda,
    const double rho,
    const double g[3],
    const double dt_timestep,
    const double gamma_newmark,
    const double beta_newmark,
    const double* aXYZ,
    unsigned int nXYZ,
    const unsigned int* aTet,
    unsigned int nTet,
    const double* aVal,
    const double* aVelo,
    const double* aAcc)
{
  const unsigned int np = nXYZ;
  std::vector<unsigned int> tmp_buffer(np, -1);
  for (unsigned int iel = 0; iel<nTet; ++iel){
    const unsigned int i0 = aTet[iel*4+0];
    const unsigned int i1 = aTet[iel*4+1];
    const unsigned int i2 = aTet[iel*4+2];
    const unsigned int i3 = aTet[iel*4+3];
    const unsigned int aIP[4] = {i0,i1,i2,i3};
    double P[4][3]; FetchData<4,3>(P, aIP, aXYZ);
    double disps[4][3]; FetchData<4,3>(disps, aIP, aVal);
    double velos[4][3]; FetchData<4,3>(velos, aIP, aVelo);
    double accs[4][3];  FetchData<4,3>(accs, aIP, aAcc);
    //
    double eres[4][3], emat[4][4][3][3];
    EMat_SolidLinear_NewmarkBeta_MeshTet3D(
        eres,emat,
        myu, lambda,
        rho, g[0], g[1], g[2],
        dt_timestep, gamma_newmark, beta_newmark,
        disps, velos, accs, P,
        true);
    for (int ino = 0; ino<4; ino++){
      const unsigned int ip = aIP[ino];
      vec_b[ip*3+0] += eres[ino][0];
      vec_b[ip*3+1] += eres[ino][1];
      vec_b[ip*3+2] += eres[ino][2];
    }
//    mat_A.Mearge(4, aIP, 4, aIP, 9, &emat[0][0][0][0], tmp_buffer);
    Merge<4,4,3,3,double>(mat_A,aIP,aIP,emat,tmp_buffer);
  }
}

template <class MAT>
void MergeLinSys_SolidLinear_BEuler_MeshTet3D(
    MAT& mat_A,
    double* vec_b,
    const double myu,
    const double lambda,
    const double rho,
    const double *g,
    const double dt,
    const double* aXYZ,
    unsigned int nXYZ,
    const unsigned int* aTet,
    unsigned int nTet,
    const double* aDisp,
    const double* aVelo)
{
  const unsigned int np = nXYZ;
  std::vector<unsigned int> tmp_buffer(np, UINT_MAX);
  for(unsigned int iel=0; iel<nTet; ++iel){
    const unsigned int i0 = aTet[iel*4+0];
    const unsigned int i1 = aTet[iel*4+1];
    const unsigned int i2 = aTet[iel*4+2];
    const unsigned int i3 = aTet[iel*4+3];
    const unsigned int aIP[4] = { i0, i1, i2, i3 };
    double P[4][3]; FetchData<4,3>(P, aIP, aXYZ);
    double emat[4][4][3][3];
    double eres[4][3];
    const double vol = femutil::TetVolume3D(P[0], P[1], P[2], P[3]);
    {
      double dldx[4][3], const_term[4];
      TetDlDx(dldx, const_term, P[0], P[1], P[2], P[3]);
      ddW_SolidLinear_Tet3D(
          &emat[0][0][0][0],
          lambda, myu, vol, dldx, false, 3);
    }
    {
      double u[4][3]; FetchData<4,3>(u, aIP, aDisp);
      double v[4][3]; FetchData<4,3>(v, aIP, aVelo);
      for(int ino=0;ino<4;++ino){
        for(int idim=0;idim<3;++idim){
          eres[ino][idim] = vol*rho*g[idim]*0.25;
          for(int jno=0;jno<4;++jno){
            eres[ino][idim] -= emat[ino][jno][idim][0]*(u[jno][0]+dt*v[jno][0]);
            eres[ino][idim] -= emat[ino][jno][idim][1]*(u[jno][1]+dt*v[jno][1]);
            eres[ino][idim] -= emat[ino][jno][idim][2]*(u[jno][2]+dt*v[jno][2]);
          }
        }
      }
    }
    {
      for(int ino=0;ino<4;++ino){
        emat[ino][ino][0][0] += rho*vol*0.25/(dt*dt);
        emat[ino][ino][1][1] += rho*vol*0.25/(dt*dt);
        emat[ino][ino][2][2] += rho*vol*0.25/(dt*dt);
      }
    }
    for (int ino = 0; ino<4; ino++){
      const unsigned int ip = aIP[ino];
      vec_b[ip*3+0] += eres[ino][0]/dt;
      vec_b[ip*3+1] += eres[ino][1]/dt;
      vec_b[ip*3+2] += eres[ino][2]/dt;
    }
//    mat_A.Mearge(4, aIP, 4, aIP, 9, &emat[0][0][0][0], tmp_buffer);
    Merge<4,4,3,3,double>(mat_A,aIP,aIP,emat,tmp_buffer);
  }
}

template <class MAT>
void MergeLinSys_SolidStiffwarp_BEuler_MeshTet3D(
    MAT& mat_A,
    double* vec_b,
    const double myu,
    const double lambda,
    const double rho,
    const double g[3],
    const double dt,
    const double* aXYZ,
    int nXYZ,
    const unsigned int* aTet,
    int nTet,
    const double* aDisp,
    const double* aVelo,
    const std::vector<double>& aR)
{
  const int np = nXYZ;
  assert((int)aR.size()==np*9);
  // ----------------------------
  std::vector<unsigned int> tmp_buffer(np, UINT_MAX);
  for (int iel = 0; iel<nTet; ++iel){
    const unsigned int i0 = aTet[iel*4+0];
    const unsigned int i1 = aTet[iel*4+1];
    const unsigned int i2 = aTet[iel*4+2];
    const unsigned int i3 = aTet[iel*4+3];
    const unsigned int aIP[4] = { i0, i1, i2, i3 };
    double P[4][3]; FetchData<4,3>(P, aIP, aXYZ);
    const double vol = femutil::TetVolume3D(P[0], P[1], P[2], P[3]);
    ////
    double emat[4][4][3][3];
    { // make stifness matrix with stiffness warping
      double dldx[4][3], const_term[4];
      TetDlDx(dldx, const_term, P[0], P[1], P[2], P[3]);
      double emat0[4][4][3][3];
      ddW_SolidLinear_Tet3D(&emat0[0][0][0][0],
          lambda, myu, vol, dldx, false, 3);
      double mtmp[9];
      for(int ino=0;ino<4;++ino){
        const double* Mi = aR.data()+aIP[ino]*9;
        for(int jno=0;jno<4;++jno){
          femutil::MatMatTrans3(mtmp, &emat0[ino][jno][0][0], Mi);
          femutil::MatMat3(&emat[ino][jno][0][0], Mi,mtmp);
        }
      }
    }
    double eres[4][3];
    {
      for(auto & ere : eres){
        ere[0] = vol*rho*g[0]*0.25;
        ere[1] = vol*rho*g[1]*0.25;
        ere[2] = vol*rho*g[2]*0.25;
      }
      double u0[4][3]; FetchData<4,3>(u0, aIP, aDisp);
      double v0[4][3]; FetchData<4,3>(v0, aIP, aVelo);
      for(int ino=0;ino<4;++ino){
        const double* Mi = aR.data()+aIP[ino]*9;
        for(int idim=0;idim<3;++idim){
          for(int jno=0;jno<4;++jno){
            double Pj1[3]; femutil::MatVec3(Pj1, Mi,P[jno]);
            double uj1[3] = {
              P[jno][0]+u0[jno][0]+dt*v0[jno][0]-Pj1[0],
              P[jno][1]+u0[jno][1]+dt*v0[jno][1]-Pj1[1],
              P[jno][2]+u0[jno][2]+dt*v0[jno][2]-Pj1[2] };
            eres[ino][idim] -= emat[ino][jno][idim][0]*uj1[0];
            eres[ino][idim] -= emat[ino][jno][idim][1]*uj1[1];
            eres[ino][idim] -= emat[ino][jno][idim][2]*uj1[2];
          }
        }
      }
    }
    for(int ino=0;ino<4;++ino){
      emat[ino][ino][0][0] += rho*vol*0.25/(dt*dt);
      emat[ino][ino][1][1] += rho*vol*0.25/(dt*dt);
      emat[ino][ino][2][2] += rho*vol*0.25/(dt*dt);
    }
    for (int ino = 0; ino<4; ino++){
      const unsigned int ip = aIP[ino];
      vec_b[ip*3+0] += eres[ino][0]/dt;
      vec_b[ip*3+1] += eres[ino][1]/dt;
      vec_b[ip*3+2] += eres[ino][2]/dt;
    }
//    mat_A.Mearge(4, aIP, 4, aIP, 9, &emat[0][0][0][0], tmp_buffer);
    Merge<4,4,3,3,double>(mat_A,aIP,aIP,emat,tmp_buffer);
  }
}


} // namespace delfem2

#ifdef DFM2_HEADER_ONLY
#  include "delfem2/femsolidlinear.cpp"
#endif
  
#endif /* fem_ematrix_h */
