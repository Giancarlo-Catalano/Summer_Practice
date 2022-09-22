//
// Created by gian on 11/08/22.
//

#ifndef EVOCOM_XNFBREEDER_HPP
#define EVOCOM_XNFBREEDER_HPP

#include <utility>
#include <random>
#include <map>
#include "../Utilities/utilities.hpp"
#include "../XNF/XNF.hpp"

namespace GC {

    template <std::size_t GeneLength>
    class XNFBreeder {
        using Individual = XNF<GeneLength>;
        using Probability = unsigned int;
    public:
        XNFBreeder(unsigned int randomizerSeed);
        std::pair<Individual, Individual> crossover(const Individual& A, const Individual& B) const;
        Individual tweak(const Individual& A) const;

        std::string to_string();

    private:
        using AdjacencyReport = typename Individual ::AdjacencyReport;
        using Gene = typename Individual::Gene;
        using IndexInGene = typename Individual::IndexInGene;

        template <unsigned short int Min, unsigned short Max>
        struct IntRangeRandomGenerator {
            std::uniform_int_distribution<unsigned int> distribution;
            std::default_random_engine generator;

            IntRangeRandomGenerator(unsigned int seed) :
                    generator(seed),
                    distribution(Min, Max) {}

            unsigned int getRandom() { return distribution(generator); }
        };

        mutable IntRangeRandomGenerator<0, 99> randomPercentageGen;
        mutable IntRangeRandomGenerator<0, GeneLength-1> randomIndexInGeneGenerator;


        bool makeChoice(Probability probabilityOfTrue) const;

    private:
        static bool mostCommonBool(bool a, bool b, bool c);
        static bool generalTraitOfFamilyPresence(const AdjacencyReport& ar);
        static bool areEligibleForCrossOver(const AdjacencyReport& arA, const AdjacencyReport& arB);

        std::map<unsigned int, Probability> chanceOfMutationForPopCount{{0, 5}, {1, 10}, {2, 60}, {3, 80}}; //TODO: these are exaggerated for display purposes
        //std::map<unsigned int, Probability> chanceOfMutationForPopCount{{0, 30}, {1, 40}, {2, 70}, {3, 90}}; //TODO: these are exaggerated for display purposes
        Probability chanceOfAlteration = 50;
        Probability chanceOfCrossOver = 50;

        struct ToggleRecipe {
            bool toggleFirst = false;
            bool toggleSecond = false;

            void crossOver() {
                toggleFirst ^= 1;
                toggleSecond ^= 1;
            }

            void applyToggling(const AdjacencyReport& arFirst, const AdjacencyReport& arSecond, Individual& first, Individual& second) const{
                if (toggleFirst)
                    arFirst.toggle(first);
                if (toggleSecond)
                    arSecond.toggle(second);
            }
        };

        ToggleRecipe decideToggling(const AdjacencyReport& arA, const AdjacencyReport& arB) const;
        void mutateAndCrossOverOnGene(const Gene& gene, const Individual& firstParent, const Individual& secondParent, Individual& firstChild, Individual& secondChild) const;
        void tweakGene(const Gene& gene, Individual& receiver) const;

        void setMutationRateForAdjacencyFitness(unsigned int adjacencyFitness, Probability _mutationRate) {chanceOfMutationForPopCount[adjacencyFitness] = _mutationRate;};
        void setAlterationRate(Probability _alterationRate) {chanceOfAlteration = _alterationRate;};
        void setChanceOfCrossOver(Probability _crossOverRate) {chanceOfCrossOver = _crossOverRate;};

        Probability getMutationRateForAdjacencyFitness(unsigned int adjacencyFitness) const {return chanceOfMutationForPopCount.at(adjacencyFitness);};
        Probability getAlterationRate() const {return chanceOfAlteration;};
        Probability getChanceOfCrossOver() const {return chanceOfCrossOver;};

    };

} // GC

#include "XNFBreeder.cpp"

#endif //EVOCOM_XNFBREEDER_HPP
