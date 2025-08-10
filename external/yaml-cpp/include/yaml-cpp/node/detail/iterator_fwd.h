/**
 * @file iterator_fwd.h
 * @brief Core functionality for iterator fwd
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#ifndef VALUE_DETAIL_ITERATOR_FWD_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define VALUE_DETAIL_ITERATOR_FWD_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include "yaml-cpp/dll.h"
#include <list>
#include <utility>
#include <vector>

namespace YAML {

namespace detail {
struct iterator_value;
template <typename V>
class iterator_base;
}

using iterator = detail::iterator_base<detail::iterator_value>;
using const_iterator = detail::iterator_base<const detail::iterator_value>;
}

#endif  // VALUE_DETAIL_ITERATOR_FWD_H_62B23520_7C8E_11DE_8A39_0800200C9A66
