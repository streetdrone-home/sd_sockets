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

#ifndef SD_SOCKETS__SD_SOCKETS_HPP_
#define SD_SOCKETS__SD_SOCKETS_HPP_

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

namespace sd_sockets
{
// This class manages socket timeouts by running the io_context using the timed
// io_context::run_for() member function. Each asynchronous operation is given
// a timeout within which it must complete. The socket operations themselves
// use lambdas as completion handlers. For a given socket operation, the client
// object runs the io_context to block thread execution until the operation
// completes or the timeout is reached. If the io_context::run_for() function
// times out, the socket is closed and the outstanding asynchronous operation
// is cancelled.
//
class Socket
{
public:
  std::string read(const std::chrono::steady_clock::duration & timeout = std::chrono::seconds(1))
  {
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

  void write(
    const std::string & msg,
    const std::chrono::steady_clock::duration & timeout = std::chrono::seconds(1))
  {
    auto len = htonl(msg.length());
    auto prefix = std::string{reinterpret_cast<const char *>(&len), 4};
    auto data = prefix + msg;
    auto error = std::error_code{};

    asio::async_write(
      socket_, asio::buffer(data), asio::transfer_exactly(data.length()),
      [&](const std::error_code & result_error, size_t /*result_length*/) {
        error = result_error;
      });

    run(timeout);

    if (error) throw std::system_error(error);
  }

  bool is_open() { return socket_.is_open(); }

  void close()
  {
    // Close the socket to cancel the outstanding asynchronous operation.
    socket_.close();

    // Run the io_context again until the operation completes.
    io_context_.run();
  }

protected:
  std::vector<std::byte> read_exactly(
    size_t n_bytes, const std::chrono::steady_clock::duration & timeout)
  {
    auto data = std::vector<std::byte>{};
    auto error = std::error_code{};
    auto length = size_t{};

    asio::async_read(
      socket_, asio::dynamic_buffer(data), asio::transfer_exactly(n_bytes),
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

  void run(const std::chrono::steady_clock::duration & timeout)
  {
    // Restart the io_context, as it may have been left in the "stopped" state
    // by a previous operation.
    io_context_.restart();

    // Block until the asynchronous operation has completed, or timed out. If
    // the pending asynchronous operation is a composed operation, the deadline
    // applies to the entire operation, rather than individual operations on
    // the socket.
    io_context_.run_for(timeout);

    // If the asynchronous operation completed successfully then the io_context
    // would have been stopped due to running out of work. If it was not
    // stopped, then the io_context::run_for call must have timed out.
    if (!io_context_.stopped()) {
      std::cerr << "Socket --> Timeout Reached" << std::endl;
      close();
    }
  }

  asio::io_context io_context_;
  asio::ip::tcp::socket socket_{io_context_};
};

class Client : public Socket
{
public:
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

    run(timeout);

    if (error) throw std::system_error(error);
  }
};

class Server : public Socket
{
public:
  explicit Server(int port)
  : acceptor_(io_context_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
  {
  }
  void accept() { acceptor_.accept(socket_); }

private:
  asio::ip::tcp::acceptor acceptor_;
};
}  // namespace sd_sockets

#endif  // SD_SOCKETS__SD_SOCKETS_HPP_
