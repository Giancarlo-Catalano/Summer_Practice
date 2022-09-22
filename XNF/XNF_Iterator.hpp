#ifndef GC_XNF_ITERATOR_HPP
#define GC_XNF_ITERATOR_HPP

template <std::size_t GeneLength>
struct XNF<GeneLength>::Iterator {
    using XNFGene = typename XNF<GeneLength>::Gene;
    using SetIterator = typename XNF<GeneLength>::SetIterator;

    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = XNFGene;
    using pointer = typename SetIterator::pointer;
    using reference = typename XNF<GeneLength>::SetIterator::reference;

    Iterator(pointer ptr) : m_setIterator(ptr) {};
    Iterator(typename XNF<GeneLength>::SetIterator setIterator) : m_setIterator(setIterator) {};
private:
    SetIterator m_setIterator;
    pointer getSetPointer() const {return m_setIterator.operator->();}


public:
    reference operator*() const {return *getSetPointer();}
    pointer operator->() {return getSetPointer();}
    Iterator& operator++() {m_setIterator++; return *this;}
    Iterator operator++(int) {Iterator temp = *this; ++(*this);}

    friend bool operator==(const Iterator& a, const Iterator& b) {return a.getSetPointer() == b.getSetPointer();}
    friend bool operator!=(const Iterator& a, const Iterator& b) {return a.getSetPointer() != b.getSetPointer();}

    SetIterator getSetIterator() const {return m_setIterator;}
};
#endif // GC_XNF_ITERATOR_HPP