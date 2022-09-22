//
// Created by gian on 19/08/22.
//

#ifndef EVOCOM_ARITHMETICCODER_HPP
#define EVOCOM_ARITHMETICCODER_HPP
#include <utility>
#include "../../../Utilities/utilities.hpp"
#include <iterator>
#include "../../../Utilities/FastBitPermutationIndex.hpp"

namespace GC {

    /**
     * This class is a generic implementation of Arithmetic coding.
     * Given a sequence of integers, and their possible maximums (+1) (ie encode 5, maximum was 23...),
     * it compresses them and gives the encoded data into an handler.
     * To retrieve the values, all you need is the ranges.
     *
     * *Usage*
     * **Encoding**
         * ArithmeticCoder ac; //get an Arithmetic coder
         * ArithmeticCoder::Encoder encoder = ac.getEncoder(handler); //where handler::EncodedValue->void, it might be a function to write the encoded into a file for example
         * for (auto item: inputs) encoder.push(item);
             * NOTE: as you push items into the encoder, it automatically calls the handler when new encoded values are ready
             * NOTE: when you encode items, ALWAYS add at the end
         * std::size_t leftoverAmount = encoder.forceRemaining();
         *      which will give the handler the remaining encoded data that hasn't been handled yet, and it will return a "leftover amount", this is necessary for decoding
         *
     * **Decoding**
     * ArithmeticCoder::Decoder decoder = ac.getDecoder(handler, requester, leftoverAmount);
     *      //handler:: DecodedValue -> void, it's what happens when a decoded value is ready to be used
     *      //requester:: void -> EncodedValue,  since it's hard to predict when a new encoded value is necessary, we let the decoder choose for us.
     *      //requester initially returns the last item of an encoded stream, and every time it's called it returns the preceding encoded item of what it returned last time.
     *      //usually it's something like: iterator listIter = list.end(); auto requester = [&listIter]{return *(--listIter);}
     * for (auto item: reverse(ranges)) decoder.decode(range);
         * NOTE: the ranges have to be supplied in reverse (ie the last ranges produced in the encoding are the first ones to be decoded)
         * NOTE: the resulting decoded values will be in reverse order of what was originally encoded
     *
     *
     * TODO: when range = 0, both the encoder and decoder panic, as decoding a range of 0 involves dividing by 0
     * Given that the range is the maximum value + 1, range = 0 implies that there were -1 choices, which is impossible
     * Should we throw an error?
     */
    template <class EncodedInt>
    class ArithmeticCoder {
    public:
        using DecodedInt = EncodedInt;
        using Range = size_t;
        using Value = size_t;
        using EncodingHandler = std::function<void(const EncodedInt)>;
        using DecodingHandler = std::function<void(const DecodedInt)>;
        using DecodingRangeRequester = std::function<Range(void)>;
        using DecodingEncodedRequester = std::function<EncodedInt(void)>;

    public:

        struct OverflowChecker {
            EncodedInt accumulator = 1;

            void reset() {accumulator = 1;}

            bool causesOverflow(const size_t k) {
                size_t oldAccumulator = accumulator;
                accumulator*=k;
                if (k!=0 && (accumulator/k != oldAccumulator)) {
                    reset();
                    accumulator*=k;
                    return true;
                }
                return false;
            }
        };



        struct Encoder {
            EncodedInt encoded = 0;
            OverflowChecker overflowChecker;
            const EncodingHandler handler;

            std::size_t howManyEncodedInCurrentBuffer = 0;

            Encoder(const EncodingHandler _handler): handler(_handler), overflowChecker(){};

            void reset() {
                encoded = 0;
                howManyEncodedInCurrentBuffer = 0;
            }

            void unsafe_applyRangeAndValue(const Range range, const Value value) {
                encoded = encoded * range + value;
            }

            void push(const Range range, const Value value){
                ASSERT_NOT_EQUALS(range, 0);
                if (overflowChecker.causesOverflow(range)) {
                    handler(encoded);
                    reset();
                }
                unsafe_applyRangeAndValue(range, value);
                howManyEncodedInCurrentBuffer++;
            }

            void encodeAll(std::vector<std::pair<Range, Value>> pairs) {
                for (const auto&[range, value] : pairs)
                    push(range, value);
                forceRemaining();
            }

            /**
             * Forces the last "unhandled" encoded data to be pushed into the handler.
             * @return It returns how many encoded values are stored in this leftover encoded item, which is necessary for decoding.
             */
            std::size_t forceRemaining() {
                if (howManyEncodedInCurrentBuffer != 0)
                    handler(encoded);
                return howManyEncodedInCurrentBuffer;
            }
        };

        /**
         * note that due to the nature of arithmetic encoding, the ranges have to be suppplied in the reverse order
         * of the encoding order, and the results will also be reversed
         */
        struct Decoder {
            const DecodingHandler handler;
            const DecodingEncodedRequester encodedRequester; //when this is called, the next item in the encoded stream is returned
            OverflowChecker overflowChecker;
            EncodedInt buffer;

        public:
            void requestNewEncoded() {
                buffer = encodedRequester();
            }

            Decoder(const DecodingHandler& _decodingHandler, const DecodingEncodedRequester& _requester) :
                    handler(_decodingHandler),
                    encodedRequester(_requester),
                    overflowChecker()
            {
            }

            DecodedInt unsafe_extractFromBuffer(const Range range) {
                DecodedInt result = buffer % range;
                buffer /= range;
                return result;
            }

            DecodedInt decode(const Range range) {
                if (overflowChecker.checkForOverflow(range))
                    requestNewEncoded();
                handler(unsafe_extractFromBuffer(range));
            }



            //TODO this is not working properly
            //this works differently from the rest, as it will read the encodedUnits in order
            std::vector<Value> decodeFromRanges(const std::vector<Range>& ranges) {

                std::vector<Value> result;
                auto iterator = ranges.begin();
                auto reachedEndOfVector = [&iterator, &ranges]() {return iterator==ranges.end();};
                while (!reachedEndOfVector()) {
                    buffer = encodedRequester();
                    overflowChecker.reset();
                    std::vector<Value> decodedInUnit;
                    auto startIterator = iterator;
                    while (!reachedEndOfVector() && !overflowChecker.causesOverflow(*iterator)) {
                        iterator++;
                    }
                    auto endIterator = iterator;

                    while (true) {
                        endIterator--;
                        decodedInUnit.push_back(unsafe_extractFromBuffer(*endIterator));
                        if (endIterator == startIterator)
                            break;
                    }
                    result.insert(result.end(), decodedInUnit.rbegin(), decodedInUnit.rend());
                }
                return result;
            }

            template <size_t MaskLength>
            std::vector<DecodedInt> decodeFromPopCounts(const std::vector<typename FastBitPermutationIndex<MaskLength>::PopCount>& popCounts) {
                //TODO
                ERROR_NOT_IMPLEMENTED();
            }


        };
    public:

        static Encoder getEncoder(EncodingHandler handler) {
            return Encoder(handler);
        }

        static Decoder getDecoder(DecodingHandler handler, DecodingEncodedRequester requester) {
            return Decoder(handler, requester);
        }

    };
}

#endif //EVOCOM_ARITHMETICCODER_HPP
