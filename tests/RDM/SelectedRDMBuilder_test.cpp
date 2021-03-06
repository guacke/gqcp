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
#define BOOST_TEST_MODULE "Selected_RDM_test"


#include <boost/test/unit_test.hpp>
#include <boost/test/included/unit_test.hpp>

#include "RDM/RDMCalculator.hpp"
#include "RDM/SelectedRDMBuilder.hpp"

#include "RDM/DOCIRDMBuilder.hpp"
#include "RDM/FCIRDMBuilder.hpp"

#include "CISolver/CISolver.hpp"
#include "HamiltonianBuilder/FCI.hpp"
#include "HamiltonianBuilder/DOCI.hpp"
#include "HamiltonianParameters/HamiltonianParameters.hpp"

#include "utilities/linalg.hpp"


BOOST_AUTO_TEST_CASE ( one_rdms_fci_H2_6_31G ) {

    // Do an H2@FCI//6-31G calculation
    // test if 1-RDM SelectedRDM and FCIRDM are equal
    size_t N_a = 1;
    size_t N_b = 1;

    // Create the molecular Hamiltonian parameters in the AO basis
    auto h2 = GQCP::Molecule::Readxyz("data/h2.xyz");
    auto ham_par = GQCP::HamiltonianParameters<double>::Molecular(h2, "6-31G");
    size_t K = ham_par.get_K();  // 4

    GQCP::ProductFockSpace fock_space (K, N_a, N_b);  // dim = 16
    GQCP::FCI fci (fock_space);

    // Specify solver options and solve the eigenvalue problem
    // Solve the dense FCI eigenvalue problem
    GQCP::CISolver ci_solver (fci, ham_par);
    GQCP::DenseSolverOptions solver_options;
    ci_solver.solve(solver_options);

    GQCP::VectorX<double> coef = ci_solver.get_eigenpair().get_eigenvector();

    // Get the 1-RDM from FCI
    GQCP::FCIRDMBuilder fci_rdm(fock_space);
    GQCP::OneRDMs<double> one_rdms = fci_rdm.calculate1RDMs(coef);


    GQCP::SelectedFockSpace selected_fock_space (fock_space);

    // Get the 1-RDM from SelectedCI
    GQCP::SelectedRDMBuilder selected_rdm (selected_fock_space);
    GQCP::OneRDMs<double> one_rdms_s = selected_rdm.calculate1RDMs(coef);


    BOOST_CHECK(one_rdms_s.one_rdm.isApprox(one_rdms.one_rdm));
    BOOST_CHECK(one_rdms_s.one_rdm_aa.isApprox(one_rdms.one_rdm_aa));
    BOOST_CHECK(one_rdms_s.one_rdm_bb.isApprox(one_rdms.one_rdm_bb));
}



BOOST_AUTO_TEST_CASE ( two_rdms_fci_H2_6_31G ) {

    // Do an H2@FCI//6-31G calculation
    // test if 2-RDM SelectedRDM and FCIRDM are equal
    size_t N_a = 1;
    size_t N_b = 1;

    // Create the molecular Hamiltonian parameters in the AO basis
    auto h2 = GQCP::Molecule::Readxyz("data/h2.xyz");
    auto ham_par = GQCP::HamiltonianParameters<double>::Molecular(h2, "6-31G");
    size_t K = ham_par.get_K();  // 4

    GQCP::ProductFockSpace fock_space (K, N_a, N_b);  // dim = 16
    GQCP::FCI fci (fock_space);

    // Specify solver options and solve the eigenvalue problem
    // Solve the dense FCI eigenvalue problem
    GQCP::CISolver ci_solver (fci, ham_par);
    GQCP::DenseSolverOptions solver_options;
    ci_solver.solve(solver_options);

    GQCP::VectorX<double> coef = ci_solver.get_eigenpair().get_eigenvector();

    // Get the 1-RDM from FCI
    GQCP::FCIRDMBuilder fci_rdm(fock_space);
    GQCP::TwoRDMs<double> two_rdms = fci_rdm.calculate2RDMs(coef);


    GQCP::SelectedFockSpace selected_fock_space (fock_space);

    // Get the 1-RDM from SelectedCI
    GQCP::SelectedRDMBuilder selected_rdm (selected_fock_space);
    GQCP::TwoRDMs<double> two_rdms_s = selected_rdm.calculate2RDMs(coef);


    BOOST_CHECK(two_rdms_s.two_rdm_aaaa.isApprox(two_rdms.two_rdm_aaaa, 1.0e-06));
    BOOST_CHECK(two_rdms_s.two_rdm_aabb.isApprox(two_rdms.two_rdm_aabb, 1.0e-06));
    BOOST_CHECK(two_rdms_s.two_rdm_bbaa.isApprox(two_rdms.two_rdm_bbaa, 1.0e-06));
    BOOST_CHECK(two_rdms_s.two_rdm_bbbb.isApprox(two_rdms.two_rdm_bbbb, 1.0e-06));
    BOOST_CHECK(two_rdms_s.two_rdm.isApprox(two_rdms.two_rdm, 1.0e-06));
}


BOOST_AUTO_TEST_CASE ( one_rdms_doci_H2_6_31G ) {

    // Do an H2@doci//6-31G calculation
    // test if 1-RDM SelectedRDM and dociRDM are equal
    size_t N = 1;

    // Create the molecular Hamiltonian parameters in the AO basis
    auto h2 = GQCP::Molecule::Readxyz("data/h2.xyz");
    auto ham_par = GQCP::HamiltonianParameters<double>::Molecular(h2, "6-31G");
    size_t K = ham_par.get_K();  // 4

    GQCP::FockSpace fock_space (K, N);  // dim = 4
    GQCP::DOCI doci (fock_space);

    // Specify solver options and solve the eigenvalue problem
    // Solve the dense doci eigenvalue problem
    GQCP::CISolver ci_solver (doci, ham_par);
    GQCP::DenseSolverOptions solver_options;
    ci_solver.solve(solver_options);

    GQCP::VectorX<double> coef = ci_solver.get_eigenpair().get_eigenvector();

    // Get the 1-RDM from doci
    GQCP::DOCIRDMBuilder doci_rdm(fock_space);
    GQCP::OneRDMs<double> one_rdms = doci_rdm.calculate1RDMs(coef);


    GQCP::SelectedFockSpace selected_fock_space (fock_space);

    // Get the 1-RDM from SelectedCI
    GQCP::SelectedRDMBuilder selected_rdm (selected_fock_space);
    GQCP::OneRDMs<double> one_rdms_s = selected_rdm.calculate1RDMs(coef);


    BOOST_CHECK(one_rdms_s.one_rdm.isApprox(one_rdms.one_rdm));
    BOOST_CHECK(one_rdms_s.one_rdm_aa.isApprox(one_rdms.one_rdm_aa));
    BOOST_CHECK(one_rdms_s.one_rdm_bb.isApprox(one_rdms.one_rdm_bb));
}


BOOST_AUTO_TEST_CASE ( two_rdms_doci_H2_6_31G ) {

    // Do an H2@doci//6-31G calculation
    // test if 2-RDM SelectedRDM and dociRDM are equal
    size_t N = 1;

    // Create the molecular Hamiltonian parameters in the AO basis
    auto h2 = GQCP::Molecule::Readxyz("data/h2.xyz");
    auto ham_par = GQCP::HamiltonianParameters<double>::Molecular(h2, "6-31G");
    size_t K = ham_par.get_K();  // 4

    GQCP::FockSpace fock_space (K, N);  // dim = 4
    GQCP::DOCI doci (fock_space);

    // Specify solver options and solve the eigenvalue problem
    // Solve the dense doci eigenvalue problem
    GQCP::CISolver ci_solver (doci, ham_par);
    GQCP::DenseSolverOptions solver_options;
    ci_solver.solve(solver_options);

    GQCP::VectorX<double> coef = ci_solver.get_eigenpair().get_eigenvector();

    // Get the 1-RDM from doci
    GQCP::DOCIRDMBuilder doci_rdm(fock_space);
    GQCP::TwoRDMs<double> two_rdms = doci_rdm.calculate2RDMs(coef);


    GQCP::SelectedFockSpace selected_fock_space (fock_space);

    // Get the 1-RDM from SelectedCI
    GQCP::SelectedRDMBuilder selected_rdm (selected_fock_space);
    GQCP::TwoRDMs<double> two_rdms_s = selected_rdm.calculate2RDMs(coef);


    BOOST_CHECK(two_rdms_s.two_rdm_aaaa.isApprox(two_rdms.two_rdm_aaaa, 1.0e-06));
    BOOST_CHECK(two_rdms_s.two_rdm_aabb.isApprox(two_rdms.two_rdm_aabb, 1.0e-06));
    BOOST_CHECK(two_rdms_s.two_rdm_bbaa.isApprox(two_rdms.two_rdm_bbaa, 1.0e-06));
    BOOST_CHECK(two_rdms_s.two_rdm_bbbb.isApprox(two_rdms.two_rdm_bbbb, 1.0e-06));
    BOOST_CHECK(two_rdms_s.two_rdm.isApprox(two_rdms.two_rdm, 1.0e-06));
}


BOOST_AUTO_TEST_CASE ( throw_calculate_element ) {

    // Create a test wave function
    size_t K = 5;
    size_t N = 4;
    GQCP::FockSpace fock_space (K, N);
    GQCP::SelectedFockSpace selected_fock_space (fock_space);
    GQCP::VectorX<double> coeff (fock_space.get_dimension());
    coeff << 1, 1, -2, 4, -5;

    // not implemented yet and should throw
    GQCP::SelectedRDMBuilder selected_rdm (selected_fock_space);
    BOOST_CHECK_THROW(selected_rdm.calculateElement({0,0,1}, {1,0,2}, coeff), std::runtime_error);
}
