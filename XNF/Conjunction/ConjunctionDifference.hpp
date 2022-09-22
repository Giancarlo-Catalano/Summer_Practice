//
// Created by gian on 11/09/22.
//

#ifndef EVOCOM_CONJUNCTIONDIFFERENCE_HPP
#define EVOCOM_CONJUNCTIONDIFFERENCE_HPP


#include "Conjunction.hpp"

namespace GC {

    template<size_t Length>
    struct Conjunction<Length>::ConjunctionDifference {
        using Conj = Conjunction<Length>;
        using ConjDigit = Conj::Digit;
        using ConjIndex = Conj::Index;
        using Direction = bool;
        using DifferenceMask = uint32_t;
        using DifferencePopCount = short unsigned int;

        struct DifferenceItem {

            std::size_t position; //0 indexed from the right
            Direction directionOfChange;

            std::string to_string() {
                std::stringstream ss;
                ss << "@" << position << ":" << directionOfChange;
                return ss.str();
            }
        };

        std::vector<DifferenceItem> differences;

        std::string to_string() const {
            std::stringstream ss;
            ss << "Diff:{";
            auto showDiffItem = [&](auto diffItem) { ss << diffItem.to_string() << ", "; };
            std::for_each(differences.begin(), differences.end(), showDiffItem);
            ss << "}";
            return ss.str();
        };

        static Direction directionOfChange(ConjDigit A, ConjDigit B) {
            return ((A == Zero && B == One) || (A == One && B == Both) || (A == Both && B == Zero));
        }

        ConjunctionDifference() : differences() {
        }

        ConjunctionDifference(const Conjunction<Length> &A, const Conjunction<Length> &B) {
            for (unsigned int i = 0; i < Length; i++) {
                Digit dA = A.getAt(i);
                Digit dB = B.getAt(i);
                if (dA != dB)
                    differences.push_back(DifferenceItem{i, directionOfChange(dA, dB)});
            }
        }

        ConjunctionDifference(const DifferenceMask &mask, const std::vector<Direction> &directions) :
                differences(directions.size()) {
            std::bitset<Length> maskBitSet(mask);
            std::size_t currentIndexOfDiff = 0;
            for (Index i = 0; i < 32; i++) {
                if (maskBitSet[i]) {
                    differences[currentIndexOfDiff] = {i, directions[currentIndexOfDiff]};
                    currentIndexOfDiff++;
                }
            }
        }

        std::size_t getAmountOfDifferences() const {
            return differences.size();
        }

        std::vector<Direction> getDifferenceDirections() const {
            std::vector<Direction> directions(getAmountOfDifferences());
            for (unsigned int i = 0; i < directions.size(); i++)
                directions[i] = differences[i].directionOfChange;
            return directions;
        }

        DifferenceMask getDifferenceMask() const {
            DifferenceMask mask = 0;
            for (auto &[pos, diffDir]: differences)
                mask |= 1 << pos;
            return mask;
        }

        static Digit getDigitChange(const Digit original, bool direction) {
            switch (original) {
                case Zero:
                    return (direction ? One : Both);
                case One :
                    return (direction ? Both : Zero);
                case Both:
                    return (direction ? Zero : One);
            }
        }

        Conjunction<Length> apply(const Conjunction<Length> &conj) const {
            Conjunction<Length> result(conj);
            for (const auto &[position, directionOfChange]: differences) {
                auto oldDigit = conj.getAt(position);
                auto newDigit = getDigitChange(oldDigit, directionOfChange);
                result.setDigitAt(newDigit, position);
            }
            return result;
        }

    };

}

#endif //EVOCOM_CONJUNCTIONDIFFERENCE_HPP
