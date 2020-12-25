/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**
 * @file template for iterative solver
 * DONE(Dec. 25th 2020): splitting this file into "vecx.h" and "itersol.h" in the future
 */

#ifndef DFM2_LSITRSOL_H
#define DFM2_LSITRSOL_H

#include "delfem2/dfm2_inline.h"
#include <vector>
#include <cassert>
#include <complex>
#include <iostream>

namespace delfem2 {

// ----------------------------------------------------------------------

/**
 * @brief solve linear system using conjugate gradient method
 * @param mat (in)  a template class with member function "MatVec" with  {y} = alpha*[A]{x} + beta*{y}
 */
template<class MAT, class VEC>
std::vector<double> Solve_CG(
    VEC& r_vec,
    VEC& u_vec,
    VEC& Ap_vec,
    VEC& p_vec,
    double conv_ratio_tol,
    unsigned int max_iteration,
    const MAT& mat)
{
  std::vector<double> aConv;
  u_vec.setZero();
  double sqnorm_res = r_vec.dot(r_vec);
  if (sqnorm_res < 1.0e-30) { return aConv; }
  const double inv_sqnorm_res_ini = 1.0 / sqnorm_res;
  p_vec = r_vec;  // {p} = {r}  (Set Initial Serch Direction)
  for (unsigned int iitr = 0; iitr < max_iteration; iitr++) {
    double alpha;
    {  // alpha = (r,r) / (p,Ap)
      AddMatVec(Ap_vec, 0.0, 1.0, mat, p_vec); // {Ap_vec} = [mat]*{p_vec}
      const double pAp = p_vec.dot(Ap_vec);
//      std::cout << iitr << " pAp:" << pAp << std::endl;
      alpha = sqnorm_res / pAp;
    }
    AddScaledVec(u_vec, +alpha, p_vec);    // {x} = +alpha*{ p} + {x} (update x)
    AddScaledVec(r_vec, -alpha, Ap_vec);  // {r} = -alpha*{Ap} + {r}
    const double sqnorm_res_new = r_vec.dot(r_vec);
    double conv_ratio = sqrt(sqnorm_res_new * inv_sqnorm_res_ini);
//    std::cout << iitr << " " << conv_ratio << std::endl;
    aConv.push_back(conv_ratio);
    if (conv_ratio < conv_ratio_tol) { return aConv; }
    //
    {
      const double beta = sqnorm_res_new / sqnorm_res; // beta = (r1,r1) / (r0,r0)
      sqnorm_res = sqnorm_res_new;
      ScaleAndAddVec(p_vec,beta,r_vec); // {p} = {r} + beta*{p}
      std::cout << iitr << " alpha:" << alpha << "   beta:" << beta << " " << conv_ratio << " " << u_vec.dot(u_vec) << std::endl;
    }
  }
  return aConv;
}


/**
 * @brief solve a real-valued linear system using the conjugate gradient method with preconditioner
 */
template <class MAT, class VEC, class PREC>
std::vector<double> Solve_PCG(
    VEC& r_vec,
    VEC& x_vec,
    VEC& Pr_vec,
    VEC& p_vec,
    double conv_ratio_tol,
    unsigned int max_nitr,
    const MAT &mat,
    const PREC &ilu)
{
  std::vector<double> aResHistry;

  x_vec.setZero(); // for (unsigned int i = 0; i < N; i++) { x_vec[i] = 0; }    // {x} = 0

  double inv_sqnorm_res0;
  {
    const double sqnorm_res0 = r_vec.dot(r_vec); // DotX(r_vec, r_vec, N);
    aResHistry.push_back(sqrt(sqnorm_res0));
    if (sqnorm_res0 < 1.0e-30) { return aResHistry; }
    inv_sqnorm_res0 = 1.0 / sqnorm_res0;
  }

  // {Pr} = [P]{r}
  Pr_vec = r_vec; // std::vector<double> Pr_vec(r_vec, r_vec + N);
  SolvePrecond(Pr_vec,ilu);  // ilu.SolvePrecond(Pr_vec.data());
  // {p} = {Pr}
  p_vec = Pr_vec; // std::vector<double> p_vec = Pr_vec;
  // rPr = ({r},{Pr})
  double rPr = r_vec.dot(Pr_vec); // DotX(r_vec, Pr_vec.data(), N);
  for (unsigned int iitr = 0; iitr < max_nitr; iitr++) {
    {
      VEC& Ap_vec = Pr_vec; // just a name change
      // {Ap} = [A]{p}
      AddMatVec(Ap_vec,0.0, 1.0,mat,p_vec);
//      mat.MatVec( Ap_vec.data(), 1.0, p_vec.data(), 0.0);
      // alpha = ({r},{Pr})/({p},{Ap})
      const double pAp = p_vec.dot(Ap_vec);
      double alpha = rPr / pAp;
      AddScaledVec(r_vec,-alpha,Ap_vec); // AXPY(-alpha, Ap_vec.data(), r_vec, N);       // {r} = -alpha*{Ap} + {r}
      AddScaledVec(x_vec,+alpha,p_vec); //  AXPY(+alpha, p_vec.data(), x_vec, N);       // {x} = +alpha*{p } + {x}
    }
    {  // Converge Judgement
      const double sqnorm_res = r_vec.dot(r_vec); // DotX(r_vec, r_vec, N);
      aResHistry.push_back(sqrt(sqnorm_res));
      const double conv_ratio = sqrt(sqnorm_res * inv_sqnorm_res0);
      if (conv_ratio < conv_ratio_tol) { return aResHistry; }
    }
    {  // calc beta
      // {Pr} = [P]{r}
      Pr_vec = r_vec; // for (unsigned int i = 0; i < N; i++) { Pr_vec[i] = r_vec[i]; }
      SolvePrecond(Pr_vec,ilu); // ilu.SolvePrecond(Pr_vec.data());
      // rPr1 = ({r},{Pr})
      const double rPr1 = r_vec.dot(Pr_vec); // DotX(r_vec, Pr_vec.data(), N);
      // beta = rPr1/rPr
      double beta = rPr1 / rPr;
      rPr = rPr1;
      // {p} = {Pr} + beta*{p}
      ScaleAndAddVec(p_vec,beta,Pr_vec); // for (unsigned int i = 0; i < N; i++) { p_vec[i] = Pr_vec[i] + beta * p_vec[i]; }
    }
  }
  {
    // Converge Judgement
    double sq_norm_res = r_vec.dot(r_vec); // DotX(r_vec, r_vec, N);
    aResHistry.push_back(sqrt(sq_norm_res));
  }
  return aResHistry;
}


} // delfem2

  
#endif // DFM2_LSITRSOL
