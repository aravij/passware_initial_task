#ifndef CARTESIAN_RANGE_POWER_H
#define CARTESIAN_RANGE_POWER_H

#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_traits.hpp>

#include <boost/range/iterator.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/empty.hpp>
#include <boost/range/iterator_range.hpp>

#include <vector>
#include <algorithm>
#include <exception>
#include <iterator>

/**
 * class CartesianPowerIterator is an iterator, which with really small memory consumption
 * iterate over Cartesian power of some range.
 * Cartesian power of the range with power equals n, is a range of all possible vectors of length n
 * with elements among initial range (repetition allowed).
 */
template<class Range>
class CartesianPowerIterator:
    public boost::iterator_facade<CartesianPowerIterator<Range>,
                                  std::vector<typename boost::range_value<Range>::type> const,
                                  boost::forward_traversal_tag>
{
private:
    typedef typename boost::range_iterator<Range>::type Iterator;
    typedef typename boost::range_value<Range>::type Value;
    
    // Holding an iterator to Cartesian power N is equivalent to holding N iterators to original range.
    std::vector<Iterator> iterators;
    // Values pointed by each iterator in vector iterators.
    std::vector<Value> values;
    
    const Range range;
    bool endReached;
    
    // Needed by boost::iterator_facade
    friend class boost::iterator_core_access;
    
    const std::vector<Value> &dereference() const
    {
        return(values);
    }
    
    bool equal(const CartesianPowerIterator<Range> &anotherIterator) const
    {
        // First check if those iterators iterate same range.
        if(range != anotherIterator.range)
        {
            return(false);
        }
        
        // Than check is their power the same.
        if(iterators.size() != anotherIterator.iterators.size())
        {
            return(false);
        }
        
        // Finally check if they points to the same position in Cartesian power.
        return(std::equal(iterators.begin(), iterators.end(), anotherIterator.iterators.begin()));
    }
    
    void increment()
    {
        // The reached end iterators shouldn't be incremented anymore.
        if(endReached)
        {
            return;
        }
        
        // We increment iterators starting from the last one and updating corresponding values on the fly.
        typename std::vector<Iterator>::reverse_iterator iteratorInCartesianPower;
        typename std::vector<Value>::reverse_iterator valueInCartesianPower;
        for(valueInCartesianPower = values.rbegin(), iteratorInCartesianPower = iterators.rbegin(); 
            iteratorInCartesianPower != iterators.rend();
            ++iteratorInCartesianPower, ++valueInCartesianPower)
        {
            // Iterator iteratorInCartesianPower points itself to the iterator in range.
            // The latter one have to be incremented.
            ++(*iteratorInCartesianPower);
            
            // None of the iterator in range should point to the end of the range.
            // If such happens we should make it point to the beginning and
            // increment previous iterator (previous in sense of vector iterators).
            if(*iteratorInCartesianPower == boost::end(range))
            {
                *iteratorInCartesianPower = boost::begin(range);
                // Updating values.
                *valueInCartesianPower = *(*iteratorInCartesianPower);
            }
            else
            {
                // If we haven't reached to end of the range while incrementing iterator iteratorInCartesianPower
                // we should just update the value and stop.
                *valueInCartesianPower = *(*iteratorInCartesianPower);
                break;
            }
        }
        
        // If all iterators have been modified (all now points to the beginning of the range)
        // and break instruction never was executed this means,
        // that we have reached the end of Cartesian product.
        // If so, we fill the iterators vector with end of the range and sets the corresponding flag.
        // Also to prevent dereferencing we empty the values vector.
        if(iteratorInCartesianPower == iterators.rend())
        {
            values.resize(0);
            std::fill(iterators.begin(), iterators.end(), boost::end(range));
            endReached = true;
        }
    }
    
public:
    CartesianPowerIterator():
        iterators(), values(), range(), endReached(true)
    {}
    
    // If range is empty, values is a empty vector too and iterators is a vector of end iterators.
    CartesianPowerIterator(const Range &range, unsigned int power):
        iterators(power, boost::empty(range) ? boost::end(range) : boost::begin(range)),
        values(boost::empty(range) ? 0 : power, *boost::begin(range)), range(range), 
        endReached(boost::empty(range))
    {}
    
    CartesianPowerIterator(const Range &range, std::vector<Iterator> &iterators):
        range(range)
    {
        // If range is empty or power (size of iterators vector) is zero we point at the end of the range.
        if(boost::empty(range) || iterators.empty())
        {
            this->iterators = std::vector<Iterator>(iterators.size(), boost::end(range));
            endReached = true;
        }
        // If at all iterators point to the end, we have end Cartesian iterator.
        else if(std::all_of(iterators.begin(), iterators.end(), [&range](Iterator it)
                {
                    return(it == boost::end(range));
                }))
        {
            this->iterators = iterators;
            endReached = true;
        }
        // If we reached this point, not all iterators point to the end. But of any - it is an error.
        else if(std::any_of(iterators.begin(), iterators.end(), [&range](Iterator it)
                {
                    return(it == boost::end(range));
                }))
        {
            throw std::logic_error("ERROR in CartesianPowerIterator: "
                                   "only part of provided iterators are equaled to boost::end(range).\n"
                                   "If you want CartesianPowerIterator point to the end of Cartesian power range "
                                   "initialize it with vector of boost::end(range) of appropriate size.\n"
                                   "If you want CartesianPowerIterator to point into Cartesian power range "
                                   "initialize it with vector of non boost::end(range) iterators.\n");
        }
        // Finally usual initialization. Now we do not point to the end, so we have to properly fill values vector.
        else
        {
            this->iterators = iterators;
            std::transform(iterators.begin(), iterators.end(), std::back_inserter(values), [](Iterator it)
            {
                return(*it);
            });
            endReached = false;
        }
    }
};

/**
 * This function creates a Cartesian range by creating to iterators. One points to beginning of the range
 * and the other to the end.
 */
template <class Range>
boost::iterator_range<CartesianPowerIterator<Range>> CartesianPowerRange(const Range &range, unsigned int power)
{
    std::vector<typename boost::range_iterator<Range>::type> endIterators(power, boost::end(range));
    return(boost::make_iterator_range(CartesianPowerIterator<Range>(range, power),
                                      CartesianPowerIterator<Range>(range, endIterators)));
}

#endif
