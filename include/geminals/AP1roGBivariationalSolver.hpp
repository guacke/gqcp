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
#ifndef AP1roGBivariationalSolver_hpp
#define AP1roGBivariationalSolver_hpp


#include "geminals/BaseAP1roGSolver.hpp"
#include "geminals/AP1roGVariables.hpp"
#include "geminals/BivariationalCoefficients.hpp"


namespace GQCP {


/**
 *  A class that is able to solve the AP1roG bivariational equations
 */
class AP1roGBivariationalSolver : public BaseAP1roGSolver {
public:
    // PUBLIC ENUM CLASSES
    /**
     *  An enum class that provides the implemented extra equations
     */
    enum class ExtraEquation {
        q0,     // q_0 = 1
        norm    // <Phi(q)|Psi(p)> = 1
    };


private:
    // PRIVATE MEMBERS
    AP1roGBivariationalSolver::ExtraEquation extra_eq;
    BivariationalCoefficients bivariational_coefficients;  // the determined bivariational coefficients

public:
    // CONSTRUCTORS
    /**
     *  @param N_P          the number of electrons
     *  @param ham_par      Hamiltonian parameters in an orthonormal orbital basis
     *  @param G            the initial guess for the AP1roG gemial coefficients
     *  @param extra_eq     the specification of the extra equation
     */
    AP1roGBivariationalSolver(size_t N_P, const HamiltonianParameters<double>& ham_par, const AP1roGGeminalCoefficients& G, ExtraEquation extra_eq = ExtraEquation::q0);

    /**
     *  @param N_P          the number of electrons
     *  @param ham_par      Hamiltonian parameters in an orthonormal orbital basis
     *  @param extra_eq     the specification of the extra equation
     *
     *  The initial guess for the geminal coefficients is zero
     */
    AP1roGBivariationalSolver(size_t N_P, const HamiltonianParameters<double>& ham_par, ExtraEquation extra_eq = ExtraEquation::q0);

    /**
     *  @param molecule     the molecule used for the AP1roG calculation
     *  @param ham_par      Hamiltonian parameters in an orthonormal orbital basis
     *  @param G            the initial guess for the AP1roG gemial coefficients
     *  @param extra_eq     the specification of the extra equation
     */
    AP1roGBivariationalSolver(const Molecule& molecule, const HamiltonianParameters<double>& ham_par, const AP1roGGeminalCoefficients& G, ExtraEquation extra_eq = ExtraEquation::q0);

    /**
     *  @param molecule     the molecule used for the AP1roG calculation
     *  @param ham_par      Hamiltonian parameters in an orthonormal orbital basis
     *  @param extra_eq     the specification of the extra equation
     *
     *  The initial guess for the geminal coefficients is zero
     */
    AP1roGBivariationalSolver(const Molecule& molecule, const HamiltonianParameters<double>& ham_par, ExtraEquation extra_eq = ExtraEquation::q0);


    // GETTERS
    const BivariationalCoefficients& get_bivariational_coefficients() const { return this->bivariational_coefficients; }


    // PUBLIC METHODS
    void solve() override;
};


}  // namespace GQCP


#endif /* AP1roGBivariationalSolver_hpp */
