/**
 * @file regex_yaml.cpp
 * @brief Core functionality for regex yaml
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "regex_yaml.h"

namespace YAML {
// constructors

RegEx::RegEx(REGEX_OP op) : m_op(op), m_a(0), m_z(0), m_params{} {}
RegEx::RegEx() : RegEx(REGEX_EMPTY) {}

RegEx::RegEx(char ch) : m_op(REGEX_MATCH), m_a(ch), m_z(0), m_params{} {}

RegEx::RegEx(char a, char z) : m_op(REGEX_RANGE), m_a(a), m_z(z), m_params{} {}

RegEx::RegEx(const std::string& str, REGEX_OP op)
    : m_op(op), m_a(0), m_z(0), m_params(str.begin(), str.end()) {}

// combination constructors
RegEx operator!(const RegEx& ex) {
  RegEx ret(REGEX_NOT);
  ret.m_params.push_back(ex);
  return ret;
}

RegEx operator|(const RegEx& ex1, const RegEx& ex2) {
  RegEx ret(REGEX_OR);
  ret.m_params.push_back(ex1);
  ret.m_params.push_back(ex2);
  return ret;
}

RegEx operator&(const RegEx& ex1, const RegEx& ex2) {
  RegEx ret(REGEX_AND);
  ret.m_params.push_back(ex1);
  ret.m_params.push_back(ex2);
  return ret;
}

RegEx operator+(const RegEx& ex1, const RegEx& ex2) {
  RegEx ret(REGEX_SEQ);
  ret.m_params.push_back(ex1);
  ret.m_params.push_back(ex2);
  return ret;
}
}  // namespace YAML
