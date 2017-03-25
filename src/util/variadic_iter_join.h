#ifndef VARIADIC_ITER_TOOLS_H
#define VARIADIC_ITER_TOOLS_H

#include <boost/range/join.hpp>

/**
 * Base of recursion of variadic join. Returns boost iterator range, made from given one.
 */
template<class Range>
auto join(Range&& range)
{
    return(boost::make_iterator_range(range));
}

/**
 * Variadic version of boost::join in range library. Handles arbitrary number of arguments.
 */
template<class FirstRange, class... OtherRanges>
auto join(FirstRange&& firstRange, OtherRanges&&... otherRanges)
{
    return(boost::join(boost::make_iterator_range(firstRange),
                       join(std::forward<OtherRanges>(otherRanges)...)));
}

#endif
