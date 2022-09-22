//
// Created by gian on 15/08/22.
//

#ifndef EVOCOM_FASTCHOOSE_HPP
#define EVOCOM_FASTCHOOSE_HPP

#include <utility>
#include "utilities.hpp"


namespace GC {
/**
 * This class is used to calculate the mathematical operation of n choose k very quickly.
 * This is done by calculating the elements of Pascal's triangle in a flat array.
 * Thus, there is a simple-ish calculation to be done to check where the index of the result of "n choose k" would be.
 * @tparam maxN the maximum value we expect from n in "n choose k", determines how big the pascal triangle will be.
 */
    template<std::size_t maxN>
    struct FastChoose {
        using bigInt = uint64_t; //note that this is not a Lisp Big Int, it's just the biggest int type we can use

        /**
         * this will store the first maxN rows, each padded with zeros
         * it starts with 0 1 0 | 0 1 1 0 | 0 1 2 1 0 | 0 1 3 3 1 0 | ....
         */
        static const bigInt pascalArraySize = ((maxN + 2) * (maxN + 3))/ 2 + maxN;
        bigInt pascalTriangle[pascalArraySize];

        /**
         * fills the pascal triangle
         */
        void fillPascalTriangle() {
            pascalTriangle[0] = 0;
            pascalTriangle[1] = 1;
            pascalTriangle[2] = 0;
            bigInt readingFrom = 0;
            bigInt writingTo = 3;

            auto readNeighbourSum = [&]() -> bigInt {
                return pascalTriangle[readingFrom] + pascalTriangle[++readingFrom];
            };

            auto writeElement = [&](bigInt value) {pascalTriangle[writingTo++] = value; };

            for (int i = 1; i <= maxN; i++) {
                writeElement(0); //every row is padded
                for (int j = 0; j < i + 1; j++)
                    writeElement(readNeighbourSum());
                writeElement(0);
                readingFrom++;
            }
        }

        FastChoose() { fillPascalTriangle(); }


        bigInt getNChooseK(bigInt n, bigInt k) {
            return pascalTriangle[((n + 2) * (n + 3) / 2) -3 + k + 1]; //first it finds the nth row (n+2)*(n+3)/2 - 3, then it adds the column (k+1)
        }

    };

}

#endif //EVOCOM_FASTCHOOSE_HPP
