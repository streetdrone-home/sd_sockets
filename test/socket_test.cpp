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

TEST(is_open_test, check_before_and_after_connect)
{
  auto socket = sd_sockets::Socket{};
  auto client = sd_sockets::Client{};
  auto server = sd_sockets::Server{15243};

  EXPECT_FALSE(socket.is_open());
  EXPECT_FALSE(client.is_open());
  EXPECT_FALSE(server.is_open());

  std::thread t1([&]() { server.accept(); });
  client.connect("127.0.0.1", 15243);
  t1.join();

  EXPECT_FALSE(socket.is_open());
  EXPECT_TRUE(client.is_open());
  EXPECT_TRUE(server.is_open());
}

TEST(read_write_test, server_to_client)
{
  auto input = "Hello, World!";
  auto client = sd_sockets::Client{};
  auto server = sd_sockets::Server{15243};

  std::thread t1([&]() { server.accept(); });
  client.connect("127.0.0.1", 15243);
  t1.join();

  ASSERT_TRUE(client.is_open());
  ASSERT_TRUE(server.is_open());

  std::thread t2([&]() { server.write(input); });
  auto output = client.read();
  t2.join();

  EXPECT_EQ(input, output) << "Expected " << input << ", got " << output;
}

TEST(read_write_test, client_to_server)
{
  auto input = "Hello, World!";
  auto client = sd_sockets::Client{};
  auto server = sd_sockets::Server{15243};

  std::thread t1([&]() { server.accept(); });
  client.connect("127.0.0.1", 15243);
  t1.join();

  ASSERT_TRUE(client.is_open());
  ASSERT_TRUE(server.is_open());

  std::thread t2([&]() { client.write(input); });
  auto output = server.read();
  t2.join();

  EXPECT_EQ(input, output) << "Expected " << input << ", got " << output;
}
