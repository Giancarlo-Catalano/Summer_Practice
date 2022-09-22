//
// Created by gian on 14/08/22.
//

#ifndef EVOCOM_XNFEVOLVER_HPP
#define EVOCOM_XNFEVOLVER_HPP

#include "../Utilities/utilities.hpp"
#include "../XNF/Conjunction/Conjunction.hpp"
#include "../XNF/XNF.hpp"
#include "../XNFBreeder/XNFBreeder.hpp"
#include <sstream>
#include <random>
#include <set>


namespace GC {
        template <std::size_t GeneLength>
        class XNFEvolver {
            using Individual = XNF<GeneLength>;
            using Breeder = XNFBreeder<GeneLength>;
            using Amount = size_t;
            using Fitness = size_t;
            using Pool = std::multimap<Fitness, Individual>; //prevents identical individuals
            using Proportion = double;
        private:
            Pool population{};
            Breeder breeder;
            std::size_t populationSize; //mu
            std::size_t selectionSize;
            //bool usesSimulatedAnnealing;
            bool isElitist;
        public:
            XNFEvolver(const Individual &adam, Amount populationSize = 100, Amount selectionSize = 30, bool isElitist = true,
                       unsigned int seed = 6);

            void evolveGenerationOnce();

            void evolvePopulationForGenerations(Amount generations);

            Amount evolveUntilFitnessReached(Fitness goal);

            Amount evolveUntilFitnessStagnates(Amount generationsWithoutImprovement);

            Individual getBestOfPopulation() const;

            Fitness getCurrentBestFitness() const;

            Fitness getCurrentWorstFitness() const;

            void greedyReducePopulation();

            void sortPopulationByFitness();

            std::string to_string();

            static void for_each_individual(const Pool& pool, std::function<void(const Individual&)> operation);

        private:
            static Fitness getFitnessOfIndividual(const Individual &individual) { return individual.getGeneCount(); };

            static void repeat(Amount amount, const std::function<void()> &operation) {
                for (Amount i = 0; i < amount; i++) { operation(); }
            };


            void addIndividual(const Individual &individual);
            void addIndividual(const Individual &&individual);

            void truncationSelection();

            void TournamentSelection(std::size_t poolSize);

            void UniformSelection();

            void replenishPopulation_fitnessProportionate();

            void removeRedundant();


            struct FitnessProportionteDistribution {
                std::vector <Amount> cumulativeDistribution{};
                std::uniform_int_distribution<unsigned int> distribution;
                std::default_random_engine generator;
                const Pool& populationToBeSampled;

                FitnessProportionteDistribution(const Pool &population, unsigned int seed = 7) :
                        generator(seed),
                        populationToBeSampled(population){
                    Amount accumulatedFitness = 0;
                    auto findWorstFitness = [&]() {
                        return std::prev(populationToBeSampled.end())->first; //TODO this should be rewritten to make it less reliant on the specific definition of fitness as the size of the XNF
                    };

                    const Amount worstFitness = findWorstFitness() + 1;
                    auto betterFitnessMakesItBigger = [&](Fitness fitness) {
                        return worstFitness - fitness;
                    };

                    for (const auto&[fitness, individual] : populationToBeSampled) {
                        accumulatedFitness+=fitness;
                        cumulativeDistribution.emplace_back(accumulatedFitness);
                    }

                    distribution = std::uniform_int_distribution<unsigned int>(0, accumulatedFitness);
                }

                const Individual& chooseIndividual() {
                    Amount randomValue = distribution(generator);
                    auto iterator = populationToBeSampled.begin();
                    for (const auto& cumula : cumulativeDistribution) {
                        if (cumula>=randomValue)  //TODO this is dangerous, as the population might only have individuals of fitness 0. >= is necessary, but I don't trust it
                            break;
                        iterator++;
                    }
                    ASSERT_NOT_EQUALS(iterator, populationToBeSampled.end())
                    return iterator->second;
                };
            };

        };
} // GC

#include "XNFEvolver.cpp"

#endif //EVOCOM_XNFEVOLVER_HPP
