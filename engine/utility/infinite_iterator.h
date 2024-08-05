#pragma once

template <typename Container>
struct infinite_iterator
{
    using iterator_type = typename Container::iterator;
    using iterator_category = typename std::iterator_traits<iterator_type>::iterator_category;
    using difference_type = typename std::iterator_traits<iterator_type>::difference_type;
    using value_type = typename std::iterator_traits<iterator_type>::value_type;
    using pointer = typename std::iterator_traits<iterator_type>::pointer;
    using reference = typename std::iterator_traits<iterator_type>::reference;

    infinite_iterator(Container& container, iterator_type it)
        : m_container{container}, m_it{it}
    {

    }

    reference operator*() const { return *m_it; }
    pointer operator->() { return &(*m_it); }

    infinite_iterator& operator++() {
        ++m_it;
        if (m_it == m_container.end()) {
            m_it = m_container.begin();
        }
        return *this;
    }

    infinite_iterator operator++(int) {
        infinite_iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    void set(size_t index)
    {
        if(index > m_container.size())
        {
            throw std::runtime_error("index out of range");
        }
        m_it = m_container.begin() + index;
    }

    friend bool operator==(const infinite_iterator& a, const infinite_iterator& b) {
        return a.m_it == b.m_it;
    }

    friend bool operator!=(const infinite_iterator& a, const infinite_iterator& b) {
        return a.m_it != b.m_it;
    }

    Container& m_container;
    iterator_type m_it;
};

template <typename Container>
struct const_infinite_iterator
{
    using iterator_type = typename Container::const_iterator;
    using iterator_category = typename std::iterator_traits<iterator_type>::iterator_category;
    using difference_type = typename std::iterator_traits<iterator_type>::difference_type;
    using value_type = typename std::iterator_traits<iterator_type>::value_type;
    using pointer = typename std::iterator_traits<iterator_type>::pointer;
    using reference = typename std::iterator_traits<iterator_type>::reference;

    const_infinite_iterator(const Container& container, iterator_type it)
        : m_container{container}, m_it{it}
    {

    }

    reference operator*() const { return *m_it; }
    pointer operator->() { return &(*m_it); }

    const_infinite_iterator& operator++() {
        ++m_it;
        if (m_it == m_container.end()) {
            m_it = m_container.begin();
        }
        return *this;
    }

    const_infinite_iterator operator++(int) {
        const_infinite_iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    void set(size_t index)
    {
        if(index > m_container.size())
        {
            throw std::runtime_error("index out of range");
        }
        m_it = m_container.begin() + index;
    }

    friend bool operator==(const const_infinite_iterator& a, const const_infinite_iterator& b) {
        return a.m_it == b.m_it;
    }

    friend bool operator!=(const const_infinite_iterator& a, const const_infinite_iterator& b) {
        return a.m_it != b.m_it;
    }

    const Container& m_container;
    iterator_type m_it;
};