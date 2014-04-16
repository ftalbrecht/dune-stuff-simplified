// This file is part of the dune-stuff project:
//   http://users.dune-project.org/projects/dune-stuff
// Copyright holders: Rene Milk, Felix Schindler
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

#ifndef LOCALMATRIX_PROXY_HH
#define LOCALMATRIX_PROXY_HH

#include <vector>
#include <assert.h>
#include <dune/common/float_cmp.hh>
#include <dune/stuff/common/debug.hh>
#include <dune/stuff/common/type_utils.hh>
#include <dune/stuff/aliases.hh>

namespace Dune {
namespace Stuff {
namespace Fem {


template <class MatrixObjectType, class Enable = void>
struct LocalMatrixProxyTraits
{
    typedef typename MatrixObjectType::LocalMatrixType LocalMatrixType;
    typedef typename MatrixObjectType::DomainSpaceType DomainSpaceType;
    typedef typename MatrixObjectType::RangeSpaceType RangeSpaceType;
    typedef typename MatrixObjectType::DomainSpaceType::GridType GridType;
    typedef typename MatrixObjectType::DomainSpaceType::EntityType DomainEntityType;
    typedef typename MatrixObjectType::RangeSpaceType::EntityType RangeEntityType;
    typedef typename MatrixObjectType::RangeSpaceType::RangeFieldType FieldType;
};


template <class MatrixObjectType>
struct LocalMatrixProxyTraits<MatrixObjectType, typename std::enable_if<DSC::is_smart_ptr<MatrixObjectType>::value>::type>
{
    typedef typename MatrixObjectType::element_type::LocalMatrixType LocalMatrixType;
    typedef typename MatrixObjectType::element_type::DomainSpaceType DomainSpaceType;
    typedef typename MatrixObjectType::element_type::RangeSpaceType RangeSpaceType;
    typedef typename MatrixObjectType::element_type::DomainSpaceType::GridType GridType;
    typedef typename MatrixObjectType::element_type::DomainSpaceType::EntityType DomainEntityType;
    typedef typename MatrixObjectType::element_type::RangeSpaceType::EntityType RangeEntityType;
    typedef typename MatrixObjectType::element_type::RangeSpaceType::RangeFieldType FieldType;
};

//! a small proxy object that automagically prevents near-0 value fill-in
template< class MatrixObjectType>
class LocalMatrixProxy
{
  typedef LocalMatrixProxyTraits<MatrixObjectType> TraitsType;
  typedef typename TraitsType::LocalMatrixType LocalMatrixType;
  typedef typename TraitsType::DomainSpaceType::GridType GridType;
  typedef typename TraitsType::DomainSpaceType::EntityType DomainEntityType;
  typedef typename TraitsType::RangeSpaceType::EntityType RangeEntityType;
  typedef typename TraitsType::RangeSpaceType::RangeFieldType FieldType;
  typedef Dune::FloatCmpOps<FieldType> CompareType;
  typedef Dune::FloatCmp::DefaultEpsilon<typename CompareType::EpsilonType, CompareType::cstyle> DefaultEpsilon;
  LocalMatrixType local_matrix_;
  const double eps_;
  const unsigned int rows_;
  const unsigned int cols_;
  std::vector< FieldType > entries_;

public:
  LocalMatrixProxy(MatrixObjectType& object, const DomainEntityType& self, const RangeEntityType& neigh,
                   const double eps = DefaultEpsilon::value())
    : local_matrix_( DSC::PtrCaller<MatrixObjectType>::call(object).localMatrix(self, neigh) )
    , eps_(eps)
    , rows_( local_matrix_.rows() )
    , cols_( local_matrix_.columns() )
    , entries_( rows_ * cols_, FieldType(0.0) )
  {}

  inline void add(const unsigned int row, const unsigned int col, const FieldType val) {
    ASSERT_LT(row, rows_);
    ASSERT_LT(col, cols_);
    entries_[row * cols_ + col] += val;
  }

#if DUNE_FEM_IS_LOCALFUNCTIONS_COMPATIBLE
  auto domainBaseFunctionSet() const -> decltype(local_matrix_.domainBaseFunctionSet()) {
    return local_matrix_.domainBaseFunctionSet();
  }

  auto rangeBaseFunctionSet() const -> decltype(local_matrix_.rangeBaseFunctionSet()) {
    return local_matrix_.rangeBaseFunctionSet();
  }
#else
  auto domainBasisFunctionSet() const -> decltype(local_matrix_.domainBasisFunctionSet()) {
    return local_matrix_.domainBasisFunctionSet();
  }

  auto rangeBasisFunctionSet() const -> decltype(local_matrix_.rangeBasisFunctionSet()) {
    return local_matrix_.rangeBasisFunctionSet();
  }
#endif

  ~LocalMatrixProxy() {
    const auto comp = CompareType(eps_);
    for (unsigned int i = 0; i < rows_; ++i)
    {
      for (unsigned int j = 0; j < cols_; ++j)
      {
        const FieldType& i_j = entries_[i * cols_ + j];
        if ( comp.ne(i_j, 0.0) )
          local_matrix_.add(i, j, i_j);
      }
    }
  }

  unsigned int rows() const { return rows_; }
  unsigned int cols() const { return cols_; }
  unsigned int columns() const { return cols_; }

  void unitRow(unsigned int row)
  {
    local_matrix_.clearRow(row);
    local_matrix_.set(row, row, 1.);
  }
};

} // namespace Fem
} // namespace Stuff
} // namespace Dune

#endif // LOCALMATRIX_PROXY_HH
/** Copyright (c) 2012, Rene Milk
   * All rights reserved.
   *
   * Redistribution and use in source and binary forms, with or without
   * modification, are permitted provided that the following conditions are met:
   *
   * 1. Redistributions of source code must retain the above copyright notice, this
   *    list of conditions and the following disclaimer.
   * 2. Redistributions in binary form must reproduce the above copyright notice,
   *    this list of conditions and the following disclaimer in the documentation
   *    and/or other materials provided with the distribution.
   *
   * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
   * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   *
   * The views and conclusions contained in the software and documentation are those
   * of the authors and should not be interpreted as representing official policies,
   * either expressed or implied, of the FreeBSD Project.
   **/
