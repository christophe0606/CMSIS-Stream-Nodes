#pragma once

#include "GenericNodes.hpp"
#include "cg_enums.h"
#include "posix/FileSource.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ostream>
#include <string>
#include <type_traits>

namespace cmsis_stream_nodes {

struct FileSinkParams {
    const char *path;
    FileNodeMode mode;
    std::size_t bytes_per_run;
    std::size_t lines_per_run;
};

namespace detail {

template <typename T>
inline void writeTextValue(std::ostream &output, const T &value)
{
    if constexpr (std::is_same<T, std::string>::value) {
        output << value;
    } else if constexpr (std::is_same<T, std::uint8_t>::value || std::is_same<T, std::int8_t>::value) {
        output << static_cast<int>(value);
    } else if constexpr (std::is_floating_point<T>::value) {
        output << static_cast<long double>(value);
    } else if constexpr (std::is_arithmetic<T>::value) {
        output << value;
    } else {
        output << static_cast<double>(value);
    }
}

} // namespace detail

template <typename IN, int inputSamples>
class FileSink final : public arm_cmsis_stream::GenericSink<IN, inputSamples> {
  public:
    template <typename Params>
    FileSink(arm_cmsis_stream::FIFOBase<IN> &src, const Params &params)
        : arm_cmsis_stream::GenericSink<IN, inputSamples>(src),
          path_(params.path != nullptr ? params.path : ""),
          mode_(detail::normalizeFileMode(params.mode)),
          bytesPerRun_(params.bytes_per_run),
          linesPerRun_(params.lines_per_run)
    {
        const std::ios::openmode openMode =
            mode_ == FileNodeMode::Binary ? std::ios::out | std::ios::binary : std::ios::out;
        output_.open(path_, openMode);
    }

    int run() final
    {
        if (!output_.is_open()) {
            return CG_BUFFER_ERROR;
        }

        const IN *input = this->getReadBuffer();

        if (mode_ == FileNodeMode::Binary) {
            return writeBinary(input);
        }
        return writeText(input);
    }

  private:
    int writeBinary(const IN *input)
    {
        if constexpr (std::is_same<IN, std::string>::value) {
            const std::size_t requested = bytesPerRun_ == 0 ? static_cast<std::size_t>(inputSamples) : bytesPerRun_;
            std::size_t written = 0;
            for (int i = 0; i < inputSamples && written < requested; ++i) {
                const std::size_t remaining = requested - written;
                const std::size_t count = input[i].size() < remaining ? input[i].size() : remaining;
                output_.write(input[i].data(), static_cast<std::streamsize>(count));
                written += count;
            }
        } else {
            const std::size_t capacity = sizeof(IN) * inputSamples;
            const std::size_t requested = bytesPerRun_ == 0 ? capacity : std::min(bytesPerRun_, capacity);
            output_.write(reinterpret_cast<const char *>(input), static_cast<std::streamsize>(requested));
        }

        return output_ ? CG_SUCCESS : CG_BUFFER_ERROR;
    }

    int writeText(const IN *input)
    {
        const std::size_t requested = linesPerRun_ == 0 ? inputSamples : std::min<std::size_t>(linesPerRun_, inputSamples);
        for (std::size_t i = 0; i < requested; ++i) {
            detail::writeTextValue(output_, input[i]);
            output_ << '\n';
        }

        return output_ ? CG_SUCCESS : CG_BUFFER_ERROR;
    }

    std::string path_;
    FileNodeMode mode_;
    std::size_t bytesPerRun_;
    std::size_t linesPerRun_;
    std::ofstream output_;
};

} // namespace cmsis_stream_nodes

using cmsis_stream_nodes::FileSink;
using cmsis_stream_nodes::FileSinkParams;
