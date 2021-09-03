//a non_trivial struct for general testing
//put message out on every step of its life cycle

#ifndef NON_TRIVIAL_H
#define NON_TRIVIAL_H

#include <iostream>

struct non_trivial
{
    friend std::ostream &operator<<(std::ostream &, const non_trivial &);

    friend void swap(non_trivial &one, non_trivial &other) noexcept;

    non_trivial();
    non_trivial(int value);
    non_trivial(const non_trivial &other);
    non_trivial(non_trivial &&other);

    non_trivial &operator=(const non_trivial &other);

    non_trivial &operator=(non_trivial &&other);

    ~non_trivial();

    operator int() const;

private:
    void deallocate();
    int *ptr_;
};

#endif