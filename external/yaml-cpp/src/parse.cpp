/**
 * @file parse.cpp
 * @brief Core functionality for parse
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "yaml-cpp/node/parse.h"

#include <fstream>
#include <sstream>

#include "nodebuilder.h"
#include "yaml-cpp/node/impl.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/parser.h"

namespace YAML {
Node Load(const std::string& input) {
  std::stringstream stream(input);
  return Load(stream);
}

Node Load(const char* input) {
  std::stringstream stream(input);
  return Load(stream);
}

Node Load(std::istream& input) {
  Parser parser(input);
  NodeBuilder builder;
  if (!parser.HandleNextDocument(builder)) {
    return Node();
  }

  return builder.Root();
}

Node LoadFile(const std::string& filename) {
  std::ifstream fin(filename);
  if (!fin) {
    throw BadFile(filename);
  }
  return Load(fin);
}

std::vector<Node> LoadAll(const std::string& input) {
  std::stringstream stream(input);
  return LoadAll(stream);
}

std::vector<Node> LoadAll(const char* input) {
  std::stringstream stream(input);
  return LoadAll(stream);
}

std::vector<Node> LoadAll(std::istream& input) {
  std::vector<Node> docs;

  Parser parser(input);
  while (true) {
    NodeBuilder builder;
    if (!parser.HandleNextDocument(builder)) {
      break;
    }
    docs.push_back(builder.Root());
  }

  return docs;
}

std::vector<Node> LoadAllFromFile(const std::string& filename) {
  std::ifstream fin(filename);
  if (!fin) {
    throw BadFile(filename);
  }
  return LoadAll(fin);
}
}  // namespace YAML
