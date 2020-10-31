/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "delfem2/cnpy/smpl_cnpy.h"
#include "cnpy/cnpy.h"
#include <iostream>
#include <vector>

DFM2_INLINE void delfem2::cnpy::LoadSmpl(
    std::vector<double>& aXYZ0,
    std::vector<double>& aW,
    std::vector<unsigned int>& aTri,
    std::vector<int>& aIndBoneParent,
    std::vector<double>& aJntRgrs,
    const std::string& fpath)
{
  ::cnpy::npz_t my_npz = ::cnpy::npz_load(fpath);
  const unsigned int nP = my_npz["vertices_template"].shape[0];
  assert( my_npz["vertices_template"].shape[1] == 3 );
  aXYZ0 = my_npz["vertices_template"].as_vec<double>();
  {
    ::cnpy::NpyArray& npT = my_npz["face_indices"];
    assert( my_npz["face_indices"].shape[1] == 3 );
    aTri = npT.as_vec<unsigned>();
    for(auto &i: aTri){ i -= 1; }
  }
  const unsigned int nBone = my_npz["weights"].shape[1];
  assert( my_npz["weights"].shape[0] == nP );
  aW = my_npz["weights"].as_vec<double>();
  {
    const ::cnpy::NpyArray& npT = my_npz["kinematic_tree"];
    const int* tree = npT.data<int>();
    aIndBoneParent.assign(tree, tree+nBone);
  }
  assert( my_npz["joint_regressor"].shape[0] == nBone );
  assert( my_npz["joint_regressor"].shape[1] == nP );
  std::cout << my_npz["joint_regressor"].fortran_order << std::endl;
  aJntRgrs = my_npz["joint_regressor"].as_vec<double>();
  assert( aJntRgrs.size() == nBone*nP );
}


