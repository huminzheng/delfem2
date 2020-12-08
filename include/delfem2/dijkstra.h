/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**
 * @file geodesic & clustering on mesh using dijkstra method
 */

#ifndef DFM2_DIJKSTRA_H
#define DFM2_DIJKSTRA_H

#include "delfem2/dfm2_inline.h"
#include <queue>
#include <climits>
#include <random>

namespace delfem2 {

namespace dijkstra {

double Distance3(
    const double p0[3], const double p1[3])
{
  return sqrt( (p1[0]-p0[0])*(p1[0]-p0[0]) + (p1[1]-p0[1])*(p1[1]-p0[1]) + (p1[2]-p0[2])*(p1[2]-p0[2]) );
}

template <typename DISTANCE>
class CNode
{
public:
  CNode(unsigned int ino, DISTANCE dist_)
  : ind(ino), dist(dist_){}
  bool operator < (const CNode& lhs) const {
    return this->dist > lhs.dist;
  }
public:
  unsigned int ind;
  DISTANCE dist;
};

}


void DijkstraElem_MeshElemTopo(
    std::vector<unsigned int> &aDist,
    std::vector<unsigned int>& aOrder,
    //
    unsigned int ielm_ker,
    const std::vector<unsigned int> &aElSuEl,
    unsigned int nelem)
{
  aOrder.assign(nelem,UINT_MAX);
  aDist.assign(nelem, UINT_MAX);
  aDist[ielm_ker] = 0;
  const unsigned int nedge = aElSuEl.size() / nelem;
  std::priority_queue<dijkstra::CNode<unsigned int>> que;
  que.push(dijkstra::CNode<unsigned int>(ielm_ker, 0));
  unsigned int icnt = 0;
  while (!que.empty()) {
    const unsigned int ielm0 = que.top().ind;
    const unsigned int idist0 = que.top().dist;
    que.pop();
    if( aOrder[ielm0] != UINT_MAX ){ continue; } // already fixed so this is not the shortest path
    aOrder[ielm0] = icnt; // found shortest path
    icnt++;
    for (unsigned int iedge = 0; iedge < nedge; ++iedge) {
      const unsigned int ielm1 = aElSuEl[ielm0 * nedge + iedge];
      if (ielm1 == UINT_MAX) { continue; }
      const unsigned int idist1 = idist0+1;
      if (idist1 >= aDist[ielm1]) { continue; }
      aDist[ielm1] = idist1; // Found the shortest path so far
      que.push(dijkstra::CNode<unsigned int>(ielm1, idist1)); // candidate of shortest path
    }
  }
  assert(icnt==nelem);
}

void Center_Elem3(
    double p[3],
    unsigned int ielm1,
    const std::vector<unsigned int> &aTri,
    unsigned int nnoel,
    const std::vector<double> &aXYZ)
{
  p[0] = 0.;
  p[1] = 0.;
  p[2] = 0.;
  for(unsigned int inoel=0;inoel<nnoel;++inoel){
    const unsigned int ip0 = aTri[ielm1*nnoel+inoel];
    p[0] += aXYZ[ ip0*3+0 ];
    p[1] += aXYZ[ ip0*3+1 ];
    p[2] += aXYZ[ ip0*3+2 ];
  }
  p[0] /= nnoel;
  p[1] /= nnoel;
  p[2] /= nnoel;
}

template <typename PROC>
void DijkstraElem_MeshElemGeo3(
    std::vector<double> &aDist,
    std::vector<unsigned int>& aOrder,
    PROC& proc,
    //
    unsigned int ielm_ker,
    const std::vector<unsigned int> &aTri,
    const unsigned int nelem,
    const std::vector<double> &aXYZ,
    const std::vector<unsigned int> &aElSuEl)
{
  aOrder.assign(nelem,UINT_MAX);
  aDist.assign(nelem, -1.0);
  aDist[ielm_ker] = 0.0;
  const unsigned int nedge = aElSuEl.size() / nelem;
  const unsigned int nnoel = aTri.size() / nelem;
  std::priority_queue<dijkstra::CNode<double>> que;
  que.push(dijkstra::CNode<double>(ielm_ker, 0.));
  unsigned int icnt = 0;
  while (!que.empty()) {
    const unsigned int ielm0 = que.top().ind;
    const double idist0 = que.top().dist;
    double p0[3]; Center_Elem3(p0, ielm0,aTri,nnoel,aXYZ);
    que.pop();
    if( aOrder[ielm0] != UINT_MAX ){ continue; } // already fixed so this is not the shortest path
    aOrder[ielm0] = icnt; // found shortest path
    proc.AddElem(ielm0,aOrder);
    icnt++;
    for (unsigned int iedge = 0; iedge < nedge; ++iedge) {
      const unsigned int ielm1 = aElSuEl[ielm0 * nedge + iedge];
      if (ielm1 == UINT_MAX) { continue; }
      double p1[3]; Center_Elem3(p1, ielm1,aTri,nnoel,aXYZ);
      const double idist1 = idist0+dijkstra::Distance3(p0,p1);
      if ( aDist[ielm1] > -0.1 && aDist[ielm1] < idist1 ) { continue; }
      aDist[ielm1] = idist1; // Found the shortest path so far
      que.push(dijkstra::CNode<double>(ielm1, idist1)); // candidate of shortest path
    }
  }
  assert(icnt==nelem);
}

void MeshClustering(
    std::vector<unsigned int> &aFlgElm,
    //
    unsigned int ncluster,
    const std::vector<unsigned int> &aTriSuTri,
    unsigned int ntri)
{
  std::vector<unsigned int> aDist0(ntri, UINT_MAX);
  std::random_device rd;
  std::mt19937 rdeng(rd());
  std::uniform_int_distribution<unsigned int> dist0(0, ntri - 1);
  const unsigned int itri_ker = dist0(rdeng);
  assert(itri_ker < ntri);
  aFlgElm.assign(ntri, 0);
  std::vector<unsigned int> aOrder;
  DijkstraElem_MeshElemTopo(
      aDist0,aOrder,
      itri_ker, aTriSuTri,ntri);
  for (unsigned int icluster = 1; icluster < ncluster; ++icluster) {
    unsigned int itri_maxdist;
    { // find triangle with maximum distance
      double idist_max = 0;
      for (unsigned int it = 0; it < ntri; ++it) {
        if (aDist0[it] <= idist_max) { continue; }
        idist_max = aDist0[it];
        itri_maxdist = it;
      }
    }
    std::vector<unsigned int> aDist1(ntri, UINT_MAX);
    DijkstraElem_MeshElemTopo(
        aDist1,aOrder,
        itri_maxdist, aTriSuTri,ntri);
    for (unsigned int it = 0; it < ntri; ++it) {
      if (aDist1[it] < aDist0[it]) {
        aDist0[it] = aDist1[it];
        aFlgElm[it] = icluster;
      }
    }
  }
}

template <typename PROC>
void DijkstraPoint_MeshTri3D(
    std::vector<double>& aDist,
    std::vector<unsigned int>& aOrder,
    PROC& proc,
    //
    unsigned int ip_ker,
    const std::vector<double>& aXYZ,
    const std::vector<unsigned int>& psup_ind,
    const std::vector<unsigned int>& psup)
{
  const unsigned int np = aXYZ.size()/3;
  aOrder.assign(np,UINT_MAX);
  aDist.assign(np,-1.);
  std::priority_queue<dijkstra::CNode<double>> que;
  unsigned int icnt = 0;
  que.push(dijkstra::CNode<double>(ip_ker, 0.0));
  while(!que.empty()) {
    const unsigned int ip0 = que.top().ind; assert(ip0<np);
    const double dist0 = que.top().dist;
    que.pop();
    if( aOrder[ip0] != UINT_MAX ){ continue; } // already fixed (this wes necessary)
    aOrder[ip0] = icnt;
    proc.AddPoint(ip0,aOrder);
    icnt++;
    for(unsigned int ipsup=psup_ind[ip0];ipsup<psup_ind[ip0+1];++ipsup) {
      const unsigned int ip1 = psup[ipsup];
      const double len01 = dijkstra::Distance3(aXYZ.data()+ip0*3,aXYZ.data()+ip1*3);
      const double dist1 = dist0 + len01;
      if( aDist[ip1] >= -0.1 && aDist[ip1] < dist1 ){ continue; }
      aDist[ip1] = dist1; // shortest distance so far
      que.push(dijkstra::CNode<double>(ip1,dist1)); // add candidate for fix
    }
  }
  assert(icnt==np);
}


}

#endif
