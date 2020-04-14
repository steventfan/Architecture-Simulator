/**
 * How To Run
 * 
 *     make
 *     ./branchsim <file name> <m> <n> [# of bits for address indexing]
 * 
 * <> required parameters
 * [] optional parameters
 * Note that # of bits for address indexing defaults to 8
 **/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

int branch(const std::string fileName, const unsigned int m, const unsigned int n, const unsigned int b) {
    std::ifstream file;
    try {
        file.open(fileName);
    }
    catch(...) {
        std::cout << "[ERROR] Failed to open file " << fileName << std::endl;

        return 1;
    }

    /**
     * Initialize bitmasks
    **/
    unsigned int mBitMask = 0;
    for(unsigned int i = 0; i < m; i++) {
        mBitMask <<= 1;
        mBitMask |= 1;
    }
    unsigned int nBit = 1;
    for(unsigned int i = 1; i < n; i++) {
        nBit <<= 1;
    }
    unsigned int bBitMask = 0;
    for(unsigned int i = 0; i < b; i++) {
        bBitMask <<= 1;
        bBitMask |= 1;
    }

    std::unordered_map<unsigned int, std::unordered_map<unsigned int, unsigned int>> branchHistory;
    unsigned int global = 0;
    unsigned int miss = 0;
    unsigned int total = 0;
    for(std::string line; std::getline(file, line);) {
        std::istringstream iss(line);
        unsigned int address;
        char taken;

        iss >> std::hex >> address >> taken;

        /**
         * Initialize or find in branch history table
        **/
        const unsigned int addressIndex = address & bBitMask;
        if(branchHistory.find(global) == branchHistory.end()) {
            branchHistory[global] = {{addressIndex, 0}};
        }
        else if(branchHistory[global].find(addressIndex) == branchHistory[global].end()) {
            branchHistory[global][addressIndex] = 0;
        }

        /**
         * Update global branch history and predictor
        **/
        if(taken == 'N') {
            if((branchHistory[global][addressIndex] & nBit) == 0) {
                branchHistory[global][addressIndex] = 0;
            }
            else {
                branchHistory[global][addressIndex]--;
                if((branchHistory[global][addressIndex] & nBit) == 0) {
                    branchHistory[global][addressIndex] = 0;
                }
                miss++;
            }
            global <<= 1;
            global &= mBitMask;
        }
        else {
            if((branchHistory[global][addressIndex] & nBit) != 0) {
                branchHistory[global][addressIndex] = (nBit * 2) - 1;
            }
            else {
                branchHistory[global][addressIndex]++;
                if((branchHistory[global][addressIndex] & nBit) != 0) {
                    branchHistory[global][addressIndex] = (nBit * 2) - 1;
                }
                miss++;
            }
            global <<= 1;
            global |= 1;
            global &= mBitMask;
        }
        total++;
    }
    file.close();

    std::cout << "For (" << m << ", " << n << ") with " << b << " bits address indexing: " << std::endl;
    std::cout << "Misprediction rate: " << double(miss) / double(total) << std::endl;
    
    unsigned int entries = 0;
    for(std::unordered_map<unsigned int, std::unordered_map<unsigned int, unsigned int>>::iterator it = branchHistory.begin(); it != branchHistory.end(); it++) {
        entries += it->second.size();
    }

    std::cout << "Number of entries utilized: " << entries << '\n' << std::endl;

    return 0;
}

int main(const int argc, const char * argv[]) {
    if(argc < 4) {
        std::cout << "[ERROR] Insufficient number of parameters" << std::endl;
        std::cout << "./branchsim <file name> <m> <n> [# of bits for address indexing]" << std::endl;

        return 1;
    }

    unsigned int m;
    unsigned int n;
    unsigned int b = 8;
    try {
        m = std::stoi(argv[2]);
        n = std::stoi(argv[3]);
        if(argc > 4) {
            b = std::stoi(argv[4]);
        }
        if(n == 0) {
            throw 1;
        }
    }
    catch(...) {
        std::cout << "[ERROR] Parameters m and # of bits must be integers and parameter n must be a nonzero integer" << std::endl;
        std::cout << "./branchsim <file name> <m> <n> [# of bits for address indexing]" << std::endl;

        return 1;
    }

    return branch(argv[1], m, n, b);
}