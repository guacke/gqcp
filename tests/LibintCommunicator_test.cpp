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
#define BOOST_TEST_MODULE "LibintCommunicator"

#include <boost/test/unit_test.hpp>
#include <boost/test/included/unit_test.hpp>  // include this to get main(), otherwise the compiler will complain

#include "LibintCommunicator.hpp"

#include "utilities/linalg.hpp"


BOOST_AUTO_TEST_CASE ( atoms_interface ) {

    std::vector<GQCP::Atom> GQCP_atoms = {
        {1, 0, 3, 0},
        {2, 0, 0, 4},
        {3, 3, 0, 0},
        {4, 0, 0, 5}
    };

    std::vector<libint2::Atom> ref_libint_atoms = {
        {1, 0, 3, 0},
        {2, 0, 0, 4},
        {3, 3, 0, 0},
        {4, 0, 0, 5}
    };


    // Use the Libint interface to obtain a std::vector<libint2::Atom> from the GQCP ones
    auto test_libint_atoms = GQCP::LibintCommunicator::get().interface(GQCP_atoms);


    /**
     *  Implement a function object to compare (libint_atom) == (libint_atom)
     */
    struct LibintAtomEqual {
        double tolerance;
        explicit LibintAtomEqual(double tolerance) : tolerance (tolerance) {};

        bool operator()(const libint2::Atom& lhs, const libint2::Atom& rhs) {
            return (lhs.atomic_number == rhs.atomic_number) &&
                   (std::abs(lhs.x - rhs.x) < tolerance) &&
                   (std::abs(lhs.y - rhs.y) < tolerance) &&
                   (std::abs(lhs.z - rhs.z) < tolerance);
        }
    };


    // Check if the interfacing between the Atom types works
    BOOST_CHECK((ref_libint_atoms.size() == test_libint_atoms.size()) &&
                std::equal(ref_libint_atoms.begin(), ref_libint_atoms.end(), test_libint_atoms.begin(), LibintAtomEqual(1.0e-08)));
}


BOOST_AUTO_TEST_CASE( Szabo_integrals_h2_sto3g ) {

    // We will follow the example in Szabo, section 3.5.2, where it is stated that R = 1.4 a.u. = 0.740848 Angstrom
    auto h2 = GQCP::Molecule::Readxyz("data/h2_szabo.xyz");
    GQCP::AOBasis basis (h2, "STO-3G");
    BOOST_CHECK_EQUAL(basis.get_number_of_basis_functions(), 2);

    // Calculate some integrals
    auto S = GQCP::LibintCommunicator::get().calculateOverlapIntegrals(basis);
    auto T = GQCP::LibintCommunicator::get().calculateKineticIntegrals(basis);
    auto V = GQCP::LibintCommunicator::get().calculateNuclearIntegrals(basis);

    auto H_core = GQCP::OneElectronOperator<double>(T + V);

    auto g = GQCP::LibintCommunicator::get().calculateCoulombRepulsionIntegrals(basis);


    // Fill in the reference values from Szabo
    GQCP::OneElectronOperator<double> ref_S (2);
    ref_S << 1.0,    0.6593,
             0.6593, 1.0;

    GQCP::OneElectronOperator<double> ref_T (2);
    ref_T << 0.7600, 0.2365,
             0.2365, 0.7600;

    GQCP::OneElectronOperator<double> ref_H_core (2);
    ref_H_core << -1.1204, -0.9584,
                  -0.9584, -1.1204;

    BOOST_CHECK(S.isApprox(ref_S, 1.0e-04));
    BOOST_CHECK(T.isApprox(ref_T, 1.0e-04));
    BOOST_CHECK(H_core.isApprox(ref_H_core, 1.0e-04));


    // The two-electron integrals in Szabo are given in chemist's notation, so this confirms that the LibintCommunicator gives them in chemist's notation as well
    BOOST_CHECK(std::abs(g(0,0,0,0) - 0.7746) < 1.0e-04);
    BOOST_CHECK(std::abs(g(0,0,0,0) - g(1,1,1,1)) < 1.0e-12);

    BOOST_CHECK(std::abs(g(0,0,1,1) - 0.5697) < 1.0e-04);

    BOOST_CHECK(std::abs(g(1,0,0,0) - 0.4441) < 1.0e-04);
    BOOST_CHECK(std::abs(g(1,0,0,0) - g(1,1,1,0)) < 1.0e-12);

    BOOST_CHECK(std::abs(g(1,0,1,0) - 0.2970) < 1.0e-04);
}


BOOST_AUTO_TEST_CASE( HORTON_integrals_h2o_sto3g ) {

    // Set up a basis
    auto water = GQCP::Molecule::Readxyz("data/h2o.xyz");
    GQCP::AOBasis basis (water, "STO-3G");
    auto nbf = basis.get_number_of_basis_functions();


    // Calculate some integrals
    libint2::BasisSet basisset = basis.get_basis_functions();
    auto S = GQCP::LibintCommunicator::get().calculateOverlapIntegrals(basis);
    auto T = GQCP::LibintCommunicator::get().calculateKineticIntegrals(basis);
    auto V = GQCP::LibintCommunicator::get().calculateNuclearIntegrals(basis);

    auto g = GQCP::LibintCommunicator::get().calculateCoulombRepulsionIntegrals(basis);


    // Read in reference data from HORTON
    GQCP::OneElectronOperator<double> ref_S = GQCP::OneElectronOperator<double>::FromFile("data/h2o_sto-3g_overlap_horton.data", nbf, nbf);
    GQCP::OneElectronOperator<double> ref_T = GQCP::OneElectronOperator<double>::FromFile("data/h2o_sto-3g_kinetic_horton.data", nbf, nbf);
    GQCP::OneElectronOperator<double> ref_V = GQCP::OneElectronOperator<double>::FromFile("data/h2o_sto-3g_nuclear_horton.data", nbf, nbf);
    GQCP::TwoElectronOperator<double> ref_g = GQCP::TwoElectronOperator<double>::FromFile("data/h2o_sto-3g_coulomb_horton.data", nbf);


    // Check if the calculated integrals are equal to those of HORTON
    BOOST_CHECK(S.isApprox(ref_S, 1.0e-08));
    BOOST_CHECK(T.isApprox(ref_T, 1.0e-08));
    BOOST_CHECK(V.isApprox(ref_V, 1.0e-08));
    BOOST_CHECK(g.isApprox(ref_g, 1.0e-06));
}
