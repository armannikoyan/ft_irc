#include <csignal>
#include <cstdlib>
#include <iostream>

#include "Server.hpp"
#include "ircdefine.hpp"

volatile sig_atomic_t g_server_running = 1;

void signalHandler(const int signum __attribute__((__unused__))) {
  std::cout << "\nInterrupt signal received. Shutting down server gracefully..." << std::endl;
  g_server_running = 0;
}

int main(const int argc, const char **argv) {
  if (argc != IRC_ARG_COUNT) {
    std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
    return 1;
  }
  struct sigaction sa;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = signalHandler;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGQUIT, &sa, NULL);

  try {
    char *endptr;
    const long port = std::strtol(argv[1], &endptr, 10);
    if (endptr == argv[1] || *endptr != '\0' || port < MIN_PORT || port > MAX_PORT)
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
