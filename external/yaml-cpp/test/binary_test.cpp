/**
 * @file binary_test.cpp
 * @brief Core functionality for binary test
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "gtest/gtest.h"
#include <yaml-cpp/binary.h>

TEST(BinaryTest, DecodingSimple) {
  std::string input{90, 71, 86, 104, 90, 71, 74, 108, 90, 87, 89, 61};
  const std::vector<unsigned char> &result = YAML::DecodeBase64(input);
  EXPECT_EQ(std::string(result.begin(), result.end()), "deadbeef");
}

TEST(BinaryTest, DecodingNoCrashOnNegative) {
  std::string input{-58, -1, -99, 109};
  const std::vector<unsigned char> &result = YAML::DecodeBase64(input);
  EXPECT_TRUE(result.empty());
}
