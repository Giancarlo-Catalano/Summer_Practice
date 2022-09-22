//
// Created by gian on 31/07/22.
//
#ifndef GC__CONJUNCTION_CPP
#define GC__CONJUNCTION_CPP

#include "Conjunction.hpp"

namespace GC {

    template <std::size_t Length>
    bool operator==(const Conjunction<Length>& lhs, const Conjunction<Length>& rhs) {
        return lhs.to_uint64() == rhs.to_uint64();
    }

    template<std::size_t Length>
    Conjunction<Length>::Conjunction() {
    }

    template<std::size_t Length>
    Conjunction<Length>::Conjunction(const uint64_t rawData) :
            data(rawData){
    }

    template<std::size_t Length>
    Conjunction<Length>::Conjunction(const Bytes &_data) :
            data(_data) {
    }

    template<std::size_t Length>
    std::string Conjunction<Length>::to_string() const {
        std::stringstream ss;
        for (int i= Length - 1; i >= 0; i--)
            ss<<(digitToString(getAt(i)));
        return ss.str();
    }

    template<std::size_t Length>
    typename Conjunction<Length>::Digit Conjunction<Length>::getAt(Conjunction::Index pos) const{
        ASSERT(pos < Length);
        return getDigitFromBools(getKind(pos), getValue(pos));
    }

    template<std::size_t Length>
    Conjunction<Length> Conjunction<Length>::getAdjacent_X(Conjunction::Index digit) const{
        Conjunction<Length> clone = (*this);
        clone.unsetKindAt(digit);
        clone.unsetValueAt(digit);
        return clone;
    }

    template<std::size_t Length>
    Conjunction<Length> Conjunction<Length>::getAdjacent_0(Conjunction::Index digit) const{
        Conjunction<Length> clone = (*this);
        clone.setKindAt(digit);
        clone.unsetValueAt(digit);
        return clone;
    }

    template<std::size_t Length>
    Conjunction<Length> Conjunction<Length>::getAdjacent_1(Conjunction::Index digit) const{
        Conjunction<Length> clone = (*this);
        clone.setKindAt(digit);
        clone.setValueAt(digit);
        return clone;
    }

    template<std::size_t Length>
    Conjunction<Length> Conjunction<Length>::getAdjacent_P(Conjunction::Index digit) const{
        Conjunction<Length> clone = (*this);
        clone.unsetKindAt(digit);
        clone.setValueAt(digit);
        return clone;
    }

    template<std::size_t Length>
    AdjacentConjunctions<Length> Conjunction<Length>::getAdjacents(Conjunction::Index digit) const{
        return {getAdjacent_0(digit), getAdjacent_1(digit), getAdjacent_X(digit)};
    }

    template<std::size_t Length>
    typename Conjunction<Length>::Bytes Conjunction<Length>::getData() const {
        return data;
    }

    template<std::size_t Length>
    void Conjunction<Length>::fromString(std::string str) {
        clear();
        auto pushIntoData = [&](bool bit) {
            data <<=1;
            data |= bit;
        };

        auto push2 = [&](bool a, bool b) {
            pushIntoData(a);
            pushIntoData(b);
        };

        if (str.length() != Length) {
            LOG("the provided length of the string \"", str, "\" does not match the required length of ", std::to_string(Length));
            ASSERT(false);
        }
        for (int i=0;i<Length;i++) {
            char strChar = str[i];
            switch (strChar) {
                case '0': push2(1, 0); break;
                case '1': push2(1, 1); break;
                case '*': push2(0, 0); break;
                case '?': push2(0, 1); break;
                default: ERROR_NOT_IMPLEMENTED();
            }
        }
    }

    template<std::size_t Length>
    void Conjunction<Length>::clear() {
        data.reset();
    }

    template<std::size_t Length>
    template<class Binary>
    void Conjunction<Length>::constructFromBinary(Binary item) {
        clear(); //TODO: is this necessary?
        // 0 -> 10
        // 1 -> 11
        for (int i=Length-1;i>=0;i--) {
            data<<=1;   //make space
            data|=1; //always kind = 1
            data<<=1;  //make space
            data|=(item >> i)&1;  //the value is the same as the bit in item
        }

    }


    /**
     * this is an alternative version of getAdjacents which uses the digit where the leftmost zero is
     * @tparam Length the maximum amount of terms possible in the conjuction
     * @return the adjacents, still in the order 0 1 *
     */
    template<std::size_t Length>
    AdjacentConjunctions<Length> Conjunction<Length>::getRightMostAdjacents() const {
        auto getRightMostBit = [](uint64_t b_bits) -> uint64_t {
            uint64_t result = (b_bits & (-b_bits));
            LOG_NOSPACES("getRightMostBit(", Utilities::asBinaryString(b_bits), ") = ", Utilities::asBinaryString(result));
            return result;
        };

        LOG_NOSPACES("myself(", to_string(), ").ZeroPositionsMask = ", getZeroPositionsMask().to_string());
        uint64_t kindSetWhereZeroDigit = getRightMostBit(getZeroPositionsMask().to_ullong());

        auto fromRawToConjunction = [](uint64_t raw) {
            return Conjunction<Length>(Bytes(raw));
        };

        uint64_t ownData = data.to_ullong();

        uint64_t adj_1 = ownData | kindSetWhereZeroDigit >> 1;
        uint64_t adj_X = ownData ^ (kindSetWhereZeroDigit);

        AdjacentConjunctions<Length> adjs;
        adjs.Adjacent_0 = *this;
        adjs.Adjacent_1 = fromRawToConjunction(adj_1);
        adjs.Adjacent_X = fromRawToConjunction(adj_X);

        return adjs;
    }

    template<std::size_t Length>
    std::vector<Conjunction<Length>> Conjunction<Length>::getEquivalentWithoutZeros() const {
        //many functions to be tested, as they are intrinsics
        //pdep https://stackoverflow.com/questions/27363906/parallel-bit-deposit-parallel-bit-extract-on-intel-compiler-llvm/27371072#27371072

        //__builtin_popcount https://iq.opengenus.org/__builtin_popcount-in-c/
        auto zeroMask = getZeroPositionsMask();
        auto enumerationsOfZeroMask = Utilities::parallelEnumerateInMask(zeroMask);
        auto adjacentTemplate = data ^ zeroMask;
        auto getAdjacentFromEnumerationItem = [&](auto item) {
            return Conjunction<Length>(adjacentTemplate | item | (item>>1));
        };


        unsigned int expectedAmount = enumerationsOfZeroMask.size();
        std::vector<Conjunction<Length>> result(expectedAmount);

        for (unsigned int i=0;i<expectedAmount;i++)
            result[i] = getAdjacentFromEnumerationItem(enumerationsOfZeroMask[i]);
        return result;
    }


    /**
     * for every pair of digits K V , zeros are detected where K=1, V=0;
     * the result is the bitset but 0 -> 0 and  1, *, ? -> *
     * @tparam Length
     * @return all digits are set to *, except for zeros
     */
    template<std::size_t Length>
    typename Conjunction<Length>::Bytes Conjunction<Length>::getZeroPositionsMask() const {
        constexpr Bytes kindMask(0xaaaaaaaaaaaaaaaaULL); //it does get cut off where necessary
        return (data & ~(data<<1)) & kindMask;
    }

    template<std::size_t Length>
    Conjunction<Length>::Conjunction(const Conjunction<Length> & other) :
            data(other.data){
    }

    template<std::size_t Length>
    uint64_t Conjunction<Length>::to_uint64() const{
        return data.to_ullong();
    }

    template<std::size_t Length>
    uint64_t Conjunction<Length>::to_uint32() const {
        return data.to_ulong();
    }

    template<std::size_t Length>
    Conjunction<Length>::Conjunction(Conjunction<Length> &&source) :
        data(std::move(source.data)){
            //TODO: verify this is a legal operation
    }

    template<std::size_t Length>
    Conjunction<Length+1> Conjunction<Length>::withPrependedZero() const {
        Conjunction<Length+1> result(data.to_ullong());
        result.setKindAt(Length);
        //assumes the new portion is zeros
        return result;
    }

    template<std::size_t Length>
    Conjunction<Length + 1> Conjunction<Length>::withPrependedOne() const {
        Conjunction<Length+1> result(data.to_ullong());
        result.setKindAt(Length);
        result.setValueAt(Length);
        return result;
    }

    template<std::size_t Length>
    Conjunction<Length + 1> Conjunction<Length>::withPrependedStar() const {
        Conjunction<Length+1> result(data.to_ullong());
        return result;
    }

    template<std::size_t Length>
    std::size_t Conjunction<Length>::getBase3Representation() const { //TODO make this more efficient
        u_int64_t result = 0;
        for (int i = Length-1;i>=0;i--) {
            result*=3;
            Digit digit = getAt(i);
            switch (digit) {
                case Zero: break;
                case One: result+=1;break;
                case Both: result+=2;break;
            }
        }

        return result;
    }

    template <std::size_t Length>
    void Conjunction<Length>::setFromBase3Representation(const size_t base3Representation) {
        clear();
        size_t temp = base3Representation;
        for (int i=0;i<Length;i++) {
            unsigned short int newDigitAsInt = temp % 3;
            Digit newDigit = getDigitFromInt(newDigitAsInt);
            setDigitAt(newDigit, i);
            temp /= 3;
        }
        //TODO check this works, i CBA right now
    }

    template<std::size_t Length>
    std::size_t Conjunction<Length>::getTernaryGrayCode() const {

        std::size_t result = 0;
        auto pushIntoResult = [&result](unsigned int digit) {
            result = (result/3)+(digit* Utilities::pow_constexpr(3, Length-1)); //this is to push it from the left
        };

        constexpr uint64_t mask = 0xaaaaaaaaaaaaaaaaULL & ((~0ULL)>>(64-Length*2));  //used to toggle all of the "kind" bits
        u_int64_t oldBase3Digits = to_uint64() ^ mask;
        /*     raw    raw ^ 10
         * 0    10      00      = 0
         * 1    11      01      = 1
         * *    00      10      = 2
         */

        unsigned short int currentSumOfDigits = 0;
        for (unsigned int i = 0;i<Length;i++) {
            currentSumOfDigits += (oldBase3Digits>>(i*2))&0b11;
        }

        for (unsigned int digitPosition = 0; digitPosition<Length; digitPosition++) {
            unsigned int tempBase3Digit = oldBase3Digits & 0b11;
            bool isOdd = currentSumOfDigits & 0b1;
            unsigned int mod6 = tempBase3Digit + 3 * isOdd;
            if (mod6 < 3)
                pushIntoResult(mod6);
            else
                pushIntoResult(5-mod6);

            oldBase3Digits>>=2;
            currentSumOfDigits -= tempBase3Digit;
        }

        return result;
    }

    template<std::size_t Length>
    std::string Conjunction<Length>::digitToString(Digit d) {
        switch (d) {
            case Zero: return "0";
            case One: return "1";
            case Both: return "*";
        }
    }

    template <std::size_t Length>
    void Conjunction<Length>::setDigitAt(const Digit d, const Index i) {
        if (d == Both) {
            unsetKindAt(i);
            unsetValueAt(i);
        }
        else {
            setKindAt(i);
            if (d == One)
                setValueAt(i);
            else
                unsetValueAt(i);
        }
    }

    template<std::size_t Length>
    template<class Binary>
    Binary Conjunction<Length>::interpretAsBinary() {
        //assumes that there are no *'s.
        // then we can just read the value bits.

        Binary result = 0;
        for (size_t i=0;i<Length;i++)
            result |= getValue(i)<<i;
        return result;
    }

    template<std::size_t Length>
    bool Conjunction<Length>::isUnbound(const size_t lengthOfTruthTable) const {
        auto getWithXreplacedBy0 = [&](const Conjunction<Length>& conj) -> Conjunction<Length> {
            Conjunction<Length> result = conj;
            for (int i=0;i<Length;i++) {
                if (result.getAt(i) == Both)
                    result.setDigitAt(Zero, i);
            }
            return result;
        };

        auto getMinimumValueCovered = [&getWithXreplacedBy0](const Conjunction<Length>& conj) -> size_t{
            return getWithXreplacedBy0(conj).template interpretAsBinary<size_t>();
        };

        return getMinimumValueCovered(*this) > lengthOfTruthTable;
    }
} // GC

#endif //GC_CONJUNCTION_CPP