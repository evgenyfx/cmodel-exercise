#pragma once

#include <cstddef>
#include <cstdint>

// Shared host/model communication API. See ../REGISTER_MAP.md for the contract.
// These are logical offsets; each slot carries a native uintptr_t value.
constexpr unsigned kRegisterSpacing = 0x08;
constexpr std::size_t kRegisterCount = 6;

enum class Register : unsigned {
    InputAddress  = 0x00, // Host writes: address of uint8_t input pixels.
    OutputAddress = 0x08, // Host writes: address of uint8_t output pixels.
    ParamsAddress = 0x10, // Host writes: address of ImageParams (median_filter.h).
    Radius        = 0x18, // Host writes: median radius, 0 through 8.
    Command       = 0x20, // Host writes: Start signals the request event.
    Status        = 0x28  // Model writes; host reads after completion.
};

enum class Command : std::uintptr_t { Start = 1 };
enum class Status : std::uintptr_t { Idle = 0, Done = 1, InvalidArgument = 2 };
