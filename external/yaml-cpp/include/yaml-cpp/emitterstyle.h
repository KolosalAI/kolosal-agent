/**
 * @file emitterstyle.h
 * @brief Core functionality for emitterstyle
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#ifndef EMITTERSTYLE_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define EMITTERSTYLE_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

namespace YAML {
struct EmitterStyle {
  enum value { Default, Block, Flow };
};
}

#endif  // EMITTERSTYLE_H_62B23520_7C8E_11DE_8A39_0800200C9A66
