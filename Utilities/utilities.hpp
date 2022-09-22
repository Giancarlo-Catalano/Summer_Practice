#ifndef GC_UTILITIES_H
#define GC_UTILITIES_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <stdexcept>
#include <stack>
#include <fstream>
#include <optional>
#include <functional>
#include <sstream>
#include <bitset>

#define GC_DEBUG 1
#define GC_ASSERTIONS 1
#define GC_PLATFORM_WINDOWS 0
#define GC_PLATFORM_LINUX 1
#define GC_ARCH_32_BIT 0
#define GC_ARCH_64_BIT 1

#define PUT_VAR_ON_STREAM(varName, sstream) sstream<<(#varName)<<" = "<<(varName)<<", ";

#define GC_NODISCARD [[nodiscard]]

#if GC_DEBUG == 1
#define LOG(...) printMany(__VA_ARGS__);printMany("\n")
#define LOG_CALLER() LOG("#Called", __func__)
#define LOG_VAR(v) \
    do { std::cout << (#v) << "=" << (v) << ";\n"; } while(false)

#define FLUSH_LOG() std::cout.flush()

#define LOG_NONEWLINE(...) printMany(__VA_ARGS__);
#define LOG_NOSPACES(...) printManyWithoutSpaces(__VA_ARGS__);printMany("\n");
#define LOG_NONEWLINE_NOSPACES(...) printManyWithoutSpaces(__VA_ARGS__);



#else

//when necessary, just put a #define nameofmacro for each macro defined in the #if, eg
#define LOG(msg)
#define LOG_CALLER()

#endif



#if GC_ASSERTIONS == 1
#define ASSERT_EQUALS(var, value)													\
	if (var!=value) {														\
		LOG("ASSERTION FAILED:", #var, "=", var, ";expected ", value);		\
		throw std::invalid_argument("Assertion failed");					\
	}

#define ASSERT_NOT_EQUALS(var, value)												\
	if (var==value) {														\
		LOG("ASSERTION_NOT FAILED:", #var, "has an unexpected value");		\
		throw std::invalid_argument("Assertion_NOT failed");					\
	}
#define ASSERT(var) ASSERT_EQUALS(var, true)

#define ASSERT_IN_BOUNDS(var, min, max) \
    if (var < min) {                    \
        LOG("ASSERTION_IN_BOUNDS FAILED:", #var, "=", var, "is below the minimum of ", min); \
        throw std::invalid_argument("ASSERTION_IN_BOUNDS failed");                           \
    }                                   \
    else if (var > max) {               \
        LOG("ASSERTION_IN_BOUNDS FAILED:", #var, "=", var, "is above the maximum of ", max); \
        throw std::invalid_argument("ASSERTION_IN_BOUNDS failed");                           \
    }

#define ASSERT_VALID_INDEX(index, container) ASSERT(index >= 0); ASSERT(index < container.size());
#define ERROR_NOT_IMPLEMENTED() throw std::invalid_argument("Not implemented yet! Sorry Gian");
#define ASSERT_UNREACHED() throw(std::invalid_argument("This code point was not supposed to be reachable.."))


#define T_IF(T_IF_condition, T_IF_then, T_IF_else) typename std::conditional<T_IF_condition, T_IF_then, T_IF_else>::type

#endif



#if GC_PLATFORM_WINDOWS == 1
#define KEEP_CONSOLE_OPEN() system("pause")
#define GC_DIRECTORY_SLASH == "\\"
#elif GC_PLATFORM_LINUX == 1
#define KEEP_CONSOLE_OPEN() LOG("Press any key to quit"); std::string s; std::cin >> s; return 0;
#define GC_DIRECTORY_SLASH == "/"
#endif


std::string indent(const std::string&);
std::string getErrorMessage();
void dumpFile(std::string fileName, std::string mode="chars");
std::ifstream openFileForInput(std::string filePath);
std::ofstream openFileForOutput(std::string filePath);




template <class Printable>
void printManyWithoutSpaces(const Printable single);

template <class Printable, typename... Many>
void printManyWithoutSpaces(const Printable head, const Many...);

template <class Printable>
void printMany(const Printable single);

template <class Printable, typename... Many>
void printMany(const Printable head, const Many...);


template <class Printable>
void printMany(const Printable single) {
    std::cout << single;
}


template <class Printable, typename... Many>
void printMany(const Printable head, const Many... rest) {
    std::cout << head << " ";
    printMany(rest...);
}

template<class Printable>
void printManyWithoutSpaces(const Printable single) {
    std::cout << single;
}

template <class Printable, typename... Many>
void printManyWithoutSpaces(const Printable head, const Many... rest) {
    std::cout << head;
    printManyWithoutSpaces(rest...);
}

template <class Unit>
bool dumpStream(std::ifstream& fileStream, std::string (*printingFunction) (Unit));


/**
 * Reads the stream contents into the LOG
 * @tparam Unit how the stream is read (chars, ints, custom structs...) needs to implement Unit(unsigned char[sizeof(Unit)] and to_string() -> std::string
 * @tparam Function Unit the type that will be extracted from the stream over and over
 * @param fileStream the (input) file stream used to fetch contents
 * @param printingFunction the function used to convert the unit into a string
 * @return whether the file reading was successful
 */
template <class Unit>
bool dumpStream(std::ifstream& fileStream) {
    LOG_CALLER();
    //returns true when the stream was read to the end without problems
    std::size_t unitCount = 0;
    std::size_t unitsPerLine = 10;

    auto printLineNumber = [](std::size_t count) { std::cout<<std::setw(3)<<count; };
    auto getUnitFromStream = [&]() -> Unit {
        unsigned char asBytes[sizeof(Unit)];
        for (int i=0;i<sizeof(Unit);i++)
            fileStream >> asBytes[i];
        return Unit(asBytes);
    };

    fileStream >> std::noskipws;
    const size_t amountOfSymbolsPerLine = 8;
    size_t inCurrentLine = 0;
    while (fileStream) {
        Unit tempUnit = getUnitFromStream();
        if (fileStream.eof()) {
            LOG("\nRead", unitCount, "units");
            return true;
        }
        LOG_NONEWLINE(tempUnit.to_string());
        if (inCurrentLine >= amountOfSymbolsPerLine-1) {
            LOG("");
            inCurrentLine = 0;
        }
        else
            inCurrentLine++;
        unitCount++;
    }
    return false; //it shouldn't get out of the loop unless the fileStream is bad
}

namespace Utilities {
    std::string doubleToString(double d, unsigned int _precision = 2);

    template <class GenericInt>
    bool getBitFromRight(GenericInt bytes, std::size_t pos);

    template <class GenericInt>
    bool getBitFromLeft(GenericInt bytes, std::size_t pos);

    template <class GenericInt>
    GenericInt withSetBit(GenericInt bytes, std::size_t pos);

    template <class GenericInt>
    GenericInt withUnsetBit(GenericInt bytes, std::size_t pos);

    template <class GenericInt>
    GenericInt withBit(GenericInt bytes, std::size_t pos, bool value);

    template <class GenericInt>
    std::string asBinaryString(GenericInt data, std::size_t length = 0);


    template <class T>
    T withOverflowFlag_x_times_y_plus_z(T x, T y, T z, bool& wasSuccessfull) {
        T temp_x_times_y = x * y;

#define setFlagToFalseAndReturn() wasSuccessfull = false; return 0;

        if ((x!=0) && ((temp_x_times_y / x) != y)) {
            setFlagToFalseAndReturn();
        }

        T temp_x_times_y_plus_z = temp_x_times_y + z;
        if (temp_x_times_y_plus_z < temp_x_times_y) {
            setFlagToFalseAndReturn();
        }
        wasSuccessfull = true;
        return temp_x_times_y_plus_z;
    }

    template <class T>
    T withOverflowFlag_x_times_y(T x, T y, bool& wasSuccessfull) {
        T temp_x_times_y = x * y;
#define setFlagToFalseAndReturn() wasSuccessfull = false; return 0;

        if ((x!=0) && ((temp_x_times_y / x) != y)) {
            setFlagToFalseAndReturn();
        }

        wasSuccessfull = true;
        return temp_x_times_y;
    }



    template <class GenericInt>
    bool getBitFromRight(GenericInt bytes, std::size_t pos) {
        return (bytes>>pos)&1;
    }

    template <class GenericInt>
    bool getBitFromLeft(GenericInt bytes, std::size_t pos) {
        getBitFromRight(bytes, sizeof(GenericInt)*8-1-pos);
        //TODO make this more efficient and make sure it works right
    }

    template <class GenericInt>
    GenericInt withSetBit(GenericInt bytes, std::size_t pos) {
        return bytes | (1<<pos);
    }

    template <class GenericInt>
    void setBit(GenericInt& input, const size_t position) {
        input |= 1<<position;
    }

    template <class GenericInt>
    GenericInt withUnsetBit(GenericInt bytes, std::size_t pos) {
        return bytes & ~(1<<pos);
    }

    template <class GenericInt>
    GenericInt withBit(GenericInt bytes, std::size_t pos, bool value) {
        GenericInt whichToChange = 1 << pos;
        GenericInt valueInPosition = value << pos;
        return ((~whichToChange & bytes)) | (whichToChange & valueInPosition);

        //TODO verify this is correct
    }

    template <class GenericInt>
    std::string asBinaryString(GenericInt data, std::size_t length) { //this is embarassing but ICBA
        std::string result;
        std::size_t actualLength = length == 0 ? (sizeof(GenericInt)*8) : length;
        for (int i=0;i<actualLength;i++) {
            bool digit = data%2;
            data >>= 1;
            result = (digit ? "1" : "0") + result;
        }
        return result;
    }


    uint64_t parallelBitDeposit(uint64_t src, uint64_t mask);

    template <std::size_t N>
    std::bitset<N> getRightMostSetBit(const std::bitset<N>& input);

    template <std::size_t N>
    std::vector<std::bitset<N>> parallelEnumerateInMask(const std::bitset<N>& mask);

    template<class T>
    inline constexpr T pow_constexpr(const T base, unsigned const exponent);




    // IMPLEMENTATION OF TEMPLATED FUNCTIONS
    template <std::size_t N>
    std::bitset<N> getRightMostSetBit(const std::bitset<N>& input) {
        //requires 64 bit architecture
#if N <= 32
    uint64_t bits = input.to_ulong();
#else
    uint64_t bits = input.to_ullong();
#endif
        return std::bitset<N>(bits & (-bits));
    }

    /**
     * considering every set bit in the mask, it generates all the possible combinations of unsetting them
     * @param mask the mask whose ones will be unset
     * @return a vector of length 2^(popcount(mask)) containing all of the combinations
     */
    template <std::size_t N>
    std::vector<std::bitset<N>> parallelEnumerateInMask(const std::bitset<N>& mask) {
        std::vector<std::bitset<N>> result(1<<mask.count()); //returns 2^popcount(mask) items
        auto mask_copy = mask;
        result[0] = 0;
        unsigned int currentLength = 1;
        while (mask_copy.any()) {
            auto isolatedBit = getRightMostSetBit(mask_copy);
            unsigned int writeTo = currentLength;

            for (unsigned int readFrom = 0; readFrom < currentLength; readFrom++, writeTo++)
                result[writeTo] = result[readFrom]^isolatedBit;
            currentLength<<=1;
            mask_copy ^= isolatedBit;
        }
        return result;
    }


    template<class T>
    inline constexpr T pow_constexpr(const T base, unsigned const exponent)
    { //copied shamelessly from https://stackoverflow.com/questions/16443682/c-power-of-integer-template-meta-programming
        // (parentheses not required in next line)
        return (exponent == 0) ? 1 : (base * pow_constexpr(base, exponent-1));
    }

    template<class T>
    inline constexpr T log2_constexpr_aux(const T input, unsigned const int accumulator) {
        return (input < 2) ? accumulator : log2_constexpr_aux(input/2, accumulator+1);
    }

    template<class T>
    inline constexpr T log2_constexpr(const T input) { return log2_constexpr_aux(input, 0);}


    void repeat(const size_t amount, const std::function<void()>& operation);

    template <class Container>
    void LOG_container(Container c) {
        LOG_NONEWLINE_NOSPACES("{");
        bool isFirst = true;
        for (auto item: c) {
            if (!isFirst)
                LOG_NONEWLINE_NOSPACES(", ");
            isFirst = false;
            LOG_NONEWLINE_NOSPACES(item);
        }
        LOG("}");
    }

    template <class Container>
    void LOG_container_to_string(Container c) {
        LOG_NONEWLINE_NOSPACES("{");
        bool isFirst = true;
        for (auto item: c) {
            if (!isFirst)
                LOG_NONEWLINE_NOSPACES(", ");
            isFirst = false;
            LOG_NONEWLINE_NOSPACES(item.to_string());
        }
        LOG("}");
    }


    size_t log2_ceil(const size_t input);


    size_t getFileSize(const std::string& fileName);


    size_t encodeSkipPower(const size_t input);
    size_t decodeSkipPower(const size_t input);

    std::vector<bool> getRiceEncoded(const size_t x);
    size_t getRiceDecoded(const std::vector<bool>& encoded);

    size_t floor_log2 (const size_t i);

    template <class T>
    size_t getPopCount(const T input) {
        return std::bitset<sizeof(decltype(input))*8>(input).count();
    }

    template <class T>
    std::function<void()> function_appendResult(const std::vector<T>& vec, std::function<T()> operation) {
        return [&](){vec.push_back(operation());};
    }

    template <class T>
    std::vector<T> vectorOfRepeatedApplication(const std::function<T()> operation, const size_t amount) {
        std::vector<T> result;
        repeat(amount, [&](){result.push_back(operation());});
        return result;
    }

}


#endif /* GC_UTILITIES_H */