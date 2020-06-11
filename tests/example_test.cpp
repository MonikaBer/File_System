#include <vector>
#include <boost/test/unit_test.hpp>
#include "simplefs.h"

BOOST_AUTO_TEST_SUITE(TestOfTest)

    BOOST_AUTO_TEST_CASE(positive)
    {
        BOOST_CHECK_EQUAL(1, 3-2);
    }

    BOOST_AUTO_TEST_CASE(negative)
    {
        BOOST_CHECK_EQUAL(1, 2);
    }

    BOOST_AUTO_TEST_CASE(CreateDuplicateFile)
    {
        BOOST_CHECK_EQUAL(create("/test", 1), 0);
        BOOST_CHECK_EQUAL(create("/test", 1), -1);
    }

BOOST_AUTO_TEST_SUITE_END()
