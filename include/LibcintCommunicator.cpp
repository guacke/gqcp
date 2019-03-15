#include "LibcintCommunicator.hpp"

#include "Molecule.hpp"

#include "Basis/BasisSet.hpp"



extern "C" {

#include <cint.h>


/*
 *  FUNCTIONS THAT AREN'T INSIDE <cint.h>
 */
int cint1e_ovlp_cart(double *buf, int *shls, int *atm, int natm, int *bas, int nbas, double *env);  // ( \| \)
int cint1e_ipnuc_cart(double *buf, int *shls, int *atm, int natm, int *bas, int nbas, double *env);


}  // extern "C"



namespace GQCP {


void LibcintCommunicator::test() const {

    /* general contracted DZ basis [3s1p/2s1p] for H2
     exponents    contract-coeff
     S   6.0          0.7               0.4
         2.0          0.6               0.3
         0.8          0.5               0.2
     P   0.9          1.
     */

    // Specify the example molecule
    Atom h1 (1,  0.0, 0.0, 0.8);  // coordinates in bohr
    Atom h2 (1,  0.0, 0.0, -0.8);


    // Put STO-3G on every H
    std::vector<double> exponents {3.42525091, 0.623913730, 0.168855400};
    Contraction contraction {0, {0.154328970, 0.535328140, 0.444634540}};

    Shell shell1 (h1, exponents, {contraction});
    Shell shell2 (h2, exponents, {contraction});

    BasisSet basisset {shell1, shell2};
    auto atoms = basisset.atoms();


    int natm = static_cast<int>(atoms.size());
    int nbf = 2;  // number of basis functions



    // TODO: avoid malloc calls and use int[n]?


    // ATM_SLOTS = 6, BAS_SLOTS = 8 are declared inside <cint.h>

    int* libcint_atm = (int*)malloc(sizeof(int) * natm * ATM_SLOTS);  // information about the atoms
    int* libcint_bas = (int*)malloc(sizeof(int) * nbf * BAS_SLOTS);  // information about the basis functions
    double* libcint_env = (double*)malloc(sizeof(double) * 10000);  // a general container (env = environment) in which libcint (probably) places intermediary calculations



    /*
     *  ATM CONFIGURATION (ATOM)
     */

    // PTR_ENV_START = 20
    int offset = PTR_ENV_START;  // an offset such that libcint can retrieve the correct index inside the environment


    for (size_t i = 0; i < natm; i++) {
        libcint_atm[CHARGE_OF + ATM_SLOTS * i] = static_cast<int>(atoms[i].atomic_number);  // insert the charge/atomic number
        libcint_atm[PTR_COORD + ATM_SLOTS * i] = offset;  // pointer to the coordinates of the atom inside the libcint environment
        libcint_env[offset + 0] = atoms[i].position.x();  // insert the position of the atoms
        libcint_env[offset + 1] = atoms[i].position.y();
        libcint_env[offset + 2] = atoms[i].position.z();
        offset += 3;
    }



    /*
     *  BAS CONFIGURATION (BASIS)
     */


    int atom_index = 0;  // index of the atom the shell is centered on
    auto previous_atom = basisset[0].get_atom();


    for (size_t n = 0; n < basisset.numberOfShells(); n++) {

        auto current_shell = basisset[n];
        auto contractions = current_shell.get_contractions();



        // If there's a new atom, increment the index
        auto current_atom = current_shell.get_atom();
        if (current_atom != previous_atom) {
            atom_index++;
            previous_atom = current_atom;
        }
        libcint_bas[ATOM_OF + BAS_SLOTS * n] = atom_index;



        for (size_t m = 0; m < contractions.size(); m++) {
            auto current_contraction = contractions[m];

            libcint_bas[ANG_OF   + BAS_SLOTS * n] = static_cast<int>(current_contraction.l);  // angular momentum

            libcint_bas[NPRIM_OF + BAS_SLOTS * n] = static_cast<int>(current_contraction.length());  // number of primitives

            libcint_bas[NCTR_OF  + BAS_SLOTS * n] = static_cast<int>(current_shell.numberOfContractions());  // number of contractions  // FIXME this can't be right if >1 contraction with the same angular momentum

            libcint_bas[PTR_EXP  + BAS_SLOTS * n] = offset;  // pointer to the exponents of the shell inside the libcint environment

            for (size_t e = 0; e < current_contraction.length(); e++, offset++) {  // also increment offset
                libcint_env[offset] = current_shell.get_exponents()[e];
            }


            libcint_bas[PTR_COEFF + BAS_SLOTS * n] = offset;  // pointer to the contraction coefficients inside the libcint environment


            // input normalized coeff.
            for (size_t c = 0; c < current_contraction.length(); c++, offset++) {  // also increment offset
                libcint_env[offset] = current_contraction.coefficients[c] * CINTgto_norm(libcint_bas[ANG_OF+BAS_SLOTS*n], libcint_env[libcint_bas[PTR_EXP+BAS_SLOTS*n]+c]);
            }


        }

    }


    // CALCULATE OVERLAP INTEGRALS

    for (size_t sh1 = 0; sh1 < basisset.numberOfShells(); sh1++) {
        for (size_t sh2 = 0; sh2 < basisset.numberOfShells(); sh2++) {

            int shls[2];
            shls[0] = static_cast<int>(sh1);
            shls[1] = static_cast<int>(sh2);

            int nbf_sh1 = CINTcgto_cart(static_cast<int>(sh1), libcint_bas);  // number of basis functions in first shell
            int nbf_sh2 = CINTcgto_cart(static_cast<int>(sh2), libcint_bas);  // number of basis functions in second shell

            std::cout << "dim i: " << nbf_sh1 << "  dim j: " << nbf_sh2 << std::endl;

            double* buf = (double*)malloc(sizeof(double) * nbf_sh1 * nbf_sh2);  // buffer where the integrals are calculated to
            cint1e_ovlp_cart(buf, shls, libcint_atm, natm, libcint_bas, nbf, libcint_env);
            std::cout << buf[0] << std::endl;

            free(buf);
        }
    }


//    const auto nsh = static_cast<size_t>(basisset.size());  // number of shells
//    for (auto sh1 = 0; sh1 != nsh; ++sh1) {  // shell 1
//        for (auto sh2 = 0; sh2 != nsh; ++sh2) {  // shell 2
//            // Calculate integrals between the two shells
//            engine.compute(basisset[sh1], basisset[sh2]);  // this updates the pointers in calculated_integrals
//
//
//            // Place the calculated integrals into the matrix representation(s): the integrals are stored in row-major form
//            auto bf1 = shell2bf[sh1];  // (index of) first bf in sh1
//            auto bf2 = shell2bf[sh2];  // (index of) first bf in sh2
//
//            auto nbf_sh1 = basisset[sh1].size();  // number of basis functions in first shell
//            auto nbf_sh2 = basisset[sh2].size();  // number of basis functions in second shell
//
//            for (auto f1 = 0; f1 != nbf_sh1; ++f1) {  // f1: index of basis function within shell 1
//                for (auto f2 = 0; f2 != nbf_sh2; ++f2) { // f2: index of basis function within shell 2
//
//                    for (size_t i = 0; i < N; i++) {
//                        double computed_integral = calculated_integrals[i][f2 + f1 * nbf_sh2];  // integrals are packed in row-major form
//                        operator_components[i](bf1 + f1, bf2 + f2) = computed_integral;
//                    }
//
//                }
//            }  // data access loops
//
//        }
//    }  // shell loops

    free(libcint_atm);
    free(libcint_bas);
    free(libcint_env);

}


}  // namespace GQCP
