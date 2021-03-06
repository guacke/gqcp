// This file is part of GQCG-gqcp.
// 
// Copyright (C) 2017-2019  the GQCG developers
// 
// GQCG-gqcp is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// GQCG-gqcp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with GQCG-gqcp.  If not, see <http://www.gnu.org/licenses/>.
// 
#ifndef Tensor_h
#define Tensor_h

#include "typedefs.hpp"
#include "math/Matrix.hpp"

#include <unsupported/Eigen/CXX11/Tensor>


namespace GQCP {


/**
 *  An extension of the Eigen::Tensor class, with extra operations
 *
 *  @tparam _Scalar     the scalar representation type
 *  @tparam _Rank       the rank of the tensor, i.e. the number of indices
 *
 *  We have decided to inherit from Eigen::Tensor, because we will use different hierarchies: see also: https://eigen.tuxfamily.org/dox-devel/TopicCustomizing_InheritingMatrix.html
 */
template <typename _Scalar, int _Rank>
class Tensor : public Eigen::Tensor<_Scalar, _Rank> {
public:

    using Scalar = _Scalar;
    enum {
        Rank = _Rank
    };

    using Self = Tensor<Scalar, Rank>;
    using Base = Eigen::Tensor<Scalar, Rank>;


public:

    /*
     *  CONSTRUCTORS
     */

    using Eigen::Tensor<Scalar, Rank>::Tensor;  // inherit base constructors


    /*
     *  NAMED CONSTRUCTORS
     */

    /**
     *  @param T            a rank-4 tensor
     *
     *  @param i            1st starting index
     *  @param j            2nd starting index
     *  @param k            3rd starting index
     *  @param l            4th starting index
     *  @param desize       early cut-off of index iteration
     *
     *  @return a rank-4 tensor from an other rank-4 tensor, starting from given indices
     */
    template <int Z = Rank>
    static enable_if_t<Z == 4, Self> FromBlock(const Self& T, size_t i, size_t j, size_t k, size_t l, size_t desize=0) {

        Tensor<double, Rank> T_result (T.dimension(0) - i - desize, T.dimension(1) - j - desize, T.dimension(2) - k - desize, T.dimension(3) - l - desize);
        T_result.setZero();

        for (size_t p = 0; p < T_result.dimension(0); p++) {
            for (size_t q = 0; q < T_result.dimension(1); q++) {
                for (size_t r = 0; r < T_result.dimension(2); r++) {
                    for (size_t s = 0; s < T_result.dimension(3); s++) {
                        T_result(p, q, r, s) = T(i + p, j + q, k + r, l + s);
                    }
                }
            }
        }

        return T_result;
    }


    /*
     *  PUBLIC METHODS
     */

    /**
     *  @return this as an Eigen::Tensor, as a work-around to fix Eigen::Tensor expressions
     */
    const Base& Eigen() const {
        return static_cast<const Base&>(*this);
    }


    /**
     *  @param other        the other tensor
     *
     *  @return if this tensor has the same dimensions as the other tensor
     */
    bool hasEqualDimensions(const Self& other) const {

        for (size_t i = 0; i < Rank; i++) {
            if (this->dimension(i) != other.dimension(i)) {
                return false;
            }
        }

        return true;
    }


    /**
     *  @param other        the other tensor
     *  @param tolerance    the tolerance for element-wise comparison
     *
     *  @return if this is approximately equal to the other
     */
    template <int Z = Rank>
    enable_if_t<Z == 4, bool> isApprox(const Self& other, double tolerance = 1.0e-12) const {

        if (!this->hasEqualDimensions(other)) {
            throw std::invalid_argument("RankFourTensor<Scalar>::isApprox(Self, double): the tensors have different dimensions");
        }

        // Check every pair of values
        for (size_t i = 0; i < this->dimension(0); i++) {
            for (size_t j = 0; j < this->dimension(1); j++) {
                for (size_t k = 0; k < this->dimension(2); k++) {
                    for (size_t l = 0; l < this->dimension(3); l++) {
                        if (std::abs(this->operator()(i,j,k,l) - other(i,j,k,l)) > tolerance) {
                            return false;
                        }
                    }
                }
            }
        }  // rank-4 tensor traversing

        return true;
    }


    /**
     *  Print the contents of this to an output stream
     *
     *  @param output_stream        the stream used for outputting
     */
    template <int Z = Rank>
    enable_if_t<Z == 4> print(std::ostream& output_stream = std::cout) const {

        for (size_t i = 0; i < this->dimension(0); i++) {
            for (size_t j = 0; j < this->dimension(1); j++) {
                for (size_t k = 0; k < this->dimension(2); k++) {
                    for (size_t l = 0; l < this->dimension(3); l++) {
                        output_stream << i << ' ' << j << ' ' << k << ' ' << l << "  " << this->operator()(i,j,k,l) << std::endl;
                    }
                }
            }
        }
    }


    /**
     *  @return a pair-wise reduced form of this rank-4 tensor. The elements of the tensor are put into the matrix such that
     *      M(m,n) = T(i,j,k,l)
     *
     *  in which
     *      m is calculated from i and j in a column-major way
     *      n is calculated from k and l in a column-major way
     */
    template <int Z = Rank>
    enable_if_t<Z == 4, Matrix<Scalar>> pairWiseReduce() const {

        // Initialize the resulting matrix
        const auto dims = this->dimensions();
        Matrix<Scalar> M (dims[0]*dims[1], dims[2]*dims[3]);


        // Calculate the compound indices and bring the elements from the tensor over into the matrix
        size_t row_index = 0;
        for (size_t j = 0; j < dims[1]; j++) {  // "column major" ordering for row_index<-i,j so we do j first, then i
            for (size_t i = 0; i < dims[0]; i++) {  // in column major indices, columns are contiguous, so the first of two indices changes more rapidly

                size_t column_index = 0;
                for (size_t l = 0; l < dims[3]; l++) {  // "column major" ordering for column_index<-k,l so we do l first, then k
                    for (size_t k = 0; k < dims[2]; k++) {  // in column major indices, columns are contiguous, so the first of two indices changes more rapidly

                        M(row_index,column_index) = this->operator()(i,j,k,l);

                        column_index++;
                    }
                }

                row_index++;
            }
        }

        return M;
    }


    /**
     *  Add a rank-4 tensor into this, starting from given indices
     *
     *  @param T        a rank-4 tensor
     *  @param i        1st starting index
     *  @param j        2nd starting index
     *  @param k        3rd starting index
     *  @param l        4th starting index
     *
     *  @return a reference to updated this
     */
    template <int Z = Rank>
    enable_if_t<Z == 4, Self&> addBlock(const Self& T, size_t i, size_t j, size_t k, size_t l) {

        for (size_t p = 0; p < T.dimension(0); p++) {
            for (size_t q = 0; q < T.dimension(1); q++) {
                for (size_t r = 0; r < T.dimension(2); r++) {
                    for (size_t s = 0; s < T.dimension(3); s++) {
                        this->operator()(i+p, j+q, k+r, l+s) += T(p,q,r,s);
                    }
                }
            }
        }

        return (*this);
    }


    /**
     *  Add a matrix to a this tensor starting from given indices
     *
     *  @tparam r               indicates which starting index (0,1,2,3) should correspond to the 1st matrix index
     *  @tparam s               indicates which starting index (0,1,2,3) should correspond to the 2nd matrix index
     *
     *  @param M                a matrix
     *  @param i                1st starting index
     *  @param j                2nd starting index
     *  @param k                3rd starting index
     *  @param l                4th starting index
     *
     *  @return a reference to updated this
     *
     *
     *  Example:
     *      Given a rank-4 tensor of dimensions (10,10,10,10), and a matrix of dimensions (3,3)
     *      When choosing r and s as 2,1 respectively, and i,j,k,l as 1,2,1,0
     *      The matrix is added to the tensor with 1st index starting at i and 4th index starting at l held fixed at 1 and 0 respectively
     *      While 2nd index starting from j (2) now is mapped to the 2nd matrix (s=1) element and 3rd index starting from k (1) is mapped to 1st (r=2)
     *      meaning that the additions follow : tensor(1,2,1,0) += M(0,0), tensor(1,3,1,0) += M(0,1), ..., tensor(1,3,4,0) += M(3,1), tensor(1,4,4,0) += M(3,2), etc
     *      and for other indices we find : tensor(2,2,1,0) += 0 (no changes, these are obviously not performed)
     */
    template<size_t r, size_t s, int Z = Rank>
    enable_if_t<Z == 4, Self&> addBlock(const MatrixX<Scalar>& M, size_t i, size_t j, size_t k, size_t l) {

        // Initialize series of arrays with 1 or 0 values, so that the correct tensor indices given by the template argument correspond to the matrix indices
        size_t ia[4] = {1,0,0,0};
        size_t ja[4] = {0,1,0,0};
        size_t ka[4] = {0,0,1,0};
        size_t la[4] = {0,0,0,1};

        for (size_t x = 0; x < M.rows(); x++) {
            for (size_t y = 0; y < M.cols(); y++) {

                size_t i_effective = i + x * ia[r] + y * ia[s];
                size_t j_effective = j + x * ja[r] + y * ja[s];
                size_t k_effective = k + x * ka[r] + y * ka[s];
                size_t l_effective = l + x * la[r] + y * la[s];

                this->operator()(i_effective, j_effective, k_effective, l_effective) += M(x,y);
            }
        }

        return (*this);
    }
};


}  // namespace GQCP


#endif  /* Tensor_h */
