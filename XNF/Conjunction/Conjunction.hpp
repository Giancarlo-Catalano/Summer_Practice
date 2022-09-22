//
// Created by gian on 31/07/22.
//

#ifndef EVOCOM_CONJUNCTION_HPP
#define EVOCOM_CONJUNCTION_HPP

#include <string>
#include "../../Utilities/utilities.hpp"
#include <bitset>

namespace GC {

    template<std::size_t Length>
    class Conjunction;

    template<std::size_t Length>
    struct AdjacentConjunctions {
        Conjunction<Length> Adjacent_0, Adjacent_1, Adjacent_X;
    };

    /**
     * NOTE: When Length > 32, this class will malfunction due to the usage of to_uint64
     * @tparam Length
     */
    template <std::size_t Length>
    class Conjunction {
        using Bytes = std::bitset<Length*2>;

        using Index = unsigned short int;

        enum Digit { Zero, One, Both };

    public:
        Conjunction();
        Conjunction(const Conjunction<Length>& other); //copy constructor
        Conjunction(Conjunction<Length>&& source); //move constructor
        Conjunction<Length>& operator=(Conjunction<Length>& other) = default; //copy assignment
        Conjunction<Length>& operator=(const Conjunction<Length>& other) = default;
        explicit Conjunction(const Bytes &data);
        explicit Conjunction(const uint64_t rawData);

        std::string to_string() const;
        Bytes getData() const;

        Digit getAt(Index pos) const;
        void setKindAt(Index where) {data.set(getPositionOfKind(where));}
        void unsetKindAt(Index where) {data.reset(getPositionOfKind(where));}
        void setValueAt(Index where) {data.set(getPositionOfValue(where));}
        void unsetValueAt(Index where) {data.reset(getPositionOfValue(where));};
        Conjunction getAdjacent_0(Index digit) const;
        Conjunction getAdjacent_1(Index digit) const;
        Conjunction getAdjacent_X(Index digit) const;
        Conjunction getAdjacent_P(Index digit) const;
        AdjacentConjunctions<Length> getAdjacents(Index digit) const;

        AdjacentConjunctions<Length> getRightMostAdjacents() const;
        std::vector<Conjunction<Length>> getEquivalentWithoutZeros() const;

        Bytes getZeroPositionsMask() const;

        uint64_t to_uint64() const;
        uint64_t to_uint32() const;

        Conjunction<Length+1> withPrependedZero() const;
        Conjunction<Length+1> withPrependedOne() const;
        Conjunction<Length+1> withPrependedStar() const;


        using IntegerRepresentation = std::size_t;
        using TernaryGrayRepresentation = std::size_t;
        std::size_t getBase3Representation() const;
        void setFromBase3Representation(size_t base3Representation);
        std::size_t getTernaryGrayCode() const;

        bool isUnbound(const size_t lengthOfTruthTable) const;

    private:
        Bytes data;
    private:
        static Index getPositionOfKind(Index digit) {return 2*digit+1;};
        static Index getPositionOfValue(Index digit) {return 2*digit;};
        static std::string digitToString(Digit d);
        static Digit getDigitFromBools(bool kind, bool value) {
            if (kind)
                return value ? One : Zero;
            else
                return Both;
        }

        static Digit getDigitFromInt(short unsigned int i) {
            switch (i) {
                case 0: return Zero;
                case 1: return One;
                case 2: return Both;
            }
        }

        bool getKind(Index where) const {return data[getPositionOfKind(where)];}
        bool getValue(Index where) const {return data[getPositionOfValue(where)];}



        void clear();

    public:
        void fromString(std::string str); //this is debug
        friend bool operator<(const Conjunction<Length>& lhs, const Conjunction& rhs)
        {
            return lhs.data.to_ullong() < rhs.data.to_ullong();
        }



        template <class Binary>
        void constructFromBinary(Binary item);

        template <class Binary>
        Binary interpretAsBinary();
        void setDigitAt(const Digit d, Index i);

        struct ConjunctionDifference;

    };

} // GC

#include "ConjunctionDifference.hpp"
#include "Conjunction.cpp"


//implementation of the hash function to be used in unordered set
namespace std {
        template <size_t Length>
        struct hash<GC::Conjunction<Length>> {
            std::size_t operator()(const GC::Conjunction<Length>& conj) const noexcept {
#if GC_ARCH_32_BIT == 1
                return static_cast<size_t>(conj.to_uint32());
#elif GC_ARCH_64_BIT == 1
                return static_cast<size_t>(conj.to_uint64());
#endif
            }
        };
    };

#endif //EVOCOM_CONJUNCTION_HPP
