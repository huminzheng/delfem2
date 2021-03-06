/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**
 * @file functions to analyze mesh topology for static meshes
 * @details the functions only care about the topology. Geometry (coordinate) information is not handled in this file
 */

// DONE(2020/12/23): change name mshuni.h
// DODO(2020/12/23): separaete jarray.h
// DONE(2020/12/12): separated mshsubdiv
// DONE(2020/12/09): separate mixed elem

#ifndef DFM2_MSHUNI_H
#define DFM2_MSHUNI_H

#include "delfem2/mshelm.h"
#include "delfem2/dfm2_inline.h"
#include <cstdio>
#include <vector>

namespace delfem2 {
  
// ---------------------------------------------
// function related to jagged array

DFM2_INLINE void JArray_AddMasterSlavePattern(
    std::vector<unsigned int> &index,
    std::vector<unsigned int> &array,
    const unsigned int* aMSFlag,
    int ndim,
    const unsigned int *psup_ind0,
    int npsup_ind0,
    const unsigned int *psup0);
// ---------------------------------------------------

DFM2_INLINE unsigned FindAdjEdgeIndex(
    unsigned int itri,
    unsigned int ied,
    unsigned int jtri,
    const unsigned int* aTri);

DFM2_INLINE void convert2Tri_Quad(
    std::vector<unsigned int>& aTri,
    const std::vector<unsigned int>& aQuad);


/**
 * @brief Make quad mesh from tri mesh by merging adjacent triangle elements
 * @param aQuad (out) element index of quad mesh
 * @param aTri (in) element index of tri mesh
 * @param nTri (in) number of triangle mesh
 * @param np (in) number of points
 */
DFM2_INLINE void ElemQuad_DihedralTri(
    std::vector<unsigned int>& aQuad,
    const unsigned int* aTri,
    unsigned int nTri,
    unsigned int np);

DFM2_INLINE void FlipElement_Tri(
    std::vector<unsigned int>& aTri);


/**
 * @brief make elem surrounding point
 */
DFM2_INLINE void JArray_ElSuP_MeshElem(
    std::vector<unsigned int> &elsup_ind,
    std::vector<unsigned int> &elsup,
    //
    const unsigned int *pElem,
    size_t nElem,
    unsigned int nPoEl,
    size_t nPo);

/**
 * @brief make elem surrounding point for triangle mesh
 */
DFM2_INLINE void JArray_ElSuP_MeshTri(
    std::vector<unsigned int> &elsup_ind,
    std::vector<unsigned int> &elsup,
    //
    const std::vector<unsigned int> &aTri,
    int nXYZ);



// -----------------
// elem sur elem

/**
 * @brief compute adjacent element index for mesh element
 * @param aElSuEl (out) neighbouring element index (UINT_MAX for boundary)
 * @param aEl (in) array of connectivity
 * @param nEl (in) number of elements
 * @param nNoEl (in) number of nodes in a element
 * @param elsup_ind (in) jagged array index of "elem surrounding point"
 * @param elsup (in) jagged array value of "elem surrounding point"
 * @param nfael (in) number of neibouring elements
 * @param nnofa (in) how many nodes are shared with a nighbouring element
 * @param noelElemFace
 */
DFM2_INLINE void ElSuEl_MeshElem(
    std::vector<unsigned int> &aElSuEl,
    const unsigned int *aEl,
    size_t nEl,
    int nNoEl,
    const std::vector<unsigned int> &elsup_ind,
    const std::vector<unsigned int> &elsup,
    int nfael,
    int nnofa,
    const int (*noelElemFace)[4]);

/**
 * @brief compute adjacent element index for mesh element
 * @param aElSuEl (ou) adjacent element index for element edge/face (UINT_MAX if face/edge is on the boundary)
 * @param aElem (in) elemnet index
 * @param nElem (in) number of elements
 * @param type (in) type of element
 * @param nXYZ (in) number of points
 */
DFM2_INLINE void ElSuEl_MeshElem(
    std::vector<unsigned int> &aElSuEl,
    const unsigned int *aElem, size_t nElem,
    delfem2::MESHELEM_TYPE type,
    size_t nXYZ);

/**
 * @brief make point surrounding point
 * @details psup -> edge bidirectional
 * edge unidir (ip0<ip1)
 * line (array of 2)
 */
DFM2_INLINE void JArrayPointSurPoint_MeshOneRingNeighborhood(
    std::vector<unsigned int>& psup_ind,
    std::vector<unsigned int>& psup,
    //
    const unsigned int* pElem,
    const std::vector<unsigned int> &elsup_ind,
    const std::vector<unsigned int> &elsup,
    unsigned int nnoel,
    size_t nPoint);

/**
 * @brief compute indexes of points surrounding a point as a jagged array
 * @param nPoEl number of nodes in an element 
 */
DFM2_INLINE void JArray_PSuP_MeshElem(
    std::vector<unsigned int>& psup_ind,
    std::vector<unsigned int>& psup,
    //
    const unsigned int* pElem,
    size_t nEl,
    unsigned int nPoEl,
    size_t nPo);

DFM2_INLINE void makeOneRingNeighborhood_TriFan(
    std::vector<int>& psup_ind,
    std::vector<int>& psup,
    //
    const std::vector<int>& aTri,
    const std::vector<int>& aTriSurRel,
    const std::vector<int>& elsup_ind,
    const std::vector<int>& elsup,
    int np);

DFM2_INLINE void JArrayEdge_MeshElem(
    std::vector<unsigned int> &edge_ind,
    std::vector<unsigned int> &edge,
    //
    const unsigned int* aElm0,
    delfem2::MESHELEM_TYPE elem_type,
    const std::vector<unsigned int> &elsup_ind,
    const std::vector<unsigned int> &elsup,
    bool is_bidirectional);
  
DFM2_INLINE void MeshLine_JArrayEdge(
    std::vector<unsigned int>& aLine,
    //
    const std::vector<unsigned int> &psup_ind,
    const std::vector<unsigned int> &psup);

DFM2_INLINE void MeshLine_MeshElem(
    std::vector<unsigned int>& aLine,
    //
    const unsigned int* aElm0,
    unsigned int nElem,
    delfem2::MESHELEM_TYPE elem_type,
    unsigned int nPo);

// ------------------------------------

DFM2_INLINE void MarkConnectedElements(
    std::vector<int>& aIndGroup,
    int itri_ker,
    int igroup,
    const std::vector<int>& aTriSurRel,
    int nfael);

DFM2_INLINE void MakeGroupElem(
    int& ngroup,
    std::vector<int>& aIndGroup,
    const std::vector<int>& aElem,
    const std::vector<int>& aElemSurRel,
    int nfael,
    int nnoel);

DFM2_INLINE void MakeGroupElem_Tri(
    int& ngroup,
    std::vector<int>& aIndGroup,
    const std::vector<int>& aTri,
    const std::vector<int>& aTriSurRel);

} // end namespace delfem2

#ifdef DFM2_HEADER_ONLY
#  include "delfem2/mshuni.cpp"
#endif
 
#endif /* meshtopo_hpp */
