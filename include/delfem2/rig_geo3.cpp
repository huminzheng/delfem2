/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <map>
#include <cassert>
#include <sstream>
#include <fstream>
#include "delfem2/geo3_v23m34q.h"
#include "delfem2/rig_geo3.h"

#ifndef M_PI 
#  define M_PI 3.1415926535
#endif

// ------------------------------------------------------------

namespace delfem2 {
namespace rig_v3q {

DFM2_INLINE double myStod(const std::string& str){
  char* e;
  double d = std::strtod(str.c_str(),&e);
  return d;
}

// probably std::stroi is safer to use but it is only for C++11
DFM2_INLINE int myStoi(const std::string& str){
  char* e;
  long d = std::strtol(str.c_str(),&e,0);
  return (int)d;
}

DFM2_INLINE bool isActive_AABB(const double aabb[6]){
    return aabb[0] <= aabb[1];
}

DFM2_INLINE void copy_AABB(double aabb[6], const double aabb0[6]){
  if( aabb == aabb0 ) return;
  for(int i=0;i<6;++i){ aabb[i] = aabb0[i]; }
}

DFM2_INLINE void myAdd_AABB(double aabb[6], const double aabb0[6], const double aabb1[6])
{
  if( !isActive_AABB(aabb0) && !isActive_AABB(aabb1) ){ aabb[0]=1; aabb[1]=-1; return; }
  if( !isActive_AABB(aabb0) ){ copy_AABB(aabb, aabb1); return; }
  if( !isActive_AABB(aabb1) ){ copy_AABB(aabb, aabb0); return; }
  aabb[0] = ( aabb0[0] < aabb1[0] ) ? aabb0[0] : aabb1[0];
  aabb[1] = ( aabb0[1] > aabb1[1] ) ? aabb0[1] : aabb1[1];
  aabb[2] = ( aabb0[2] < aabb1[2] ) ? aabb0[2] : aabb1[2];
  aabb[3] = ( aabb0[3] > aabb1[3] ) ? aabb0[3] : aabb1[3];
  aabb[4] = ( aabb0[4] < aabb1[4] ) ? aabb0[4] : aabb1[4];
  aabb[5] = ( aabb0[5] > aabb1[5] ) ? aabb0[5] : aabb1[5];
}

DFM2_INLINE void CalcInvMat(double* a, const int n, int& info )
{
  double tmp1;
  
  info = 0;
  int i,j,k;
  for(i=0;i<n;i++){
    if( fabs(a[i*n+i]) < 1.0e-30 ){
      info = 1;
      return;
    }
    if( a[i*n+i] < 0.0 ){
      info--;
    }
    tmp1 = 1.0 / a[i*n+i];
    a[i*n+i] = 1.0;
    for(k=0;k<n;k++){
      a[i*n+k] *= tmp1;
    }
    for(j=0;j<n;j++){
      if( j!=i ){
        tmp1 = a[j*n+i];
        a[j*n+i] = 0.0;
        for(k=0;k<n;k++){
          a[j*n+k] -= tmp1*a[i*n+k];
        }
      }
    }
  }
}

DFM2_INLINE  std::string MyReplace
 (const std::string& str,
  const char cf,
  const char ct)
{
  const size_t n = str.size();
  //
  std::string ss(str);
  for(unsigned int i=0;i<n;++i){
    if( ss[i] != cf ){ continue; }
    ss[i] = ct;
  }
  return ss;
}

DFM2_INLINE std::vector<std::string> MySplit
 (const std::string& str,
  char delimiter)
{
  std::vector<std::string> aToken;
  aToken.clear();
  std::stringstream data(str);
  std::string line;
  while(std::getline(data,line,delimiter)){
    if( line.empty() ){ continue; }
    aToken.push_back(line);
  }
  return aToken;
}

DFM2_INLINE double MyDotX(
                     const double* va,
                     const double* vb,
                     unsigned int n)
{
  double r = 0.0;
  for(unsigned int i=0;i<n;i++){ r += va[i]*vb[i]; }
  return r;
}

/*
static void MyMatMatX
(double* M, // [ni, nj]
 unsigned int ni, unsigned int nj,
 const double*A, // [ni, nk]
 unsigned int nk,
 const double* B) // [nk, nj]
{
  for(int i=0;i<ni;++i){
    for(int j=0;j<nj;++j){
      M[i*nj+j] = 0.0;
      for(int k=0;k<nk;++k){
        M[i*nj+j] += A[i*nk+k] * B[k*nj+j];
      }
    }
  }
}
 */

DFM2_INLINE  void MyMatMatTX
 (double* M, // [ni, nj]
  unsigned int ni, unsigned int nj,
  const double*A, // [ni, nk]
  unsigned int nk,
  const double* B) // [nj, nk]
{
  for(unsigned int i=0;i<ni;++i){
    for(unsigned int j=0;j<nj;++j){
      M[i*nj+j] = 0.0;
      for(unsigned int k=0;k<nk;++k){
        M[i*nj+j] += A[i*nk+k] * B[j*nk+k];
      }
    }
  }
}

/*
DFM2_INLINE void PickBone
    (int& ibone_selected,
     int& ielem_selected,
     const std::vector<CRigBone>& aBone,
     const CVec3d& src,
     const CVec3d& dir,
     double rad_hndlr,
     double tol)
{
  if( ibone_selected>=0 && ibone_selected<(int)aBone.size() ){
    const CRigBone& bone = aBone[ibone_selected];
    ielem_selected = bone.PickHandler(src,dir,rad_hndlr,tol);
  }
  else{
    ielem_selected = -1;
  }
  if( ielem_selected == -1 ){
    ibone_selected = -1;
    for(int ibone=0;ibone<(int)aBone.size();++ibone){
      delfem2::CVec3d pos(aBone[ibone].Pos());
      double distance = Distance(nearest_Line_Point(pos, src, dir),pos);
      if( distance < tol ){
        ibone_selected = ibone;
        break;
      }
    }
  }
}
 */


}
}

// ------------------------------------------------------------

/*
DFM2_INLINE int delfem2::CRigBone::PickHandler
(const CVec3d& org,
 const CVec3d& dir,
 double rad_handlr,
 double tol) const
{
  return PickHandlerRotation_Mat4(org,dir,
                                        affmat3Global, rad_handlr,
                                        tol);
}
 */

DFM2_INLINE void delfem2::CRigBone::SetRotationBryant
(double rx, double ry, double rz)
{
  Quat_Bryant(quatRelativeRot, rx, ry, rz);
}

DFM2_INLINE void delfem2::CRigBone::DeformSkin(double pos2[3],
                                const double pos0[3]) const 
{
  const double pos0a[4] = {pos0[0], pos0[1], pos0[2], 1.0};
  double pos1a[4]; MatVec4(pos1a,invBindMat,pos0a);
  double pos2a[4]; MatVec4(pos2a,affmat3Global,pos1a);
  pos2[0] = pos2a[0];
  pos2[1] = pos2a[1];
  pos2[2] = pos2a[2];
}

DFM2_INLINE void delfem2::CRigBone::SetTranslation
(double tx, double ty, double tz)
{
  this->transRelative[0] = tx;
  this->transRelative[1] = ty;
  this->transRelative[2] = tz;
}

DFM2_INLINE void delfem2::UpdateBoneRotTrans
(std::vector<CRigBone>& aBone)
{
  for(std::size_t ibone=0;ibone<aBone.size();++ibone){
    CMat4d m01 = CMat4d::Translate(aBone[ibone].transRelative);
    m01 = m01 * CMat4d::Quat(aBone[ibone].quatRelativeRot);
    m01 = m01 * CMat4d::Scale(aBone[ibone].scale);
    const int ibone_p = aBone[ibone].ibone_parent;
    if( ibone_p < 0 || ibone_p >= (int)aBone.size() ){ // root bone
      Copy_Mat4( aBone[ibone].affmat3Global, m01.mat );
      continue;
    }
    MatMat4(aBone[ibone].affmat3Global,
            aBone[ibone_p].affmat3Global, m01.mat);
  }
}



DFM2_INLINE void delfem2::Skinning_LBS_LocalWeight
(double* aXYZ,
 const double* aXYZ0,
 unsigned int nXYZ,
 const unsigned int* aTri,
 unsigned int nTri,
 const std::vector<CRigBone>& aBone,
 const double* aRigWeight,
 const unsigned int* aRigJoint)
{
  for(unsigned int ip=0;ip<nXYZ;++ip){
    double pos0[4] = {aXYZ0[ip*3+0],aXYZ0[ip*3+1],aXYZ0[ip*3+2],1.0};
    double pos1[3] = {0,0,0};
    double sum_w = 0.0;
    for(int iij=0;iij<4;++iij){
      double w = aRigWeight[ip*4+iij];
      if( w < 1.0e-30 ){ continue; }
      unsigned int ij = aRigJoint[ip*4+iij];
      sum_w += w;
      assert (ij<aBone.size());
      double pos0a[4]; MatVec4(pos0a,aBone[ij].invBindMat,pos0);
      double pos0b[4]; MatVec4(pos0b,aBone[ij].affmat3Global,pos0a);
      pos1[0] += w*pos0b[0];
      pos1[1] += w*pos0b[1];
      pos1[2] += w*pos0b[2];
    }
    assert( fabs(sum_w)>1.0e-10 );
    pos1[0] /= sum_w;
    pos1[1] /= sum_w;
    pos1[2] /= sum_w;
    aXYZ[ip*3+0] = pos1[0];
    aXYZ[ip*3+1] = pos1[1];
    aXYZ[ip*3+2] = pos1[2];
  }
}


DFM2_INLINE void delfem2::Skinning_LBS(
    std::vector<double>& aXYZ1,
    const std::vector<double>& aXYZ0,
    const std::vector<CRigBone>& aBone,
    const std::vector<double>& aW)
{
  const size_t nBone = aBone.size();
  const size_t nP = aXYZ0.size()/3;
  aXYZ1.resize(aXYZ0.size());
  assert( aW.size() == nBone*nP );
  for(unsigned int ip=0;ip<nP;++ip){
    const double* p0 = aXYZ0.data()+ip*3;
    double* p1 = aXYZ1.data()+ip*3;
    p1[0] = 0.0;  p1[1] = 0.0;  p1[2] = 0.0;
    for(unsigned int ibone=0;ibone<nBone;++ibone){
      double p2[3];
      aBone[ibone].DeformSkin(p2, p0);
      p1[0] += aW[ip*nBone+ibone]*p2[0];
      p1[1] += aW[ip*nBone+ibone]*p2[1];
      p1[2] += aW[ip*nBone+ibone]*p2[2];
    }
  }
}

// ---------

DFM2_INLINE void delfem2::SkinningSparseLBS(
    std::vector<double>& aXYZ1,
    const std::vector<double>& aXYZ0,
    const std::vector<CRigBone>& aBone,
    const std::vector<double>& aWBoneSparse,
    const std::vector<unsigned>& aIdBoneSparse)
{
//  const size_t nBone = aBone.size();
  const size_t nP = aXYZ0.size()/3;
  const unsigned int nBW = aWBoneSparse.size()/nP;
  assert( aWBoneSparse.size() == nBW*nP );
  assert( aIdBoneSparse.size() == nBW*nP );
  aXYZ1.resize(aXYZ0.size());
  for(unsigned int ip=0;ip<nP;++ip){
    const double* p0 = aXYZ0.data()+ip*3;
    double* p1 = aXYZ1.data()+ip*3;
    p1[0] = 0.0;  p1[1] = 0.0;  p1[2] = 0.0;
    for(unsigned int ibw=0;ibw<nBW;++ibw){
      const unsigned int ib0 =aIdBoneSparse[ip*nBW+ibw];
      const double w0 =aWBoneSparse[ip*nBW+ibw];
      double p2[3]; aBone[ib0].DeformSkin(p2, p0);
      p1[0] += w0*p2[0];
      p1[1] += w0*p2[1];
      p1[2] += w0*p2[2];
    }
  }
}

// -------

void delfem2::SparsifySkinningWeight(
    std::vector<double>& aWBone_RigSparse,
    std::vector<unsigned int>& aIdBone_RigSparse,
    const double* aW,
    unsigned int np,
    unsigned int nb,
    double thres)
{
  unsigned int nbone_nonzeroweight = 0;
  for(unsigned int ip=0;ip< np;++ip) {
    unsigned icnt = 0;
    for(unsigned int ib=0;ib<nb;++ib) {
      if( aW[ip * nb + ib] < thres ){ continue; }
      icnt++;
    }
    if( icnt > nbone_nonzeroweight ){ nbone_nonzeroweight = icnt; }
  }
  aWBone_RigSparse.resize(nbone_nonzeroweight*np);
  aIdBone_RigSparse.resize(nbone_nonzeroweight*np);
  for(unsigned int ip=0;ip< np;++ip) {
    unsigned icnt = 0;
    double w_sum = 0.0;
    for(unsigned int ib=0;ib<nb;++ib) {
      if( aW[ip * nb + ib] < thres ){ continue; }
      w_sum += aW[ip * nb + ib];
      aWBone_RigSparse[ip*nbone_nonzeroweight+icnt] = aW[ip * nb + ib];
      aIdBone_RigSparse[ip*nbone_nonzeroweight+icnt] = ib;
      icnt++;
    }
    if( icnt > nbone_nonzeroweight ){ nbone_nonzeroweight = icnt; }
  }
}


// ------------------------------------
// from here BioVisionHierarchy

DFM2_INLINE void delfem2::Read_BioVisionHierarchy
(std::vector<CRigBone>& aBone,
 std::vector<CChannel_BioVisionHierarchy>& aChannelRotTransBone,
 int& nframe,
 std::vector<double>& aValueRotTransBone,
 const std::string& path_bvh)
{
  std::ifstream fin;
  fin.open(path_bvh.c_str());
  if( !fin.is_open() ){
    std::cout << "cannot open file" << std::endl;
    return;
  }
  aBone.clear();
  aChannelRotTransBone.clear();
  //
  std::string line;
  std::vector<int> stackIndBone;
  while(std::getline(fin,line)){
    if (line[line.size()-1] == '\n') line.erase(line.size()-1); // remove the newline code
    if (line[line.size()-1] == '\r') line.erase(line.size()-1); // remove the newline code
    line = rig_v3q::MyReplace(line, '\t', ' ');
    std::vector<std::string> aToken = rig_v3q::MySplit(line,' ');
//    std::cout << aToken[0] << std::endl;
    if( aToken[0] == "HIERARCHY" ){
      assert(aBone.empty());
    }
    else if( aToken[0] == "ROOT" ){
      assert(aBone.size()==0);
      CRigBone br;
      assert( aToken.size() == 2 );
      br.name = aToken[1];
      aBone.push_back(br);
    }
    else if( aToken[0] == "{" ){
      stackIndBone.push_back(aBone.size()-1);
      if( stackIndBone.size() > 1 ){
        int ibp = stackIndBone[stackIndBone.size()-2];
        int ib = aBone.size()-1;
        aBone[ib].ibone_parent  = ibp;
      }
    }
    else if( aToken[0] == "}" ){
      stackIndBone.resize(stackIndBone.size()-1);
    }
    else if( aToken[0] == "OFFSET"){
      assert( aToken.size()==4 );
      int ib = aBone.size()-1;
      double org_x = rig_v3q::myStod(aToken[1]);
      double org_y = rig_v3q::myStod(aToken[2]);
      double org_z = rig_v3q::myStod(aToken[3]);
      aBone[ib].invBindMat[ 3] = -org_x;
      aBone[ib].invBindMat[ 7] = -org_y;
      aBone[ib].invBindMat[11] = -org_z;
      if( stackIndBone.size() > 1 ){
        const int ibp = stackIndBone[stackIndBone.size()-2];
        assert(ibp<(int)aBone.size());
        aBone[ib].invBindMat[ 3] += aBone[ibp].invBindMat[ 3];
        aBone[ib].invBindMat[ 7] += aBone[ibp].invBindMat[ 7];
        aBone[ib].invBindMat[11] += aBone[ibp].invBindMat[11];
      }
    }
    else if( aToken[0] == "CHANNELS" ){
      assert(aToken.size()>=2);
      int nch = rig_v3q::myStoi(aToken[1]);
      assert((int)aToken.size()==nch+2);
      assert( !aBone.empty() );
      const std::size_t ib = aBone.size()-1;
      for(int ich=0;ich<nch;++ich){
        const std::string& type_ch = aToken[ich+2];
        if(      type_ch == "Xposition" ){ aChannelRotTransBone.emplace_back(ib,0,false ); }
        else if( type_ch == "Yposition" ){ aChannelRotTransBone.emplace_back(ib,1,false ); }
        else if( type_ch == "Zposition" ){ aChannelRotTransBone.emplace_back(ib,2,false ); }
        else if( type_ch == "Xrotation" ){ aChannelRotTransBone.emplace_back(ib,0,true ); }
        else if( type_ch == "Yrotation" ){ aChannelRotTransBone.emplace_back(ib,1,true ); }
        else if( type_ch == "Zrotation" ){ aChannelRotTransBone.emplace_back(ib,2,true ); }
        else{
          std::cout << "ERROR-->undefiend type" << std::endl;
        }
      }
    }
    else if( aToken[0] == "JOINT" ){
      CRigBone br;
      assert( aToken.size() == 2 );
      br.name = aToken[1];
      aBone.push_back(br);
    }
    else if( aToken[0] == "End" ){
      assert(aToken[1] == "Site");
      CRigBone br;
      assert( aToken.size() == 2 );
      br.name = aToken[1];
      aBone.push_back(br);
    }
    else if( aToken[0] == "MOTION"){
      break;
    }
  }
  nframe = 0;
  {
    std::string stmp0;
    {
      std::getline(fin,line);
      std::stringstream ss(line);
      ss >> stmp0 >> nframe;
//      std::cout << "frame: " << nframe << std::endl;
    }
    std::getline(fin,line);
//    std::cout << "frametime: " << line << std::endl;
  }
  const int nchannel = aChannelRotTransBone.size();
  aValueRotTransBone.resize(nframe*nchannel);
  for(int iframe=0;iframe<nframe;++iframe){
    std::getline(fin,line);
    line = rig_v3q::MyReplace(line, '\t', ' ');
    if (line[line.size()-1] == '\n') line.erase(line.size()-1); // remove the newline code
    if (line[line.size()-1] == '\r') line.erase(line.size()-1); // remove the newline code
    std::vector<std::string> aToken = rig_v3q::MySplit(line,' ');
//    std::cout << aToken.size() << " " << aChannelRotTransBone.size() << std::endl;
    assert(aToken.size()==aChannelRotTransBone.size());
    for(int ich=0;ich<nchannel;++ich){
      aValueRotTransBone[iframe*nchannel+ich] = rig_v3q::myStod(aToken[ich]);
    }
  }
  // ---------------
  for(std::size_t ibone=0;ibone<aBone.size();++ibone){
    CRigBone& bone = aBone[ibone];
    bone.scale = 1.0;
    bone.quatRelativeRot[0] = 1.0;
    bone.quatRelativeRot[1] = 0.0;
    bone.quatRelativeRot[2] = 0.0;
    bone.quatRelativeRot[3] = 0.0;
    bone.transRelative[0] = 0.0;
    bone.transRelative[1] = 0.0;
    bone.transRelative[2] = 0.0;
    if( bone.ibone_parent != -1 ){
      const CRigBone& bone_p = aBone[bone.ibone_parent];
      bone.transRelative[0] = (-bone.invBindMat[ 3])-(-bone_p.invBindMat[ 3]);
      bone.transRelative[1] = (-bone.invBindMat[ 7])-(-bone_p.invBindMat[ 7]);
      bone.transRelative[2] = (-bone.invBindMat[11])-(-bone_p.invBindMat[11]);
    }
  }
  for(auto & bone : aBone){
    for(int i=0;i<16;++i){ bone.affmat3Global[i] = bone.invBindMat[i]; }
    int info; rig_v3q::CalcInvMat(bone.affmat3Global, 4, info);
  }
}


DFM2_INLINE void delfem2::SetPose_BioVisionHierarchy
(std::vector<CRigBone>& aBone,
 const std::vector<CChannel_BioVisionHierarchy>& aChannelRotTransBone,
 const double *aVal)
{
  for(auto & bone : aBone){
    bone.quatRelativeRot[0] = 1.0;
    bone.quatRelativeRot[1] = 0.0;
    bone.quatRelativeRot[2] = 0.0;
    bone.quatRelativeRot[3] = 0.0;
  }
  const int nch = aChannelRotTransBone.size();
  for(int ich=0;ich<nch;++ich){
    const int ibone = aChannelRotTransBone[ich].ibone;
    const int iaxis = aChannelRotTransBone[ich].iaxis;
    const bool isrot = aChannelRotTransBone[ich].isrot;
    const double val = aVal[ich];
    assert(ibone<(int)aBone.size());
    assert(iaxis>=0&&iaxis<3);
    if( !isrot ){
      aBone[ibone].transRelative[iaxis] = val;
    }
    else{
      const double ar = val*M_PI/180.0;
      double v0[3] = {0,0,0};
      v0[iaxis] = 1.0;
      double dq[4] = { cos(ar*0.5), v0[0]*sin(ar*0.5), v0[1]*sin(ar*0.5), v0[2]*sin(ar*0.5) };
      double qtmp[4]; QuatQuat(qtmp,
                                     aBone[ibone].quatRelativeRot, dq);
      Copy_Quat(aBone[ibone].quatRelativeRot,qtmp);
    }
  }
  UpdateBoneRotTrans(aBone);
}

// ----------------------------------

DFM2_INLINE void delfem2::Smpl2Rig(
              std::vector<CRigBone>& aBone,
              const std::vector<int>& aIndBoneParent,
              const std::vector<double>& aXYZ0,
              const std::vector<double>& aJntRgrs)
{
  const unsigned int nbone = aIndBoneParent.size();
  std::vector<double> aJntPos0;
  {
    const unsigned int nP = aXYZ0.size()/3;
    const unsigned int nBone = aIndBoneParent.size();
    aJntPos0.assign(nBone*3, 0.0);
    for(unsigned int ib=0;ib<nBone;++ib){
      aJntPos0[ib*3+0] = 0;
      aJntPos0[ib*3+1] = 0;
      aJntPos0[ib*3+2] = 0;
      for(unsigned int ip=0;ip<nP;++ip){
        aJntPos0[ib*3+0] += aJntRgrs[ip*nBone+ib]*aXYZ0[ip*3+0];
        aJntPos0[ib*3+1] += aJntRgrs[ip*nBone+ib]*aXYZ0[ip*3+1];
        aJntPos0[ib*3+2] += aJntRgrs[ip*nBone+ib]*aXYZ0[ip*3+2];
      }
    }
  }
  aBone.resize(nbone);
  for(unsigned int ib=0;ib<nbone;++ib){
    int ibp = aIndBoneParent[ib];
    aBone[ib].ibone_parent = ibp;
    aBone[ib].invBindMat[ 3] = -aJntPos0[ib*3+0];
    aBone[ib].invBindMat[ 7] = -aJntPos0[ib*3+1];
    aBone[ib].invBindMat[11] = -aJntPos0[ib*3+2];
    if( ibp != -1 ){
      aBone[ib].transRelative[0] = +aJntPos0[ib*3+0] - aJntPos0[ibp*3+0];
      aBone[ib].transRelative[1] = +aJntPos0[ib*3+1] - aJntPos0[ibp*3+1];
      aBone[ib].transRelative[2] = +aJntPos0[ib*3+2] - aJntPos0[ibp*3+2];
    }
    else{
      aBone[ib].transRelative[0] = +aJntPos0[ib*3+0];
      aBone[ib].transRelative[1] = +aJntPos0[ib*3+1];
      aBone[ib].transRelative[2] = +aJntPos0[ib*3+2];
    }
  }
  UpdateBoneRotTrans(aBone);
}

// -------------------------------------------





DFM2_INLINE void delfem2::SetMat4AffineBone_FromJointRelativeRotation
 (std::vector<double>& aMat4AffineBone,
  const double trans_root[3],
  const std::vector<double>& aQuatRelativeRot,
  const std::vector<int>& aIndBoneParent,
  const std::vector<double>& aJntPos0)
{
  const unsigned int nBone = aIndBoneParent.size();
  assert( nBone >= 1 );
  assert( aMat4AffineBone.size() == nBone*16 );
  Mat4_ScaleRotTrans(aMat4AffineBone.data(),
                           1.0, aQuatRelativeRot.data(), trans_root);
  for(unsigned int ibone=1;ibone<nBone;++ibone){
    int ibp = aIndBoneParent[ibone];
    assert( ibp >= 0 && ibp < (int)nBone );
    // inv binding mat
    double p1[3] = {aJntPos0[ibone*3+0], aJntPos0[ibone*3+1], aJntPos0[ibone*3+2]};
    CMat4<double> M0, M1, M2;
    M0.Set_AffineTranslate(-p1[0], -p1[1], -p1[2]);
    Mat4_Quat(M1.mat,
                    aQuatRelativeRot.data()+ibone*4);
    M2.Set_AffineTranslate(+p1[0], +p1[1], +p1[2]);
    CMat4<double> M3 = M1.MatMat(M0);
    CMat4<double> M4 = M2.MatMat(M3);
    MatMat4(aMat4AffineBone.data()+ibone*16,
                  aMat4AffineBone.data()+ibp*16,
                  M4.mat);
  }
}


DFM2_INLINE void delfem2::Rig_SkinReferncePositionsBoneWeighted
 (std::vector<double>& aRefPosAff,  // [ np, nBone*4 ]
  const std::vector<delfem2::CRigBone> aBone1,
  const std::vector<double>& aXYZ0,
  const std::vector<double>& aW)
{
  const unsigned int np = aXYZ0.size()/3;
  const unsigned int nb = aBone1.size();
  aRefPosAff.resize(np*nb*4);
  for(unsigned int ip=0;ip<np;++ip){
    double p0a[4] = {aXYZ0[ip*3+0], aXYZ0[ip*3+1], aXYZ0[ip*3+2], 1.0};
    for(unsigned int ib=0;ib<nb;++ib){
      double p0b[4]; delfem2::MatVec4(p0b,
                                   aBone1[ib].invBindMat, p0a);
      aRefPosAff[ip*(nb*4)+ib*4+0] = aW[ip*nb+ib]*p0b[0];
      aRefPosAff[ip*(nb*4)+ib*4+1] = aW[ip*nb+ib]*p0b[1];
      aRefPosAff[ip*(nb*4)+ib*4+2] = aW[ip*nb+ib]*p0b[2];
      aRefPosAff[ip*(nb*4)+ib*4+3] = aW[ip*nb+ib];
    }
  }
}





// --------------------------------------


DFM2_INLINE void delfem2::CTarget::WdW
 (std::vector<double>& aW,
  std::vector<double>& adW,
  const std::vector<CRigBone>& aBone,
  std::vector<double>& aL) const // [ [nb, 3],  [ndim(3), nBone, ndim(4)] ]
{
  const CVec3d p0 = aBone[ib].Pos();
  const unsigned int ncnst = 2;
  {
    double sqx = pos.x()-p0.x();
    double sqy = pos.y()-p0.y();
    aW.push_back(sqx);
    aW.push_back(sqy);
  }
  const unsigned int nb = aBone.size();
  const unsigned int istat = adW.size();
  adW.resize(istat+ncnst*nb*3);
  for(unsigned int ibs=0;ibs<nb;++ibs){
    for(int idims=0;idims<3;++idims){
      double dx = aL[(idims*nb+ibs)*(3*nb*4)+0*(nb*4)+ib*4+3];
      double dy = aL[(idims*nb+ibs)*(3*nb*4)+1*(nb*4)+ib*4+3];
//      double dz = aL[(idims*nb+ibs)*(3*nb*4)+2*(nb*4)+ib*4+3];
      adW[istat+0*(nb*3)+ibs*3+idims] = -dx;
      adW[istat+1*(nb*3)+ibs*3+idims] = -dy;
    }
  }
}

DFM2_INLINE void delfem2::Rig_SensitivityBoneTransform
(double* aL, // [ ndim(3), nBone, ndim(4) ]
 unsigned int ib_s,
 unsigned int idim_s,
 const std::vector<CRigBone> aBone1)
{
  const unsigned int nb = aBone1.size();
  std::vector<double> aM(nb*16);
  {
    for(std::size_t ibone=0;ibone<aBone1.size();++ibone){
      CMat4d m01 = CMat4d::Scale(aBone1[ibone].scale);
      m01 = CMat4d::Quat(aBone1[ibone].quatRelativeRot) * m01;
      if( ibone == ib_s ){
        CMat3d dn1 = CMat3d::Spin(CVec3d::Axis(idim_s).p) + CMat3d::Identity();
        CMat4d dm1 = CMat4d::Mat3(dn1.mat);
        m01 = dm1 * m01;
      }
      m01 = CMat4d::Translate(aBone1[ibone].transRelative) * m01;
      const int ibone_p = aBone1[ibone].ibone_parent;
      if( ibone_p < 0 || ibone_p >= (int)aBone1.size() ){ // root bone
        Copy_Mat4( aM.data()+ibone*16, m01.mat );
        continue;
      }
      MatMat4(aM.data()+ibone*16,
                    aM.data()+ibone_p*16, m01.mat);
    }
  }
  for(int idim=0;idim<3;++idim){
    for(std::size_t ib=0;ib<nb;++ib){
      for(int jdim=0;jdim<4;++jdim){
        aL[idim*nb*4+ib*4+jdim] = aM[ib*16+idim*4+jdim] - aBone1[ib].affmat3Global[idim*4+jdim];
      }
    }
  }
}


DFM2_INLINE void delfem2::Rig_SensitivityBoneTransform_Eigen
(std::vector<double>& Lx, // [ nsns, nBone*4 ]
 std::vector<double>& Ly, // [ nsns, nBone*4 ]
 std::vector<double>& Lz, // [ nsns, nBone*4 ]
 unsigned int ib_s,
 double idim_s,
 bool is_rot,
 const std::vector<CRigBone> aBone1)
{
  const unsigned int nb = aBone1.size();
  unsigned int istat = Lx.size();
  assert( Ly.size() == istat );
  assert( Lz.size() == istat );
  Lx.resize(istat+nb*4);
  Ly.resize(istat+nb*4);
  Lz.resize(istat+nb*4);
  std::vector<double> aM(nb*16);
  
  for(std::size_t ibone=0;ibone<aBone1.size();++ibone){
    CMat4d m01 = CMat4d::Scale(aBone1[ibone].scale);
    m01 = CMat4d::Quat(aBone1[ibone].quatRelativeRot) * m01;
    if( ibone == ib_s && is_rot ){
      CMat3d dn1 = CMat3d::Spin(CVec3d::Axis(idim_s).p) + CMat3d::Identity();
      CMat4d dm1 = CMat4d::Mat3(dn1.mat);
      m01 = dm1 * m01;
    }
    m01 = CMat4d::Translate(aBone1[ibone].transRelative) * m01;
    if( ibone == ib_s && !is_rot ){
      m01 = CMat4d::Translate(CVec3d::Axis(idim_s).p) * m01;
    }
    const int ibone_p = aBone1[ibone].ibone_parent;
    if( ibone_p < 0 || ibone_p >= (int)aBone1.size() ){ // root bone
      Copy_Mat4( aM.data()+ibone*16, m01.mat );
      continue;
    }
    MatMat4(aM.data()+ibone*16,
                  aM.data()+ibone_p*16, m01.mat);
  }
  for(std::size_t ib=0;ib<nb;++ib){
    for(int jdim=0;jdim<4;++jdim){
      Lx[istat+ib*4+jdim] = aM[ib*16+0*4+jdim] - aBone1[ib].affmat3Global[0*4+jdim];
      Ly[istat+ib*4+jdim] = aM[ib*16+1*4+jdim] - aBone1[ib].affmat3Global[1*4+jdim];
      Lz[istat+ib*4+jdim] = aM[ib*16+2*4+jdim] - aBone1[ib].affmat3Global[2*4+jdim];
    }
  }
  
}




DFM2_INLINE void delfem2::Rig_SensitivitySkin_BoneRotation
 (std::vector<double>& aSns, // [nb*3, np*ndim(3)]
  const std::vector<CRigBone> aBone1,
  const std::vector<double>& aXYZ0,
  const std::vector<double>& aW,
  const std::vector<double>& aL) // [ [3, nb],  ndim(3),  [nBone, ndim(4)]] ]
{
  std::vector<double> aRefPos; // [np, nb*4]
  Rig_SkinReferncePositionsBoneWeighted(aRefPos,
                                        aBone1, aXYZ0, aW);
  // -------------
  //  std::vector<double> aL; // 3*nb*4
  const unsigned int nb = aBone1.size();
  const unsigned int np = aXYZ0.size()/3;
  aSns.resize(nb*3 * np*3);
  for(unsigned int ib_s=0;ib_s<nb;++ib_s){
    for(unsigned int idim_s=0;idim_s<3;++idim_s){
      for(unsigned int ip=0;ip<np;++ip){
        for(int idim=0;idim<3;++idim){
          aSns[(np*3)*(ib_s*3+idim_s)+(ip*3+idim)] = rig_v3q::MyDotX(aL.data()+(idim_s*nb+ib_s)*(3*nb*4)+idim*nb*4,
                                                                     aRefPos.data()+ip*nb*4,
                                                                     nb*4);
        }
      }
    }
  }
}

/*
 void Rig_SkinReferncePositionsBoneWeighted_Eigen
 (Eigen::MatrixXd& emRefPosAff,
 const std::vector<CRigBone> aBone1,
 const std::vector<double>& aXYZ0,
 const std::vector<double>& aW)
 {
 const unsigned int np = aXYZ0.size()/3;
 const unsigned int nb = aBone1.size();
 emRefPosAff.resize(np, nb*4);
 for(int ip=0;ip<np;++ip){
 double p0a[4] = {aXYZ0[ip*3+0], aXYZ0[ip*3+1], aXYZ0[ip*3+2], 1.0};
 for(int ib=0;ib<nb;++ib){
 double p0b[4]; MatVec4(p0b,
 aBone1[ib].invBindMat, p0a);
 emRefPosAff(ip, ib*4+0) = aW[ip*nb+ib]*p0b[0];
 emRefPosAff(ip, ib*4+1) = aW[ip*nb+ib]*p0b[1];
 emRefPosAff(ip, ib*4+2) = aW[ip*nb+ib]*p0b[2];
 emRefPosAff(ip, ib*4+3) = aW[ip*nb+ib];
 }
 }
 }
 */



DFM2_INLINE void delfem2::Rig_SensitivitySkin_BoneRotation_Eigen
(std::vector<double>& dSkinX, // [ np, nsns ]
 std::vector<double>& dSkinY, // [ np, nsns ]
 std::vector<double>& dSkinZ, // [ np, nsns ]
 const std::vector<CRigBone>& aBone1,
 const std::vector<double>& aXYZ0,
 const std::vector<double>& aW,
 const std::vector<double>& Lx, // [ nsns, nBone*4 ]
 const std::vector<double>& Ly, // [ nsns, nBone*4 ]
 const std::vector<double>& Lz) // [ nsns, nBone*4 ]
{
  const unsigned int nb = aBone1.size();
  const unsigned int np = aXYZ0.size()/3;
  const unsigned int nsns = nb*3;
  assert( Lx.size() == nb*4 * nsns );
  assert( Ly.size() == nb*4 * nsns );
  assert( Lz.size() == nb*4 * nsns );
  
  std::vector<double> aRefPos; // [ np, nBone*4 ]
  Rig_SkinReferncePositionsBoneWeighted(aRefPos,
                                        aBone1,aXYZ0,aW);
  
  dSkinX.resize(np*nsns);
  dSkinY.resize(np*nsns);
  dSkinZ.resize(np*nsns);
  rig_v3q::MyMatMatTX(dSkinX.data(),
                      np, nsns, aRefPos.data(), nb*4, Lx.data());
  rig_v3q::MyMatMatTX(dSkinY.data(),
                      np, nsns, aRefPos.data(), nb*4, Ly.data());
  rig_v3q::MyMatMatTX(dSkinZ.data(),
                      np, nsns, aRefPos.data(), nb*4, Lz.data());
}


DFM2_INLINE void delfem2::Rig_WdW_Target_Eigen
 (std::vector<double>& aW,
  std::vector<double>& adW,
  const std::vector<CRigBone>& aBone,
  const CTarget& target,
  const std::vector<double>& Lx, // [ nsns, nBone*4 ]
  const std::vector<double>& Ly, // [ nsns, nBone*4 ]
  const std::vector<double>& Lz) // [ nsns, nBone*4 ]
{
  const unsigned int nb = aBone.size();
  const unsigned int nsns = Lx.size()/(nb*4);
  assert( Lx.size() == nsns*nb*4 );
  assert( Ly.size() == nsns*nb*4 );
  assert( Lz.size() == nsns*nb*4 );
  // --------
  unsigned int ib = target.ib;
  const CVec3d pos = target.pos;
  // ---------------
  const CVec3d p0 = aBone[ib].Pos();
  const unsigned int ncnst = 2;
  {
    double sqx = pos.x()-p0.x();
    double sqy = pos.y()-p0.y();
    aW.push_back(sqx);
    aW.push_back(sqy);
  }
  const unsigned int istat = adW.size();
  adW.resize(istat+ncnst*nsns);
  for(unsigned int isns=0;isns<nsns;++isns){
    double dx = Lx[isns*(nb*4) + ib*4+3];
    double dy = Ly[isns*(nb*4) + ib*4+3];
//    double dz = Lz[isns*(nb*4) + ib*4+3];
    adW[istat+0*nsns+isns] = -dx;
    adW[istat+1*nsns+isns] = -dy;
  }
}


