#include <cstdlib>
#include <iostream>

#include "Server.hpp"
#include "ircdefine.hpp"

int main(const int argc, const char **argv) {
  if (argc != IRC_ARG_COUNT) {
    std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
    return 1;
  }

  try {
    char *endptr;
    const size_t port = std::strtoll(argv[1], &endptr, 10);
    if (endptr == argv[1] || port < MIN_PORT || port > MAX_PORT)
      throw std::runtime_error("Invalid port number: Port must be from 1024 to 65535");

    const std::string password = std::string(argv[2]);

    Server server(static_cast<uint16_t>(port), password);
    server.start();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
