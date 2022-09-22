//
// Created by gian on 31/07/22.
//
#ifndef GC_XNF_CPP
#define GC_XNF_CPP

#include "XNF.hpp"


namespace GC {
    template<std::size_t Length>
    XNF<Length>::XNF():
            genes({}){
        LOG("Initialized an XNF list");
    }

    template<std::size_t Length>
    std::string XNF<Length>::to_string() const{
        std::stringstream result;
        result << "{XNF<" << Length <<  "> size = "<< getGeneCount() << ", elements = {";

        if (Length < 8) {
            for (auto gene: genes)
                result << gene.to_string()<<", ";
            result <<"}";
        }
        else {
            unsigned int leftBeforeTooMuch = 10;
            for (auto gene: genes) {
                result << "\n\t" << gene.to_string();
                leftBeforeTooMuch--;
                if (leftBeforeTooMuch<1) {
                    result << "\n\t...";
                    break;
                }
            }
            result << "\n}";
        }

        return result.str();
    }

    template<std::size_t Length>
    std::size_t XNF<Length>::getGeneCount() const{
        return genes.size();
    }

    template<std::size_t Length>
    typename XNF<Length>::Iterator XNF<Length>::activateGene(const Gene &g) {
        return Iterator(genes.emplace(g).first);
    }

    template<std::size_t Length>
    void XNF<Length>::deactivateGene(const Gene &g) {
        genes.erase(g);
    }

    template<std::size_t Length>
    void XNF<Length>::toggleGene(const Gene &g) {
        auto iterator = find(g);
        if (iterator == end())  //gene is not in the set and needs to be activated
            activateGene(g);
        else //gene is in the set and needs to be deactivated
            deactivateGene_hint(iterator);
    }

    template<std::size_t Length>
    bool XNF<Length>::contains(const Gene &g) const {
        return genes.find(g) != genes.end();
    }
    template<std::size_t Length>
    typename XNF<Length>::FamilyPresence XNF<Length>::getPresenceOfFamily(const Gene &g, const IndexInGene i) const {
        auto family = g.getAdjacents(i);
        return {contains(family.Adjacent_0), contains(family.Adjacent_1), contains(family.Adjacent_X)};
    }

    template<std::size_t Length>
    void XNF<Length>::toggleFamily(const Gene &g, const IndexInGene i) {
        auto family = g.getAdjacents(i);
        toggleGene(family.Adjacent_0);
        toggleGene(family.Adjacent_1);
        toggleGene(family.Adjacent_X);
    }



    /**
     * Precondition: the gene contains at least one zero digit
     * @tparam Length
     * @param g
     */
    template<std::size_t Length>
    void XNF<Length>::toggleFamilyOnRightMostZero(const Gene &g) {
        auto adjacents = g.getRightMostAdjacents();
        toggleGene(g); //the gene should be zero
        toggleGene(adjacents.Adjacent_1);
        toggleGene(adjacents.Adjacent_X);
    }

    template<std::size_t Length>
    void XNF<Length>::addEquivalentWithoutZeros(Gene &g) {
        auto equivalents = g.getEquivalentWithoutZeros();
        for (auto equiv: equivalents)
            toggleGene(equiv);
    }

    template<std::size_t GeneLength>
    void XNF<GeneLength>::swap(XNF<GeneLength> &other) {
        genes.swap(other.genes);
    }

    template<std::size_t GeneLength>
    void XNF<GeneLength>::deactivateGene_hint(const Iterator &iterator) {
        genes.erase(iterator.getSetIterator());
    }

    template<std::size_t GeneLength>
    typename XNF<GeneLength>::Iterator XNF<GeneLength>::begin() const{
        return Iterator(genes.begin());
    }

    template<std::size_t GeneLength>
    typename XNF<GeneLength>::Iterator XNF<GeneLength>::end() const {
        return Iterator(genes.end());
    }

    template<std::size_t GeneLength>
    typename XNF<GeneLength>::Iterator XNF<GeneLength>::find(const Gene& g) const{
        return Iterator(genes.find(g));
    }

    template<std::size_t GeneLength>
    bool XNF<GeneLength>::empty() const {
        return genes.empty();
    }

    template<std::size_t GeneLength>
    bool XNF<GeneLength>::isEquivalent(const XNF<GeneLength>& other) const {
        XNF<GeneLength> temp(other);
        for (const auto& gene: (*this)) //TODO:check it's not doing a copy
            temp.toggleGene(gene);

        return temp.transform_remove_0().empty();
    }


    template<std::size_t GeneLength>
    XNF<GeneLength>::XNF(XNF<GeneLength> &&source) :
        genes(std::move(source.genes)){
        //TODO: this causes crashes when put in a vector to be sorted
    }

    template<std::size_t GeneLength>
    XNF<GeneLength>::XNF(const XNF<GeneLength> &other) :
        genes(other.genes){

    }

    template <std::size_t OriginalGeneLength>
    XNF<OriginalGeneLength + 1> mergeXNFs(const XNF<OriginalGeneLength> &A, const XNF<OriginalGeneLength> &B) {
        XNF<OriginalGeneLength+1> result;
        auto remainingInB(B);
        for (const auto& gene: A) {
            if (B.contains(gene)) {
                result.activateGene(gene.withPrependedStar());
                remainingInB.deactivateGene(gene);
            }
            else
                result.activateGene(gene.withPrependedZero());
        }
        for (const auto& gene: remainingInB)
            result.activateGene(gene.withPrependedOne());

        return result;
    }

    template<std::size_t GeneLengthBis>
    double similarityScore(const XNF<GeneLengthBis> &A, const XNF<GeneLengthBis> &B) {
        if (A.empty() && B.empty()) {
            return 1;
        }
        unsigned int intersectionSize = 0;
        for (const auto& gene: A)
            intersectionSize += B.contains(gene);

        LOG_NOSPACES("|A|=", A.size(), ", |B|=", B.size(), "|A intersect B|=", intersectionSize);


        return (static_cast<double>(intersectionSize))/(A.size()+B.size()-intersectionSize);


    }

    template<std::size_t GeneLength>
    std::size_t XNF<GeneLength>::size() const {
        return genes.size();
    }

    template<std::size_t GeneLength>
    template<bool Prefer_None, bool Prefer_0, bool Prefer_1, bool Prefer_X>
    void XNF<GeneLength>::transform_repeat(){
        while (transform_singlePass<Prefer_None, Prefer_0, Prefer_1, Prefer_X>()) {
        };
    }

    template<std::size_t GeneLength>
    template<bool Prefer_None, bool Prefer_0, bool Prefer_1, bool Prefer_X>
    bool XNF<GeneLength>::transformOnGene(const XNF::Gene &gene) {
        auto shouldToggle = [&] (bool _0, bool _1, bool _X) {
            if (!_0 && !_1 && !_X) return !Prefer_None;
            else if (!_0 && !_1 && _X) return !Prefer_X;
            else if (!_0 && _1 && !_X) return !Prefer_1;
            else if (!_0 && _1 && _X) return Prefer_0;
            else if (_0 && !_1 && !_X) return !Prefer_0;
            else if (_0 && !_1 && _X) return Prefer_1;
            else if (_0 && _1 && !_X) return Prefer_X;
            else if (_0 && _1 && _X) return Prefer_None;
        };

        auto toggleAdjacencyIfNecessary = [&](const Gene& gene, IndexInGene index) ->bool {
            auto adjacents = AdjacencyReport(gene, index, *this);

            if (shouldToggle(adjacents.present_0, adjacents.present_1, adjacents.present_X)) {
                adjacents.toggle(*this);
                return true;
            }
            return false;
        };

        for (IndexInGene i = 0; i<GeneLength;i++) {
            if (toggleAdjacencyIfNecessary(gene, i)) {
                return true;
            }

        }
        return false;
    }

    template<std::size_t GeneLength>
    template<bool Prefer_None, bool Prefer_0, bool Prefer_1, bool Prefer_X>
    bool XNF<GeneLength>::transform_singlePass() {
        XNF<GeneLength> transformed(*this);
        bool transformDidSomething = false;
        for (const Gene& gene: genes)
            transformDidSomething |= transformed.transformOnGene<Prefer_None, Prefer_0, Prefer_1, Prefer_X>(gene);
        if (transformDidSomething) //the transformed one is kept
            swap(transformed);
        return transformDidSomething;
    }

    template<std::size_t GeneLength>
    void XNF<GeneLength>::debug_showDiff() {
        //TODO
    }

    template<std::size_t GeneLengthBis>
    bool operator==(const XNF<GeneLengthBis>& A, const XNF<GeneLengthBis>& B) {
        return A.genes == B.genes;
    }

    template<std::size_t GeneLengthBis>
    bool operator<(const XNF<GeneLengthBis> &A, const XNF<GeneLengthBis> &B) {
        return A.getGeneCount() < B.getGeneCount();
    }

    template<std::size_t GeneLength>
    std::vector<Conjunction<GeneLength>> XNF<GeneLength>::getSortedByTernaryGray() const {
        std::vector<ConjAndTernaryGray> conjAndGray;
        for (const auto& conj: genes)
            conjAndGray.template emplace_back(conj);

        std::sort(conjAndGray.begin(), conjAndGray.end());

        std::vector<Conjunction<GeneLength>> result;
        for (const auto& item : conjAndGray)
            result.template emplace_back(item.conjunction);

        return result;
    }

    template<std::size_t GeneLength>
    bool XNF<GeneLength>::isEmptyXNF() const {
        return size()==0;
    }


    /**
     * reinterprets the XNF as a truth table, which should be identical to the one that originated this XNF.
     * The method consumes the XNF, because the purpose of an XNF is to "pack" the truth table, and at some point to be "unpacked" again.
     * @tparam GeneLength
     * @tparam UnpackingUnit
     * @param destination
     */
    template <size_t GeneLength>
    template <class UnpackingUnit>
    void XNF<GeneLength>::unpackIntoVector(std::vector<UnpackingUnit>& destination, const size_t whichBit) {
        auto setInDestination = [&](size_t whichUnit) {
            Utilities::setBit(destination[whichUnit], whichBit);
        };

        transform_remove_X();
        LOG("after removing the *s, the xnf is", to_string());
        std::for_each(genes.begin(), genes.end(), [&](Conjunction<GeneLength> conj) {
            setInDestination(conj.template interpretAsBinary<size_t>());
        });
    }

    template<std::size_t GeneLength>
    std::vector<Conjunction<GeneLength>> XNF<GeneLength>::getSortedToReduceMaskDifference() const {
        auto getDifferenceScore = [&](const Gene& ga, const Gene& gb) -> size_t {
            typename Gene::ConjunctionDifference delta(ga, gb);
            return Utilities::encodeSkipPower(delta.getDifferenceMask());
        };
        std::vector<Gene> result;
        XNF<GeneLength> genesLeft = *this;

        auto addGeneIntoResult = [&](const Gene& gene) {
            result.push_back(gene);
            genesLeft.toggleGene(gene);
        };
        auto findGeneWithLowestDifference = [&](const Gene& source){
            size_t currentBestScore = ~0ULL;
            Gene currentBestGene;
            for (auto gene: genesLeft) {
                size_t score = getDifferenceScore(source, gene);
                if (score < currentBestScore) {
                    currentBestGene = gene;
                    currentBestScore = score;
                }
            }
            return currentBestGene;
        };



        Gene lastGene = *(genesLeft.begin());
        addGeneIntoResult(lastGene);

        auto addNext = [&](){
            auto nextGene = findGeneWithLowestDifference(lastGene);
            addGeneIntoResult(nextGene);
            lastGene = nextGene;
        };

        while (!genesLeft.empty())
            addNext();

        return result;
    }

    template<std::size_t GeneLength>
    template <class PredicateOnBoundedness>
    bool XNF<GeneLength>::transform_bounded_singlePass(PredicateOnBoundedness toggleCriteria,
                                                       const size_t fileSize) {

        XNF<GeneLength> transformed = *this;
        auto transformBoundedOnSingleGene = [&](const Gene& gene) -> bool {
            bool changed = false;
            for (size_t i=0;i<GeneLength;i++) {
                BoundednessReport br(gene, i, *this, fileSize);
                if (toggleCriteria(br)) {
                    LOG("The criteria is true !");
                    br.toggle(transformed);
                    changed = true;
                }
            }
            return changed;
        };

        bool changed = false;
        for (const Gene& gene : (*this)) {
            changed |= transformBoundedOnSingleGene(gene);
        }

        if (changed)
            swap(transformed);
        return changed;
    }

    template<std::size_t GeneLength>
    template <class PredicateOnBoundedness>
    bool XNF<GeneLength>::transform_bounded(PredicateOnBoundedness toggleCriteria, //TODO fix this
                                            const size_t fileSize) {
        bool changed = false;
        while (transform_bounded_singlePass(toggleCriteria, fileSize)) {
            changed = true;
        };
        return changed;
    }

    template<std::size_t GeneLength>
    bool XNF<GeneLength>::transform_bounded_greedy(const size_t fileSize) {
        auto isMostlyOnes = [&](const BoundednessReport& br) {
            short int howMany = 0;
            if (br.whichBounded != BoundednessReport::NoneBound) {
                howMany += br.is0Present;
                howMany += br.isXPresent;
            }
            if (br.whichBounded == BoundednessReport::AllBound)
                howMany += br.is1Present;
            return howMany > 1;
        };
        transform_bounded(isMostlyOnes, fileSize);
    }


    template<std::size_t GeneLength>
    bool XNF<GeneLength>::transform_bounded_remove_0(const size_t fileSize) {
        auto zeroisActive = [&](const BoundednessReport& br) {
            if (br.whichBounded != BoundednessReport::NoneBound)
                return br.is0Present;
            else
                return false;
        };
        transform_bounded(zeroisActive, fileSize);
    }

    template<std::size_t GeneLength>
    bool XNF<GeneLength>::transform_bounded_remove_1(const size_t fileSize) {
        auto oneisActive = [&](const BoundednessReport& br) {
            if (br.whichBounded == BoundednessReport::AllBound) //Note this is different from transform_bounded_remove_0
                return br.is1Present;
            else
                return false;
        };
        transform_bounded(oneisActive, fileSize);
    }

    template<std::size_t GeneLength>
    bool XNF<GeneLength>::transform_bounded_remove_X(const size_t fileSize) {
        auto starisActive = [&](const BoundednessReport& br) {
            if (br.whichBounded != BoundednessReport::NoneBound)
                return br.isXPresent;
            else
                return false;
        };
        transform_bounded(starisActive, fileSize);
    }
} // GC

#endif //GC_XNF_CPP