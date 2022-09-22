//
// Created by gian on 15/08/22.
//

#ifndef EVOCOM_FASTBITPERMUTATIONINDEX_HPP
#define EVOCOM_FASTBITPERMUTATIONINDEX_HPP
#include <utility>
#include "utilities.hpp"
#include "FastChoose.hpp"


namespace GC {

/**
 * This class is used for the efficient storage of bit masks with known popcount.
 * Knowing that for bit length N and popcount P, there are N choose P alternatives.
 * The purpose of this class is to say:
 *      - the amount of alternatives given N and P
 *      - if they were to be enumerated in a consistent order (used for compression), what index would the given mask appear in?
 * @tparam maxN  maximum expected value of N
 */
    template<std::size_t maxN>
    struct FastBitPermutationIndex {
        using BigInt = uint64_t; //note that this is not a Lisp Big Int, it's just the biggest int type we can use
        using Mask = BigInt;
        using PopCount = BigInt;
        using MaskLength = BigInt;
        using IndexInEnumeration = BigInt;

        FastChoose<maxN> fastChoose;

        FastBitPermutationIndex() : fastChoose() {};

        IndexInEnumeration getIndexInEnumeration(const Mask mask, const MaskLength length, const PopCount popCount) {
            Mask maskCopy = mask;
            MaskLength lengthLeft = length;
            PopCount onesLeft = popCount;
            IndexInEnumeration result = 0;
            while (maskCopy) {
                bool isOne = maskCopy & 1;
                const IndexInEnumeration alternativesIfOne = fastChoose.getNChooseK(lengthLeft-1, onesLeft-1);
                if (isOne)
                    onesLeft--;
                else
                    result += alternativesIfOne;
                lengthLeft--;
                maskCopy>>=1;
            }
            return result;
        }

        IndexInEnumeration getEnumerationRange(const MaskLength maskLength, const PopCount popCount) {
            return fastChoose.getNChooseK(maskLength, popCount);
        }


        Mask decodeFromEnumeration(const IndexInEnumeration encoded, const MaskLength length, const PopCount popCount) {
            MaskLength lengthLeft = length;
            PopCount onesLeft = popCount;

            Mask result = 0;
            IndexInEnumeration accumulator = 0;

            BigInt currentDigitPosition = 0;
            auto putIntoResult = [&](bool value) {
                result |= (value<<currentDigitPosition);
                currentDigitPosition++;
                onesLeft -= value;
            };

            while (onesLeft > 0) {
                BigInt alternativesIfDigitIsOne = fastChoose.getNChooseK(lengthLeft - 1, onesLeft - 1);
                if (accumulator + alternativesIfDigitIsOne <= encoded) {
                    putIntoResult(0);
                    accumulator+=alternativesIfDigitIsOne;
                }
                else {
                    putIntoResult(1);
                }
                lengthLeft--;
            }
            return result;
        }

    };

};

#endif //EVOCOM_FASTBITPERMUTATIONINDEX_HPP
