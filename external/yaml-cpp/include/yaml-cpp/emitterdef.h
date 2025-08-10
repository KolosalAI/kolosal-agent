/**
 * @file emitterdef.h
 * @brief Core functionality for emitterdef
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#ifndef EMITTERDEF_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define EMITTERDEF_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

namespace YAML {
struct EmitterNodeType {
  enum value { NoType, Property, Scalar, FlowSeq, BlockSeq, FlowMap, BlockMap };
};
}

#endif  // EMITTERDEF_H_62B23520_7C8E_11DE_8A39_0800200C9A66
