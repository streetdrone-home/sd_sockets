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

#ifndef SD_SOCKETS_SERVER_SESSION_HPP
#define SD_SOCKETS_SERVER_SESSION_HPP

#include "sd_sockets/socket.hpp"

#include <thread>
#include <functional>

namespace sd_sockets {
    class ServerSession {
        public:
        ServerSession(sd_sockets::Socket socket, const std::function<void(sd_sockets::Socket)> handler): socket_(std::move(socket)), handler_(handler) {}


        void start() {
            std::thread(handler_, std::move(socket_)).detach();
        }

        private:
        const std::function<void(sd_sockets::Socket)> handler_;
        sd_sockets::Socket socket_;
    };
}

#endif  // SD_SOCKETS_SERVER_SESSION_HPP