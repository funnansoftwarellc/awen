#pragma once

#include <gtest/gtest.h>

/// @brief Defines a unit test case.
/// @details Wraps the gtest TEST macro and prefixes the test name with UNIT_,
/// so the test appears as suite.UNIT_name in the gtest output.
/// Use --gtest_filter=*.UNIT_* to run only unit tests.
/// @param suite The test suite name.
/// @param name  The test case name (will be prefixed with UNIT_).
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNIT_TEST(suite, name) TEST(UNIT_##suite, name)

/// @brief Defines a graphics test case.
/// @details Wraps the gtest TEST macro and prefixes the test name with GRAPHICS_,
/// so the test appears as suite.GRAPHICS_name in the gtest output.
/// Use --gtest_filter=*.GRAPHICS_* to run only graphics tests.
/// @param suite The test suite name.
/// @param name  The test case name (will be prefixed with GRAPHICS_).
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define GRAPHICS_TEST(suite, name) TEST(GRAPHICS_##suite, name)
