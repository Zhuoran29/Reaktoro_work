// Reaktoro is a unified framework for modeling chemically reactive systems.
//
// Copyright (C) 2014-2015 Allan Leal
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include "LU.hpp"

// Eigen includes
#include <Reaktoro/Math/Eigen/LU>

namespace Reaktoro {
namespace {

/// Return true if two matrices are equal.
template<typename DerivedL, typename DerivedR>
auto equal(const Eigen::MatrixBase<DerivedL>& l, const Eigen::MatrixBase<DerivedR>& r) -> bool
{
    return l.rows() == r.rows() && l.cols() == r.cols() && l == r;
}

} // namespace

LU::LU()
{}

LU::LU(MatrixConstRef A)
{
    compute(A);
}

LU::LU(MatrixConstRef A, VectorConstRef W)
{
    compute(A, W);
}

auto LU::empty() const -> bool
{
    return L.size();
}

auto LU::compute(MatrixConstRef A) -> void
{
    // Check if matrix A is equal to the last one used
    if(equal(A, A_last) && !W_last.size())
        return;

    // Update A_last and W_last
    A_last = A;
    W_last.conservativeResize(0);

    // The number of rows and cols of A
    const Index m = A.rows();
    const Index n = A.cols();
    const Index r = std::min(m, n);

    // Compute the full-pivoting LU of A
    Eigen::FullPivLU<Matrix> lu(A);

    // Set the rank of the formula matrix Ae
    rank = lu.rank();

    // Initialize the L, U, P, Q matrices so that P*A*Q = L*U
    L = lu.matrixLU().leftCols(r).triangularView<Eigen::UnitLower>();
    U = lu.matrixLU().topRows(r).triangularView<Eigen::Upper>();
    P = lu.permutationP();
    Q = lu.permutationQ();
}

auto LU::compute(MatrixConstRef A, VectorConstRef W) -> void
{
    // Check if matrix A is equal to the last one used
    if(equal(A, A_last) && equal(W, W_last))
        return;

    // Update A_last and W_last
    A_last = A;
    W_last = W;

    // The number of rows and cols of A
    const Index m = A.rows();
    const Index n = A.cols();
    const Index r = std::min(m, n);

    // Initialize the weighted formula matrix
    const Matrix AW = A * diag(W);

    // Compute the full-pivoting LU of A
    Eigen::FullPivLU<Matrix> lu(tr(AW));

    // Set the rank of the matrix A
    rank = lu.rank();

    // Initialize the L, U, P, Q matrices so that P*A*Q = L*U
    L = tr(lu.matrixLU()).leftCols(r).triangularView<Eigen::Lower>();
    U = tr(lu.matrixLU()).triangularView<Eigen::UnitUpper>();
    P = lu.permutationQ().inverse();
    Q = lu.permutationP().inverse();

    // Correct the U matrix by unscaling it by weights
    U = U * Q.inverse() * diag(inv(W)) * Q;
}

auto LU::solve(MatrixConstRef B) -> Matrix
{
    const Index n = U.cols();
    const Index k = B.cols();
    Matrix X = zeros(n, k);

    auto solve_column = [&](Index icol)
    {
        auto xx = X.col(icol).segment(0, rank);
        xx = (P * B.col(icol)).segment(0, rank);
        xx = L.topLeftCorner(rank, rank).triangularView<Eigen::Lower>().solve(xx);
        xx = U.topLeftCorner(rank, rank).triangularView<Eigen::Upper>().solve(xx);
        X.col(icol) = Q * X.col(icol);
    };

    for(Index i = 0; i < k; ++i)
        solve_column(i);

    return X;
}

auto LU::trsolve(MatrixConstRef B) -> Matrix
{
    const Index m = L.rows();
    const Index k = B.cols();
    Matrix X(m, k);

    auto trsolve_column = [&](Index icol)
    {
        const auto& indices = Q.indices();
        for(Index i = 0; i < rank; ++i)
            X.col(icol)[i] = B.col(icol)[indices[i]];
        auto xx = X.col(icol).segment(0, rank);
        xx = tr(U).topLeftCorner(rank, rank).triangularView<Eigen::Lower>().solve(xx);
        xx = tr(L).topLeftCorner(rank, rank).triangularView<Eigen::Upper>().solve(xx);
        X.col(icol) = P.inverse() * X.col(icol);
    };

    for(Index i = 0; i < k; ++i)
        trsolve_column(i);

    return X;
}

} // namespace Reaktoro
