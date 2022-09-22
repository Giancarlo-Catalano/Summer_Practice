//
// Created by gian on 22/08/22.
//

#ifndef EVOCOM_CODECUTILITIES_HPP
#define EVOCOM_CODECUTILITIES_HPP

#include "../../../Utilities/utilities.hpp"
#include <memory>



//Stolen from https://github.com/swansontec/map-macro
//this is necessary to fully expand macros, that is to apply MACRO expansion multiple times if necessary
//if more expansion is needed, add more EVALN's, this currently evals 40 times, should be enough
#define EVAL0(...) __VA_ARGS__
#define EVAL1(...) EVAL0 (EVAL0 (EVAL0 (__VA_ARGS__)))
#define EVAL2(...) EVAL1 (EVAL1 (EVAL1 (__VA_ARGS__)))
#define EVAL(...)  EVAL0 (EVAL0 (__VA_ARGS__))

//this is ugly, but it's simple and more understandable than what I found elsewhere.
//when it causes errors, the errors are a lot less ugly than in the alternatives

#define GC_MAP_FROM_1_TO_32(M) \
                    EVAL(M(1))\
                    EVAL(M(2))\
                    EVAL(M(3))\
                    EVAL(M(4))\
                    EVAL(M(5))\
                    EVAL(M(6))\
                    EVAL(M(7))\
                    EVAL(M(8))\
                    EVAL(M(9))\
                    EVAL(M(10))\
                    EVAL(M(11))\
                    EVAL(M(12))\
                    EVAL(M(13))\
                    EVAL(M(14))\
                    EVAL(M(15))\
                    EVAL(M(16))\
                    EVAL(M(17))\
                    EVAL(M(18))\
                    EVAL(M(19))\
                    EVAL(M(20))\
                    EVAL(M(21))\
                    EVAL(M(22))\
                    EVAL(M(23))\
                    EVAL(M(24))\
                    EVAL(M(25))\
                    EVAL(M(26))\
                    EVAL(M(27))\
                    EVAL(M(28))\
                    EVAL(M(29))\
                    EVAL(M(30))\
                    EVAL(M(31))\
                    EVAL(M(32))


namespace GC {




}

#endif //EVOCOM_CODECUTILITIES_HPP
