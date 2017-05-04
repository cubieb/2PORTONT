/**
 * C++ functions: asserttestexit.h
 *
 * Description: assert functions used for tests
 *
 * Author: Lorenzo Bettini <http://www.lorenzobettini.it>, (C) 2005-2008
 * Copyright: See COPYING file that comes with this distribution
 */


#ifndef _ASSERTTESTEXIT_H_
#define _ASSERTTESTEXIT_H_

#include <string>
#include <iostream>
#include <stdlib.h>
#include <exception>

#include <sstream>

static std::exception std_exception;

/**
 * returns the string representing a standard exception (which
 * can be different from system to system)
 */
const std::string stdExceptionToString() {
    return std_exception.what();
}

const std::string stdCausedBy() {
    return "Caused by: " + stdExceptionToString();
}

template <typename T>
void
assertEquals(T expected, T actual)
{
    if (expected != actual) {
        std::cerr << "assertEquals failed" << std::endl;
        std::cerr << "expected: " << expected << std::endl;
        std::cerr << "actual  : " << actual << std::endl;

        exit(EXIT_FAILURE);
    }
}

template <typename T2>
void
assertEqualsException(const std::string &expected, T2 actual)
{
    std::ostringstream o;

    o << actual;

    assertEquals(expected, o.str());
}

void
assertEquals(const std::string &expected, const std::string &actual)
{
    if (expected != actual) {
        std::cerr << "assertEquals failed" << std::endl;
        std::cerr << "expected: " << expected << std::endl;
        std::cerr << "actual  : " << actual << std::endl;

        exit(EXIT_FAILURE);
    }
}

void
assertTrue(bool actual, const std::string &expl = "") {
    if (!actual) {
        std::cerr << "assertion failed! " << expl << std::endl;
        exit(EXIT_FAILURE);
    }
}

void
assertFalse(bool actual, const std::string &expl = "") {
    if (actual) {
        std::cerr << "assertion failed! " << expl << std::endl;
        exit(EXIT_FAILURE);
    }
}

template <typename T1, typename T2>
void
assertEqualsCollections(const T1 &expected, const T2 &actual) {
    assertEquals(expected.size(), actual.size());

    typename T1::const_iterator it1 = expected.begin();
    typename T2::const_iterator it2 = actual.begin();

    for (; it1 != expected.end(); ++it1, ++it2)
        assertEquals(*it1, *it2);
}

#endif /*_ASSERTTESTEXIT_H_*/
