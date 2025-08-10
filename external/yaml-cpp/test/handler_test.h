/**
 * @file handler_test.h
 * @brief Core functionality for handler test
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "mock_event_handler.h"
#include "yaml-cpp/yaml.h"  // IWYU pragma: keep

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::StrictMock;

namespace YAML {
class HandlerTest : public ::testing::Test {
 protected:
  void Parse(const std::string& example) {
    std::stringstream stream(example);
    Parser parser(stream);
    while (parser.HandleNextDocument(handler)) {
    }
  }

  void IgnoreParse(const std::string& example) {
    std::stringstream stream(example);
    Parser parser(stream);
    while (parser.HandleNextDocument(nice_handler)) {
    }
  }

  InSequence sequence;
  StrictMock<MockEventHandler> handler;
  NiceMock<MockEventHandler> nice_handler;
};
}
