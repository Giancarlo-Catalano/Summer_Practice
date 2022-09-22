//
// Created by gian on 23/08/22.
//

#ifndef EVOCOM_XNF_GENERIC_HPP
#define EVOCOM_XNF_GENERIC_HPP

#include "../Utilities/utilities.hpp"
#include <string>
#include <memory>


namespace GC {

    /**
     * this will be the base class for all of the other XNF templated by Gene Length
     * Each specialisation of XNF<n> inherits from this class.
     * The purpose is to be able to store all of the XNFs in a data structure despite their different types, and merge them when necessary
     *
     * The only functions to be necessary for the purpose of evolution should be merging, transforms, and toggling of individual families
     */

    class XNF_Generic {

    public:

        XNF_Generic(){};

        virtual void transform_remove_0() {};
        virtual void transform_remove_1() {};
        virtual void transform_remove_X() {};
        virtual void transform_greedy() {};
        virtual size_t getConjunctionLength() {return 0;};

        virtual std::string to_string() const {return "this class is too generic";};

        virtual std::size_t size() const {return 0;}


    };

} // GC

#endif //EVOCOM_XNF_GENERIC_HPP
