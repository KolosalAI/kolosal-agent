/**
 * @file directives.cpp
 * @brief Core functionality for directives
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "directives.h"

namespace YAML {
Directives::Directives() : version{true, 1, 2}, tags{} {}

std::string Directives::TranslateTagHandle(
    const std::string& handle) const {
  auto it = tags.find(handle);
  if (it == tags.end()) {
    if (handle == "!!")
      return "tag:yaml.org,2002:";
    return handle;
  }

  return it->second;
}
}  // namespace YAML
