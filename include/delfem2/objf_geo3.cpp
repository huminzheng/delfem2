/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "delfem2/geo3_v23m34q.h"
#include "delfem2/objf_geo3.h"

// =======================================

DFM2_INLINE void delfem2::PBD_Post
(std::vector<double>& aXYZ,
 std::vector<double>& aUVW,
 double dt,
 const std::vector<double>& aXYZt,
 const std::vector<int>& aBCFlag)
{
  const unsigned int np = aXYZ.size()/3;
  assert( aBCFlag.size() == np );
  assert( aXYZ.size() == np*3 );
  assert( aUVW.size() == np*3 );
  for(unsigned int ip=0;ip<np;++ip){
    if( aBCFlag[ip] != 0 ) continue;
    aUVW[ip*3+0] = (aXYZt[ip*3+0]-aXYZ[ip*3+0])/dt;
    aUVW[ip*3+1] = (aXYZt[ip*3+1]-aXYZ[ip*3+1])/dt;
    aUVW[ip*3+2] = (aXYZt[ip*3+2]-aXYZ[ip*3+2])/dt;
    aXYZ[ip*3+0] = aXYZt[ip*3+0];
    aXYZ[ip*3+1] = aXYZt[ip*3+1];
    aXYZ[ip*3+2] = aXYZt[ip*3+2];
  }
}

DFM2_INLINE void delfem2::PBD_Pre3D
(std::vector<double>& aXYZt,
 double dt,
 const double gravity[3],
 const std::vector<double>& aXYZ,
 const std::vector<double>& aUVW,
 const std::vector<int>& aBCFlag)
{
  const unsigned int np = aXYZ.size()/3;
  assert( aBCFlag.size() == np );
  assert( aUVW.size() == np*3 );
  aXYZt.resize(np*3);
  for(unsigned int ip=0;ip<np;++ip){
    if( aBCFlag[ip] != 0 ){
      aXYZt[ip*3+0] = aXYZ[ip*3+0];
      aXYZt[ip*3+1] = aXYZ[ip*3+1];
      aXYZt[ip*3+2] = aXYZ[ip*3+2];
      continue;
    }
    aXYZt[ip*3+0] = aXYZ[ip*3+0]+dt*aUVW[ip*3+0]+dt*dt*gravity[0];
    aXYZt[ip*3+1] = aXYZ[ip*3+1]+dt*aUVW[ip*3+1]+dt*dt*gravity[1];
    aXYZt[ip*3+2] = aXYZ[ip*3+2]+dt*aUVW[ip*3+2]+dt*dt*gravity[2];
  }
}


DFM2_INLINE void delfem2::PBD_Update_Const3
(double* aXYZt,
 const int np,
 const int ndim,
 const double* m,
 const double* C,
 const double* dCdp,
 const unsigned int* aIP,
 double ratio)
{
  std::vector<double> mi(np);
  for(int ip=0;ip<np;++ip){ mi[ip] = 1.0/m[ip]; }
  //
  const int nc = 3;
  std::vector<double> MinvC(nc*np*ndim);
  for(int ic=0;ic<nc;++ic){
    for(int ine=0;ine<np;++ine){
      MinvC[ic*np*ndim+ine*3+0] = dCdp[ic*np*ndim+ine*ndim+0]*mi[ine];
      MinvC[ic*np*ndim+ine*3+1] = dCdp[ic*np*ndim+ine*ndim+1]*mi[ine];
      MinvC[ic*np*ndim+ine*3+2] = dCdp[ic*np*ndim+ine*ndim+2]*mi[ine];
    }
  }
  double A[nc*nc] = {0,0,0, 0,0,0, 0,0,0};
  for(int i=0;i<np*ndim;++i){
    A[0*3+0] += MinvC[0*np*ndim+i]*dCdp[0*np*ndim+i];
    A[0*3+1] += MinvC[0*np*ndim+i]*dCdp[1*np*ndim+i];
    A[0*3+2] += MinvC[0*np*ndim+i]*dCdp[2*np*ndim+i];
    A[1*3+0] += MinvC[1*np*ndim+i]*dCdp[0*np*ndim+i];
    A[1*3+1] += MinvC[1*np*ndim+i]*dCdp[1*np*ndim+i];
    A[1*3+2] += MinvC[1*np*ndim+i]*dCdp[2*np*ndim+i];
    A[2*3+0] += MinvC[2*np*ndim+i]*dCdp[0*np*ndim+i];
    A[2*3+1] += MinvC[2*np*ndim+i]*dCdp[1*np*ndim+i];
    A[2*3+2] += MinvC[2*np*ndim+i]*dCdp[2*np*ndim+i];
  }
  double Ainv[nc*nc]; Inverse_Mat3(Ainv, A);
  double lmd[nc]; MatVec3(lmd, Ainv, C);
  for(int ine=0;ine<np;++ine){
    const int ip0 = aIP[ine];
    for(int ic=0;ic<nc;++ic){
      for(int idim=0;idim<ndim;++idim){
        aXYZt[ip0*3+idim] -= ratio*MinvC[ic*np*ndim+ine*ndim+idim]*lmd[ic];
      }
    }

  }
}

DFM2_INLINE void delfem2::PBD_ConstProj_Rigid3D
(double* aXYZt,
 double stiffness,
 const int* clstr_ind, int nclstr_ind,
 const int* clstr,     int nclstr0,
 const double* aXYZ0,  int nXYZ0)
{
  const int nclstr = nclstr_ind-1;
  for(int iclstr=0;iclstr<nclstr;++iclstr){
    CVec3d pc(0, 0, 0), qc(0, 0, 0);
    for (int iip=clstr_ind[iclstr];iip<clstr_ind[iclstr+1]; iip++){
      const int ip = clstr[iip];
      assert( ip < nclstr0 );
      qc += CVec3d(aXYZ0[ip*3+0],aXYZ0[ip*3+1],aXYZ0[ip*3+2]);
      pc += CVec3d(aXYZt[ip*3+0],aXYZt[ip*3+1],aXYZt[ip*3+2]);
    }
    qc /= (clstr_ind[iclstr+1]-clstr_ind[iclstr]);
    pc /= (clstr_ind[iclstr+1]-clstr_ind[iclstr]);
    
    double A[9] = { 0,0,0,  0,0,0, 0,0,0 };
    for (int iip=clstr_ind[iclstr];iip<clstr_ind[iclstr+1]; iip++){
      const int ip = clstr[iip];
      const CVec3d dq = CVec3d(aXYZ0[ip*3+0],aXYZ0[ip*3+1],aXYZ0[ip*3+2])-qc; // undeform
      const CVec3d dp = CVec3d(aXYZt[ip*3+0],aXYZt[ip*3+1],aXYZ0[ip*3+2])-pc; // deform
      A[0*3+0] += dp[0]*dq[0];
      A[0*3+1] += dp[0]*dq[1];
      A[0*3+2] += dp[0]*dq[2];
      A[1*3+0] += dp[1]*dq[0];
      A[1*3+1] += dp[1]*dq[1];
      A[1*3+2] += dp[1]*dq[2];
      A[2*3+0] += dp[2]*dq[0];
      A[2*3+1] += dp[2]*dq[1];
      A[2*3+2] += dp[2]*dq[2];
    }
    double R[9]; GetRotPolarDecomp(R,A, 20);
    //    std::cout << R[0] << " " << R[1] << " " << R[2] << " " << R[3] << std::endl;
    
    for (int iip=clstr_ind[iclstr];iip<clstr_ind[iclstr+1]; iip++){
      const int ip = clstr[iip];
      CVec3d dq = CVec3d(aXYZ0[ip*3+0],aXYZ0[ip*3+1],aXYZ0[ip*3+2])-qc;
      CVec3d pg = pc+Mat3Vec(R, dq); // goal position
      CVec3d pg2 = stiffness*pg+(1-stiffness)*CVec3d(aXYZt[ip*3+0],aXYZt[ip*3+1],aXYZt[ip*3+2]);
      aXYZt[ip*3+0] = pg2.p[0];
      aXYZt[ip*3+1] = pg2.p[1];
      aXYZt[ip*3+2] = pg2.p[2];
    }
  }  
}

DFM2_INLINE void delfem2::PBD_ConstProj_Rigid2D(
    double* aXYt,
    double stiffness,
    const unsigned int *clstr_ind, unsigned int nclstr_ind,
    const unsigned int *clstr, unsigned int nclstr0,
    const double* aXY0, unsigned int nXY0)
{
  const unsigned int nclstr = nclstr_ind-1;
  for(unsigned int iclstr=0;iclstr<nclstr;++iclstr){
    CVec2d pc(0, 0), qc(0, 0);
    for (unsigned int iip=clstr_ind[iclstr];iip<clstr_ind[iclstr+1]; iip++){
      assert( iip < nclstr0 );
      const unsigned int ip = clstr[iip]; assert( ip < nXY0 );
      qc += CVec2d(aXY0[ip*2+0],aXY0[ip*2+1]);
      pc += CVec2d(aXYt[ip*2+0],aXYt[ip*2+1]);
    }
    qc /= (clstr_ind[iclstr+1]-clstr_ind[iclstr]);
    pc /= (clstr_ind[iclstr+1]-clstr_ind[iclstr]);
    
    double A[4] = { 0, 0, 0, 0 };
    for (unsigned int iip=clstr_ind[iclstr];iip<clstr_ind[iclstr+1]; iip++){
      const unsigned int ip = clstr[iip];
      const CVec2d dq = CVec2d(aXY0[ip*2+0],aXY0[ip*2+1])-qc; // undeform
      const CVec2d dp = CVec2d(aXYt[ip*2+0],aXYt[ip*2+1])-pc; // deform
      A[0*2+0] += dp[0]*dq[0];
      A[0*2+1] += dp[0]*dq[1];
      A[1*2+0] += dp[1]*dq[0];
      A[1*2+1] += dp[1]*dq[1];
    }
    double R[4]; RotationalComponentOfMatrix2(R,A);
    //    std::cout << R[0] << " " << R[1] << " " << R[2] << " " << R[3] << std::endl;
    
    for (unsigned int iip=clstr_ind[iclstr];iip<clstr_ind[iclstr+1]; iip++){
      const unsigned int ip = clstr[iip];
      CVec2d dq = CVec2d(aXY0[ip*2+0],aXY0[ip*2+1])-qc;
      CVec2d pg = pc+Mat2Vec(R, dq); // goal position
      CVec2d pg2 = stiffness*pg+(1-stiffness)*CVec2d(aXYt[ip*2+0],aXYt[ip*2+1]);
      aXYt[ip*2+0] = pg2.x();
      aXYt[ip*2+1] = pg2.y();
    }
  }
}


DFM2_INLINE void delfem2::PBD_CdC_TriStrain2D3D(
    double C[3],
    double dCdp[3][9],
    const double P[3][2], // (in) undeformed triangle vertex positions
    const double p[3][3] // (in) deformed triangle vertex positions
)
{
  const CVec3d Gd0( P[1][0]-P[0][0], P[1][1]-P[0][1], 0.0 );
  const CVec3d Gd1( P[2][0]-P[0][0], P[2][1]-P[0][1], 0.0 );
  CVec3d Gd2 = Cross(Gd0, Gd1);
  const double Area = Gd2.Length()*0.5;
  Gd2 /= (Area*2.0);
  
  CVec3d Gu0 = Cross( Gd1, Gd2 ); Gu0 /= Dot(Gu0,Gd0);
  CVec3d Gu1 = Cross( Gd2, Gd0 ); Gu1 /= Dot(Gu1,Gd1);
  
  const CVec3d gd0( p[1][0]-p[0][0], p[1][1]-p[0][1], p[1][2]-p[0][2] );
  const CVec3d gd1( p[2][0]-p[0][0], p[2][1]-p[0][1], p[2][2]-p[0][2] );
  
  const double E2[3] = {  // green lagrange strain (with engineer's notation)
    0.5*( Dot(gd0,gd0) - Dot(Gd0,Gd0) ),
    0.5*( Dot(gd1,gd1) - Dot(Gd1,Gd1) ),
    1.0*( Dot(gd0,gd1) - Dot(Gd0,Gd1) ) };
  
  const double GuGu_xx[3] = { Gu0.x()*Gu0.x(), Gu1.x()*Gu1.x(), Gu0.x()*Gu1.x() };
  const double GuGu_yy[3] = { Gu0.y()*Gu0.y(), Gu1.y()*Gu1.y(), Gu0.y()*Gu1.y() };
  const double GuGu_xy[3] = { 2.0*Gu0.x()*Gu0.y(), 2.0*Gu1.x()*Gu1.y(), Gu0.x()*Gu1.y()+Gu0.y()*Gu1.x() };
  C[0] = E2[0]*GuGu_xx[0] + E2[1]*GuGu_xx[1] + E2[2]*GuGu_xx[2];
  C[1] = E2[0]*GuGu_yy[0] + E2[1]*GuGu_yy[1] + E2[2]*GuGu_yy[2];
  C[2] = E2[0]*GuGu_xy[0] + E2[1]*GuGu_xy[1] + E2[2]*GuGu_xy[2];
  
  const CVec3d dC0dp0 = -(GuGu_xx[0]+GuGu_xx[2])*gd0 - (GuGu_xx[1]+GuGu_xx[2])*gd1;
  const CVec3d dC0dp1 = GuGu_xx[0]*gd0 + GuGu_xx[2]*gd1;
  const CVec3d dC0dp2 = GuGu_xx[1]*gd1 + GuGu_xx[2]*gd0;
  const CVec3d dC1dp0 = -(GuGu_yy[0]+GuGu_yy[2])*gd0 - (GuGu_yy[1]+GuGu_yy[2])*gd1;
  const CVec3d dC1dp1 = GuGu_yy[0]*gd0 + GuGu_yy[2]*gd1;
  const CVec3d dC1dp2 = GuGu_yy[1]*gd1 + GuGu_yy[2]*gd0;
  const CVec3d dC2dp0 = -(GuGu_xy[0]+GuGu_xy[2])*gd0 - (GuGu_xy[1]+GuGu_xy[2])*gd1;
  const CVec3d dC2dp1 = GuGu_xy[0]*gd0 + GuGu_xy[2]*gd1;
  const CVec3d dC2dp2 = GuGu_xy[1]*gd1 + GuGu_xy[2]*gd0;
  
  dC0dp0.CopyTo(dCdp[0]+0*3);
  dC0dp1.CopyTo(dCdp[0]+1*3);
  dC0dp2.CopyTo(dCdp[0]+2*3);
  dC1dp0.CopyTo(dCdp[1]+0*3);
  dC1dp1.CopyTo(dCdp[1]+1*3);
  dC1dp2.CopyTo(dCdp[1]+2*3);
  dC2dp0.CopyTo(dCdp[2]+0*3);
  dC2dp1.CopyTo(dCdp[2]+1*3);
  dC2dp2.CopyTo(dCdp[2]+2*3);
}

DFM2_INLINE void delfem2::PBD_ConstraintProjection_DistanceTri2D3D(
    double C[3],
    double dCdp[3][9],
    const double P[3][2], // (in) undeformed triangle vertex positions
    const double p[3][3] // (in) deformed triangle vertex positions
)
{
  const double L12 = Distance2(P[1],P[2]);
  const double L20 = Distance2(P[2],P[0]);
  const double L01 = Distance2(P[0],P[1]);
  CVec3d v12(p[1][0]-p[2][0], p[1][1]-p[2][1], p[1][2]-p[2][2]);
  CVec3d v20(p[2][0]-p[0][0], p[2][1]-p[0][1], p[2][2]-p[0][2]);
  CVec3d v01(p[0][0]-p[1][0], p[0][1]-p[1][1], p[0][2]-p[1][2]);
  const double l12 = v12.Length();
  const double l20 = v20.Length();
  const double l01 = v01.Length();
  C[0] = l12-L12;
  C[1] = l20-L20;
  C[2] = l01-L01;
  v12 /= l12;
  v20 /= l20;
  v01 /= l01;
  for(int i=0;i<27;++i){ (&dCdp[0][0])[i] = 0.0; }
  v12.CopyTo(dCdp[0]+3*1);  v12.CopyToScale(dCdp[0]+3*2,-1.0);
  v20.CopyTo(dCdp[1]+3*2);  v20.CopyToScale(dCdp[1]+3*0,-1.0);
  v01.CopyTo(dCdp[2]+3*0);  v01.CopyToScale(dCdp[2]+3*1,-1.0);
}

DFM2_INLINE void delfem2::PBD_ConstraintProjection_EnergyStVK
(double& C, // (out) energy
 double dCdp[9], // (out) 1st derivative of energy
 ////
 const double P[3][2], // (in) undeformed triangle vertex positions
 const double p[3][3], // (in) deformed triangle vertex positions
 const double lambda, // (in) Lame's 1st parameter
 const double myu)     // (in) Lame's 2nd parameter
{
  
  const CVec3d Gd0( P[1][0]-P[0][0], P[1][1]-P[0][1], 0.0 );
  const CVec3d Gd1( P[2][0]-P[0][0], P[2][1]-P[0][1], 0.0 );
  CVec3d Gd2 = Cross(Gd0, Gd1);
  const double Area = Gd2.Length()*0.5;
  Gd2 /= (Area*2.0);
  
  CVec3d Gu0 = Cross( Gd1, Gd2 ); Gu0 /= Dot(Gu0,Gd0);
  CVec3d Gu1 = Cross( Gd2, Gd0 ); Gu1 /= Dot(Gu1,Gd1);
  
  const CVec3d gd0( p[1][0]-p[0][0], p[1][1]-p[0][1], p[1][2]-p[0][2] );
  const CVec3d gd1( p[2][0]-p[0][0], p[2][1]-p[0][1], p[2][2]-p[0][2] );
  
  const double E2[3] = {  // green lagrange strain (with engineer's notation)
    0.5*( Dot(gd0,gd0) - Dot(Gd0,Gd0) ),
    0.5*( Dot(gd1,gd1) - Dot(Gd1,Gd1) ),
    1.0*( Dot(gd0,gd1) - Dot(Gd0,Gd1) ) };
  
  /*
   double Gd[3][3] = { // undeformed edge vector
   { C[1][0]-C[0][0], C[1][1]-C[0][1], C[1][2]-C[0][2] },
   { C[2][0]-C[0][0], C[2][1]-C[0][1], C[2][2]-C[0][2] }, { 0,0,0 } };
   double Area;
   UnitNormalAreaTri3D(Gd[2], Area, C[0], C[1], C[2]);
   
   double Gu[2][3]; // inverse of Gd
   {
   Cross3D(Gu[0], Gd[1], Gd[2]);
   const double invtmp1 = 1.0/Dot3D(Gu[0],Gd[0]);
   Gu[0][0] *= invtmp1;  Gu[0][1] *= invtmp1;  Gu[0][2] *= invtmp1;
   ////
   Cross3D(Gu[1], Gd[2], Gd[0]);
   const double invtmp2 = 1.0/Dot3D(Gu[1],Gd[1]);
   Gu[1][0] *= invtmp2;  Gu[1][1] *= invtmp2;  Gu[1][2] *= invtmp2;
   }
   
   const double gd[2][3] = { // deformed edge vector
   { c[1][0]-c[0][0], c[1][1]-c[0][1], c[1][2]-c[0][2] },
   { c[2][0]-c[0][0], c[2][1]-c[0][1], c[2][2]-c[0][2] } };
   
   const double E2[3] = {  // green lagrange strain (with engineer's notation)
   0.5*( Dot3D(gd[0],gd[0]) - Dot3D(Gd[0],Gd[0]) ),
   0.5*( Dot3D(gd[1],gd[1]) - Dot3D(Gd[1],Gd[1]) ),
   1.0*( Dot3D(gd[0],gd[1]) - Dot3D(Gd[0],Gd[1]) ) };
   */
  const double GuGu2[3] = { Gu0*Gu0, Gu1*Gu1, Gu1*Gu0 };
  const double Cons2[3][3] = { // constitutive tensor
    { lambda*GuGu2[0]*GuGu2[0] + 2*myu*(GuGu2[0]*GuGu2[0]),
      lambda*GuGu2[0]*GuGu2[1] + 2*myu*(GuGu2[2]*GuGu2[2]),
      lambda*GuGu2[0]*GuGu2[2] + 2*myu*(GuGu2[0]*GuGu2[2]) },
    { lambda*GuGu2[1]*GuGu2[0] + 2*myu*(GuGu2[2]*GuGu2[2]),
      lambda*GuGu2[1]*GuGu2[1] + 2*myu*(GuGu2[1]*GuGu2[1]),
      lambda*GuGu2[1]*GuGu2[2] + 2*myu*(GuGu2[2]*GuGu2[1]) },
    { lambda*GuGu2[2]*GuGu2[0] + 2*myu*(GuGu2[0]*GuGu2[2]),
      lambda*GuGu2[2]*GuGu2[1] + 2*myu*(GuGu2[2]*GuGu2[1]),
      lambda*GuGu2[2]*GuGu2[2] + 1*myu*(GuGu2[0]*GuGu2[1] + GuGu2[2]*GuGu2[2]) } };
  const double S2[3] = {  // 2nd Piola-Kirchhoff stress
    Cons2[0][0]*E2[0] + Cons2[0][1]*E2[1] + Cons2[0][2]*E2[2],
    Cons2[1][0]*E2[0] + Cons2[1][1]*E2[1] + Cons2[1][2]*E2[2],
    Cons2[2][0]*E2[0] + Cons2[2][1]*E2[1] + Cons2[2][2]*E2[2] };
  
  // compute energy
  C = 0.5*Area*(E2[0]*S2[0] + E2[1]*S2[1] + E2[2]*S2[2]);
  
  // compute 1st derivative
  const double dNdr[3][2] = { {-1.0, -1.0}, {+1.0, +0.0}, {+0.0, +1.0} };
  for(int ino=0;ino<3;ino++){
    dCdp[ino*3+0] = Area*(+S2[0]*gd0[0]*dNdr[ino][0] + S2[2]*gd0[0]*dNdr[ino][1] + S2[2]*gd1[0]*dNdr[ino][0] + S2[1]*gd1[0]*dNdr[ino][1]);
    dCdp[ino*3+1] = Area*(+S2[0]*gd0[1]*dNdr[ino][0] + S2[2]*gd0[1]*dNdr[ino][1] + S2[2]*gd1[1]*dNdr[ino][0] + S2[1]*gd1[1]*dNdr[ino][1]);
    dCdp[ino*3+2] = Area*(+S2[0]*gd0[2]*dNdr[ino][0] + S2[2]*gd0[2]*dNdr[ino][1] + S2[2]*gd1[2]*dNdr[ino][0] + S2[1]*gd1[2]*dNdr[ino][1]);
  }
}


DFM2_INLINE void delfem2::PBD_ConstraintProjection_DistanceTet
(double C[6],
 double dCdp[6][12],
 const double P[4][3], // (in) undeformed triangle vertex positions
 const double p[4][3] // (in) deformed triangle vertex positions
)
{
  const double L01 = Distance3(P[0],P[1]);
  const double L02 = Distance3(P[0],P[2]);
  const double L03 = Distance3(P[0],P[3]);
  const double L12 = Distance3(P[1],P[2]);
  const double L13 = Distance3(P[1],P[3]);
  const double L23 = Distance3(P[2],P[3]);
  CVec3d v01(p[0][0]-p[1][0], p[0][1]-p[1][1], p[0][2]-p[1][2]);
  CVec3d v02(p[0][0]-p[2][0], p[0][1]-p[2][1], p[0][2]-p[2][2]);
  CVec3d v03(p[0][0]-p[3][0], p[0][1]-p[3][1], p[0][2]-p[3][2]);
  CVec3d v12(p[1][0]-p[2][0], p[1][1]-p[2][1], p[1][2]-p[2][2]);
  CVec3d v13(p[1][0]-p[3][0], p[1][1]-p[3][1], p[1][2]-p[3][2]);
  CVec3d v23(p[2][0]-p[3][0], p[2][1]-p[3][1], p[2][2]-p[3][2]);
  const double l01 = v01.Length();
  const double l02 = v02.Length();
  const double l03 = v03.Length();
  const double l12 = v12.Length();
  const double l13 = v13.Length();
  const double l23 = v23.Length();
  C[0] = l01-L01;
  C[1] = l02-L02;
  C[2] = l03-L03;
  C[3] = l12-L12;
  C[4] = l13-L13;
  C[5] = l23-L23;
  ////
  v01 /= l01;
  v02 /= l02;
  v03 /= l03;
  v12 /= l12;
  v13 /= l13;
  v23 /= l23;
  ////
  for(int i=0;i<6*3*4;++i){ (&dCdp[0][0])[i] = 0.0; }
  v01.CopyTo(dCdp[0]+0*3);  v01.CopyToScale(dCdp[0]+1*3,-1.0);
  v02.CopyTo(dCdp[1]+0*3);  v02.CopyToScale(dCdp[1]+2*3,-1.0);
  v03.CopyTo(dCdp[2]+0*3);  v03.CopyToScale(dCdp[2]+3*3,-1.0);
  v12.CopyTo(dCdp[3]+1*3);  v12.CopyToScale(dCdp[3]+2*3,-1.0);
  v13.CopyTo(dCdp[4]+1*3);  v13.CopyToScale(dCdp[4]+3*3,-1.0);
  v23.CopyTo(dCdp[5]+2*3);  v23.CopyToScale(dCdp[5]+3*3,-1.0);
}


DFM2_INLINE void delfem2::PBD_CdC_QuadBend(
    double C[3],
    double dCdp[3][12],
    const double P[4][3],
    const double p[4][3])
{
  const double A0 = Area_Tri3(P[0],P[2],P[3]);
  const double A1 = Area_Tri3(P[1],P[3],P[2]);
  const double L = Distance3(P[2],P[3]);
  const double H0 = 2.0*A0/L;
  const double H1 = 2.0*A1/L;
  const CVec3d e23(P[3][0]-P[2][0], P[3][1]-P[2][1], P[3][2]-P[2][2]);
  const CVec3d e02(P[2][0]-P[0][0], P[2][1]-P[0][1], P[2][2]-P[0][2]);
  const CVec3d e03(P[3][0]-P[0][0], P[3][1]-P[0][1], P[3][2]-P[0][2]);
  const CVec3d e12(P[2][0]-P[1][0], P[2][1]-P[1][1], P[2][2]-P[1][2]);
  const CVec3d e13(P[3][0]-P[1][0], P[3][1]-P[1][1], P[3][2]-P[1][2]);
  const double cot023 = -(e02*e23)/H0;
  const double cot032 = +(e03*e23)/H0;
  const double cot123 = -(e12*e23)/H1;
  const double cot132 = +(e13*e23)/H1;
  const double tmp0 = sqrt(3.0)/(sqrt(A0+A1)*L);
  const double K[4] = {
    (-cot023-cot032)*tmp0,
    (-cot123-cot132)*tmp0,
    (+cot032+cot132)*tmp0,
    (+cot023+cot123)*tmp0 };
  C[0] = K[0]*p[0][0] + K[1]*p[1][0] + K[2]*p[2][0] + K[3]*p[3][0];
  C[1] = K[0]*p[0][1] + K[1]*p[1][1] + K[2]*p[2][1] + K[3]*p[3][1];
  C[2] = K[0]*p[0][2] + K[1]*p[1][2] + K[2]*p[2][2] + K[3]*p[3][2];
  for(int i=0;i<36;++i){ (&dCdp[0][0])[i] = 0.0; }
  for(int idim=0;idim<3;++idim){
    dCdp[idim][0*3+idim] = K[0];
    dCdp[idim][1*3+idim] = K[1];
    dCdp[idim][2*3+idim] = K[2];
    dCdp[idim][3*3+idim] = K[3];
  }
}


DFM2_INLINE void delfem2::PBD_Seam(
    double* aXYZt,
    unsigned int nXYZ,
    const unsigned int* aLine,
    unsigned int nline)
{
  for(unsigned int il=0;il<nline;++il){
    const unsigned int ip0 = aLine[il*2+0];
    const unsigned int ip1 = aLine[il*2+1];
    const double p[2][3] = {
      {aXYZt[ip0*3+0], aXYZt[ip0*3+1], aXYZt[ip0*3+2]},
      {aXYZt[ip1*3+0], aXYZt[ip1*3+1], aXYZt[ip1*3+2]} };
    double d0 = Distance3(p[0], p[1]);
    double dLen = 0.01;
    if( d0 > dLen ){
      double n01[3] = {p[1][0]-p[0][0], p[1][1]-p[0][1], p[1][2]-p[0][2]};
      const double l01 = Length3(n01);
      const double invl01 = 1.0/l01;
      n01[0] *= invl01;
      n01[1] *= invl01;
      n01[2] *= invl01;
      aXYZt[ip0*3+0] += n01[0]*dLen*0.5;
      aXYZt[ip0*3+1] += n01[1]*dLen*0.5;
      aXYZt[ip0*3+2] += n01[2]*dLen*0.5;
      aXYZt[ip1*3+0] -= n01[0]*dLen*0.5;
      aXYZt[ip1*3+1] -= n01[1]*dLen*0.5;
      aXYZt[ip1*3+2] -= n01[2]*dLen*0.5;
    }
    else{
      aXYZt[ip0*3+0] = (p[0][0]+p[1][0])*0.5;
      aXYZt[ip0*3+1] = (p[0][1]+p[1][1])*0.5;
      aXYZt[ip0*3+2] = (p[0][2]+p[1][2])*0.5;
      aXYZt[ip1*3+0] = (p[0][0]+p[1][0])*0.5;
      aXYZt[ip1*3+1] = (p[0][1]+p[1][1])*0.5;
      aXYZt[ip1*3+2] = (p[0][2]+p[1][2])*0.5;
    }
  }
}


DFM2_INLINE void delfem2::WdWddW_MIPS
 (double& E, double dE[3][3], double ddE[3][3][3][3],
  const double c[3][3],
  const double C[3][3])
{
  /*
   double area = TriArea3D(c[0], c[1], c[2]);
   double Area = TriArea3D(C[0], C[1], C[2]);
   double la = SquareDistance3D(c[1], c[2]);
   double lb = SquareDistance3D(c[2], c[0]);
   double lc = SquareDistance3D(c[0], c[1]);
   double lA = SquareDistance3D(C[1], C[2]);
   double lB = SquareDistance3D(C[2], C[0]);
   double lC = SquareDistance3D(C[0], C[1]);
   double cot0 = (-la+lb+lc);
   double cot1 = (+la-lb+lc);
   double cot2 = (+la+lb-lc);
   //  double E_angle = (cot0*lA+cot1*lB+cot2*lC)/(8*Area*area);
   //  double E_area = Area/area + area/Area;
   //  E = E_angle*E_area;
   */
  CVec3d p0(c[0][0],c[0][1],c[0][2]);
  CVec3d p1(c[1][0],c[1][1],c[1][2]);
  CVec3d p2(c[2][0],c[2][1],c[2][2]);
  CVec3d P0(C[0][0],C[0][1],C[0][2]);
  CVec3d P1(C[1][0],C[1][1],C[1][2]);
  CVec3d P2(C[2][0],C[2][1],C[2][2]);
  CVec3d v01 = p1-p0;
  CVec3d v12 = p2-p1;
  CVec3d v20 = p0-p2;
  CVec3d n = v01^v20;
  double area = n.Length()*0.5;
  double Area = ((P1-P0)^(P2-P0)).Length()*0.5;
  double la = (p1-p2)*(p1-p2);
  double lb = (p2-p0)*(p2-p0);
  double lc = (p0-p1)*(p0-p1);
  double lA = (P1-P2)*(P1-P2);
  double lB = (P2-P0)*(P2-P0);
  double lC = (P0-P1)*(P0-P1);
  double cot0 = (-la+lb+lc);
  double cot1 = (+la-lb+lc);
  double cot2 = (+la+lb-lc);
  double tmp0 = 1.0/(8*Area);
  double EC = (cot0*lA+cot1*lB+cot2*lC)*tmp0;
  //  CVector3 dECd0 = (2*p0-p2-p1)*lA*2 + (p2-p1)*lB*2 + (p1-p2)*lC*2;
  //  CVector3 dECd1 = (p2-p0)*lA*2 + (2*p1-p2-p0)*lB*2 + (p0-p2)*lC*2;
  //  CVector3 dECd2 = (p1-p0)*lA*2 + (p0-p1)*lB*2 + (2*p2-p0-p1)*lC*2;
  double t00 = 4*lA*tmp0;
  double t11 = 4*lB*tmp0;
  double t22 = 4*lC*tmp0;
  double t01 = (-2*lA-2*lB+2*lC)*tmp0;
  double t02 = (-2*lA+2*lB-2*lC)*tmp0;
  double t12 = (+2*lA-2*lB-2*lC)*tmp0;
  CVec3d dECd0 = t00*p0 + t01*p1 + t02*p2;
  CVec3d dECd1 = t01*p0 + t11*p1 + t12*p2;
  CVec3d dECd2 = t02*p0 + t12*p1 + t22*p2;
  ////
  dE[0][0]=dECd0.x(); dE[0][1]=dECd0.y(); dE[0][2]=dECd0.z();
  dE[1][0]=dECd1.x(); dE[1][1]=dECd1.y(); dE[1][2]=dECd1.z();
  dE[2][0]=dECd2.x(); dE[2][1]=dECd2.y(); dE[2][2]=dECd2.z();
  
  CMat3d (*op)(const CVec3d&, const CVec3d&) = Mat3_OuterProduct;
  
  double tmp1 = 0.25/area;
  CVec3d dad0 = ((v20*v12)*v01-(v01*v12)*v20)*tmp1;
  CVec3d dad1 = ((v01*v20)*v12-(v12*v20)*v01)*tmp1;
  CVec3d dad2 = ((v12*v01)*v20-(v20*v01)*v12)*tmp1;
  CMat3d ddad0d0 = (CMat3d::Identity(v12*v12) - op(v12,v12)                   - 4*op(dad0,dad0))*tmp1;
  CMat3d ddad0d1 = (CMat3d::Identity(v20*v12) - op(v20,v12-v01) - op(v01,v20) - 4*op(dad0,dad1))*tmp1;
  CMat3d ddad0d2 = (CMat3d::Identity(v01*v12) - op(v01,v12-v20) - op(v20,v01) - 4*op(dad0,dad2))*tmp1;
  CMat3d ddad1d0 = (CMat3d::Identity(v12*v20) - op(v12,v20-v01) - op(v01,v12) - 4*op(dad1,dad0))*tmp1;
  CMat3d ddad1d1 = (CMat3d::Identity(v20*v20) - op(v20,v20)                   - 4*op(dad1,dad1))*tmp1;
  CMat3d ddad1d2 = (CMat3d::Identity(v01*v20) - op(v01,v20-v12) - op(v12,v01) - 4*op(dad1,dad2))*tmp1;
  CMat3d ddad2d0 = (CMat3d::Identity(v12*v01) - op(v12,v01-v20) - op(v20,v12) - 4*op(dad2,dad0))*tmp1;
  CMat3d ddad2d1 = (CMat3d::Identity(v20*v01) - op(v20,v01-v12) - op(v12,v20) - 4*op(dad2,dad1))*tmp1;
  CMat3d ddad2d2 = (CMat3d::Identity(v01*v01) - op(v01,v01)                   - 4*op(dad2,dad2))*tmp1;
  
  double ADR = Area/area+area/Area;
  double EA = ADR;
  double dADR = 1.0/Area-Area/(area*area);
  double dEA = dADR;
  double ddADR = 2*Area/(area*area*area);
  double ddEA = ddADR;
  E = EC*EA;
  for(int idim=0;idim<3;++idim){
    dE[0][idim]=EC*dEA*dad0[idim]+EA*dECd0[idim];
    dE[1][idim]=EC*dEA*dad1[idim]+EA*dECd1[idim];
    dE[2][idim]=EC*dEA*dad2[idim]+EA*dECd2[idim];
  }
  CMat3d ddEd0d0 = EC*dEA*ddad0d0 + EC*ddEA*op(dad0,dad0) + EA*CMat3d::Identity(t00) + dEA*op(dad0,dECd0)*2.0;
  CMat3d ddEd0d1 = EC*dEA*ddad0d1 + EC*ddEA*op(dad0,dad1) + EA*CMat3d::Identity(t01) + dEA*op(dad0,dECd1) + dEA*op(dad1,dECd0);
  CMat3d ddEd0d2 = EC*dEA*ddad0d2 + EC*ddEA*op(dad0,dad2) + EA*CMat3d::Identity(t02) + dEA*op(dad0,dECd2) + dEA*op(dad2,dECd0);
  CMat3d ddEd1d0 = EC*dEA*ddad1d0 + EC*ddEA*op(dad1,dad0) + EA*CMat3d::Identity(t01) + dEA*op(dad1,dECd0) + dEA*op(dad0,dECd1);
  CMat3d ddEd1d1 = EC*dEA*ddad1d1 + EC*ddEA*op(dad1,dad1) + EA*CMat3d::Identity(t11) + dEA*op(dad1,dECd1)*2.0;
  CMat3d ddEd1d2 = EC*dEA*ddad1d2 + EC*ddEA*op(dad1,dad2) + EA*CMat3d::Identity(t12) + dEA*op(dad1,dECd2) + dEA*op(dad2,dECd1);
  CMat3d ddEd2d0 = EC*dEA*ddad2d0 + EC*ddEA*op(dad2,dad0) + EA*CMat3d::Identity(t02) + dEA*op(dad2,dECd0) + dEA*op(dad0,dECd2);
  CMat3d ddEd2d1 = EC*dEA*ddad2d1 + EC*ddEA*op(dad2,dad1) + EA*CMat3d::Identity(t12) + dEA*op(dad2,dECd1) + dEA*op(dad1,dECd2);
  CMat3d ddEd2d2 = EC*dEA*ddad2d2 + EC*ddEA*op(dad2,dad2) + EA*CMat3d::Identity(t22) + dEA*op(dad2,dECd2)*2.0;
  
  for(int idim=0;idim<3;++idim){
    for(int jdim=0;jdim<3;++jdim){
      ddE[0][0][idim][jdim] = ddEd0d0(idim,jdim);
      ddE[0][1][idim][jdim] = ddEd0d1(idim,jdim);
      ddE[0][2][idim][jdim] = ddEd0d2(idim,jdim);
      ddE[1][0][idim][jdim] = ddEd1d0(idim,jdim);
      ddE[1][1][idim][jdim] = ddEd1d1(idim,jdim);
      ddE[1][2][idim][jdim] = ddEd1d2(idim,jdim);
      ddE[2][0][idim][jdim] = ddEd2d0(idim,jdim);
      ddE[2][1][idim][jdim] = ddEd2d1(idim,jdim);
      ddE[2][2][idim][jdim] = ddEd2d2(idim,jdim);
    }
  }
}




DFM2_INLINE double delfem2::W_ArapEnergy
(const std::vector<double>& aXYZ0,
 const std::vector<double>& aXYZ1,
 const std::vector<double>& aQuat1,
 const std::vector<unsigned int>& psup_ind,
 const std::vector<unsigned int>& psup)
{
  const unsigned int np = aXYZ0.size()/3;
  assert( aXYZ1.size() == np*3 );
  assert( aQuat1.size() == np*4 );
  assert( psup_ind.size() == np+1 );
  double w = 0.0;
  for(unsigned int ip=0;ip<np;++ip){
    CVec3d Pi(aXYZ0.data()+ip*3);
    CVec3d pi(aXYZ1.data()+ip*3);
    CQuatd Qi(aQuat1.data()+4*ip);
    for(unsigned int ipsup=psup_ind[ip];ipsup<psup_ind[ip+1];++ipsup){
      const unsigned int jp = psup[ipsup];
      const CVec3d v0 = Qi*(CVec3d(aXYZ0.data()+jp*3)-Pi);
      CVec3d pj(aXYZ1.data()+jp*3);
      const CVec3d v1 = pj-pi;
      CVec3d v = v0-v1;
      w += v*v;
      //      w += v1*v1;
    }
  }
  return 0.5*w;
}

DFM2_INLINE void delfem2::dW_ArapEnergy
(std::vector<double>& aRes,
 const std::vector<double>& aXYZ0,
 const std::vector<double>& aXYZ1,
 const std::vector<double>& aQuat1,
 const std::vector<unsigned int>& psup_ind,
 const std::vector<unsigned int>& psup)
{
  const unsigned int np = aXYZ0.size()/3;
  assert( aXYZ1.size() == np*3 );
  aRes.assign(np*3, 0.0);
  for(unsigned int ip=0;ip<np;++ip){
    const CVec3d Pi(aXYZ0.data()+ip*3);
    const CVec3d pi(aXYZ1.data()+ip*3);
    const CQuatd Qi(aQuat1.data()+ip*4);
    CMat3d LM; LM.SetZero();
    for(unsigned int ipsup=psup_ind[ip];ipsup<psup_ind[ip+1];++ipsup){
      const unsigned int jp = psup[ipsup];
      const CVec3d v0 = Qi*(CVec3d(aXYZ0.data()+jp*3)-Pi);
      CVec3d pj(aXYZ1.data()+jp*3);
      const CVec3d v1 = pj-pi;
      const CVec3d r = -(v1-v0);
      //      const CVec3d r = -v1;
      r.AddToScale(aRes.data()+ip*3, +1);
      r.AddToScale(aRes.data()+jp*3, -1);
    }
  }
}

DFM2_INLINE void delfem2::ddW_ArapEnergy
 (std::vector<double>& eM,
  const std::vector<unsigned int>& aIP,
  const std::vector<double>& aXYZ0,
  const std::vector<double>& aQuat1)
{
  const unsigned int nIP = aIP.size();
  const unsigned int nNg = nIP-1; // number of neighbor
  unsigned int ip = aIP[nNg];
  const CVec3d Pi(aXYZ0.data()+ip*3);
  CMat3d LM; LM.SetZero();
  for(unsigned int iip=0;iip<nNg;++iip){
    const unsigned int jp = aIP[iip];
    const CVec3d v0 = (CVec3d(aXYZ0.data()+jp*3)-Pi);
    LM += Mat3_CrossCross(v0);
  }
  CMat3d LMi = LM.Inverse();
  CMat3d Mrot = CMat3d::Quat(aQuat1.data()+ip*4);
  //    LMi = R*LMi*R.Trans();
  eM.assign(nIP*nIP*9, 0.0);
  for(unsigned int jjp=0;jjp<nNg;++jjp){
    for(unsigned int kkp=0;kkp<nNg;++kkp){
      const CVec3d vj = (CVec3d(aXYZ0.data()+aIP[jjp]*3)-Pi);
      const CVec3d vk = (CVec3d(aXYZ0.data()+aIP[kkp]*3)-Pi);
      CMat3d L1 = Mrot*CMat3d::Spin(vk.p)*LMi*CMat3d::Spin(vj.p)*Mrot.Trans();
      //        L1 = CMat3d::Spin(vk.p)*LMi*CMat3d::Spin(vj.p);
      L1.AddToScale(eM.data()+(kkp*nIP+jjp)*9, -1.0);
      L1.AddToScale(eM.data()+(nNg*nIP+nNg)*9, -1.0);
      L1.AddToScale(eM.data()+(nNg*nIP+jjp)*9, +1.0);
      L1.AddToScale(eM.data()+(kkp*nIP+nNg)*9, +1.0);
    }
    {
      CMat3d L1 = CMat3d::Identity();
      L1.AddToScale(eM.data()+(jjp*nIP+jjp)*9, +1.0);
      L1.AddToScale(eM.data()+(nNg*nIP+nNg)*9, +1.0);
      L1.AddToScale(eM.data()+(nNg*nIP+jjp)*9, -1.0);
      L1.AddToScale(eM.data()+(jjp*nIP+nNg)*9, -1.0);
    }
  }
}



template <typename T>
DFM2_INLINE void delfem2::GetConstConstDiff_Bend(
    double& C, CVec3<T> dC[4],
    const CVec3<T>& p0,
    const CVec3<T>& p1,
    const CVec3<T>& p2,
    const CVec3<T>& p3)
{
  const CVec3<T> v02 = p2-p0;
  const CVec3<T> v03 = p3-p0;
  const CVec3<T> v12 = p2-p1;
  const CVec3<T> v13 = p3-p1;
  const CVec3<T> v23 = p3-p2;
  ////
  const CVec3<T> A = v02^v03;
  const CVec3<T> B = v13^v12;
  const double lA = A.Length();
  const double lB = B.Length();
  const CVec3<T> a = A/lA;
  const CVec3<T> b = B/lB;
  const double ab = a*b;
  //  C = acos(ab);
  C = ab-1;
  const double sab = 1.0;//-1.0/sin(C);
  const CVec3<T> tmpBA = (b-a*(a*b))*(sab/lA);
  const CVec3<T> tmpAB = (a-b*(b*a))*(sab/lB);
  dC[0] = (tmpBA^v23);
  dC[1] = (v23^tmpAB);
  dC[2] = (v03^tmpBA) + (tmpAB^v13);
  dC[3] = (tmpBA^v02) + (v12^tmpAB);
}
template void delfem2::GetConstConstDiff_Bend(
    double&,
    CVec3d dC[4],
    const CVec3d&,
    const CVec3d&,
    const CVec3d&,
    const CVec3d&);
