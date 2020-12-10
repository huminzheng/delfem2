/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef DFM2_MATS_H
#define DFM2_MATS_H

#include "delfem2/dfm2_inline.h"
#include <vector>
#include <cassert>
#include <complex>

namespace delfem2 {

/**
 * @class sparse matrix class
 * @tparam T float, double and std::complex<double>
 */
template<typename T>
class CMatrixSparse {
public:
  CMatrixSparse() : nrowblk(0), ncolblk(0), nrowdim(0), ncoldim(0) {}

  virtual ~CMatrixSparse() {
    this->Clear();
  }
  
  void Clear(){
    colInd.clear();
    rowPtr.clear();
    valCrs.clear();
    valDia.clear();
    this->nrowblk = 0;
    this->nrowdim = 0;
    this->ncolblk = 0;
    this->ncoldim = 0;
  }

  void Initialize(unsigned int nblk, unsigned int len, bool is_dia) {
    this->nrowblk = nblk;
    this->nrowdim = len;
    this->ncolblk = nblk;
    this->ncoldim = len;
    colInd.assign(nblk + 1, 0);
    rowPtr.clear();
    valCrs.clear();
    if (is_dia) { valDia.assign(nblk * len * len, 0.0); }
    else { valDia.clear(); }
  }

  void operator=(const CMatrixSparse &m) {
    this->nrowblk = m.nrowblk;
    this->nrowdim = m.nrowdim;
    this->ncolblk = m.ncolblk;
    this->ncoldim = m.ncoldim;
    colInd = m.colInd;
    rowPtr = m.rowPtr;
    valCrs = m.valCrs;
    valDia = m.valDia; // copy value
  }

  void SetPattern(const unsigned int *colind, size_t ncolind,
                  const unsigned int *rowptr, size_t nrowptr) {
    assert(rowPtr.empty());
    assert(ncolind == nrowblk + 1);
    for (unsigned int iblk = 0; iblk < nrowblk + 1; iblk++) { colInd[iblk] = colind[iblk]; }
    const unsigned int ncrs = colind[nrowblk];
    assert(ncrs == nrowptr);
    rowPtr.resize(ncrs);
    for (unsigned int icrs = 0; icrs < ncrs; icrs++) { rowPtr[icrs] = rowptr[icrs]; }
    valCrs.resize(ncrs * nrowdim * ncoldim);
  }

  bool SetZero() {
    if (valDia.size() != 0) {
      assert(nrowdim == ncoldim);
      assert(nrowblk == ncolblk);
      const unsigned int n = valDia.size();
      assert(n == nrowdim * nrowdim * nrowblk);
      for (unsigned int i = 0; i < n; ++i) { valDia[i] = 0; }
    }
    {
      const unsigned int n = valCrs.size();
      assert(n == nrowdim * ncoldim * rowPtr.size());
      for (unsigned int i = 0; i < n; i++) { valCrs[i] = 0.0; }
    }
    return true;
  }

  bool Mearge(unsigned int nblkel_col, const unsigned int *blkel_col,
              unsigned int nblkel_row, const unsigned int *blkel_row,
              unsigned int blksize, const T *emat,
              std::vector<int> &m_marge_tmp_buffer);

  /**
   * @func Matrix vector product as: {y} = alpha * [A]{x} + beta * {y}
   */
  void MatVec(T *y,
              T alpha, const T *x,
              T beta) const;

  /**
   * @func Matrix vector product as: {y} = alpha * [A]{x} + beta * {y}.
   *  the sparse matrix is regared as block sparse matrix where each blcok is diagonal
   */
  void MatVecDegenerate(T *y,
                        unsigned nlen,
                        T alpha, const T *x,
                        T beta) const;
  
  /**
   * @func Matrix vector product as: {y} = alpha * [A]^T{x} + beta * {y}
   */
  void MatTVec(T *y,
               T alpha, const T *x,
               T beta) const;
  
  /**
   * @func set fixed bc for diagonal block matrix where( pBCFlag[i] != 0).
   */
  void SetFixedBC_Dia(const int *pBCFlag, T val_dia);

  void SetFixedBC_Col(const int *pBCFlag);

  void SetFixedBC_Row(const int *pBCFlag);

  /**
   * @func     if pBCFlag is *not* 0 for a dof, set all the off-diagonal componenet to zero and set diagonal to one.
   * @details pBCFlag need to have memory at least larger than nlen*nblk
   * This matrix need to be a squared matrix
   */
  void SetFixedBC(const int *pBCFlag){
    this->SetFixedBC_Dia(pBCFlag,1.0);
    this->SetFixedBC_Row(pBCFlag);
    this->SetFixedBC_Col(pBCFlag);
  }

  void AddDia(T eps) {
    assert(this->ncolblk == this->nrowblk);
    assert(this->ncoldim == this->nrowdim);
    const int blksize = nrowdim * ncoldim;
    const int nlen = this->nrowdim;
    if (valDia.empty()) { return; }
    for (unsigned int ino = 0; ino < nrowblk; ++ino) {
      for (int ilen = 0; ilen < nlen; ++ilen) {
        valDia[ino * blksize + ilen * nlen + ilen] += eps;
      }
    }
  }

  /**
   * @func add vector to diagonal component
   * @param lm        (in) a lumped mass vector with size of nblk
   * @param scale (in) scaling factor for the lumped mass (typically 1/dt^2).
   * @details the matrix need to be square matrix
   */
  void AddDia_LumpedMass(const T *lm, double scale) {
    assert(this->nblk_row == this->nblk_col);
    assert(this->len_row == this->nrowdim);
    const int blksize = nrowdim * ncoldim;
    const int nlen = this->nrowdim;
    if (valDia.empty()) { return; }
    for (unsigned int iblk = 0; iblk < nrowblk; ++iblk) {
      for (int ilen = 0; ilen < nlen; ++ilen) {
        valDia[iblk * blksize + ilen * nlen + ilen] += lm[iblk];
      }
    }
  }

public:
  unsigned int nrowblk;
  unsigned int ncolblk;
  unsigned int nrowdim;
  unsigned int ncoldim;
  /**
   * @param colInd indeces where the row starts in CRS data structure
   */
  std::vector<unsigned int> colInd;
  /**
   * @param rowPtr index of CRS data structure
   */
  std::vector<unsigned int> rowPtr;
  std::vector<T> valCrs;
  std::vector<T> valDia;
};

DFM2_INLINE double CheckSymmetry(
    const delfem2::CMatrixSparse<double> &mat);
  
DFM2_INLINE void SetMasterSlave(
    delfem2::CMatrixSparse<double> &mat,
    const unsigned int *aMSFlag);

DFM2_INLINE void MatSparse_ScaleBlk_LeftRight(
    delfem2::CMatrixSparse<double> &mat,
    const double *scale);

DFM2_INLINE void MatSparse_ScaleBlkLen_LeftRight(
    delfem2::CMatrixSparse<double> &mat,
    const double *scale);

} // delfem2

#ifdef DFM2_HEADER_ONLY
#  include "delfem2/mats.cpp"
#endif
  
#endif // MATDIA_CRS_H
