

#include "non_trivial.h"

non_trivial::non_trivial() : ptr_(nullptr) { std::cout << "default construct null" << std::endl; }

non_trivial::non_trivial(int value) : ptr_(new int(value)) { std::cout << "construct value " << value << std::endl; }

non_trivial::non_trivial(const non_trivial &other) : ptr_(nullptr)
{
    if (other.ptr_ == nullptr)
        std::cout << "copy construct null" << std::endl;
    else
    {
        std::cout << "copy construct value " << *other.ptr_ << std::endl;
        ptr_ = new int(*other.ptr_);
    }
}

non_trivial::non_trivial(non_trivial &&other) : ptr_(nullptr)
{
    if (other.ptr_ == nullptr)
        std::cout << "move construct null" << std::endl;
    else
    {
        std::cout << "move construct value " << *other.ptr_ << std::endl;
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
    }
}

non_trivial &non_trivial::operator=(const non_trivial &other)
{
    if (other.ptr_ == nullptr)
    {
        std::cout << "copy assign null" << std::endl;
        deallocate();
    }
    else if (*ptr_ != *other.ptr_)
    {
        std::cout << "copy assign value: " << *other.ptr_ << std::endl;
        deallocate();
        ptr_ = new int(*other.ptr_);
    }
    else
    {
        std::cout << "copy assign same value " << *ptr_ << std::endl;
    }

    return *this;
}

non_trivial &non_trivial::operator=(non_trivial &&other)
{
    std::cout << "move assign..." << std::endl;
    ptr_ = other.ptr_;
    other.ptr_ = nullptr;
    return *this;
}

void non_trivial::deallocate()
{
    if (ptr_ == nullptr)
        std::cout << "deallocate nullptr" << std::endl;
    else
    {
        std::cout << "deallocate " << *ptr_ << std::endl;
        delete ptr_;
        ptr_ = nullptr;
    }
}

non_trivial::~non_trivial()
{
    std::cout << "deconstruct..." << std::endl;
    deallocate();
}

non_trivial::operator int() const
{
    if (ptr_ == nullptr)
        throw std::runtime_error("nullptr");
    auto val = *ptr_;
    std::cout << "cast to: " << val << std::endl;
    return val;
}

std::ostream &operator<<(std::ostream &out, const non_trivial &nt)
{
    std::cout << "non_trivial " << std::endl;

    if (nt.ptr_ == nullptr)
        return out << "null";
    return out << *nt.ptr_;
}

void swap(non_trivial &one, non_trivial &other) noexcept
{
    using std::swap;

    swap(one.ptr_, other.ptr_);
}