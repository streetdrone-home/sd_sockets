# sd_sockets

This is a header-only library that uses [Asio](https://think-async.com/Asio/) to provide classes for constructing easy-to-use TCP clients and servers.

## Communication Protocol

The client and server in this library communicate with each other using exact reading so that the recipient always knows exactly how many bytes to read from the socket.
This is done by prefixing the message with an unsigned 32-bit integer that describes the length of the message.
The prefix is in network byte order.

By reading the first 4 bytes and translating that number from network to host byte order, the recipient then has enough information to retrieve the rest of the message precisely.

## Classes

### __`Socket`__

This is the base class that both `Client` and `Server` inherit from.
It provides methods for reading and writing to the socket, and a few methods helpful in managing the socket.

### *Public Methods*

#### __`read(const std::chrono::steady_clock::duration & timeout = std::chrono::seconds(1))`__

Following the communication protocol, this method reads a message from the socket by first reading 4 bytes, converting this value into an unsigned integer and from network to host byte order, then reads that number more bytes from the socket.
The result is a byte array containing the message, which is then cast to a `std::string` and returned.

The method will fail if the timeout duration passes without a successful read.
If no timeout is provided, the method will block for 1 second.

#### __`write(const std::string & msg, const std::chrono::steady_clock::duration & timeout = std::chrono::seconds(1))`__

The inversion of `read()`, this method accepts a message string and gets the length.
The length is converted from host to network byte order before being prepended to the message and written to the socket.

As with the read method, this will fail if no socket completes reading the message before the timeout duration passes.
If no timeout is provided, the method will block for 1 second.

#### __`is_open()`__

A simple method that returns `true` if the socket is currently open and `false` otherwise.
This is a useful value to check before attempting to `read()` or `write()` using the socket, because doing so while the socket is closed will result in an error.

#### __`close()`__

Cleanly closes the socket by first closing the socket object and then running the `io_context` to make sure all asynchronous operations have finished.

### *Protected Methods*

#### __`read_exactly(size_t n_bytes, const std::chrono::steady_clock::duration & timeout)`__

This method assists with the precise reading that is executed in the `read()` method.
It takes the number of bytes and a timeout as parameters, then returns a `std::vector<std::byte>` containing the bytes read.
The timeout is required for the contained `run()` method (see below for description).

#### __`run(const std::chrono::steady_clock::duration & timeout)`__

This is the method that runs waiting asynchronous operations.

As the `io_context` stops when it runs out of 'work' to do, this method starts by restarting it.
It then runs the `io_context` for the `timeout` duration, which completes either when the `io_context` finishes (runs out of 'work'), or when the timeout is reached.

Once this has finished, the `io_context` is checked again to see if it has stopped running.
If it hasn't, this is because the timeout was reached.
It outputs to `stderr` stating that the timeout was reached and then closes the socket.

### __`Client`__

The `Client` inherits from the `Socket` class and contains only one public method.

#### __`connect(const std::string & host, int port, const std::chrono::steady_clock::duration & timeout = std::chrono::seconds(1))`__

This method perfroms an asynchronous connection call to the specified host and port.
The server must be running at the specified address before the connection is attempted.
If the timeout passes before the connection has been completed, the method fails.
If no timeout is provided, the method will block for 1 second.

### __`Server`__

The `Server` also inherits from the `Socket` class and contains two public methods.

#### __`Server(int port)`__

This constructor creates an acceptor object on the local host at the specified port.
The acceptor is responsible for handling socket connections with clients.

#### __`accept()`__

Accepts an incomming connection from a client.

## Requirements

- [Asio](https://think-async.com/Asio/) (non-Boost) >= 1.18
- CMake >= 3.12
- C++ 17

### __Installing Asio using a Package Manager__

Asio is a header-only library by default, so you could just dowload the files and put them in your include directory.

Alternatively, follow these commands to install Asio through your package manager (probably easiest).

### Ubuntu

```console
sudo apt install libasio-dev
```

### Fedora

```console
sudo dnf install asio-devel
```

### Arch
```
sudo pacman -S asio
```

To install Asio

## Download

Clone this repository:

```bash
git clone https://github.com/streetdrone-home/sd_sockets.git
```

## Install

Run these commands to install the interface library locally.

```bash
cd sd_sockets
mkdir build && cd build
cmake ..
cmake --build . --config Release --target install -- -j $(nproc)
```

Depending on your system's default install location, you may need to run the build command with `sudo`.

## Usage

To include `sd_sockets` in your C++ code, simply use:

```cpp
#include "sd_sockets/sd_sockets.hpp"
```

Then in the CMakeLists.txt of your project add the following:

```cmake
find_package(sd_sockets CONFIG REQUIRED)
```

Finally, following any `add_executable()` call to a target that includes the library, add:

```cmake
target_link_libraries(<target_name> sd_sockets::sd_sockets)
```

## Version Policy

This package follows [semantic versioning](https://semver.org/).
