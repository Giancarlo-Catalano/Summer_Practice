//
// Created by gian on 31/07/22.
//

#ifndef GC_XNF_HPP
#define GC_XNF_HPP

#include <unordered_set>
#include <utility>
#include "Conjunction/Conjunction.hpp"
#include "XNF_Generic.hpp"

namespace GC {

    template <std::size_t GeneLength>
    class XNF : public XNF_Generic {

    public:
        using Gene = Conjunction<GeneLength>;
        using IndexInGene = short unsigned int;
        using GeneSet = std::unordered_set<Gene>;
        using SetIterator = typename GeneSet::const_iterator;

        struct Iterator;
        struct FamilyPresence {
            bool present_0, present_1, present_X;
            std::string to_string() {std::stringstream ss; ss<<"{"<<present_0<<", "<<present_1<<", "<<present_X<<"}";return ss.str();}
        };

        struct AdjacencyReport {
            AdjacentConjunctions<GeneLength> adjacents;
            bool present_0, present_1, present_X;
            AdjacencyReport(const Gene& mainGene, IndexInGene index, const XNF<GeneLength>& xnf) :
                adjacents(mainGene.getAdjacents(index)),
                present_0(xnf.find(adjacents.Adjacent_0) != xnf.end()),
                present_1(xnf.find(adjacents.Adjacent_1) != xnf.end()),
                present_X(xnf.find(adjacents.Adjacent_X) != xnf.end())
                {
                }

            void toggleGene(const Gene& adjacent, const bool present, XNF<GeneLength>& xnf) const {
                if (present)
                    xnf.deactivateGene(adjacent);
                else
                    xnf.activateGene(adjacent);
            }

            void toggle(XNF<GeneLength>& xnf) const {
                toggleGene(adjacents.Adjacent_0, present_0, xnf);
                toggleGene(adjacents.Adjacent_1, present_1, xnf);
                toggleGene(adjacents.Adjacent_X, present_X, xnf);
            }

            unsigned int getPopCount() const {
                return present_0+present_1+present_X;
            }


            std::string to_string() {
                std::stringstream ss;
                ss<<"{"<<adjacents.Adjacent_0.to_string()<<":"<<present_0<<", ";
                ss<<adjacents.Adjacent_1.to_string()<<":"<<present_1<<", ";
                ss<<adjacents.Adjacent_X.to_string()<<":"<<present_X;
                ss<<"}";
                return ss.str();
            }
        };


    public: //general use
        XNF();
        XNF(const XNF<GeneLength>& other); //copy constructor
        XNF(XNF<GeneLength>&& source); //move constructor

        size_t getConjunctionLength() override {return GeneLength;};
        bool contains(const Gene &g) const;
        Iterator activateGene(const Gene &g);
        void deactivateGene(const Gene &g);

        void transform_remove_0() override {transform_repeat<1, 0, 1, 1>();}
        void transform_remove_1() override {transform_repeat<1, 1, 0, 1>();}
        void transform_remove_X() override {transform_repeat<1, 1, 1, 0>();}
        void transform_greedy() override {transform_repeat<1, 1, 1, 1>();}




        XNF<GeneLength>& operator=(const XNF<GeneLength>& other) = default; //copy assignment operator
        void swap(XNF<GeneLength>& other);
        std::string to_string() const override;

    public: //main functions

        void deactivateGene_hint(const Iterator &iterator);
        void toggleGene(const Gene &g);
        std::size_t getGeneCount() const;
        void toggleFamily(const Gene &g, const IndexInGene i);


        bool isEquivalent(const XNF<GeneLength>& other) const;

        template<bool Prefer_None, bool Prefer_0, bool Prefer_1, bool Prefer_X>
        bool transformOnGene(const Gene& gene);

        template<bool Prefer_None, bool Prefer_0, bool Prefer_1, bool Prefer_X>
        bool transform_singlePass();

        template<bool Prefer_None, bool Prefer_0, bool Prefer_1, bool Prefer_X>
        void transform_repeat();

        void debug_showDiff();


        template<std::size_t OriginalGeneLength>
        friend XNF<OriginalGeneLength+1> mergeXNFs(const XNF<OriginalGeneLength>& A, const XNF<OriginalGeneLength>&B);


        template <std::size_t GeneLengthBis>
        friend double similarityScore(const XNF<GeneLengthBis>& A, const XNF<GeneLengthBis>& B);

        template<std::size_t GeneLengthBis>
        friend bool operator==(const XNF<GeneLengthBis>& A, const XNF<GeneLengthBis>& B);

        template<std::size_t GeneLengthBis>
        friend bool operator<(const XNF<GeneLengthBis>& A, const XNF<GeneLengthBis>& B);






    public: //iterator functions
        Iterator begin() const;
        Iterator end() const;
        Iterator find(const Gene &g) const;
        bool empty()  const;
        std::size_t size() const override;

    private: //members
        std::unordered_set<Gene> genes{};

    private: //helper methods
        FamilyPresence getPresenceOfFamily(const Gene &g, const IndexInGene i) const;
        void toggleFamilyOnRightMostZero(const Gene &g);
        void addEquivalentWithoutZeros(Gene &g);
        bool tryGreedyReduceOnGene(const Gene &g);


    public: //helpers for compression
        struct ConjAndTernaryGray {
            Conjunction<GeneLength> conjunction;
            typename Conjunction<GeneLength>::TernaryGrayRepresentation grayRepresentation;

            ConjAndTernaryGray() : conjunction(), grayRepresentation() {
            }

            ConjAndTernaryGray(const Conjunction<GeneLength>& _conj) :
                    conjunction(_conj),
                   grayRepresentation(_conj.getTernaryGrayCode()){
            }
            friend bool operator<(const ConjAndTernaryGray& lhs, const ConjAndTernaryGray& rhs) {
                return lhs.grayRepresentation < rhs.grayRepresentation;
            }
        };

        std::vector<Gene> getSortedByTernaryGray() const;
        std::vector<Gene> getSortedToReduceMaskDifference() const;

        bool isEmptyXNF() const;


        template <class UnpackingUnit>
        void unpackIntoVector(std::vector<UnpackingUnit>& destination, const size_t whichBit);


        struct BoundednessReport {
            enum WhichBounded {
                AllBound, OneUnbound, NoneBound
            } whichBounded;

            bool is0Present, is1Present, isXPresent;

            AdjacentConjunctions<GeneLength> adjancents;

            std::string to_string() {

                std::stringstream ss;
                ss<<"{";
                if (whichBounded == AllBound)
                    ss << is0Present << ", " << is1Present << ", " <<isXPresent;
                else if (whichBounded == OneUnbound)
                    ss << is0Present << ", U, " <<isXPresent;
                else
                    ss << "U, U, U";
                ss << "}";
                return ss.str();
            }

            BoundednessReport(const Conjunction<GeneLength>& conj, const IndexInGene index, XNF<GeneLength>& xnf, const size_t sizeOfFile) :
                adjancents(conj.getAdjacents(index))
                {
                if (adjancents.Adjacent_0.isUnbound(sizeOfFile)) {
                    whichBounded = NoneBound;
                }
                else if (adjancents.Adjacent_1.isUnbound(sizeOfFile)) {
                    whichBounded = OneUnbound;
                    is0Present = xnf.contains(adjancents.Adjacent_0);
                    isXPresent = xnf.contains(adjancents.Adjacent_X);
                }
                else {
                    whichBounded = AllBound;
                    is0Present = xnf.contains(adjancents.Adjacent_0);
                    is1Present = xnf.contains(adjancents.Adjacent_1);
                    isXPresent = xnf.contains(adjancents.Adjacent_X);
                }
            }

            void toggle(XNF<GeneLength>& xnf) const{
                //the unbound items will not be toggled, as they are "useless" and should remain off
                //since they will never get turned on, there is no need to turn them off
                //therefore, all we do is toggle the bound genes
                auto toggle0 = [&](){xnf.toggleGene(adjancents.Adjacent_0);};
                auto toggle1 = [&](){xnf.toggleGene(adjancents.Adjacent_1);};
                auto toggleX = [&](){xnf.toggleGene(adjancents.Adjacent_X);};
                if (whichBounded == AllBound) {
                    toggle0();toggle1();toggleX();
                }
                else if (whichBounded == OneUnbound) {
                    toggle0();toggleX();
                }
            }
        };

        template <class PredicateOnBoundedness>
        bool transform_bounded_singlePass(const PredicateOnBoundedness toggleCriteria,
                                          const size_t fileSize);

        template <class PredicateOnBoundedness>
        bool transform_bounded(PredicateOnBoundedness toggleCriteria, const size_t fileSize);

        bool transform_bounded_greedy(const size_t fileSize);
        bool transform_bounded_remove_0(const size_t fileSize);
        bool transform_bounded_remove_1(const size_t fileSize);
        bool transform_bounded_remove_X(const size_t fileSize);
    };



#include "XNF_Iterator.hpp"

} // GC

#include "XNF.cpp"

#endif //GC_XNF_HPP
