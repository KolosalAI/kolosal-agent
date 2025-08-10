/**
 * @file memory.h
 * @brief Core functionality for memory
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#ifndef VALUE_DETAIL_MEMORY_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define VALUE_DETAIL_MEMORY_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <set>

#include "yaml-cpp/dll.h"
#include "yaml-cpp/node/ptr.h"

namespace YAML {
namespace detail {
class node;
}  // namespace detail
}  // namespace YAML

namespace YAML {
namespace detail {
class YAML_CPP_API memory {
 public:
  memory() : m_nodes{} {}
  node& create_node();
  void merge(const memory& rhs);

 private:
  using Nodes = std::set<shared_node>;
  Nodes m_nodes;
};

class YAML_CPP_API memory_holder {
 public:
  memory_holder() : m_pMemory(new memory) {}

  node& create_node() { return m_pMemory->create_node(); }
  void merge(memory_holder& rhs);

 private:
  shared_memory m_pMemory;
};
}  // namespace detail
}  // namespace YAML

#endif  // VALUE_DETAIL_MEMORY_H_62B23520_7C8E_11DE_8A39_0800200C9A66
