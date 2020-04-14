/**
 * How To Run
 * 
 *     make
 *     ./cachesim <file Name> <cache size in bytes> <block size in bytes> [# of ways]
 * 
 * <> required parameters
 * [] optional parameters
 * Note that [# of ways] defaults to 1
 **/

#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <unordered_map>

double cache(const std::string fileName, const unsigned int ways, const unsigned int setIndexBits, const unsigned int blockIndexBits) {
    /**
     * Set sets index bits bitmask
     **/
    unsigned long long int setBitMask = 0;
    for(int i = 0; i < setIndexBits; i++) {
        setBitMask <<= 1;
        setBitMask |= 1;
    }
    setBitMask <<= blockIndexBits;

    /**
     * Set tag index bits bitmask
     **/
    unsigned long long int tagBitMask = ~0;
    tagBitMask <<= blockIndexBits;
    tagBitMask &= ~setBitMask;

    std::ifstream file;
    file.open(fileName);

    std::unordered_map<unsigned long long int, std::unordered_map<unsigned long long int, std::list<unsigned long long int>::iterator>> cacheMap;
    std::unordered_map<unsigned long long int, std::list<unsigned long long int>> cachePriorityQueue;
    unsigned int miss = 0;
    unsigned int total = 0;
    for(std::string line; std::getline(file, line);) {
        std::istringstream iss(line);
        char ignore;
        unsigned long long int offset;
        unsigned long long int address;

        iss >> ignore >> offset >> std::hex >> address;

        const unsigned long long int offsetAddress = address + offset;
        const unsigned long long int setIndex = offsetAddress & setBitMask;
        const unsigned long long int tagIndex = offsetAddress & tagBitMask;
        /**
         * Check whether set index bits is in cache
         **/
        if(cacheMap.find(setIndex) == cacheMap.end()) {
            cachePriorityQueue[setIndex] = {tagIndex};
            cacheMap[setIndex][tagIndex] = cachePriorityQueue[setIndex].begin();
            miss++;
        }
        else {
            /**
             * Check whether tag index bits is in set
             **/
            if(cacheMap[setIndex].find(tagIndex) == cacheMap[setIndex].end()) {
                /**
                 * Check if blocks in set are all taken
                 **/
                if(cachePriorityQueue[setIndex].size() >= ways) {
                    const unsigned long long int tagIndexKey = cachePriorityQueue[setIndex].front();
                    cachePriorityQueue[setIndex].pop_front();
                    cacheMap[setIndex].erase(tagIndexKey);
                }
                miss++;
            }
            else {
                const std::list<unsigned long long int>::iterator it = cacheMap[setIndex][tagIndex];
                cachePriorityQueue[setIndex].erase(it);
            }
            cachePriorityQueue[setIndex].push_back(tagIndex);
            std::list<unsigned long long int>::iterator it = cachePriorityQueue[setIndex].end();
            it--;
            cacheMap[setIndex][tagIndex] = it;
        }
        total++;
    }

    file.close();

    return double(miss) / double(total);
}

int main(const int argc, const char * argv[]) {
    if(argc < 4) {
        std::cout << "[ERROR] Insufficient number of parameters\n";
        std::cout << "./cachesim <file Name> <cache size in bytes> <block size in bytes> [# of ways]" << std::endl;
        
        return 1;
    }

    unsigned int sets;
    unsigned int ways = 1;
    unsigned long long int setIndexBits = 0;
    unsigned long long int blockIndexBits = 0;
    try {
        const unsigned int cacheSize = std::stoi(argv[2]);
        const unsigned int blockSize = std::stoi(argv[3]);
        if(argc > 4) {
            ways = std::stoi(argv[4]);
        }

        if(cacheSize == 0 || blockSize == 0) {
            throw 'e';
        }
        if((cacheSize & (cacheSize - 1) != 0) || ((blockSize & (blockSize - 1)) != 0) || (cacheSize % blockSize != 0)) {
            throw 1;
        }
        const unsigned int totalBlocks = cacheSize / blockSize;
        if(ways != 0) {
            sets = totalBlocks / ways;
        }
        else {
            ways = totalBlocks;
            sets = 1;
        }

        while(sets != 1) {
            sets >>= 1;
            setIndexBits++;
        }
        for(int i = 0; i < setIndexBits; i++) {
            sets <<= 1;
        }

        unsigned int logBlockSize = blockSize;
        while(logBlockSize != 1) {
            logBlockSize >>= 1;
            blockIndexBits++;
        }
    }
    catch(int e) {
        std::cout << "[ERROR] Parameters <cache size in bytes> and <block size in bytes> must be a power of 2\n";
        std::cout << "./cachesim <file Name> <cache size in bytes> <block size in bytes> [# of ways]" << std::endl;

        return 1;
    }
    catch(...) {
        std::cout << "[ERROR] Parameters <cache size in bytes> and <block size in bytes> must be nonzero integers and [# of ways] must be an integer\n";
        std::cout << "./cachesim <file Name> <cache size in bytes> <block size in bytes> [# of ways]" << std::endl;

        return 1;
    }
    std::string fileName = argv[1];

    double misRate;
    try {
        misRate = cache(fileName, ways, setIndexBits, blockIndexBits);
    }
    catch(...) {
        std::cout << "[ERROR] Failed to open file " << fileName << std::endl;
    }

    std::cout << "Miss Rate: " << misRate << '\n';
    std::cout << "Hit Rate: " << 1 - misRate << '\n';
    std::cout << "# of Sets (Power of 2): " << sets << '\n';
    std::cout << "# of Ways: " << ways << '\n';
    std::cout << "# of Tag Bits: " << 32 - (setIndexBits + blockIndexBits) << '\n';
    std::cout << "# of Index Bits: " << setIndexBits << '\n';
    std::cout << "# of Offset Bits: " << blockIndexBits << '\n' << std::endl;

    return 0;
}