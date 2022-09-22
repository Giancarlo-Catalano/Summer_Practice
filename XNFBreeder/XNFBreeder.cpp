//
// Created by gian on 11/08/22.
//

#ifndef GC_XNFBREEDER_CPP
#define GC_XNFBREEDER_CPP

#include "../Utilities/utilities.hpp"

#include "XNFBreeder.hpp"
#include <sstream>

namespace GC {

    template<std::size_t GeneLength>
    XNFBreeder<GeneLength>::XNFBreeder(unsigned int randomizerSeed):
            randomPercentageGen(randomizerSeed),
            randomIndexInGeneGenerator(randomizerSeed) {
    }

    template<std::size_t GeneLength>
    bool XNFBreeder<GeneLength>::makeChoice(XNFBreeder::Probability probabilityOfTrue) const{
        return probabilityOfTrue > randomPercentageGen.getRandom();
    }

    template<std::size_t GeneLength>
    bool XNFBreeder<GeneLength>::mostCommonBool(bool a, bool b, bool c) {
        return ((a & (b ^ c)) ^ (b & c));
    }

    template<std::size_t GeneLength>
    bool XNFBreeder<GeneLength>::generalTraitOfFamilyPresence(const XNFBreeder::AdjacencyReport &ar) {
        return mostCommonBool(ar.present_0, ar.present_1, ar.present_X);
    }

    template<std::size_t GeneLength>
    bool XNFBreeder<GeneLength>::areEligibleForCrossOver(const XNFBreeder::AdjacencyReport &arA,
                                                         const XNFBreeder::AdjacencyReport &arB) {
        return generalTraitOfFamilyPresence(arA) != generalTraitOfFamilyPresence(arB);
    }

    template<std::size_t GeneLength>
    typename XNFBreeder<GeneLength>::ToggleRecipe XNFBreeder<GeneLength>::decideToggling(const XNFBreeder::AdjacencyReport &arA,
                                                                    const XNFBreeder::AdjacencyReport &arB) const{
        ToggleRecipe toggleRecipe;
        toggleRecipe.toggleFirst ^= makeChoice(chanceOfMutationForPopCount.at(arA.getPopCount()));
        toggleRecipe.toggleSecond ^= makeChoice(chanceOfMutationForPopCount.at(arB.getPopCount()));

        if (areEligibleForCrossOver(arA, arB)) {
            if (makeChoice(chanceOfCrossOver)) {}
            toggleRecipe.crossOver();
        }
        return toggleRecipe;
    }

    template<std::size_t GeneLength>
    void XNFBreeder<GeneLength>::mutateAndCrossOverOnGene(const XNFBreeder::Gene &gene, const Individual& firstParent, const Individual& secondParent, Individual& firstChild, Individual& secondChild) const {
        IndexInGene randomIndex = randomIndexInGeneGenerator.getRandom();
        AdjacencyReport adjA(gene, randomIndex, firstParent);
        AdjacencyReport adjB(gene, randomIndex, secondParent);

        ToggleRecipe toggleRecipe = decideToggling(adjA, adjB);
        toggleRecipe.applyToggling(adjA, adjB, firstChild, secondChild);
    }

    template<std::size_t GeneLength>
    std::pair<typename XNFBreeder<GeneLength>::Individual, typename XNFBreeder<GeneLength>::Individual>
    XNFBreeder<GeneLength>::crossover(const XNFBreeder::Individual &A, const XNFBreeder::Individual &B) const{
        XNF<GeneLength> firstChild(A);
        XNF<GeneLength> secondChild(B);

        for (const auto& gene: A) {
            if (makeChoice(chanceOfAlteration))
                mutateAndCrossOverOnGene(gene, A, B, firstChild, secondChild);
        }
        for (const auto& gene: B) {
            if (!(A.contains(gene)) && (makeChoice((chanceOfAlteration)))) //prevents mutating genes that have already been mutated
                mutateAndCrossOverOnGene(gene, A, B, firstChild, secondChild);
        }
        return {firstChild, secondChild};
    }

    template<std::size_t GeneLength>
    typename XNFBreeder<GeneLength>::Individual XNFBreeder<GeneLength>::tweak(const XNFBreeder::Individual &A) const {
        XNF<GeneLength> child(A);
        for (const auto& gene: A) {
            if (makeChoice(chanceOfAlteration))
                tweakGene(gene, child);
        }
        return child;
    }

    template<std::size_t GeneLength>
    void XNFBreeder<GeneLength>::tweakGene(const XNFBreeder::Gene &gene, Individual& receiver) const {
        IndexInGene randomIndex = randomIndexInGeneGenerator.getRandom();
        receiver.toggleFamily(gene, randomIndex);
    }

    template<std::size_t GeneLength>
    std::string XNFBreeder<GeneLength>::to_string() {
        std::stringstream ss;
        ss<<"XNFBreeder: {";
        PUT_VAR_ON_STREAM(chanceOfAlteration, ss);
        PUT_VAR_ON_STREAM(chanceOfCrossOver, ss);
        ss<<"chanceOfMutationForAdjancencyFitness:{";
        auto addChanceOfMutationGivenAF = [&](int fitness) {
            ss<<fitness<<"->"<<chanceOfMutationForPopCount.at(fitness)<<"%"<<", ";
        };

        addChanceOfMutationGivenAF(0);
        addChanceOfMutationGivenAF(1);
        addChanceOfMutationGivenAF(2);
        addChanceOfMutationGivenAF(3);
        ss<<"}}";
        return ss.str();
    }


} // GC

#endif //GC_XNFBREEDER_CPP