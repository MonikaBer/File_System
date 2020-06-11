#include <vector>
#include <boost/test/unit_test.hpp>
#include "simplefs.h"

BOOST_AUTO_TEST_SUITE(CreateFunc)

    BOOST_AUTO_TEST_CASE(CreateDuplicateFile)
    {
        if(create("/test", 1) == 0)
            BOOST_CHECK_EQUAL(create("/test", 1), -1);
    }

BOOST_AUTO_TEST_SUITE_END()
