// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright 2022 StreetDrone
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SD_SOCKETS_SOCKET_HPP
#define SD_SOCKETS_SOCKET_HPP

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "asio/buffer.hpp"
#include "asio/connect.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/read.hpp"
#include "asio/system_error.hpp"
#include "asio/write.hpp"

namespace sd_sockets {
    class Socket: public asio::ip::tcp::socket {
        public:
        Socket(asio::io_context *io_context): asio::ip::tcp::socket(*io_context), io_context_(io_context) {}

        std::string read(const std::chrono::steady_clock::duration & timeout = std::chrono::seconds(1)) {
            auto prefix_bytes = read_exactly(4, timeout);

            auto prefix = uint32_t{};
            std::copy(
            prefix_bytes.begin(), std::next(prefix_bytes.begin(), 4),
            reinterpret_cast<std::byte *>(&prefix));
            prefix = ntohl(prefix);

            auto msg_bytes = read_exactly(prefix, timeout);
            msg_bytes.emplace_back(std::byte{0});
            auto msg = std::string{reinterpret_cast<char *>(msg_bytes.data())};

            return msg;
        }

        void write(const std::string & msg, const std::chrono::steady_clock::duration & timeout = std::chrono::seconds(1)) {
            auto len = htonl(msg.length());
            auto prefix = std::string{reinterpret_cast<const char *>(&len), 4};
            auto data = prefix + msg;
            auto error = std::error_code{};

            asio::async_write(
            *this, asio::buffer(data), asio::transfer_exactly(data.length()),
            [&](const std::error_code & result_error, size_t /*result_length*/) {
                error = result_error;
            });

            run(timeout);

            if (error) throw std::system_error(error);
        }

        void run(const std::chrono::steady_clock::duration & timeout)
        {
            // Restart the io_context, as it may have been left in the "stopped" state
            // by a previous operation.
            io_context_->restart();

            // Block until the asynchronous operation has completed, or timed out. If
            // the pending asynchronous operation is a composed operation, the deadline
            // applies to the entire operation, rather than individual operations on
            // the socket.
            io_context_->run_for(timeout);

            // If the asynchronous operation completed successfully then the io_context
            // would have been stopped due to running out of work. If it was not
            // stopped, then the io_context::run_for call must have timed out.
            if (!io_context_->stopped()) {
                std::cerr << "Socket --> Timeout Reached" << std::endl;
                close();
            }
        }


        protected:
        std::vector<std::byte> read_exactly(size_t n_bytes, const std::chrono::steady_clock::duration & timeout) {
            auto data = std::vector<std::byte>{};
            auto error = std::error_code{};
            auto length = size_t{};

            asio::async_read(
            *this, asio::dynamic_buffer(data), asio::transfer_exactly(n_bytes),
            [&](const std::error_code & result_error, size_t result_length) {
                error = result_error;
                length = result_length;
            });

            run(timeout);

            if (length != n_bytes) {
            throw std::runtime_error("Failed to get " + std::to_string(n_bytes) + " bytes");
            }

            if (error) throw std::system_error(error);

            return data;
        }

        std::shared_ptr<asio::io_context> io_context_;
    };
}

#endif  // SD_SOCKETS_SOCKET_HPP