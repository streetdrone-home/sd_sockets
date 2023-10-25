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

namespace sd_sockets
{
class Server
{
public:
  explicit Server(int port)
  : sockets(), acceptor_(io_context_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)), mutex_()
  {
  }

  std::string read(
    const std::string & socketId,
    const std::chrono::steady_clock::duration & timeout = std::chrono::seconds(1))
  {
    auto prefix_bytes = read_exactly(socketId, 4, timeout);

    auto prefix = uint32_t{};
    std::copy(
      prefix_bytes.begin(), std::next(prefix_bytes.begin(), 4),
      reinterpret_cast<std::byte *>(&prefix));
    prefix = ntohl(prefix);

    auto msg_bytes = read_exactly(socketId, prefix, timeout);
    msg_bytes.emplace_back(std::byte{0});
    auto msg = std::string(reinterpret_cast<char *>(msg_bytes.data()));

    return std::move(msg);
  }

  void write(
    const std::string & socketId, const std::string & msg,
    const std::chrono::steady_clock::duration & timeout = std::chrono::seconds(1))
  {
    auto len = htonl(msg.length());
    auto prefix = std::string{reinterpret_cast<const char *>(&len), 4};
    auto data = prefix + msg;
    auto error = std::error_code{};

    auto socket = getSocket(socketId);

    asio::async_write(
      *socket, asio::buffer(data), asio::transfer_exactly(data.length()),
      [&](const std::error_code & result_error, size_t /*result_length*/) {
        error = result_error;
      });

    run(timeout);

    if (error) throw std::system_error(error);
  }

  void close()
  {
    // Close the socket to cancel the outstanding asynchronous operation.
    for (auto item : sockets) {
      destroySocket(item.first);
    }

    // Run the io_context again until the operation completes.
    io_context_.run();
  }

  void destroySocket(const std::string & socketId)
  {
    getSocket(socketId)->close();
    sockets.erase(socketId);
  }

  void lock() { mutex_.lock(); }

  void unlock() { mutex_.unlock(); }

  void accept(const std::function<void(Server *, const std::string &)> handler)
  {
    std::shared_ptr<asio::ip::tcp::socket> socket =
      std::make_shared<asio::ip::tcp::socket>(io_context_);
    acceptor_.accept(*socket);

    std::string socketId = generateUuid();
    sockets[socketId] = std::move(socket);
    int index = sockets.size() - 1;

    std::thread(handler, this, std::move(socketId)).detach();
  }

  std::shared_ptr<asio::ip::tcp::socket> getSocket(const std::string & socketId)
  {
    auto entry = sockets.find(socketId);

    if (entry == sockets.end()) return nullptr;

    return entry->second;
  }

private:
  template <typename... Args>
  std::string format(std::string str, Args... args)
  {
    const char * std_ = str.c_str();
    char msg_[1024];
    sprintf(msg_, std_, args...);
    std::string msg = std::string(msg_);
    return msg;
  }

  std::string generateUuid()
  {
    std::string strUuid = format(
      "%x-%x-%x-%x-%x5678f", rand(), ((rand() & 0x0fff) | 0x4000), ((rand() & 0x0fff) | 0x4000),
      rand() % 0x3fff + 0x8000, rand(), rand());

    return strUuid;
  }

  std::vector<std::byte> read_exactly(
    std::string socketId, size_t n_bytes, const std::chrono::steady_clock::duration & timeout)
  {
    auto data = std::vector<std::byte>{};
    auto error = std::error_code{};
    auto length = size_t{};

    auto socket = getSocket(socketId);

    asio::async_read(
      *socket, asio::dynamic_buffer(data), asio::transfer_exactly(n_bytes),
      [&](const std::error_code & result_error, size_t result_length) {
        error = result_error;
        length = result_length;
      });

    run(timeout);

    if (length != n_bytes) {
      throw std::runtime_error("Failed to get " + std::to_string(n_bytes) + " bytes");
    }

    if (error) throw std::system_error(error);

    return std::move(data);
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
  std::map<std::string, std::shared_ptr<asio::ip::tcp::socket>> sockets;
  asio::ip::tcp::acceptor acceptor_;

  std::mutex mutex_;
};

}  // namespace sd_sockets

#endif  // SD_SOCKETS_SERVER_HPP