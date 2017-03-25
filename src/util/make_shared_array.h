#ifndef MAKE_SHARED_ARRAY_H
#define MAKE_SHARED_ARRAY_H

#include <boost/smart_ptr/shared_array.hpp>

#include <cstddef>

/**
 * This is a helping function, which creates a shared array by only specifying its type and size.
 */
template <class Type>
boost::shared_array<Type> make_shared_array(const std::size_t arraySize)
{
    return(boost::shared_array<Type>(new Type[arraySize]));
}

#endif
