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
#define BOOST_TEST_MODULE "HamiltonianParameters"
#include <boost/math/constants/constants.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/test/included/unit_test.hpp>  // include this to get main(), otherwise the compiler will complain

#include "HamiltonianParameters/HamiltonianParameters.hpp"

#include "utilities/miscellaneous.hpp"
#include "RHF/PlainRHFSCFSolver.hpp"
#include "utilities/linalg.hpp"



/*
 *  HELPER FUNCTIONS
 */

/**
 *  @return a toy 2-RDM where
 *      d(i,j,k,l) = l + 2k + 4j + 8i
 */
GQCP::TwoRDM<double> calculateToy2RDMTensor() {

    GQCP::TwoRDM<double> d (2);
    for (size_t i = 0; i < 2; i++) {
        for (size_t j = 0; j < 2; j++) {
            for (size_t k = 0; k < 2; k++) {
                for (size_t l = 0; l < 2; l++) {
                    auto i_ = static_cast<double>(i);
                    auto j_ = static_cast<double>(j);
                    auto k_ = static_cast<double>(k);
                    auto l_ = static_cast<double>(l);

                    d(i,j,k,l) = l_ + 2*k_ + 4*j_ + 8*i_;
                }
            }
        }
    }

    return d;
};



/**
 *  @return toy 2-electron integrals where
 *      g(i,j,k,l) = delta_ij delta_kl - delta_il delta_jk
 */
GQCP::TwoElectronOperator<double> calculateToyTwoElectronIntegralsTensor() {

    GQCP::TwoElectronOperator<double> g (2);
    g.setZero();

    for (size_t i = 0; i < 2; i++) {
        for (size_t j = 0; j < 2; j++) {
            for (size_t k = 0; k < 2; k++) {
                for (size_t l = 0; l < 2; l++) {
                    if ((i == j) and (k == l)) {
                        g(i,j,k,l) += 1;
                    }

                    if ((i == l) and (j == k)) {
                        g(i,j,k,l) -= 1;
                    }
                }
            }
        }
    }

    return g;
};



/*
 *  UNIT TESTS - CONSTRUCTORS
 */

BOOST_AUTO_TEST_CASE ( HamiltonianParameters_constructor ) {

    // Create an AOBasis
    auto water = GQCP::Molecule::Readxyz("data/h2o.xyz");
    auto ao_basis_ptr = std::make_shared<GQCP::AOBasis>(water, "STO-3G");


    // Create One- and TwoElectronOperators (and a transformation matrix) with compatible dimensions
    size_t K = ao_basis_ptr->get_number_of_basis_functions();
    GQCP::OneElectronOperator<double> S = GQCP::OneElectronOperator<double>::Random(K, K);
    GQCP::OneElectronOperator<double> H_core = GQCP::OneElectronOperator<double>::Random(K, K);

    GQCP::TwoElectronOperator<double> g (K);
    g.setRandom();

    GQCP::SquareMatrix<double> C = GQCP::SquareMatrix<double>::Random(K, K);


    // Check if a correct constructor works
    GQCP::HamiltonianParameters<double> random_hamiltonian_parameters (ao_basis_ptr, S, H_core, g, C);


    // Check if wrong arguments result in a throw
    GQCP::OneElectronOperator<double> S_faulty = GQCP::OneElectronOperator<double>::Random(K+1, K+1);
    GQCP::OneElectronOperator<double> H_core_faulty = GQCP::OneElectronOperator<double>::Random(K+1, K+1);

    GQCP::TwoElectronOperator<double> g_faulty (K+1);

    GQCP::SquareMatrix<double> C_faulty = GQCP::SquareMatrix<double>::Random(K+1, K+1);

    BOOST_CHECK_THROW(GQCP::HamiltonianParameters<double> (ao_basis_ptr, S_faulty, H_core, g, C), std::invalid_argument);
    BOOST_CHECK_THROW(GQCP::HamiltonianParameters<double> (ao_basis_ptr, S, H_core_faulty, g, C), std::invalid_argument);
    BOOST_CHECK_THROW(GQCP::HamiltonianParameters<double> (ao_basis_ptr, S, H_core, g_faulty, C), std::invalid_argument);
    BOOST_CHECK_THROW(GQCP::HamiltonianParameters<double> (ao_basis_ptr, S, H_core, g, C_faulty), std::invalid_argument);

    // Check if we can't use a zero matrix as overlap matrix
    GQCP::OneElectronOperator<double> S_zero = GQCP::OneElectronOperator<double>::Zero(K, K);
    BOOST_CHECK_THROW(GQCP::HamiltonianParameters<double> (ao_basis_ptr, S_zero, H_core, g, C), std::invalid_argument);
}


BOOST_AUTO_TEST_CASE ( rotate_argument ) {

    // Create well-behaved Hamiltonian parameters
    size_t K = 3;
    GQCP::OneElectronOperator<double> S_op = GQCP::OneElectronOperator<double>::Random(K, K);
    GQCP::OneElectronOperator<double> H_op = GQCP::OneElectronOperator<double>::Random(K, K);
    GQCP::TwoElectronOperator<double> g_op (K);
    g_op.setRandom();

    GQCP::HamiltonianParameters<double> ham_par (nullptr, S_op, H_op, g_op, GQCP::SquareMatrix<double>::Random(K, K));


    // Check if we can't rotate with a non-unitary matrix
    GQCP::SquareMatrix<double> T (K);
    T << 0.5, 0.5, -2.0,
         3.0, 0.0,  1.5,
         0.0, 0.0,  2.5;
    BOOST_CHECK_THROW(ham_par.rotate(GQCP::SquareMatrix<double>(T)), std::invalid_argument);
}


BOOST_AUTO_TEST_CASE ( rotate_overlap_matrix ) {

    // Check if a rotation that interchanges two orbitals changes the non-identity overlap matrix
    GQCP::JacobiRotationParameters jacobi_rotation_parameters {1, 0, boost::math::constants::half_pi<double>()};  // interchanges two orbitals

    size_t K = 3;
    GQCP::OneElectronOperator<double> S_op (K);
    S_op << 1.0, 0.5, 0.0,
            0.5, 2.0, 0.0,
            0.0, 0.0, 1.0;

    GQCP::OneElectronOperator<double> S_rotated_ref (K);  // manual calculation
    S_rotated_ref <<  2.0, -0.5, 0.0,
                     -0.5,  1.0, 0.0,
                      0.0,  0.0, 1.0;

    GQCP::OneElectronOperator<double> H_op = GQCP::OneElectronOperator<double>::Random(K, K);

    GQCP::TwoElectronOperator<double> g_op (K);
    g_op.setRandom();


    // Check the Jacobi rotation
    GQCP::HamiltonianParameters<double> ham_par_jacobi (nullptr, S_op, H_op, g_op, GQCP::SquareMatrix<double>::Random(K, K));
    ham_par_jacobi.rotate(jacobi_rotation_parameters);
    BOOST_CHECK(ham_par_jacobi.get_S().isApprox(S_rotated_ref, 1.0e-08));


    // Check for a unitary transformation
    GQCP::HamiltonianParameters<double> ham_par (nullptr, S_op, H_op, g_op, GQCP::SquareMatrix<double>::Random(K, K));
    auto J = GQCP::SquareMatrix<double>::FromJacobi(jacobi_rotation_parameters, K);
    ham_par.rotate(GQCP::SquareMatrix<double>(J));
    BOOST_CHECK(ham_par.get_S().isApprox(S_rotated_ref, 1.0e-08));
}


BOOST_AUTO_TEST_CASE ( constructor_C ) {

    // Create dummy Hamiltonian parameters
    std::shared_ptr<GQCP::AOBasis> ao_basis;
    size_t K = 4;
    GQCP::OneElectronOperator<double> S = GQCP::OneElectronOperator<double>::Random(K, K);
    GQCP::OneElectronOperator<double> H_core = GQCP::OneElectronOperator<double>::Random(K, K);

    GQCP::TwoElectronOperator<double> g (K);

    GQCP::SquareMatrix<double> C = GQCP::SquareMatrix<double>::Random(K, K);

    GQCP::HamiltonianParameters<double> random_hamiltonian_parameters (ao_basis, S, H_core, g, C);


    // Check if we can create transformed Hamiltonian parameters
    GQCP::SquareMatrix<double> T = GQCP::SquareMatrix<double>::Random(K, K);
    GQCP::HamiltonianParameters<double> transformed_random_hamiltonian_parameters (random_hamiltonian_parameters, T);
}


/*
 *  UNIT TESTS - NAMED CONSTRUCTORS
 */

BOOST_AUTO_TEST_CASE ( constructMolecularHamiltonianParameters ) {

    // Set up a basis
    auto h2 = GQCP::Molecule::Readxyz("data/h2_szabo.xyz");
    auto ao_basis = std::make_shared<GQCP::AOBasis>(h2, "STO-3G");


    // Check if we can construct the molecular Hamiltonian parameters
    auto mol_ham_par = GQCP::HamiltonianParameters<double>::Molecular(ao_basis);
    auto g = mol_ham_par.get_g();


    // Check with reference values from Szabo
    GQCP::OneElectronOperator<double> ref_S (2);
    ref_S << 1.0,    0.6593,
             0.6593, 1.0;

    GQCP::OneElectronOperator<double> ref_H_core (2);
    ref_H_core << -1.1204, -0.9584,
                  -0.9584, -1.1204;


    BOOST_CHECK(mol_ham_par.get_S().isApprox(ref_S, 1.0e-04));
    BOOST_CHECK(mol_ham_par.get_h().isApprox(ref_H_core, 1.0e-04));

    BOOST_CHECK(std::abs(g(0,0,0,0) - 0.7746) < 1.0e-04);
    BOOST_CHECK(std::abs(g(0,0,0,0) - g(1,1,1,1)) < 1.0e-12);
    BOOST_CHECK(std::abs(g(0,0,1,1) - 0.5697) < 1.0e-04);
    BOOST_CHECK(std::abs(g(1,0,0,0) - 0.4441) < 1.0e-04);
    BOOST_CHECK(std::abs(g(1,0,0,0) - g(1,1,1,0)) < 1.0e-12);
    BOOST_CHECK(std::abs(g(1,0,1,0) - 0.2970) < 1.0e-04);
}


BOOST_AUTO_TEST_CASE ( FCIDUMP_reader ) {

    auto fcidump_ham_par = GQCP::HamiltonianParameters<double>::ReadFCIDUMP("data/beh_cation_631g_caitlin.FCIDUMP");

    // Check if the one-electron integrals are read in correctly from a previous implementation
    GQCP::OneElectronOperator<double> h_SO = fcidump_ham_par.get_h();

    BOOST_CHECK(std::abs(h_SO(0,0) - (-8.34082)) < 1.0e-5);
    BOOST_CHECK(std::abs(h_SO(5,1) - 0.381418) < 1.0e-6);
    BOOST_CHECK(std::abs(h_SO(14,0) - 0.163205) < 1.0e-6);
    BOOST_CHECK(std::abs(h_SO(13,6) - (-5.53204e-16)) < 1.0e-16);
    BOOST_CHECK(std::abs(h_SO(15,11) - (-0.110721)) < 1.0e-6);


    // Check if the two-electron integrals are read in correctly from a previous implementation
    GQCP::TwoElectronOperator<double> g_SO = fcidump_ham_par.get_g();

    BOOST_CHECK(std::abs(g_SO(2,5,4,4) - 0.0139645) < 1.0e-6);
    BOOST_CHECK(std::abs(g_SO(2,6,3,0) - 5.16622e-18) < 1.0e-17);
    BOOST_CHECK(std::abs(g_SO(3,1,3,0) - (-0.0141251)) <  1.0e-6);
    BOOST_CHECK(std::abs(g_SO(4,6,4,6) - 0.0107791) < 1.0e-6);
    BOOST_CHECK(std::abs(g_SO(4,15,11,1) - (9.33375e-19)) < 1.0e-17);
    BOOST_CHECK(std::abs(g_SO(6,10,5,9) - (-3.81422e-18)) < 1.0e-17);
    BOOST_CHECK(std::abs(g_SO(7,7,2,1) - (-0.031278)) < 1.0e-6);
    BOOST_CHECK(std::abs(g_SO(8,15,9,9) - (-2.80093e-17)) < 1.0e-16);
    BOOST_CHECK(std::abs(g_SO(9,14,0,9) - 0.00161985) < 1.0e-7);
    BOOST_CHECK(std::abs(g_SO(10,1,4,3) - 0.00264603) < 1.0e-7);
    BOOST_CHECK(std::abs(g_SO(11,4,9,3) - (-0.0256623)) < 1.0e-6);
    BOOST_CHECK(std::abs(g_SO(12,9,0,4) - 0.0055472) < 1.0e-6);
    BOOST_CHECK(std::abs(g_SO(13,15,15,13) - 0.00766898) < 1.0e-7);
    BOOST_CHECK(std::abs(g_SO(14,2,12,3) - 0.0104266) < 1.0e-7);
    BOOST_CHECK(std::abs(g_SO(15,5,10,10) - 0.00562608) < 1.0e-7);
}


BOOST_AUTO_TEST_CASE ( FCIDUMP_reader_HORTON ) {

    // Check the same reference value that HORTON does
    auto fcidump_ham_par = GQCP::HamiltonianParameters<double>::ReadFCIDUMP("data/h2_psi4_horton.FCIDUMP");

    GQCP::TwoElectronOperator<double> g_SO = fcidump_ham_par.get_g();
    BOOST_CHECK(std::abs(g_SO(6,5,1,0) - 0.0533584656) <  1.0e-7);
}



/*
 *  UNIT TESTS - METHODS
 */

BOOST_AUTO_TEST_CASE ( calculate_generalized_Fock_matrix_and_super_invalid_arguments ) {

    // Initialize toy HamiltonianParameters
    std::shared_ptr<GQCP::AOBasis> ao_basis;
    GQCP::OneElectronOperator<double> S = GQCP::OneElectronOperator<double>::Identity(2, 2);
    GQCP::OneElectronOperator<double> h = GQCP::OneElectronOperator<double>::Zero(2, 2);
    GQCP::TwoElectronOperator<double> g (2);
    GQCP::HamiltonianParameters<double> ham_par (ao_basis, S, h, g, GQCP::SquareMatrix<double>::Identity(2, 2));


    // Create valid and invalid density matrices (with respect to the dimensions of the SOBasis)
    GQCP::OneRDM<double> D_valid = GQCP::OneRDM<double>::Zero(2, 2);
    GQCP::OneRDM<double> D_invalid = GQCP::OneRDM<double>::Zero(3, 3);

    GQCP::TwoRDM<double> d_valid (2);
    GQCP::TwoRDM<double> d_invalid (3);


    // Test a faulty function calls
    BOOST_REQUIRE_THROW(ham_par.calculateGeneralizedFockMatrix(D_invalid, d_valid), std::invalid_argument);
    BOOST_REQUIRE_THROW(ham_par.calculateGeneralizedFockMatrix(D_valid, d_invalid), std::invalid_argument);

    BOOST_REQUIRE_THROW(ham_par.calculateSuperGeneralizedFockMatrix(D_invalid, d_valid), std::invalid_argument);
    BOOST_REQUIRE_THROW(ham_par.calculateSuperGeneralizedFockMatrix(D_valid, d_invalid), std::invalid_argument);


    // Test correct function calls
    ham_par.calculateGeneralizedFockMatrix(D_valid, d_valid);
    ham_par.calculateSuperGeneralizedFockMatrix(D_valid, d_valid);
}


BOOST_AUTO_TEST_CASE ( calculate_generalized_Fock_matrix_and_super ) {

    // We test the function by a manual calculation of nonsensical toy 1- and 2-RDMS and one- and two-electron integrals
    // Set up the toy 1- and 2-RDMs
    GQCP::OneRDM<double> D (2);
    D << 0, 1,
         2, 3;

    auto d = calculateToy2RDMTensor();

    // Set up the toy SOBasis
    std::shared_ptr<GQCP::AOBasis> ao_basis;
    GQCP::OneElectronOperator<double> S = GQCP::OneElectronOperator<double>::Identity(2, 2);
    GQCP::OneElectronOperator<double> h (2);
    h << 1, 0,
         0, 1;

    auto g = calculateToyTwoElectronIntegralsTensor();
    GQCP::HamiltonianParameters<double> ham_par (ao_basis, S, h, g, GQCP::SquareMatrix<double>::Identity(2, 2));


    // Construct the reference generalized Fock matrix
    GQCP::OneElectronOperator<double> F_ref = GQCP::OneElectronOperator<double>::Zero(2, 2);
    for (size_t p = 0; p < 2; p++) {
        for (size_t q = 0; q < 2; q++) {
            auto p_ = static_cast<double>(p);
            auto q_ = static_cast<double>(q);

            // One-electron part is simplified by manual calculation
            F_ref(p,q) += q_ + 2*p_;

            // Two-electron part is simplified by manual calculation
            for (size_t r = 0; r < 2; r++) {
                auto r_ = static_cast<double>(r);

                F_ref(p,q) += r_ + 4*q_;
                F_ref(p,q) -= q_ + 4*r_;
            }
        }
    }


    // Construct the reference super generalized Fock matrix
    GQCP::TwoElectronOperator<double> W_ref (2);
    W_ref.setZero();
    for (size_t p = 0; p < 2; p++) {
        for (size_t q = 0; q < 2; q++) {
            for (size_t r = 0; r < 2; r++) {
                for (size_t s = 0; s < 2; s++) {
                    auto q_ = static_cast<double>(q);
                    auto r_ = static_cast<double>(r);

                    if (r == q) {
                        W_ref(p,q,r,s) += F_ref(p,s);
                    }

                    // One-electron part is simplified by manual calculation
                    if (s == p) {
                        W_ref(p,q,r,s) -= q_ + 2*r_;
                    }

                    // Two-electron part is simplified by manual calculation
                    if (s == p) {
                        for (size_t t = 0; t < 2; t++) {
                            auto t_ = static_cast<double>(t);

                            W_ref(p,q,r,s) += 3*t_ - 3*q_;
                        }
                    }
                }
            }
        }
    }

    BOOST_CHECK(F_ref.isApprox(ham_par.calculateGeneralizedFockMatrix(D, d), 1.0e-12));
    BOOST_CHECK(W_ref.isApprox(ham_par.calculateSuperGeneralizedFockMatrix(D, d), 1.0e-12));
}


BOOST_AUTO_TEST_CASE ( calculateEdmistonRuedenbergLocalizationIndex ) {

    // Create toy Hamiltonian parameters: only the two-electron integrals are important
    size_t K = 5;
    GQCP::OneElectronOperator<double> S_op = GQCP::OneElectronOperator<double>::Identity(K, K);
    GQCP::OneElectronOperator<double> H_op = GQCP::OneElectronOperator<double>::Random(K, K);

    GQCP::TwoElectronOperator<double> g_op (K);
    g_op.setZero();
    for (size_t i = 0; i < K; i++) {
        g_op(i,i,i,i) = 2*static_cast<float>(i);
    }

    GQCP::HamiltonianParameters<double> ham_par (nullptr, S_op, H_op, g_op, GQCP::SquareMatrix<double>::Identity(K, K));


    BOOST_CHECK(std::abs(ham_par.calculateEdmistonRuedenbergLocalizationIndex(3) - 6.0) < 1.0e-08);
    BOOST_CHECK(std::abs(ham_par.calculateEdmistonRuedenbergLocalizationIndex(4) - 12.0) < 1.0e-08);
}


BOOST_AUTO_TEST_CASE ( effective_one_electron_integrals ) {

    size_t K = 4;
    auto K_ = static_cast<double>(K);

    // Set up toy 2-electron integrals and put them into Hamiltonian parameters
    GQCP::SquareRankFourTensor<double> g_op (K);
    g_op.setZero();

    for (size_t i = 0; i < K; i++) {
        for (size_t j = 0; j < K; j++) {
            for (size_t k = 0; k < K; k++) {
                for (size_t l = 0; l < K; l++) {
                    g_op(i,j,k,l) = (i+1) + 2*(j+1) + 4*(k+1) + 8*(l+1);
                }
            }
        }
    }

    GQCP::OneElectronOperator<double> S_op = GQCP::OneElectronOperator<double>::Identity(K, K);
    GQCP::OneElectronOperator<double> h_op = GQCP::OneElectronOperator<double>::Zero(K, K);
    GQCP::SquareMatrix<double> C = GQCP::SquareMatrix<double>::Identity(K, K);
    GQCP::HamiltonianParameters<double> ham_par (nullptr, S_op, h_op, g_op, C);


    // Set up the reference effective one-electron integrals by manual calculation
    GQCP::OneElectronOperator<double> k_ref = GQCP::OneElectronOperator<double>::Zero(K, K);
    for (size_t p = 0; p < K; p++) {
        for (size_t q = 0; q < K; q++) {
            auto p_ = static_cast<double>(p) + 1;
            auto q_ = static_cast<double>(q) + 1;

            k_ref(p,q) = -K_ / 2 * (p_ + 8*q_ + 3*K_ + 3);
        }
    }

    BOOST_CHECK(k_ref.isApprox(ham_par.calculateEffectiveOneElectronIntegrals(), 1.0e-08));
}


BOOST_AUTO_TEST_CASE ( areOrbitalsOrthonormal ) {

    // We assume that the orbitals in an FCIDUMP file are orthonormal
    auto ham_par_fcidump = GQCP::HamiltonianParameters<double>::ReadFCIDUMP("data/h2_psi4_horton.FCIDUMP");
    BOOST_CHECK(ham_par_fcidump.areOrbitalsOrthonormal());


    // The orbitals in an AO basis are not orthonormal
    auto h2o = GQCP::Molecule::Readxyz("data/h2o.xyz");
    auto ao_ham_par = GQCP::HamiltonianParameters<double>::Molecular(h2o, "STO-3G");
    BOOST_CHECK(!ao_ham_par.areOrbitalsOrthonormal());


    // The orbitals in the RHF basis are orthonormal
    GQCP::PlainRHFSCFSolver plain_scf_solver (ao_ham_par, h2o);
    plain_scf_solver.solve();
    auto rhf = plain_scf_solver.get_solution();
    auto mol_ham_par = GQCP::HamiltonianParameters<double>(ao_ham_par, rhf.get_C());
    BOOST_CHECK(mol_ham_par.areOrbitalsOrthonormal());
}
