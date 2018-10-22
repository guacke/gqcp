// This file is part of GQCG-gqcp.
// 
// Copyright (C) 2017-2018  the GQCG developers
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

#include <fstream>



#include "WaveFunction/WaveFunctionReader.hpp"




namespace GQCP {


/*
 * CONSTRUCTORS
 */

/**
 *  Constructor based on a given @param GAMESS_filename
 */
WaveFunctionReader::WaveFunctionReader(const std::string& GAMESS_filename)
{
    /*
    // If the filename isn't properly converted into an input file stream, we assume the user supplied a wrong file
    std::ifstream input_file_stream(GAMESS_filename);
    std::ifstream input_file_stream_count(GAMESS_filename);  // made to count the expansion size

    if (!input_file_stream.good()) {
        throw std::runtime_error("WaveFunctionReader(): The provided GAMESS file is illegible. Maybe you specified a wrong path?");
    }

    // Do the actual parsing
    // Read in dummy lines up until we actually get to the ONVs and coefficients
    std::string line;
    std::string buffer;  // dummy for the getline()  TODO: find "correcter" way if possible
    while (std::getline(input_file_stream, line)) {
        std::getline(input_file_stream_count, buffer);
        if (line.find("ALPHA") != std::string::npos
            && line.find("BETA") != std::string::npos
            && line.find("COEFFICIENT") != std::string::npos) {  // if find returns an index that's different from the 'not-found' index

            // this line should have dashes and we skip it
            std::getline(input_file_stream, line);
            std::getline(input_file_stream_count, buffer);
            break;
        }
    }

    size_t space_size = 0;
    // Count the amount of configurations
    while (std::getline(input_file_stream, buffer)) {
        if (buffer.length() != 0 | buffer[0] != '\n') {
            space_size++;
        }
    }

    this->coefficients = Eigen::VectorXd::Zero(space_size);


    std::getline(input_file_stream, line);
    // Read the first line containing the configurations
    std::vector<std::string> splitted_line;
    boost::split(splitted_line, line, boost::is_any_of("|"));  // split on '|'


    // Create an alpha SpinString for the first field
    std::string trimmed_alpha = boost::algorithm::trim_copy(splitted_line[0]);
    std::string reversed_alpha (trimmed_alpha.rbegin(), trimmed_alpha.rend());

    // Create a beta SpinString for the second field
    std::string trimmed_beta = boost::algorithm::trim_copy(splitted_line[1]);
    std::string reversed_beta(trimmed_beta.rbegin(), trimmed_beta.rend());

    size_t counter = 0;
    // add the double
    this->coefficients(counter) = std::stod(splitted_line[2]);

    // dynamic bitset provides us with functionallity
    boost::dynamic_bitset<> alpha_transfer (trimmed_alpha);
    boost::dynamic_bitset<> beta_transfer (trimmed_beta);

    size_t K = alpha_transfer.size();
    size_t N_alpha = alpha_transfer.count();
    size_t N_beta = beta_transfer.count();

    this->fock_space = SelectedFockSpace(K, N_alpha, N_beta);

    this->fock_space.addConfiguration(trimmed_alpha, trimmed_beta);


    // Read in the ONVs and the coefficients by splitting the line on '|', and then trimming whitespace
    // In the GAMESS format, the bit strings are ordered in reverse
    while (std::getline(input_file_stream, line)) {

        counter++;
        std::vector<std::string> splitted_line;
        boost::split(splitted_line, line, boost::is_any_of("|"));  // split on '|'


        // Create an alpha SpinString for the first field
        std::string trimmed_alpha = boost::algorithm::trim_copy(splitted_line[0]);
        if (trimmed_alpha.length() != K) {
            throw std::invalid_argument("WaveFunctionReader(): One of the provided alpha ONVs does not have the correct number of orbitals.");
        }
        std::string reversed_alpha (trimmed_alpha.rbegin(), trimmed_alpha.rend());

        // Create a beta SpinString for the second field
        std::string trimmed_beta = boost::algorithm::trim_copy(splitted_line[1]);
        if (trimmed_beta.length() != K) {
            throw std::invalid_argument("WaveFunctionReader(): One of the provided beta ONVs does not have the correct number of orbitals.");
        }
        std::string reversed_beta(trimmed_beta.rbegin(), trimmed_beta.rend());


        // Create a double for the third field
        this->coefficients(counter) = std::stod(splitted_line[2]);
        this->fock_space.addConfiguration(trimmed_alpha, trimmed_beta);

    }  // while getline

    this->wave_function = WaveFunction(this->fock_space, this->coefficients);
     */
}


}  // namespace GQCP
