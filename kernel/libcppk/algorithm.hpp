namespace std {

class InputIterator {
public:
    virtual ~InputIterator() {}
    virtual const InputIterator& operator*() const = 0;
    virtual bool operator!=(const InputIterator& other) const = 0;
    virtual const InputIterator& operator++() = 0;
};

class UnaryPred {
public:
    virtual ~UnaryPred() {}
    virtual bool operator()(const InputIterator& it) const = 0;
};

template<class InputIterator, class T>
constexpr InputIterator find(InputIterator first, InputIterator last, const T& val) {
    while (first!=last) {
        if (*first==val) return first;
        ++first;
    }
    return last;
}

template<class InputIterator, class UnaryPred>
constexpr InputIterator find_if(InputIterator first, InputIterator last, UnaryPred p)
{
    for (; first != last; ++first)
        if (p(*first))
            return first;
 
    return last;
}

}
