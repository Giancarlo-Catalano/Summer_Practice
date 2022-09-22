//
// Created by gian on 14/08/22.
//
#ifndef GC_XNFEVOLVER_CPP
#define GC_XNFEVOLVER_CPP

#include "XNFEvolver.hpp"
#include <algorithm>


namespace GC{

    template<std::size_t GeneLength>
    XNFEvolver<GeneLength>::XNFEvolver(const XNFEvolver::Individual &adam, Amount _populationSize,
                                       Amount _selectionSize, bool _isElitist, unsigned int seed):
            populationSize(_populationSize),
            selectionSize(_selectionSize),
            isElitist(_isElitist),
            breeder(seed),
            population(){


        Individual temp(adam); addIndividual(temp);
        temp.transform_remove_0(); addIndividual(temp);
        temp.transform_remove_1(); addIndividual(temp);
        temp.transform_remove_X(); addIndividual(temp);
        temp.transform_greedy(); addIndividual(temp);

        replenishPopulation_fitnessProportionate();
    }

    template<std::size_t GeneLength>
    std::string XNFEvolver<GeneLength>::to_string() {
        std::stringstream ss;
        ss << "XNFEvolver {";

        PUT_VAR_ON_STREAM(populationSize, ss);
        PUT_VAR_ON_STREAM(selectionSize, ss);
        PUT_VAR_ON_STREAM(isElitist, ss);
        ss << breeder.to_string() << ", ";

        auto showPopulation = [&]() {
            ss << "Population:{";
            for_each_individual(population, [&](auto i){ss << "\n\t" << i.to_string() << ", ";});
            ss << "}";
        };

        //showPopulation();
        ss << "}";

        return ss.str();
    }

    template<std::size_t GeneLength>
    void XNFEvolver<GeneLength>::truncationSelection() {
        if (population.size() < selectionSize) {
            LOG("The population was already smaller than the selection size! (population has", population.size(), "individuals)");
            return;
        }
        Amount individualsThatPassedSoFar = 0;
        typename Pool::iterator startOfKilling = population.begin();
        for (Amount allowedSoFar = 0;allowedSoFar<selectionSize;allowedSoFar++)
            startOfKilling++;
        population.erase(startOfKilling, population.end());
    }

    template<std::size_t GeneLength>
    void XNFEvolver<GeneLength>::replenishPopulation_fitnessProportionate() {

        //the population is the parents used for breeding
        Pool parents = population;
        if (!isElitist)
            population.clear();

        FitnessProportionteDistribution fpd(parents); //TODO add a seed

        auto addFromCrossedParents = [&]() {
            Individual A = fpd.chooseIndividual();
            Individual B = fpd.chooseIndividual();
            std::pair <Individual, Individual> offspring = breeder.crossover(A, B);
            addIndividual(offspring.first);
            addIndividual(offspring.second);
        };

        auto addMutatedIndividual = [&](){
            addIndividual(breeder.tweak(fpd.chooseIndividual()));
        };

        auto ceilToEven = [&](Amount input) { return input + (input % 2); };

        Amount leftToAdd = populationSize - population.size();  //why does it overflow here???
        Amount amountFromCrossOver = ceilToEven(leftToAdd / 2);
        Amount amountFromMutation = leftToAdd - amountFromCrossOver;

        repeat(amountFromCrossOver, addFromCrossedParents);
        repeat(amountFromMutation, addMutatedIndividual);
    }


/**
 * assumes the population starts with a popSize amount
 * @tparam GeneLength
 * @return returns the fitness of the best individual
 */
    template<std::size_t GeneLength>
    void XNFEvolver<GeneLength>::evolveGenerationOnce() {
        removeRedundant();
        truncationSelection();
        replenishPopulation_fitnessProportionate();
    }

    template<std::size_t GeneLength>
    void XNFEvolver<GeneLength>::evolvePopulationForGenerations(Amount generations) {
        //LOG_CALLER();
        repeat(generations, [&]() {this->evolveGenerationOnce(); });
    }

    template<std::size_t GeneLength>
    typename XNFEvolver<GeneLength>::Individual XNFEvolver<GeneLength>::getBestOfPopulation() const {
        return population.begin()->second;
    }

    template<std::size_t GeneLength>
    void XNFEvolver<GeneLength>::addIndividual(const XNFEvolver::Individual &individual) {
        population.insert(std::pair<Fitness, Individual>(getFitnessOfIndividual(individual), individual));
    }

    template<std::size_t GeneLength>
    void XNFEvolver<GeneLength>::addIndividual(const XNFEvolver::Individual &&individual) {
        population.insert(std::pair<Fitness, Individual>(getFitnessOfIndividual(individual), std::move(individual)));
    }

    template<std::size_t GeneLength>
    void XNFEvolver<GeneLength>::for_each_individual(const Pool& pool, const std::function<void(const Individual&)> operation){
        for (const auto&[key, individual] : pool) {
            operation(individual);
        }
    }

    template<std::size_t GeneLength>
    void XNFEvolver<GeneLength>::removeRedundant() {
        Pool newPopulation;

        for (auto iterator = population.begin(), end = population.end();
             iterator != end;
             iterator = population.upper_bound(iterator->first))
        {
            newPopulation.insert(*iterator);
        }
        population.swap(newPopulation);
    }

}


#endif //GC_XNFEVOLVER_CPP