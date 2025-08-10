/**
 * @file singledocparser.h
 * @brief Core functionality for singledocparser
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#ifndef SINGLEDOCPARSER_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define SINGLEDOCPARSER_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <map>
#include <memory>
#include <string>

#include "yaml-cpp/anchor.h"

namespace YAML {
class CollectionStack;
template <int> class DepthGuard; // depthguard.h
class EventHandler;
class Node;
class Scanner;
struct Directives;
struct Mark;
struct Token;

class SingleDocParser {
 public:
  SingleDocParser(Scanner& scanner, const Directives& directives);
  SingleDocParser(const SingleDocParser&) = delete;
  SingleDocParser(SingleDocParser&&) = delete;
  SingleDocParser& operator=(const SingleDocParser&) = delete;
  SingleDocParser& operator=(SingleDocParser&&) = delete;
  ~SingleDocParser();

  void HandleDocument(EventHandler& eventHandler);

 private:
  void HandleNode(EventHandler& eventHandler);

  void HandleSequence(EventHandler& eventHandler);
  void HandleBlockSequence(EventHandler& eventHandler);
  void HandleFlowSequence(EventHandler& eventHandler);

  void HandleMap(EventHandler& eventHandler);
  void HandleBlockMap(EventHandler& eventHandler);
  void HandleFlowMap(EventHandler& eventHandler);
  void HandleCompactMap(EventHandler& eventHandler);
  void HandleCompactMapWithNoKey(EventHandler& eventHandler);

  void ParseProperties(std::string& tag, anchor_t& anchor,
                       std::string& anchor_name);
  void ParseTag(std::string& tag);
  void ParseAnchor(anchor_t& anchor, std::string& anchor_name);

  anchor_t RegisterAnchor(const std::string& name);
  anchor_t LookupAnchor(const Mark& mark, const std::string& name) const;

 private:
  int depth = 0;
  Scanner& m_scanner;
  const Directives& m_directives;
  std::unique_ptr<CollectionStack> m_pCollectionStack;

  using Anchors = std::map<std::string, anchor_t>;
  Anchors m_anchors;

  anchor_t m_curAnchor;
};
}  // namespace YAML

#endif  // SINGLEDOCPARSER_H_62B23520_7C8E_11DE_8A39_0800200C9A66
