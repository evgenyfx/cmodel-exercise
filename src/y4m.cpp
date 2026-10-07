#include "y4m.h"
#include "median_filter.h"

#include <charconv>
#include <fstream>
#include <sstream>
#include <utility>

namespace {
bool parse_dimension(const std::string& token, int& value) {
    const char* begin = token.data() + 1;
    const char* end = token.data() + token.size();
    const auto parsed = std::from_chars(begin, end, value);
    return parsed.ec == std::errc{} && parsed.ptr == end &&
           value >= 1 && value <= MEDIAN_MAX_DIMENSION;
}

bool valid_dimensions(const MonoImage& image) {
    return image.width >= 1 && image.width <= MEDIAN_MAX_DIMENSION &&
           image.height >= 1 && image.height <= MEDIAN_MAX_DIMENSION;
}
} // namespace

bool read_y4m(const std::string& path, MonoImage& image, std::string& error) {
    error.clear();
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        error = "Cannot open input: " + path;
        return false;
    }

    std::string line;
    std::getline(file, line);
    std::istringstream header(line); // Whitespace parsing also accepts CRLF.
    std::string token;
    header >> token;
    if (token != "YUV4MPEG2") {
        error = "Input is not a YUV4MPEG2 file";
        return false;
    }

    MonoImage frame;
    bool mono = false;
    while (header >> token) {
        if (token[0] == 'W' || token[0] == 'H') {
            int& dimension = token[0] == 'W' ? frame.width : frame.height;
            if (!parse_dimension(token, dimension)) {
                error = "Y4M width and height must be between 1 and 4096";
                return false;
            }
        } else if (token[0] == 'C') {
            if (token != "Cmono") {
                error = "Only 8-bit Cmono Y4M input is supported";
                return false;
            }
            mono = true;
        }
        // Frame rate, aspect ratio, and other tags are not used by this task.
    }
    if (!valid_dimensions(frame)) {
        error = "Y4M header is missing width or height";
        return false;
    }
    if (!mono) {
        error = "Y4M header must explicitly specify Cmono";
        return false;
    }

    if (!std::getline(file, line)) {
        error = "Y4M input is missing its first FRAME header";
        return false;
    }
    std::istringstream frame_header(line);
    if (!(frame_header >> token) || token != "FRAME") {
        error = "Expected a Y4M FRAME header";
        return false;
    }
    // Optional parameters after FRAME are ignored.
    frame.pixels.resize(static_cast<std::size_t>(frame.width) * frame.height);
    file.read(reinterpret_cast<char*>(frame.pixels.data()),
              static_cast<std::streamsize>(frame.pixels.size()));
    if (file.gcount() != static_cast<std::streamsize>(frame.pixels.size())) {
        error = "Y4M first frame is truncated";
        return false;
    }

    image = std::move(frame);
    return true;
}

bool write_y4m(const std::string& path, const std::uint8_t* pixels,
               int width, int height, int stride, std::string& error) {
    error.clear();
    if (!pixels || width < 1 || width > MEDIAN_MAX_DIMENSION ||
        height < 1 || height > MEDIAN_MAX_DIMENSION || stride < width) {
        error = "Y4M output requires pixels, dimensions between 1 and 4096, and stride >= width";
        return false;
    }

    std::ofstream file(path, std::ios::binary);
    if (!file) {
        error = "Cannot open output: " + path;
        return false;
    }

    file << "YUV4MPEG2 W" << width << " H" << height
         << " F1:1 Ip A0:0 Cmono\nFRAME\n";
    for (int y = 0; y < height; ++y) {
        const auto* row = pixels + static_cast<std::size_t>(y) * stride;
        file.write(reinterpret_cast<const char*>(row), width);
    }
    file.close();
    if (!file) {
        error = "Cannot write complete Y4M output: " + path;
        return false;
    }
    return true;
}

