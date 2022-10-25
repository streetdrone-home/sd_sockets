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

#include <gtest/gtest.h>

#include "sd_sockets/sd_sockets.hpp"

TEST(is_open_test, check_closed_on_init)
{
  auto socket = sd_sockets::Socket{};
  auto client = sd_sockets::Client{};
  auto server = sd_sockets::Server{15243};

  EXPECT_FALSE(socket.is_open());
  EXPECT_FALSE(client.is_open());
  EXPECT_FALSE(server.is_open());
}
