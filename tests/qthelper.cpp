#ifndef QT_HELPER_CPP
#define QT_HELPER_CPP

/**
 * This file contains functions to print Qt Object in googletest
 */

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <QString>
#include <ostream>
#include <sstream>

QT_BEGIN_NAMESPACE
inline void PrintTo(const QString& qString, ::std::ostream* os) {
  *os << qUtf8Printable(qString);
}
QT_END_NAMESPACE

#endif
