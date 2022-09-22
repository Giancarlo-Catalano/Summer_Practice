
#include "Utilities/utilities.hpp"
#include "CoDec/XNFCoDec/XNFCoDec.hpp"


int main() {
#define GeneLength 6
    using Gene = GC::Conjunction<GeneLength>;
    using Input = int;

    //TODO: add more evolution methods
    //TODO: add a mutation which adds random genes, and consider if that would be useful

#if 0 //tests for evolution
    auto fromInts = [&](const std::vector<unsigned int>& items, GC::XNF<GeneLength>& xnf) {
        auto addFromInt = [&](unsigned int item) {
            Gene g;
            g.constructFromBinary(item);
            xnf.activateGene(g);
        };
        std::for_each(items.begin(), items.end(), addFromInt);
    };


    GC::XNF<GeneLength> adam;
    fromInts(std::vector<unsigned int>({2, 10, 18, 26, 34, 42, 50, 58}), adam);
    unsigned int populationSize = 12;
    unsigned int selectionSize = 6;
    bool isElitist = true;

    GC::XNFEvolver<GeneLength> e(adam, populationSize, selectionSize, isElitist);

    LOG("initially, adam is ", adam.to_string());
    LOG("And the evolver", e.to_string());

    e.evolvePopulationForGenerations(100);
    LOG("then e is ", e.to_string());
#endif
#if 0 //tests for XNF_Generic
    auto fromInts = [&](const std::vector<unsigned int>& items, GC::XNF<GeneLength>& xnf) {
        auto addFromInt = [&](unsigned int item) {
            Gene g;
            g.constructFromBinary(item);
            xnf.activateGene(g);
        };
        std::for_each(items.begin(), items.end(), addFromInt);
    };

    LOG("derivation test");

    GC::XNF<5> x5;
    GC::XNF<6> x6;

    GC::Conjunction<5> fiveZeros; fiveZeros.fromString("00000");
    GC::Conjunction<6> sixOnes; sixOnes.fromString("111111");

    x5.activateGene(fiveZeros);
    x6.activateGene(sixOnes);
    x6.transform_remove_1();

    std::vector<GC::XNF_Generic> vectorOfXNFs = {x5, x6};

    LOG("The vectors inside are:");
    LOG("x5=", x5.to_string());
    LOG("x6=", x6.to_string());



    //TODO understand this error: undefined reference to vtable for GC::XNF_Generic
#endif

#if 1 //tests for XNF compression and decompression
    std::string originalFileName = "/home/gian/CLionProjects/Evocom/TestFiles/rawTestFile.txt";
    std::string compressedFileName = "/home/gian/CLionProjects/Evocom/TestFiles/compressedTestFile.gac";
    std::string decompressedFileName = "/home/gian/CLionProjects/Evocom/TestFiles/decompressed.txt";

    GC::XNFCoDec::compressFile(originalFileName, compressedFileName);
    LOG("compression completed, here's the dump");

    dumpFile(compressedFileName, "binary");

    GC::XNFCoDec::decompressFile(compressedFileName, decompressedFileName);

    LOG("The original file was", Utilities::getFileSize(originalFileName), "bytes, compressed into",
        Utilities::getFileSize(compressedFileName), "bytes");


    //TODO: improve compression
    //TODO: implement a variation of the TSP to sort the genes
    //TODO: implement an algorithm that improves an XNF knowing which genes can be toggled without affecting the original files
#endif
#if 0 //tests for restoring XNFs
    auto improveXNF = [&](GC::XNF<GeneLength>& xnf) {

        if (xnf.isEmptyXNF()) {
            LOG("XNF was empty, it will be left as is");
            return;
        }
        unsigned int populationSize = 12;
        unsigned int selectionSize = populationSize/2;
        bool isElitist = true;

        GC::XNFEvolver<GeneLength> e(xnf, populationSize, selectionSize, isElitist);

        LOG("Improving the given XNF.");
        LOG("And the evolver", e.to_string());

        e.evolvePopulationForGenerations(12);
        auto newXNF = e.getBestOfPopulation();
        xnf = newXNF;
        xnf.transform_greedy();
    };

    auto addIntsIntoXNF = [&](const std::vector<size_t>& input, GC::XNF<GeneLength>& xnf) {
        if (input.size()>1<<GeneLength) {
            LOG("The input size (", input.size(), ") is too big, exceeds max of ", GeneLength, "bits (", (1<<GeneLength), ")");
            ERROR_NOT_IMPLEMENTED();
        }
        std::for_each(input.begin(), input.end(), [&](int i) {
            Gene g; g.constructFromBinary(i); xnf.activateGene(g);
        });
    };

    GC::XNF<GeneLength> xnf;
    std::vector<size_t> inputs = {6, 59, 25, 2, 22, 37, 40, 57, 36};
    addIntsIntoXNF(inputs, xnf);

    LOG("initialised the xnf as ", xnf.to_string());
    improveXNF(xnf);
    LOG("after improvement, it's ", xnf.to_string());





    using Unit = char;
    std::vector<Unit> units(1<<GeneLength);
    LOG("the vector has size", units.size());
    xnf.unpackIntoVector(units, 0);
    LOG("vectors which have been decoded into:");
    for (int i=0;i<units.size();i++) {
        if (units[i] != 0) {
            LOG(i, ", value is ", (int)units[i]);
        }
    }
#endif


#if 0 //testing powerskip and rice coding, outdated
    LOG("testing ternary gray compression");


    auto getPopCount = [&](const size_t input) -> size_t {
        return std::bitset<sizeof(decltype(input))*8>(input).count();
    };


    for (int i=0;i<1<<8;i++) {
        if (getPopCount(i) < 2) {
            LOG("i =", i, "has small popcount and will be skipped");
        }
        else {
            auto skipPower = Utilities::encodeSkipPower(i);
            auto rice = Utilities::getRiceEncoded(skipPower);
            auto decodedRice = Utilities::getRiceDecoded(rice);
            auto decodedI = Utilities::decodeSkipPower(decodedRice);

            LOG_NONEWLINE("i =", i, "=", Utilities::asBinaryString(i, 8), ", skipPower =", skipPower, ", rice =");
            for (bool item: rice) {
                LOG_NONEWLINE(item);
            }
            LOG(", decodedRice =", decodedRice, ", decodedI =", decodedI);
        }
    }
#endif

#if 0 //tests for unboundedness checks
    const size_t F = 0;
    Gene g;
    for (int i=0;i< Utilities::pow_constexpr(3, GeneLength);i++) {
        g.setFromBase3Representation(i);
        LOG(g.to_string(), (g.isUnbound(F) ? "is NOT" : "is"), "bound to size F =", F);
    }
#endif



#if GC_PLATFORM_WINDOWS == 1
    KEEP_CONSOLE_OPEN();
#endif
    return 0;
}