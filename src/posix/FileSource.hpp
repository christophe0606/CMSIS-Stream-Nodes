#pragma once

#include "GenericNodes.hpp"
#include "cg_enums.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <type_traits>

namespace cmsis_stream_nodes {

enum class FileNodeMode : std::uint8_t {
    Binary = 0,
    Text = 1,
};

struct FileSourceParams {
    const char *path;
    FileNodeMode mode;
    std::size_t bytes_per_run;
    std::size_t lines_per_run;
};

namespace detail {

inline FileNodeMode normalizeFileMode(FileNodeMode mode)
{
    return mode;
}

template <typename Mode>
inline FileNodeMode normalizeFileMode(Mode mode)
{
    return static_cast<int>(mode) == 0 ? FileNodeMode::Binary : FileNodeMode::Text;
}

template <typename T>
inline bool parseTextValue(const std::string &line, T &value)
{
    if constexpr (std::is_same<T, std::string>::value) {
        value = line;
        return true;
    } else if constexpr (std::is_integral<T>::value && std::is_unsigned<T>::value) {
        std::size_t parsed = 0;
        unsigned long long converted = std::stoull(line, &parsed, 0);
        if (parsed == 0) {
            return false;
        }
        value = static_cast<T>(converted);
        return true;
    } else if constexpr (std::is_integral<T>::value && std::is_signed<T>::value) {
        std::size_t parsed = 0;
        long long converted = std::stoll(line, &parsed, 0);
        if (parsed == 0) {
            return false;
        }
        value = static_cast<T>(converted);
        return true;
    } else {
        std::size_t parsed = 0;
        long double converted = std::stold(line, &parsed);
        if (parsed == 0) {
            return false;
        }
        value = static_cast<T>(converted);
        return true;
    }
}

} // namespace detail

template <typename OUT, int outputSamples>
class FileSource final : public arm_cmsis_stream::GenericSource<OUT, outputSamples> {
  public:
    template <typename Params>
    FileSource(arm_cmsis_stream::FIFOBase<OUT> &dst, const Params &params)
        : arm_cmsis_stream::GenericSource<OUT, outputSamples>(dst),
          path_(params.path != nullptr ? params.path : ""),
          mode_(detail::normalizeFileMode(params.mode)),
          bytesPerRun_(params.bytes_per_run),
          linesPerRun_(params.lines_per_run)
    {
        const std::ios::openmode openMode =
            mode_ == FileNodeMode::Binary ? std::ios::in | std::ios::binary : std::ios::in;
        input_.open(path_, openMode);
    }

    int run() final
    {
        if (!input_.is_open()) {
            return CG_BUFFER_ERROR;
        }

        OUT *output = this->getWriteBuffer();
        std::fill(output, output + outputSamples, OUT{});

        if (mode_ == FileNodeMode::Binary) {
            return readBinary(output);
        }
        return readText(output);
    }

  private:
    int readBinary(OUT *output)
    {
        if constexpr (std::is_same<OUT, std::string>::value) {
            (void)output;
            return CG_BUFFER_ERROR;
        } else {
            const std::size_t capacity = sizeof(OUT) * outputSamples;
            const std::size_t requested = bytesPerRun_ == 0 ? capacity : std::min(bytesPerRun_, capacity);
            input_.read(reinterpret_cast<char *>(output), static_cast<std::streamsize>(requested));
            const std::streamsize read = input_.gcount();

            if (read <= 0 && input_.eof()) {
                return CG_STOP_SCHEDULER;
            }
            return CG_SUCCESS;
        }
    }

    int readText(OUT *output)
    {
        const std::size_t requested = linesPerRun_ == 0 ? outputSamples : std::min<std::size_t>(linesPerRun_, outputSamples);
        std::string line;
        std::size_t produced = 0;

        for (; produced < requested; ++produced) {
            if (!std::getline(input_, line)) {
                break;
            }

            try {
                if (!detail::parseTextValue(line, output[produced])) {
                    return CG_BUFFER_ERROR;
                }
            } catch (...) {
                return CG_BUFFER_ERROR;
            }
        }

        if (produced == 0 && input_.eof()) {
            return CG_STOP_SCHEDULER;
        }
        return CG_SUCCESS;
    }

    std::string path_;
    FileNodeMode mode_;
    std::size_t bytesPerRun_;
    std::size_t linesPerRun_;
    std::ifstream input_;
};

} // namespace cmsis_stream_nodes

using cmsis_stream_nodes::FileNodeMode;
using cmsis_stream_nodes::FileSource;
using cmsis_stream_nodes::FileSourceParams;
