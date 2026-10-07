#include "median_device.h"
#include "median_filter.h"

MedianDevice::MedianDevice() : worker_(&MedianDevice::run, this) {}

MedianDevice::~MedianDevice() {
    stopping_.store(true);
    request_.signal();
    worker_.join();
}

void MedianDevice::write_register(Register reg, std::uintptr_t value) {
    registers_[static_cast<unsigned>(reg) / kRegisterSpacing] = value;
    if (reg == Register::Command) {
        request_.signal();
    }
}

std::uintptr_t MedianDevice::read_register(Register reg) const {
    return registers_[static_cast<unsigned>(reg) / kRegisterSpacing];
}

void MedianDevice::wait_interrupt() {
    completion_.wait();
}

void MedianDevice::run() {
    for (;;) {
        request_.wait();
        if (stopping_.load()) return;

        const auto* input = reinterpret_cast<const std::uint8_t*>(
            read_register(Register::InputAddress));
        auto* output = reinterpret_cast<std::uint8_t*>(
            read_register(Register::OutputAddress));
        const auto* descriptor = reinterpret_cast<const ImageParams*>(
            read_register(Register::ParamsAddress));
        const auto radius = read_register(Register::Radius);
        Status status = Status::InvalidArgument;

        // Valid allocated buffers and their lifetime are the host's responsibility.
        if (read_register(Register::Command) == static_cast<std::uintptr_t>(Command::Start) &&
            input && output && descriptor && input != output && radius <= MEDIAN_MAX_RADIUS) {
            const ImageParams params = *descriptor;
            if (params.width > 0 && params.height > 0 &&
                params.width <= MEDIAN_MAX_DIMENSION && params.height <= MEDIAN_MAX_DIMENSION &&
                params.input_stride >= params.width && params.output_stride >= params.width) {
                median_histogram(input, output, &params, static_cast<int>(radius));
                status = Status::Done;
            }
        }

        // Completion also publishes output writes to the waiting host.
        registers_[static_cast<unsigned>(Register::Status) / kRegisterSpacing] =
            static_cast<std::uintptr_t>(status);
        completion_.signal();
    }
}
