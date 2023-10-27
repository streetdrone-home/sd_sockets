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

#ifndef SD_SOCKETS_SERVER_HPP
#define SD_SOCKETS_SERVER_HPP

#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#include "asio/buffer.hpp"
#include "asio/connect.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/read.hpp"
#include "asio/system_error.hpp"
#include "asio/write.hpp"

#include "socket.hpp"
#include "sd_sockets/server_session.hpp"

namespace sd_sockets
{
class Server
{
public:
  explicit Server(int port)
  :acceptor_(*io_context_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
  {
  }

  void close()
  {
    // Run the io_context again until the operation completes.
    io_context_->run();
  }

  void accept(const std::function<void(Server *, const std::string &)> handler)
  {
    sd_sockets::Socket socket(io_context_.get());
    acceptor_.accept(socket);
    
    std::make_shared<ServerSession>(std::move(socket), handler)->start();
  }

private:

  std::shared_ptr<asio::io_context> io_context_;
  asio::ip::tcp::acceptor acceptor_;
};

}  // namespace sd_sockets

#endif  // SD_SOCKETS_SERVER_HPP