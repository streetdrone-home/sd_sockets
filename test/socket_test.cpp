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

#include <thread>

#include "gtest/gtest.h"
#include "sd_sockets/sd_sockets.hpp"

class SocketPairTest : public ::testing::Test
{
protected:
  sd_sockets::Client client_{};
  sd_sockets::Server server_{15243};

  void connect_sockets()
  {
    std::thread t1([&]() { server_.accept(); });
    client_.connect("127.0.0.1", 15243);
    t1.join();
  }
};

TEST_F(SocketPairTest, check_is_open)
{
  EXPECT_FALSE(client_.is_open());
  EXPECT_FALSE(server_.is_open());

  connect_sockets();

  EXPECT_TRUE(client_.is_open());
  EXPECT_TRUE(server_.is_open());
}

TEST_F(SocketPairTest, server_to_client)
{
  auto input = "Hello, World!";

  connect_sockets();

  ASSERT_TRUE(client_.is_open());
  ASSERT_TRUE(server_.is_open());

  std::thread t2([&]() { server_.write(input); });
  auto output = client_.read();
  t2.join();

  EXPECT_EQ(input, output);
}

TEST_F(SocketPairTest, client_to_server)
{
  auto input = "Hello, World!";

  connect_sockets();

  ASSERT_TRUE(client_.is_open());
  ASSERT_TRUE(server_.is_open());

  std::thread t2([&]() { client_.write(input); });
  auto output = server_.read();
  t2.join();

  EXPECT_EQ(input, output);
}
