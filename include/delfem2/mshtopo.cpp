/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <vector>
#include <cassert>
#include <iterator>
#include <stack>
#include <set>
#include <iostream>
#include <climits>

#include "delfem2/mshtopo.h"

// ---------------------------------------------

DFM2_INLINE void delfem2::JArray_Print
(const std::vector<int>& index,
 const std::vector<int>& array)
{
  assert( index.size() >= 2 );
  const std::size_t np = index.size()-1;
  for(unsigned int ip=0;ip<np;++ip){
    std::cout << ip << " --> ";
    for(int ipsup=index[ip];ipsup<index[ip+1];++ipsup){
      std::cout << array[ipsup] << " ";
    }
    std::cout << std::endl;
  }
}

DFM2_INLINE void delfem2::JArray_Sort
(const std::vector<unsigned int>& index,
 std::vector<unsigned int>& array)
{
  if( index.empty() ) return;
  const int size = (int)index.size()-1;
  for(int ipoin=0;ipoin<size;ipoin++){
    const int is = index[ipoin  ];
    const int ie = index[ipoin+1];
    if( is == ie ) continue;
    assert( is < ie );
    int itmp;
    for(int i=is;i<ie-1;i++){
      for(int j=ie-1;j>i;j--){
        if( array[j] < array[j-1] ){
          itmp = array[j];
          array[j] = array[j-1];
          array[j-1] = itmp;
        }
      }
    }
  }
}

DFM2_INLINE void delfem2::JArray_Sort(
    const unsigned int* index,
    const unsigned int size,
    unsigned int* array)
{
  if( size == 0 ) return;
//  if( index.size() == 0 ) return;
//  const int size = (int)index.size()-1;
  for(unsigned int ipoin=0;ipoin<size;ipoin++){
    const unsigned int is = index[ipoin  ];
    const unsigned int ie = index[ipoin+1];
    if( is == ie ) continue;
    assert( is < ie );
    for(unsigned int i=is;i<ie-1;i++){
      for(int j=(int)ie-1;j>(int)i;j--){
        if( array[j] < array[j-1] ){
          unsigned int itmp = array[j];
          array[j] = array[j-1];
          array[j-1] = itmp;
        }
      }
    }
  }
}

DFM2_INLINE void delfem2::JArray_AddDiagonal
(std::vector<unsigned int> &psup_ind1,
 std::vector<unsigned int> &psup1,
 const unsigned int *psup_ind0, int npsup_ind0,
 const unsigned int *psup0, int npsup0)
{
  const int np = npsup_ind0-1;
  std::vector<int> tmp(np,-1);
  psup_ind1.assign(np+1,0);
  for(int ip=0;ip<np;++ip){
    for(std::size_t ipsup=psup_ind0[ip];ipsup<psup_ind0[ip+1];++ipsup){
      const unsigned int jp = psup0[ipsup];
      assert( tmp[jp] != ip );
      tmp[jp] = ip;
      psup_ind1[ip+1] += 1;
    }
    if( tmp[ip] != ip ){
      tmp[ip] = ip;
      psup_ind1[ip+1] += 1;
    }
  }
  for(int ip=0;ip<np;++ip){
    psup_ind1[ip+1] += psup_ind1[ip];
  }
  const int npsup = psup_ind1[np];
  psup1.resize(npsup);
  tmp.assign(np,-1);
  for(int ip=0;ip<np;++ip){
    for(std::size_t ipsup=psup_ind0[ip];ipsup<psup_ind0[ip+1];++ipsup){
      const unsigned int jp = psup0[ipsup];
      assert( tmp[jp] != ip );
      tmp[jp] = ip;
      int iclstr  = psup_ind1[ip];
      psup1[ iclstr ] = jp;
      psup_ind1[ip] += 1;
    }
    if( tmp[ip] != ip ){
      int iclstr  = psup_ind1[ip];
      psup1[ iclstr ] = ip;
      psup_ind1[ip] += 1;
    }
  }
  for(int ip=np-1;ip>=0;--ip){
    psup_ind1[ip+1] = psup_ind1[ip];
  }
  psup_ind1[0] = 0;
}

// in the edge ip -> jp, it holds (ip < jp)
DFM2_INLINE void delfem2::JArrayEdgeUnidir_PointSurPoint
(std::vector<unsigned int> &edge_ind,
 std::vector<unsigned int> &edge,
 //
 const std::vector<unsigned int> &psup_ind,
 const std::vector<unsigned int> &psup)
{
  assert( psup_ind.size() >= 2 );
  const std::size_t np = psup_ind.size()-1;
  edge_ind.resize(np+1);
  edge_ind[0] = 0;
  edge.clear();
  //
  for(unsigned int ip=0;ip<np;++ip){
    for(unsigned int ipsup=psup_ind[ip];ipsup<psup_ind[ip+1];++ipsup){
      unsigned int ip0 = psup[ipsup];
      if( ip0 <= ip ) continue;
      edge_ind[ip+1]++;
    }
  }
  for(unsigned int ip=0;ip<np;ip++){
    edge_ind[ip+1] += edge_ind[ip];
  }
  const unsigned int nedge = edge_ind[np];
  edge.resize(nedge);
  for(unsigned int ip=0;ip<np;++ip){
    for(unsigned int ipsup=psup_ind[ip];ipsup<psup_ind[ip+1];++ipsup){
      const unsigned int ip0 = psup[ipsup];
      if( ip0 <= ip ) continue;
      const int iedge = edge_ind[ip];
      edge[iedge] = ip0;
      edge_ind[ip]++;
    }
  }
  for(int ip=(int)np;ip>0;ip--){
    edge_ind[ip] = edge_ind[ip-1];
  }
  edge_ind[0] = 0;
}

DFM2_INLINE void delfem2::JArray_ElSuP_MeshElem
(std::vector<unsigned int> &elsup_ind,
 std::vector<unsigned int> &elsup,
 // ----------
 const unsigned int* pElem,
 unsigned int nElem,
 unsigned int nPoEl,
 unsigned int nPo)
{
  //  const int nElem = (int)aElem.size()/nPoEl;
  elsup_ind.assign(nPo+1,0);
  for(unsigned int ielem=0;ielem<nElem;ielem++){
    for(unsigned int inoel=0;inoel<nPoEl;inoel++){
      const unsigned int ino1 = pElem[ielem*nPoEl+inoel];
      elsup_ind[ino1+1] += 1;
    }
  }
  for(unsigned int ino=0;ino<nPo;++ino){
    elsup_ind[ino+1] += elsup_ind[ino];
  }
  unsigned int nelsup = elsup_ind[nPo];
  elsup.resize(nelsup);
  for(unsigned int ielem=0;ielem<nElem;ielem++){
    for(unsigned int inoel=0;inoel<nPoEl;inoel++){
      int unsigned ino1 = pElem[ielem*nPoEl+inoel];
      int ind1 = elsup_ind[ino1];
      elsup[ind1] = ielem;
      elsup_ind[ino1] += 1;
    }
  }
  for(int ino=(int)nPo;ino>=1;ino--){
    elsup_ind[ino] = elsup_ind[ino-1];
  }
  elsup_ind[0] = 0;
}

/**
 * @details compute 2-ring neighborhood from 1-ring neighborhood
 */
DFM2_INLINE void delfem2::JArray_Extend
(std::vector<unsigned int>& psup_ind1,
 std::vector<unsigned int>& psup1,
 const unsigned int *psup_ind0,
 unsigned int npsup_ind0,
 const unsigned int *psup0)
{
  const size_t np = npsup_ind0-1;
  psup_ind1.assign(np+1, 0);
  std::vector<unsigned > aflg(np,UINT_MAX);
  for(unsigned int ip=0;ip<np;++ip){
    for(unsigned int ipsup=psup_ind0[ip];ipsup<psup_ind0[ip+1];++ipsup){
      unsigned int jp0 = psup0[ipsup];
      for(unsigned int jpsup=psup_ind0[jp0];jpsup<psup_ind0[jp0+1];++jpsup){
        unsigned int kp0 = psup0[jpsup];
        if( aflg[kp0] == ip || kp0 == ip ){ continue; }
        ++psup_ind1[ip+1];
        aflg[kp0] = ip;
      }
    }
  }
  // ---------
  for(unsigned int ip=0;ip<np;++ip){
    psup_ind1[ip+1] += psup_ind1[ip];
  }
  psup1.resize(psup_ind1[np]);
  // ---------
  aflg.assign(np,UINT_MAX);
  for(unsigned int ip=0;ip<np;++ip){
    for(unsigned int ipsup=psup_ind0[ip];ipsup<psup_ind0[ip+1];++ipsup){
      unsigned int jp0 = psup0[ipsup];
      for(unsigned int jpsup=psup_ind0[jp0];jpsup<psup_ind0[jp0+1];++jpsup){
        unsigned int kp0 = psup0[jpsup];
        if( aflg[kp0] == ip || kp0 == ip ){ continue; }
        unsigned int kpsup = psup_ind1[ip];
        ++psup_ind1[ip];
        psup1[kpsup] = kp0;
        aflg[kp0] = ip;
      }
    }
  }
  for(int ip=np;ip>=1;--ip){
    psup_ind1[ip] = psup_ind1[ip-1];
  }
  psup_ind1[0] = 0;
}

// JArray
// -----------------------------------------------------------------
// Elem

DFM2_INLINE unsigned int delfem2::FindAdjEdgeIndex
(unsigned int itri0,
 unsigned int ied0,
 unsigned int jtri0,
 const unsigned int* aTri)
{
  const unsigned int iv0 = aTri[itri0*3+(ied0+1)%3];
  const unsigned int iv1 = aTri[itri0*3+(ied0+2)%3];
  assert( iv0 != iv1 );
  assert( jtri0 != UINT_MAX );
  if( aTri[jtri0*3+1] == iv1 && aTri[jtri0*3+2] == iv0 ){ return 0; }
  if( aTri[jtri0*3+2] == iv1 && aTri[jtri0*3+0] == iv0 ){ return 1; }
  if( aTri[jtri0*3+0] == iv1 && aTri[jtri0*3+1] == iv0 ){ return 2; }
  return UINT_MAX;
}

DFM2_INLINE void delfem2::ElemQuad_DihedralTri(
    std::vector<unsigned int>& aQuad,
    const unsigned int* aTri,
    const unsigned int nTri,
    const unsigned int np)
{
  std::vector<unsigned int> aElSuEl;
  ElSuEl_MeshElem(aElSuEl,
                  aTri, nTri, MESHELEM_TRI,
                  np);
  assert( aElSuEl.size() == nTri*3 );
  for(unsigned int itri=0; itri<nTri; ++itri){
    for(int iedtri=0;iedtri<3;++iedtri){
      const unsigned int jtri = aElSuEl[itri*3+iedtri];
      if( jtri == UINT_MAX ) continue; // on the boundary
      if( jtri < itri ) continue;
      const unsigned int jedtri = FindAdjEdgeIndex(itri, iedtri, jtri, aTri);
      assert( jedtri != UINT_MAX );
      const unsigned int ipo0 = aTri[itri*3+iedtri];
      const unsigned int ipo1 = aTri[jtri*3+jedtri];
      const unsigned int ipo2 = aTri[itri*3+(iedtri+1)%3];
      const unsigned int ipo3 = aTri[itri*3+(iedtri+2)%3];
      assert( aTri[jtri*3+(jedtri+2)%3] == ipo2 );
      assert( aTri[jtri*3+(jedtri+1)%3] == ipo3 );
      aQuad.push_back(ipo0);
      aQuad.push_back(ipo1);
      aQuad.push_back(ipo2);
      aQuad.push_back(ipo3);
    }
  }
}

// ---------------------------------

DFM2_INLINE void delfem2::convert2Tri_Quad
(std::vector<unsigned int>& aTri,
 const std::vector<unsigned int>& aQuad)
{
  const unsigned long nq = aQuad.size()/4;
  aTri.resize(nq*6);
  for(unsigned int iq=0;iq<nq;++iq){
    const unsigned int i0 = aQuad[iq*4+0];
    const unsigned int i1 = aQuad[iq*4+1];
    const unsigned int i2 = aQuad[iq*4+2];
    const unsigned int i3 = aQuad[iq*4+3];
    aTri[iq*6+0] = i0;  aTri[iq*6+1] = i1;  aTri[iq*6+2] = i2;
    aTri[iq*6+3] = i2;  aTri[iq*6+4] = i3;  aTri[iq*6+5] = i0;
  }
}


DFM2_INLINE void delfem2::FlipElement_Tri(std::vector<unsigned int>& aTri)
{
  for (std::size_t itri = 0; itri<aTri.size()/3; itri++){
    //    int i0 = aTri[itri*3+0];
    int i1 = aTri[itri*3+1];
    int i2 = aTri[itri*3+2];
    aTri[itri*3+1] = i2;
    aTri[itri*3+2] = i1;
  }
}


// -------------------------------------

// --------------------------------------


DFM2_INLINE void delfem2::JArray_ElSuP_MeshTri
(std::vector<unsigned int> &elsup_ind,
 std::vector<unsigned int> &elsup,
 // --
 const std::vector<unsigned int>& aTri,
 int nXYZ)
{
  JArray_ElSuP_MeshElem(elsup_ind, elsup,
                           aTri.data(), aTri.size()/3, 3, nXYZ);
}

// ----------------------------------------------------------------------------------------------------------

DFM2_INLINE void delfem2::ElSuEl_MeshElem
(std::vector<unsigned int>& aElSuEl,
 const unsigned int* aEl, unsigned int nEl, int nNoEl,
 const std::vector<unsigned int> &elsup_ind,
 const std::vector<unsigned int> &elsup,
 const int nfael,
 const int nnofa,
 const int (*noelElemFace)[4])
{
  assert( elsup_ind.size()>=1 );
  const std::size_t np = elsup_ind.size()-1;
  
  aElSuEl.assign(nEl*nfael,UINT_MAX);
  
  std::vector<int> flg_point(np,0);
  std::vector<unsigned int> inpofa(nnofa);
  for (unsigned int iel = 0; iel<nEl; iel++){
    for (int ifael=0; ifael<nfael; ifael++){
      for (int ipofa=0; ipofa<nnofa; ipofa++){
        int int0 = noelElemFace[ifael][ipofa];
        const unsigned int ip = aEl[iel*nNoEl+int0];
        assert( ip<np );
        inpofa[ipofa] = ip;
        flg_point[ip] = 1;
      }
      const int ipoin0 = inpofa[0];
      bool iflg = false;
      for (unsigned int ielsup = elsup_ind[ipoin0]; ielsup<elsup_ind[ipoin0+1]; ielsup++){
        const unsigned int jelem0 = elsup[ielsup];
        if (jelem0==iel) continue;
        for (int jfael = 0; jfael<nfael; jfael++){
          iflg = true;
          for (int jpofa = 0; jpofa<nnofa; jpofa++){
            int jnt0 = noelElemFace[jfael][jpofa];
            const unsigned int jpoin0 = aEl[jelem0*nNoEl+jnt0];
            if (flg_point[jpoin0]==0){ iflg = false; break; }
          }
          if (iflg){
            aElSuEl[iel*nfael+ifael] = jelem0;
            break;
          }
        }
        if (iflg) break;
      }
      if (!iflg){
        aElSuEl[iel*nfael+ifael] = UINT_MAX;
      }
      for (int ipofa = 0; ipofa<nnofa; ipofa++){
        flg_point[inpofa[ipofa]] = 0;
      }
    }
  }
}

/*
void makeSurroundingRelationship
(std::vector<int>& aElSurRel,
 const int* aEl, int nEl, int nNoEl,
 MESHELEM_TYPE type,
 const std::vector<int>& elsup_ind,
 const std::vector<int>& elsup)
{
  const int nfael = nFaceElem(type);
  const int nnofa = nNodeElemFace(type, 0);
  assert( nNoEl == nNodeElem(type) );
  makeSurroundingRelationship(aElSurRel,
                              aEl, nEl, nNoEl,
                              elsup_ind, elsup,
                              nfael, nnofa, noelElemFace(type));
}
 */

DFM2_INLINE void delfem2::ElSuEl_MeshElem
(std::vector<unsigned int>& aElSuEl,
 const unsigned int* aElem,
 size_t nElem,
 MESHELEM_TYPE type,
 const size_t nXYZ)
{
  const int nNoEl = nNodeElem(type);
  std::vector<unsigned int> elsup_ind, elsup;
  JArray_ElSuP_MeshElem(elsup_ind, elsup,
      aElem, nElem, nNoEl, nXYZ);
  const int nfael = nFaceElem(type);
  const int nnofa = nNodeElemFace(type, 0);
  ElSuEl_MeshElem(aElSuEl,
      aElem, nElem, nNoEl,
      elsup_ind,elsup,
      nfael, nnofa, noelElemFace(type));
  assert( aElSuEl.size() == nElem*nfael );
}


// -------------------------------------------------------------------------

DFM2_INLINE void delfem2::JArrayPointSurPoint_MeshOneRingNeighborhood(
    std::vector<unsigned int>& psup_ind,
    std::vector<unsigned int>& psup,
    //
    const unsigned int* pElem,
    const std::vector<unsigned int> &elsup_ind,
    const std::vector<unsigned int> &elsup,
    unsigned int nnoel,
    unsigned int nPoint)
{
  std::vector<int> aflg(nPoint,-1);
  psup_ind.assign(nPoint+1,0);
  for(unsigned int ipoint=0;ipoint<nPoint;ipoint++){
    aflg[ipoint] = ipoint;
    for(unsigned int ielsup=elsup_ind[ipoint];ielsup<elsup_ind[ipoint+1];ielsup++){
      unsigned int jelem = elsup[ielsup];
      for(unsigned int jnoel=0;jnoel<nnoel;jnoel++){
        unsigned int jnode = pElem[jelem*nnoel+jnoel];
        if( aflg[jnode] != (int)ipoint ){
          aflg[jnode] = ipoint;
          psup_ind[ipoint+1]++;
        }
      }
    }
  }
  for(unsigned int ipoint=0;ipoint<nPoint;ipoint++){
    psup_ind[ipoint+1] += psup_ind[ipoint];
  }
  const int npsup = psup_ind[nPoint];
  psup.resize(npsup);
  for(unsigned int ipoint=0;ipoint<nPoint;ipoint++){ aflg[ipoint] = -1; }
  for(unsigned int ipoint=0;ipoint<nPoint;ipoint++){
    aflg[ipoint] = ipoint;
    for(unsigned int ielsup=elsup_ind[ipoint];ielsup<elsup_ind[ipoint+1];ielsup++){
      unsigned int jelem = elsup[ielsup];
      for(unsigned int jnoel=0;jnoel<nnoel;jnoel++){
        unsigned int jnode = pElem[jelem*nnoel+jnoel];
        if( aflg[jnode] != (int)ipoint ){
          aflg[jnode] = ipoint;
          const int ind = psup_ind[ipoint];
          psup[ind] = jnode;
          psup_ind[ipoint]++;
        }
      }
    }
  }
  for(unsigned int ipoint=nPoint;ipoint>0;ipoint--){
    psup_ind[ipoint] = psup_ind[ipoint-1];
  }
  psup_ind[0] = 0;
}

DFM2_INLINE void delfem2::JArray_PSuP_MeshElem(
    std::vector<unsigned int>& psup_ind,
    std::vector<unsigned int>& psup,
    //
    const unsigned int* pElem,
    size_t nEl,
    unsigned int nPoEl,
    size_t nPo)
{
  std::vector<unsigned int> elsup_ind, elsup;
  JArray_ElSuP_MeshElem(elsup_ind, elsup,
      pElem, nEl, nPoEl, nPo);
  JArrayPointSurPoint_MeshOneRingNeighborhood(psup_ind, psup,
                          pElem, elsup_ind,elsup, nPoEl, nPo);
}

DFM2_INLINE void delfem2::makeOneRingNeighborhood_TriFan
(std::vector<int>& psup_ind,
 std::vector<int>& psup,
 // ----------------------
 const std::vector<int>& aTri,
 const std::vector<int>& aTriSurRel,
 const std::vector<int>& elsup_ind,
 const std::vector<int>& elsup,
 int npoint)
{
  psup_ind.resize(npoint+1);
  psup_ind[0] = 0;
  psup.clear();
  for(int ipoint=0;ipoint<npoint;++ipoint){
    int iel0 = -1;
    int inoel0 = -1;
    {
      int ielsup0 = elsup_ind[ipoint];
      iel0 = elsup[ielsup0];
      if( aTri[iel0*3+0] == ipoint ){ inoel0 = 0; }
      if( aTri[iel0*3+1] == ipoint ){ inoel0 = 1; }
      if( aTri[iel0*3+2] == ipoint ){ inoel0 = 2; }
      assert( inoel0 != -1 );
    }
    int iel_cur = iel0;
    int inoel_cur = inoel0;
    for(;;){
      int jnoel_cur = (inoel_cur+1)%3;
      int jp0 = aTri[iel_cur*3+jnoel_cur];
      psup.push_back(jp0);
      int iel_next = aTriSurRel[iel_cur*6+2*jnoel_cur+0];
      int inoel_next = -1;
      if( aTri[iel_next*3+0] == ipoint ){ inoel_next = 0; }
      if( aTri[iel_next*3+1] == ipoint ){ inoel_next = 1; }
      if( aTri[iel_next*3+2] == ipoint ){ inoel_next = 2; }
      assert( inoel_next != -1 );
      if( iel_next == iel0 ) break;
      iel_cur = iel_next;
      inoel_cur = inoel_next;
    }
    psup_ind[ipoint+1] = (int)psup.size();
  }
}

DFM2_INLINE void delfem2::JArrayEdge_MeshElem
(std::vector<unsigned int> &edge_ind,
 std::vector<unsigned int> &edge,
 //
 const unsigned int* aElm0,
 MESHELEM_TYPE elem_type,
 const std::vector<unsigned int> &elsup_ind,
 const std::vector<unsigned int> &elsup,
 bool is_bidirectional)
{
  const int neElm = mapMeshElemType2NEdgeElem[elem_type];
  const int nnoelElm = mapMeshElemType2NNodeElem[elem_type];
  const int (*aNoelEdge)[2] = noelElemEdge(elem_type);
  const std::size_t nPoint0 = elsup_ind.size()-1;
  edge_ind.resize(nPoint0+1);
  edge_ind[0] = 0;
  for(unsigned int ip=0;ip<nPoint0;++ip){
    std::set<int> setIP;
    for(unsigned int ielsup=elsup_ind[ip];ielsup<elsup_ind[ip+1];++ielsup){
      int iq0 = elsup[ielsup];
      for(int ie=0;ie<neElm;++ie){
        int inoel0 = aNoelEdge[ie][0];
        int inoel1 = aNoelEdge[ie][1];
        unsigned int ip0 = aElm0[iq0*nnoelElm+inoel0];
        unsigned int ip1 = aElm0[iq0*nnoelElm+inoel1];
        if( ip0 != ip && ip1 != ip ) continue;
        if( ip0 == ip ){
          if( is_bidirectional || ip1 > ip ){ setIP.insert(ip1); }
        }
        else{
          if( is_bidirectional || ip0 > ip ){ setIP.insert(ip0); }
        }
      }
    }
    for(int itr : setIP){
      edge.push_back(itr);
    }
    edge_ind[ip+1] = edge_ind[ip] + (int)setIP.size();
  }
}


DFM2_INLINE void delfem2::MeshLine_JArrayEdge
(std::vector<unsigned int>& aLine,
 //
 const std::vector<unsigned int> &psup_ind,
 const std::vector<unsigned int> &psup)
{
  aLine.reserve(psup.size()*2);
  const std::size_t np = psup_ind.size()-1;
  for(unsigned int ip=0;ip<np;++ip){
    for(unsigned int ipsup=psup_ind[ip];ipsup<psup_ind[ip+1];++ipsup){
      unsigned int jp = psup[ipsup];
      aLine.push_back(ip);
      aLine.push_back(jp);
    }
  }
}

DFM2_INLINE void delfem2::MeshLine_MeshElem
(std::vector<unsigned int>& aLine,
 const unsigned int* aElm0,
 unsigned int nElem,
 MESHELEM_TYPE elem_type,
 unsigned int nPo)
{
  std::vector<unsigned int> elsup_ind,elsup;
  const unsigned int nPoEl = mapMeshElemType2NNodeElem[elem_type];
  JArray_ElSuP_MeshElem(elsup_ind, elsup,
      aElm0, nElem, nPoEl, nPo);
  std::vector<unsigned int> edge_ind, edge;
  JArrayEdge_MeshElem(edge_ind, edge,
      aElm0,
      elem_type,
      elsup_ind,elsup,false);
  MeshLine_JArrayEdge(aLine,
      edge_ind,edge);
}


// -----------------------------------------

DFM2_INLINE void
delfem2::JArray_AddMasterSlavePattern(
    std::vector<unsigned int> &index,
    std::vector<unsigned int> &array,
    const unsigned int* aMSFlag,
    int ndim,
    const unsigned int *psup_ind0,
    int npsup_ind0,
    const unsigned int *psup0)
{
  assert(npsup_ind0>0);
  const int nno = npsup_ind0-1;
  //assert( aMSFlag.size() == nno*ndim );
  std::vector< std::vector<int> > mapM2S(nno);
  for(int ino1=0;ino1<nno;++ino1){
    for(int idim1=0;idim1<ndim;++idim1){
      int idof0 = aMSFlag[ino1*ndim+idim1];
      if( idof0 == -1 ){ continue; }
      int ino0 = idof0/ndim;
//      int idim0 = idof0 - ino0*ndim;
      assert( ino0 < nno && idof0 - ino0*ndim < ndim );
//      std::cout << idim1 << " " << idim0 << " " << ndim << std::endl;
      assert( idim1 == idof0 - ino0*ndim );
      mapM2S[ino0].push_back(ino1);
    }
  }
  //
  index.assign(nno+1,0);
  array.clear();
  std::vector<int> aflg(nno,-1);
  ///
  for(int ino0=0;ino0<nno;++ino0){
    aflg[ino0] = ino0;
    for(unsigned int icrs=psup_ind0[ino0];icrs<psup_ind0[ino0+1];++icrs){
      const unsigned int jno = psup0[icrs];
      if( aflg[jno] == ino0 ){ continue; }
      aflg[jno] = ino0;
      index[ino0+1]++;
    }
    for(int iino1=0;iino1<(int)mapM2S[ino0].size();++iino1){
      const int ino1 = mapM2S[ino0][iino1];
      if( aflg[ino1] != ino0 ){
        aflg[ino1] = ino0;
        index[ino0+1]++;
      }
      for(unsigned int jcrs=psup_ind0[ino1];jcrs<psup_ind0[ino1+1];++jcrs){
        const unsigned int jno1 = psup0[jcrs];
        if( aflg[jno1] == ino0 ){ continue; }
        aflg[jno1] = ino0;
        index[ino0+1]++;
      }
    }
    for(unsigned int icrs=psup_ind0[ino0];icrs<psup_ind0[ino0+1];++icrs){
      const unsigned int jno = psup0[icrs];
      for(int jdim=0;jdim<ndim;++jdim){
        int kdof = aMSFlag[jno*ndim+jdim];
        if( kdof == -1 ) continue;
        int kno = kdof/ndim;
        if( aflg[kno] == ino0 ){ continue; }
        aflg[kno] = ino0;
        index[ino0+1]++;
      }
    }
  }
  ////
  for(int ino=0;ino<nno;ino++){ index[ino+1] += index[ino]; }
  const int narray = index[nno];
  array.resize(narray);
  for(int ino=0;ino<nno;ino++){ aflg[ino] = -1; }
  ////
  for(int ino0=0;ino0<nno;++ino0){
    aflg[ino0] = ino0;
    for(unsigned int icrs=psup_ind0[ino0];icrs<psup_ind0[ino0+1];++icrs){
      const unsigned int jno = psup0[icrs];
      if( aflg[jno] == ino0 ){ continue; }
      aflg[jno] = ino0;
      const int ind = index[ino0];
      array[ind] = jno;
      index[ino0]++;
    }
    for(std::size_t jjno=0;jjno<mapM2S[ino0].size();++jjno){
      const int jno = mapM2S[ino0][jjno];
      if( aflg[jno] != ino0 ){
        aflg[jno] = ino0;
        const int ind = index[ino0];
        array[ind] = jno;
        index[ino0]++;
      }
      for(unsigned int jcrs=psup_ind0[jno];jcrs<psup_ind0[jno+1];++jcrs){
        const unsigned int kno = psup0[jcrs];
        if( aflg[kno] == ino0 ){ continue; }
        aflg[kno] = ino0;
        const int ind = index[ino0];
        array[ind] = kno;
        index[ino0]++;
      }
    }
    for(unsigned int icrs=psup_ind0[ino0];icrs<psup_ind0[ino0+1];++icrs){
      const unsigned int jno = psup0[icrs];
      for(int jdim=0;jdim<ndim;++jdim){
        int kdof = aMSFlag[jno*ndim+jdim];
        if( kdof == -1 ) continue;
        int kno = kdof/ndim;
        if( aflg[kno] == ino0 ){ continue; }
        aflg[kno] = ino0;
        const int ind = index[ino0];
        array[ind] = kno;
        index[ino0]++;
      }
    }
  }
  // ---------
  for(int ino=nno;ino>0;ino--){ index[ino] = index[ino-1]; }
  index[0] = 0;
}


// ---------------------------------------

DFM2_INLINE void delfem2::MarkConnectedElements
(std::vector<int>& aIndGroup,
 int itri_ker,
 int igroup,
 const std::vector<int>& aTriSurRel,
 const int nfael)
{
  aIndGroup[itri_ker] = igroup;
  std::stack<int> next;
  next.push(itri_ker);
  while(!next.empty()){
    int itri0 = next.top();
    next.pop();
    for(int ie=0;ie<nfael;++ie){
      const int ita = aTriSurRel[(itri0*nfael+ie)*2+0];
      if( ita == -1 ) continue;
      if( aIndGroup[ita] != igroup ){
        aIndGroup[ita] = igroup;
        next.push(ita);
      }
    }
  }
}

DFM2_INLINE void delfem2::MakeGroupElem
(int& ngroup,
 std::vector<int>& aIndGroup,
 const std::vector<int>& aTri,
 const std::vector<int>& aTriSurRel,
 const int nfael,
 const int nnoel)
{
  const std::size_t nelem = aTri.size()/nnoel;
  aIndGroup.assign(nelem,-1);
  int igroup = -1;
  for(;;){
    unsigned int itri_ker = 0;
    for(;itri_ker<nelem;++itri_ker){
      if( aIndGroup[itri_ker]==-1) break;
    }
    if( itri_ker == nelem ) break;
    igroup++;
    MarkConnectedElements(aIndGroup, itri_ker, igroup, aTriSurRel,nfael);
  }
  ngroup = igroup+1;
}

DFM2_INLINE void delfem2::MakeGroupElem_Tri
(int& ngroup,
 std::vector<int>& aIndGroup,
 const std::vector<int>& aTri,
 const std::vector<int>& aTriSurRel)
{
  MakeGroupElem(ngroup,aIndGroup,
                aTri,aTriSurRel,3,3);
}


// ----------------------------------------------------

DFM2_INLINE int delfem2::findEdge
(unsigned int ip0, unsigned int ip1,
 const std::vector<unsigned int> &psup_ind,
 const std::vector<unsigned int> &psup)
{
  if( ip1 > ip0 ){
    for(unsigned int ipsup=psup_ind[ip0];ipsup<psup_ind[ip0+1];++ipsup){
      unsigned int ip2 = psup[ipsup];
      if( ip2 == ip1 ){ return (int)ipsup; }
    }
  }
  else{
    for(unsigned int ipsup=psup_ind[ip1];ipsup<psup_ind[ip1+1];++ipsup){
      unsigned int ip2 = psup[ipsup];
      if( ip2 == ip0 ){ return (int)ipsup; }
    }
  }
  return -1;
}

DFM2_INLINE int delfem2::findFace(
    unsigned int ip0,
    unsigned int ip1,
    unsigned int ip2,
    unsigned int ip3,
    const std::vector<unsigned int>& aQuad,
    const std::vector<unsigned int> &elsupInd,
    const std::vector<unsigned int> &elsup)
{
  if( ip0 >= elsupInd.size()-1 ) return -1;
  for(unsigned int ielsup=elsupInd[ip0];ielsup<elsupInd[ip0+1];++ielsup){
    unsigned int ie0 = elsup[ielsup];
    unsigned int iq0 = aQuad[ie0*4+0];
    unsigned int iq1 = aQuad[ie0*4+1];
    unsigned int iq2 = aQuad[ie0*4+2];
    unsigned int iq3 = aQuad[ie0*4+3];
    if( ip0!=iq0 && ip0!=iq1 && ip0!=iq2 && ip0!=iq3 ) continue;
    if( ip1!=iq0 && ip1!=iq1 && ip1!=iq2 && ip1!=iq3 ) continue;
    if( ip2!=iq0 && ip2!=iq1 && ip2!=iq2 && ip2!=iq3 ) continue;
    if( ip3!=iq0 && ip3!=iq1 && ip3!=iq2 && ip3!=iq3 ) continue;
    return (int)ie0;
  }
  return -1;
}

// new points is in the order of [old points], [edge points], [face points]
DFM2_INLINE void delfem2::SubdivTopo_MeshQuad
(std::vector<unsigned int>& aQuad1,
 std::vector<unsigned int> &psup_ind,
 std::vector<unsigned int> &psup,
 std::vector<int>& aEdgeFace0, // two points on the edge and two quads touching the edge
 const unsigned int* aQuad0, unsigned int nQuad0,
 unsigned int nPoint0)
{
  const unsigned int nq0 = nQuad0;
  std::vector<unsigned int> elsup_ind, elsup;
  JArray_ElSuP_MeshElem(elsup_ind,elsup,
                              aQuad0,nQuad0,4,nPoint0);
  JArrayEdge_MeshElem(psup_ind,psup,
                       aQuad0, MESHELEM_QUAD, elsup_ind, elsup,
                       false); // is_bidirectional = false
  const unsigned int ne0 = (int)psup.size();
  aEdgeFace0.resize(0);
  aEdgeFace0.reserve(ne0*4);
  for(int ip=0;ip<(int)nPoint0;++ip){
    for(unsigned int ipsup=psup_ind[ip];ipsup<psup_ind[ip+1];++ipsup){
      int ip1 = psup[ipsup];
      aEdgeFace0.push_back(ip);
      aEdgeFace0.push_back(ip1);
      int iq0=-1, iq1=-1;
      for(unsigned int ielsup=elsup_ind[ip];ielsup<elsup_ind[ip+1];++ielsup){
        int jq0 = elsup[ielsup];
        unsigned int jp0 = aQuad0[jq0*4+0];
        unsigned int jp1 = aQuad0[jq0*4+1];
        unsigned int jp2 = aQuad0[jq0*4+2];
        unsigned int jp3 = aQuad0[jq0*4+3];
        if( ((int)jp0!=ip) && ((int)jp1!=ip) && ((int)jp2!=ip) && ((int)jp3!=ip) ){ continue; }
        if( ((int)jp0!=ip1) && ((int)jp1!=ip1) && ((int)jp2!=ip1) && ((int)jp3!=ip1) ){ continue; }
        // ----------------------------
        if( iq0 == -1 ){ iq0 = jq0; }
        else{
          assert( iq1 == -1 );
          iq1 = jq0;
        }
      }
      aEdgeFace0.push_back(iq0);
      aEdgeFace0.push_back(iq1);
    }
  }
  aQuad1.resize(0);
  aQuad1.reserve(nQuad0*4);
  for(unsigned int iq=0;iq<nq0;++iq){
    unsigned int ip0 = aQuad0[iq*4+0];
    unsigned int ip1 = aQuad0[iq*4+1];
    unsigned int ip2 = aQuad0[iq*4+2];
    unsigned int ip3 = aQuad0[iq*4+3];
    int ie01 = findEdge(ip0,ip1, psup_ind,psup); assert( ie01 != -1 );
    int ie12 = findEdge(ip1,ip2, psup_ind,psup); assert( ie12 != -1 );
    int ie23 = findEdge(ip2,ip3, psup_ind,psup); assert( ie23 != -1 );
    int ie30 = findEdge(ip3,ip0, psup_ind,psup); assert( ie30 != -1 );
    unsigned int ip01 = ie01 + nPoint0;
    unsigned int ip12 = ie12 + nPoint0;
    unsigned int ip23 = ie23 + nPoint0;
    unsigned int ip30 = ie30 + nPoint0;
    unsigned int ip0123 = iq + nPoint0 + ne0;
    aQuad1.push_back(ip0);   aQuad1.push_back(ip01); aQuad1.push_back(ip0123); aQuad1.push_back(ip30);
    aQuad1.push_back(ip1);   aQuad1.push_back(ip12); aQuad1.push_back(ip0123); aQuad1.push_back(ip01);
    aQuad1.push_back(ip2);   aQuad1.push_back(ip23); aQuad1.push_back(ip0123); aQuad1.push_back(ip12);
    aQuad1.push_back(ip3);   aQuad1.push_back(ip30); aQuad1.push_back(ip0123); aQuad1.push_back(ip23);
  }
}


// new points is in the order of [old points], [edge points]
DFM2_INLINE void delfem2::SubdivTopo_MeshTet
(std::vector<unsigned int>& aTet1,
 std::vector<unsigned int> &psup_ind,
 std::vector<unsigned int> &psup,
 const unsigned int* aTet0, int nTet0,
 unsigned int nPoint0)
{
  const int nt0 = nTet0;
  std::vector<unsigned int> elsup_ind, elsup;
  JArray_ElSuP_MeshElem(elsup_ind,elsup,
      aTet0,nTet0,4,nPoint0);
  JArrayEdge_MeshElem(psup_ind,psup,
                       aTet0, MESHELEM_TET, elsup_ind, elsup,
                       false);
  aTet1.resize(0);
  aTet1.reserve(nTet0*4);
  for(int it=0;it<nt0;++it){
    unsigned int ip0 = aTet0[it*4+0];
    unsigned int ip1 = aTet0[it*4+1];
    unsigned int ip2 = aTet0[it*4+2];
    unsigned int ip3 = aTet0[it*4+3];
    int ie01 = findEdge(ip0,ip1, psup_ind,psup); assert( ie01 != -1 );
    int ie02 = findEdge(ip0,ip2, psup_ind,psup); assert( ie02 != -1 );
    int ie03 = findEdge(ip0,ip3, psup_ind,psup); assert( ie03 != -1 );
    int ie12 = findEdge(ip1,ip2, psup_ind,psup); assert( ie12 != -1 );
    int ie13 = findEdge(ip1,ip3, psup_ind,psup); assert( ie13 != -1 );
    int ie23 = findEdge(ip2,ip3, psup_ind,psup); assert( ie23 != -1 );
    unsigned int ip01 = ie01 + nPoint0;
    unsigned int ip02 = ie02 + nPoint0;
    unsigned int ip03 = ie03 + nPoint0;
    unsigned int ip12 = ie12 + nPoint0;
    unsigned int ip13 = ie13 + nPoint0;
    unsigned int ip23 = ie23 + nPoint0;
    aTet1.push_back(ip0);  aTet1.push_back(ip01); aTet1.push_back(ip02); aTet1.push_back(ip03);
    aTet1.push_back(ip1);  aTet1.push_back(ip01); aTet1.push_back(ip13); aTet1.push_back(ip12);
    aTet1.push_back(ip2);  aTet1.push_back(ip02); aTet1.push_back(ip12); aTet1.push_back(ip23);
    aTet1.push_back(ip3);  aTet1.push_back(ip03); aTet1.push_back(ip23); aTet1.push_back(ip13);
    aTet1.push_back(ip01); aTet1.push_back(ip23); aTet1.push_back(ip13); aTet1.push_back(ip12);
    aTet1.push_back(ip01); aTet1.push_back(ip23); aTet1.push_back(ip12); aTet1.push_back(ip02);
    aTet1.push_back(ip01); aTet1.push_back(ip23); aTet1.push_back(ip02); aTet1.push_back(ip03);
    aTet1.push_back(ip01); aTet1.push_back(ip23); aTet1.push_back(ip03); aTet1.push_back(ip13);
  }
}

/*
// TODO: This one is imcomplete
void VoxSubdiv
(std::vector<int>& aVox1,
 std::vector<int>& psupIndHex0,
 std::vector<int>& psupHex0,
 std::vector<int>& aQuadHex0,
 ///
 const std::vector<int>& aVox0,
 const int nhp0)
{
  //  int nhp0 = (int)aHexPoint0.size(); // hex point
  std::vector<int> elsupIndHex0, elsupHex0;
  makeElemSurroundingPoint(elsupIndHex0, elsupHex0,
                           aVox0,8,nhp0);
  
  //edge
  makeEdgeVox(psupIndHex0, psupHex0,
              aVox0, elsupIndHex0,elsupHex0, nhp0);
  
  //face
  aQuadHex0.clear();
  {
    std::vector<int> aHexSurRel0;
    makeSurroundingRelationship(aHexSurRel0,
                                aVox0,FEMELEM_VOX,
                                elsupIndHex0,elsupHex0);
    for(int ih=0;ih<aVox0.size()/8;++ih){
      for(int ifh=0;ifh<6;++ifh){
        int jh0 = aHexSurRel0[ih*6*2+ifh*2+0];
        if( jh0!=-1 && ih>jh0 ) continue;
        for(int inofa=0;inofa<4;++inofa){
          int inoel0 = noelElemFace_Hex[ifh][inofa];
          int igp0 = aVox0[ih*8+inoel0];
          aQuadHex0.push_back(igp0);
        }
      }
    }
  }
  std::vector<int> elsupIndQuadHex0, elsupQuadHex0;
  makeElemSurroundingPoint(elsupIndQuadHex0,elsupQuadHex0,
                           aQuadHex0,4,nhp0);
  
  const int neh0 = (int)psupHex0.size();
  const int nfh0 = (int)aQuadHex0.size()/4;
  std::cout << nfh0 << " " << aQuadHex0.size() << std::endl;
  
  // making vox
  aVox1.clear();
  for(int ih=0;ih<aVox0.size()/8;++ih){
    int ihc0 = aVox0[ih*8+0];
    int ihc1 = aVox0[ih*8+1];
    int ihc2 = aVox0[ih*8+2];
    int ihc3 = aVox0[ih*8+3];
    int ihc4 = aVox0[ih*8+4];
    int ihc5 = aVox0[ih*8+5];
    int ihc6 = aVox0[ih*8+6];
    int ihc7 = aVox0[ih*8+7];
    int ihc01 = findEdge(ihc0,ihc1, psupIndHex0,psupHex0)+nhp0; assert(ihc01>=nhp0&&ihc01<nhp0+neh0);
    int ihc32 = findEdge(ihc3,ihc2, psupIndHex0,psupHex0)+nhp0; assert(ihc32>=nhp0&&ihc32<nhp0+neh0);
    int ihc45 = findEdge(ihc4,ihc5, psupIndHex0,psupHex0)+nhp0; assert(ihc45>=nhp0&&ihc45<nhp0+neh0);
    int ihc76 = findEdge(ihc7,ihc6, psupIndHex0,psupHex0)+nhp0; assert(ihc76>=nhp0&&ihc76<nhp0+neh0);
    int ihc03 = findEdge(ihc0,ihc3, psupIndHex0,psupHex0)+nhp0; assert(ihc03>=nhp0&&ihc03<nhp0+neh0);
    int ihc12 = findEdge(ihc1,ihc2, psupIndHex0,psupHex0)+nhp0; assert(ihc12>=nhp0&&ihc12<nhp0+neh0);
    int ihc47 = findEdge(ihc4,ihc7, psupIndHex0,psupHex0)+nhp0; assert(ihc47>=nhp0&&ihc47<nhp0+neh0);
    int ihc56 = findEdge(ihc5,ihc6, psupIndHex0,psupHex0)+nhp0; assert(ihc56>=nhp0&&ihc56<nhp0+neh0);
    int ihc04 = findEdge(ihc0,ihc4, psupIndHex0,psupHex0)+nhp0; assert(ihc04>=nhp0&&ihc04<nhp0+neh0);
    int ihc15 = findEdge(ihc1,ihc5, psupIndHex0,psupHex0)+nhp0; assert(ihc15>=nhp0&&ihc15<nhp0+neh0);
    int ihc37 = findEdge(ihc3,ihc7, psupIndHex0,psupHex0)+nhp0; assert(ihc37>=nhp0&&ihc37<nhp0+neh0);
    int ihc26 = findEdge(ihc2,ihc6, psupIndHex0,psupHex0)+nhp0; assert(ihc26>=nhp0&&ihc26<nhp0+neh0);
    int ihc0462 = findFace(ihc0,ihc4,ihc6,ihc2, aQuadHex0,elsupIndQuadHex0,elsupQuadHex0)+nhp0+neh0; assert(ihc0462>=nhp0+neh0&&ihc0462<nhp0+neh0+nfh0);
    int ihc1375 = findFace(ihc1,ihc3,ihc7,ihc5, aQuadHex0,elsupIndQuadHex0,elsupQuadHex0)+nhp0+neh0; assert(ihc1375>=nhp0+neh0&&ihc1375<nhp0+neh0+nfh0);
    int ihc0154 = findFace(ihc0,ihc1,ihc5,ihc4, aQuadHex0,elsupIndQuadHex0,elsupQuadHex0)+nhp0+neh0; assert(ihc0154>=nhp0+neh0&&ihc0154<nhp0+neh0+nfh0);
    int ihc2673 = findFace(ihc2,ihc6,ihc7,ihc3, aQuadHex0,elsupIndQuadHex0,elsupQuadHex0)+nhp0+neh0; assert(ihc2673>=nhp0+neh0&&ihc2673<nhp0+neh0+nfh0);
    int ihc0231 = findFace(ihc0,ihc2,ihc3,ihc1, aQuadHex0,elsupIndQuadHex0,elsupQuadHex0)+nhp0+neh0; assert(ihc0231>=nhp0+neh0&&ihc0231<nhp0+neh0+nfh0);
    int ihc4576 = findFace(ihc4,ihc5,ihc7,ihc6, aQuadHex0,elsupIndQuadHex0,elsupQuadHex0)+nhp0+neh0; assert(ihc4576>=nhp0+neh0&&ihc4576<nhp0+neh0+nfh0);
    int ihc0473 = findFace(ihc0,ihc4,ihc7,ihc3, aQuadHex0,elsupIndQuadHex0,elsupQuadHex0)+nhp0+neh0; assert(ihc0473>=nhp0+neh0&&ihc0473<nhp0+neh0+nfh0);
    int ihc01234567 = ih + nhp0 + neh0 + nfh0;
    aVox1.push_back(ihc0);
    aVox1.push_back(ihc01);
    aVox1.push_back(ihc02);
    aVox1.push_back(ihc03);
    aVox1.push_back(ihc04);
    aVox1.push_back(ihc0154);
    aVox1.push_back(ihc01234567);
    aVox1.push_back(ihc0473);
    /////
    aVox1.push_back(ihc01);
    aVox1.push_back(ihc1);
    aVox1.push_back(ihc12);
    aVox1.push_back(ihc0321);
    aVox1.push_back(ihc0154);
    aVox1.push_back(ihc15);
    aVox1.push_back(ihc1265);
    aVox1.push_back(ihc01234567);
    /////
    aVox1.push_back(ihc0321);
    aVox1.push_back(ihc12);
    aVox1.push_back(ihc2);
    aVox1.push_back(ihc32);
    aVox1.push_back(ihc01234567);
    aVox1.push_back(ihc1265);
    aVox1.push_back(ihc26);
    aVox1.push_back(ihc3762);
    /////
    aVox1.push_back(ihc03);
    aVox1.push_back(ihc0321);
    aVox1.push_back(ihc32);
    aVox1.push_back(ihc3);
    aVox1.push_back(ihc0473);
    aVox1.push_back(ihc01234567);
    aVox1.push_back(ihc3762);
    aVox1.push_back(ihc37);
    /////
    aVox1.push_back(ihc04);
    aVox1.push_back(ihc0154);
    aVox1.push_back(ihc01234567);
    aVox1.push_back(ihc0473);
    aVox1.push_back(ihc4);
    aVox1.push_back(ihc45);
    aVox1.push_back(ihc4567);
    aVox1.push_back(ihc47);
    ////
    aVox1.push_back(ihc0154);
    aVox1.push_back(ihc15);
    aVox1.push_back(ihc1265);
    aVox1.push_back(ihc01234567);
    aVox1.push_back(ihc45);
    aVox1.push_back(ihc5);
    aVox1.push_back(ihc56);
    aVox1.push_back(ihc4567);
    /////
    aVox1.push_back(ihc01234567);
    aVox1.push_back(ihc1265);
    aVox1.push_back(ihc26);
    aVox1.push_back(ihc3762);
    aVox1.push_back(ihc4567);
    aVox1.push_back(ihc56);
    aVox1.push_back(ihc6);
    aVox1.push_back(ihc76);
    /////
    aVox1.push_back(ihc0473);
    aVox1.push_back(ihc01234567);
    aVox1.push_back(ihc3762);
    aVox1.push_back(ihc37);
    aVox1.push_back(ihc47);
    aVox1.push_back(ihc4567);
    aVox1.push_back(ihc76);
    aVox1.push_back(ihc7);
  }
}
*/


// -------------------------------------

void delfem2::SubdivTopo_MeshHex
(std::vector<unsigned int>& aHex1,
 std::vector<unsigned int> &psupIndHex0,
 std::vector<unsigned int> &psupHex0,
 std::vector<unsigned int>& aQuadHex0,
 //
 const unsigned int* aHex0, int nHex0,
 const int nhp0)
{
  //  int nhp0 = (int)aHexPoint0.size(); // hex point
  std::vector<unsigned int> elsupIndHex0, elsupHex0;
  JArray_ElSuP_MeshElem(elsupIndHex0, elsupHex0,
      aHex0,nHex0,8,nhp0);
  
  //edge
  JArrayEdge_MeshElem(psupIndHex0, psupHex0,
      aHex0, MESHELEM_HEX, elsupIndHex0,elsupHex0,
      false); // is_directional = false
  
  //face
  aQuadHex0.clear();
  {
    std::vector<unsigned int> aHexSuHex0;
    ElSuEl_MeshElem(aHexSuHex0,
                    aHex0,nHex0,8,
                    elsupIndHex0,elsupHex0,
                    nFaceElem(MESHELEM_HEX),
                    nNodeElemFace(MESHELEM_HEX, 0),
                    noelElemFace(MESHELEM_HEX));
    for(unsigned int ih=0;ih<(unsigned int)nHex0;++ih){
      for(int ifh=0;ifh<6;++ifh){
        unsigned int jh0 = aHexSuHex0[ih*6+ifh];
        if( jh0!=UINT_MAX && ih>jh0 ) continue;
        for(int inofa=0;inofa<4;++inofa){
          int inoel0 = noelElemFace_Hex[ifh][inofa];
          unsigned int igp0 = aHex0[ih*8+inoel0];
          aQuadHex0.push_back(igp0);
        }
      }
    }
  }
  std::vector<unsigned int> elsupIndQuadHex0, elsupQuadHex0;
  JArray_ElSuP_MeshElem(elsupIndQuadHex0,elsupQuadHex0,
      aQuadHex0.data(),aQuadHex0.size()/4,4,nhp0);
  
  const int neh0 = (int)psupHex0.size();
  const int nfh0 = (int)aQuadHex0.size()/4;
//  std::cout << nfh0 << " " << aQuadHex0.size() << std::endl;

  /*
  const int aNoelEdge[12][2] = {
    {0,1},{1,2},{2,3},{3,0},
    {4,5},{5,6},{6,7},{7,4},
    {0,4},{1,5},{2,6},{3,7} };
   */
  /*
  const int noelElemFace_Hex[8][4] = { // this numbering is corresponds to VTK_VOXEL
    { 0, 4, 7, 3 }, // -x
    { 1, 2, 6, 5 }, // +x
    { 0, 1, 5, 4 }, // -y
    { 3, 7, 6, 2 }, // +y
    { 0, 3, 2, 1 }, // -z
    { 4, 5, 6, 7 }  // +z
  };
   */
  
  // making hex
  aHex1.clear();
  for(int ih=0;ih<nHex0;++ih){
    unsigned int ihc0 = aHex0[ih*8+0];
    unsigned int ihc1 = aHex0[ih*8+1];
    unsigned int ihc2 = aHex0[ih*8+2];
    unsigned int ihc3 = aHex0[ih*8+3];
    unsigned int ihc4 = aHex0[ih*8+4];
    unsigned int ihc5 = aHex0[ih*8+5];
    unsigned int ihc6 = aHex0[ih*8+6];
    unsigned int ihc7 = aHex0[ih*8+7];
    int ihc01 = findEdge(ihc0,ihc1, psupIndHex0,psupHex0)+nhp0; assert(ihc01>=nhp0&&ihc01<nhp0+neh0);
    int ihc12 = findEdge(ihc1,ihc2, psupIndHex0,psupHex0)+nhp0; assert(ihc12>=nhp0&&ihc12<nhp0+neh0);
    int ihc23 = findEdge(ihc2,ihc3, psupIndHex0,psupHex0)+nhp0; assert(ihc23>=nhp0&&ihc23<nhp0+neh0);
    int ihc30 = findEdge(ihc3,ihc0, psupIndHex0,psupHex0)+nhp0; assert(ihc30>=nhp0&&ihc30<nhp0+neh0);
    int ihc45 = findEdge(ihc4,ihc5, psupIndHex0,psupHex0)+nhp0; assert(ihc45>=nhp0&&ihc45<nhp0+neh0);
    int ihc56 = findEdge(ihc5,ihc6, psupIndHex0,psupHex0)+nhp0; assert(ihc56>=nhp0&&ihc56<nhp0+neh0);
    int ihc67 = findEdge(ihc6,ihc7, psupIndHex0,psupHex0)+nhp0; assert(ihc67>=nhp0&&ihc67<nhp0+neh0);
    int ihc74 = findEdge(ihc7,ihc4, psupIndHex0,psupHex0)+nhp0; assert(ihc74>=nhp0&&ihc74<nhp0+neh0);
    int ihc04 = findEdge(ihc0,ihc4, psupIndHex0,psupHex0)+nhp0; assert(ihc04>=nhp0&&ihc04<nhp0+neh0);
    int ihc15 = findEdge(ihc1,ihc5, psupIndHex0,psupHex0)+nhp0; assert(ihc15>=nhp0&&ihc15<nhp0+neh0);
    int ihc26 = findEdge(ihc2,ihc6, psupIndHex0,psupHex0)+nhp0; assert(ihc26>=nhp0&&ihc26<nhp0+neh0);
    int ihc37 = findEdge(ihc3,ihc7, psupIndHex0,psupHex0)+nhp0; assert(ihc37>=nhp0&&ihc37<nhp0+neh0);
    int ihc0473 = findFace(ihc0,ihc4,ihc7,ihc3, aQuadHex0,elsupIndQuadHex0,elsupQuadHex0)+nhp0+neh0; assert(ihc0473>=nhp0+neh0&&ihc0473<nhp0+neh0+nfh0);
    int ihc1265 = findFace(ihc1,ihc2,ihc6,ihc5, aQuadHex0,elsupIndQuadHex0,elsupQuadHex0)+nhp0+neh0; assert(ihc1265>=nhp0+neh0&&ihc1265<nhp0+neh0+nfh0);
    int ihc0154 = findFace(ihc0,ihc1,ihc5,ihc4, aQuadHex0,elsupIndQuadHex0,elsupQuadHex0)+nhp0+neh0; assert(ihc0154>=nhp0+neh0&&ihc0154<nhp0+neh0+nfh0);
    int ihc3762 = findFace(ihc3,ihc7,ihc6,ihc2, aQuadHex0,elsupIndQuadHex0,elsupQuadHex0)+nhp0+neh0; assert(ihc3762>=nhp0+neh0&&ihc3762<nhp0+neh0+nfh0);
    int ihc0321 = findFace(ihc0,ihc3,ihc2,ihc1, aQuadHex0,elsupIndQuadHex0,elsupQuadHex0)+nhp0+neh0; assert(ihc0321>=nhp0+neh0&&ihc0321<nhp0+neh0+nfh0);
    int ihc4567 = findFace(ihc4,ihc5,ihc6,ihc7, aQuadHex0,elsupIndQuadHex0,elsupQuadHex0)+nhp0+neh0; assert(ihc4567>=nhp0+neh0&&ihc4567<nhp0+neh0+nfh0);
    int ihc01234567 = ih + nhp0 + neh0 + nfh0;
    //0
    aHex1.push_back(ihc0);
    aHex1.push_back(ihc01);
    aHex1.push_back(ihc0321);
    aHex1.push_back(ihc30);
    aHex1.push_back(ihc04);
    aHex1.push_back(ihc0154);
    aHex1.push_back(ihc01234567);
    aHex1.push_back(ihc0473);
    //1
    aHex1.push_back(ihc01);
    aHex1.push_back(ihc1);
    aHex1.push_back(ihc12);
    aHex1.push_back(ihc0321);
    aHex1.push_back(ihc0154);
    aHex1.push_back(ihc15);
    aHex1.push_back(ihc1265);
    aHex1.push_back(ihc01234567);
    //2
    aHex1.push_back(ihc0321);
    aHex1.push_back(ihc12);
    aHex1.push_back(ihc2);
    aHex1.push_back(ihc23);
    aHex1.push_back(ihc01234567);
    aHex1.push_back(ihc1265);
    aHex1.push_back(ihc26);
    aHex1.push_back(ihc3762);
    //3
    aHex1.push_back(ihc30);
    aHex1.push_back(ihc0321);
    aHex1.push_back(ihc23);
    aHex1.push_back(ihc3);
    aHex1.push_back(ihc0473);
    aHex1.push_back(ihc01234567);
    aHex1.push_back(ihc3762);
    aHex1.push_back(ihc37);
    //4
    aHex1.push_back(ihc04);
    aHex1.push_back(ihc0154);
    aHex1.push_back(ihc01234567);
    aHex1.push_back(ihc0473);
    aHex1.push_back(ihc4);
    aHex1.push_back(ihc45);
    aHex1.push_back(ihc4567);
    aHex1.push_back(ihc74);
    //5
    aHex1.push_back(ihc0154);
    aHex1.push_back(ihc15);
    aHex1.push_back(ihc1265);
    aHex1.push_back(ihc01234567);
    aHex1.push_back(ihc45);
    aHex1.push_back(ihc5);
    aHex1.push_back(ihc56);
    aHex1.push_back(ihc4567);
    //6
    aHex1.push_back(ihc01234567);
    aHex1.push_back(ihc1265);
    aHex1.push_back(ihc26);
    aHex1.push_back(ihc3762);
    aHex1.push_back(ihc4567);
    aHex1.push_back(ihc56);
    aHex1.push_back(ihc6);
    aHex1.push_back(ihc67);
    //7
    aHex1.push_back(ihc0473);
    aHex1.push_back(ihc01234567);
    aHex1.push_back(ihc3762);
    aHex1.push_back(ihc37);
    aHex1.push_back(ihc74);
    aHex1.push_back(ihc4567);
    aHex1.push_back(ihc67);
    aHex1.push_back(ihc7);
  }
}

