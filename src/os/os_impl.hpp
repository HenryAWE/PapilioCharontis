#ifndef PAPILIO_SRC_OS_OS_IMPL_HPP
#define PAPILIO_SRC_OS_OS_IMPL_HPP

#pragma once

#include <papilio/macros.hpp>
#include <papilio/os/os.hpp>
#include <papilio/detail/prefix.hpp>

#ifdef PAPILIO_PLATFORM_WINDOWS
#    define PAPILIO_OS_IMPL_WINAPI 1
#else
#    define PAPILIO_OS_IMPL_POSIX 1
#endif

#include <papilio/detail/suffix.hpp>

#endif
