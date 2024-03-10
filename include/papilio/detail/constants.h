// Defines constant values as macros for interoperability between C and C++

#ifndef PAPILIO_DETAIL_CONSTANTS_H
#define PAPILIO_DETAIL_CONSTANTS_H

#pragma once

// clang-format off
// The root CMakeLists.txt will use following macros to determine library version.

#define PAPILIO_VERSION_MAJOR 0
#define PAPILIO_VERSION_MINOR 3
#define PAPILIO_VERSION_PATCH 0

// clang-format on

#define PAPILIO_ERR_NO_ERROR           0x0
#define PAPILIO_ERR_END_OF_STRING      0x1
#define PAPILIO_ERR_INVALID_FIELD_NAME 0x2
#define PAPILIO_ERR_INVALID_CONDITION  0x3
#define PAPILIO_ERR_INVALID_INDEX      0x4
#define PAPILIO_ERR_INVALID_ATTRIBUTE  0x5
#define PAPILIO_ERR_INVALID_OPERATOR   0x6
#define PAPILIO_ERR_INVALID_STRING     0x7
#define PAPILIO_ERR_UNCLOSED_BRACE     0x8

#define PAPILIO_ERR_UNKNOWN_ERROR      (-1)

#endif
