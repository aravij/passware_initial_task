#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <boost/range/irange.hpp>

#include <vector>
#include <set>
#include <algorithm>
#include <cmath>

#include "util/cartesian_range_power.h"

BOOST_DATA_TEST_CASE(cartesian_range_power_test,
                     boost::unit_test_framework::data::random(0, 1000) ^
                     boost::unit_test_framework::data::random(0, 1000) ^
                     boost::unit_test_framework::data::random(1, 50) ^
                     boost::unit_test_framework::data::xrange(1, 4),
                     begin, length, step, power)
{
    auto initialRange = boost::irange(begin, begin + length, step);
    typedef typename boost::range_value<decltype(initialRange)>::type Value;
    
    const std::size_t cartesianRangeSize = std::pow(boost::size(initialRange), power);
    std::size_t observedTuples = 0;
    
    std::set<Value> values(initialRange.begin(), initialRange.end());
    
    auto testingRange = CartesianPowerRange(initialRange, power);
    auto testinRangeIter = testingRange.begin();
    auto prevValue = *testinRangeIter;
    ++testinRangeIter;
    ++observedTuples;
    
    BOOST_TEST(prevValue == std::vector<Value>(power, *initialRange.begin()), boost::test_tools::per_element());
    
    while(testinRangeIter != testingRange.end())
    {
        auto curValue = *testinRangeIter;
        BOOST_TEST(curValue.size() == power);
        for(Value val: prevValue)
        {
            BOOST_TEST(values.count(val) != 0);
        }
        BOOST_TEST(prevValue < curValue, boost::test_tools::lexicographic());
        
        prevValue = curValue;
        ++testinRangeIter;
        ++observedTuples;
    }
    
    BOOST_TEST(observedTuples == cartesianRangeSize);
}
