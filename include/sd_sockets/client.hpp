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

#ifndef SD_SOCKETS_CLIENT_HPP
#define SD_SOCKETS_CLIENT_HPP

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

#include "socket.hpp"

namespace sd_sockets
{
class Client
{
public:
  std::string read(const std::chrono::steady_clock::duration & timeout = std::chrono::seconds(1))
  {
    std::string msg = socket_.read();

    socket_.run(timeout);

    return msg;
  }

  void write(
    const std::string & msg,
    const std::chrono::steady_clock::duration & timeout = std::chrono::seconds(1))
  {
    socket_.write(msg);

    socket_.run(timeout);
  }

  bool is_open() { return socket_.is_open(); }

  void close()
  {
    // Close the socket to cancel the outstanding asynchronous operation.
    socket_.close();

    // Run the io_context again until the operation completes.
    io_context_.run();
  }

  void connect(
    const std::string & host, int port,
    const std::chrono::steady_clock::duration & timeout = std::chrono::seconds(1))
  {
    auto endpoints = asio::ip::tcp::resolver(io_context_).resolve(host, std::to_string(port));
    auto error = std::error_code{};

    asio::async_connect(
      socket_, endpoints,
      [&](
        const std::error_code & result_error, const asio::ip::tcp::endpoint & /*result_endpoint*/) {
        error = result_error;
      });

    socket_.run(timeout);

    if (error) throw std::system_error(error);
  }

private:
  asio::io_context io_context_;
  sd_sockets::Socket socket_{&io_context_};
};

}  // namespace sd_sockets

#endif  // SD_SOCKETS_CLIENT_HPP