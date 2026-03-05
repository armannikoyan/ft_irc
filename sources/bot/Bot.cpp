#include <arpa/inet.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

volatile sig_atomic_t g_keepRunning = 1;
int g_sock = -1;

void handleSigint(const int signum) {
  (void) signum;
  g_keepRunning = 0;

  if (g_sock != -1) {
    close(g_sock);
    g_sock = -1;
  }

  const char msg[] = "\nInterrupt signal received. Shutting down bot...\n";
  write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}

void sendCommand(int sock, const std::string &cmd) {
  if (sock == -1)
    return;
  const std::string fullCmd = cmd + "\r\n";
  send(sock, fullCmd.c_str(), fullCmd.length(), 0);
  std::cout << ">> " << cmd << std::endl;
}

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./bot <ip> <port> <password>" << std::endl;
    return 1;
  }

  struct sigaction sa;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handleSigint;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGQUIT, &sa, NULL);

  const std::string ip = argv[1];
  const int port = std::atoi(argv[2]);
  const std::string password = argv[3];

  g_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (g_sock == -1) {
    std::cerr << "Failed to create socket." << std::endl;
    return 1;
  }

  struct sockaddr_in serverAddr = {};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);

  if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0) {
    std::cerr << "Invalid address / Address not supported." << std::endl;
    if (g_sock != -1)
      close(g_sock);
    return 1;
  }

  if (connect(g_sock, reinterpret_cast<struct sockaddr *>(&serverAddr), sizeof(serverAddr)) < 0) {
    std::cerr << "Connection failed." << std::endl;
    if (g_sock != -1)
      close(g_sock);
    return 1;
  }

  std::cout << "Connected to server! Authenticating..." << std::endl;

  sendCommand(g_sock, "PASS " + password);
  sendCommand(g_sock, "NICK HelpBot");
  sendCommand(g_sock, "USER bot 0 * :I am a simple bot");

  char buffer[1024];
  std::string readBuffer;

  while (g_keepRunning) {
    std::memset(buffer, 0, sizeof(buffer));

    ssize_t bytesRead = recv(g_sock, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
      if (g_keepRunning) {
        std::cout << "Server closed connection or error occurred." << std::endl;
      }
      break;
    }

    readBuffer += buffer;

    size_t pos;
    while ((pos = readBuffer.find('\n')) != std::string::npos) {
      std::string line = readBuffer.substr(0, pos);
      readBuffer.erase(0, pos + 1);

      if (!line.empty() && line[line.length() - 1] == '\r') {
        line.erase(line.length() - 1);
      }

      std::cout << "<< " << line << std::endl;

      if (line.find("PING ") == 0) {
        std::string serverName = line.substr(5);
        sendCommand(g_sock, "PONG " + serverName);
      } else if (line.find("PRIVMSG") != std::string::npos) {
        size_t targetStart = line.find("PRIVMSG ") + 8;
        size_t messageStart = line.find(" :", targetStart);

        if (messageStart != std::string::npos) {
          std::string target = line.substr(targetStart, messageStart - targetStart);
          std::string message = line.substr(messageStart + 2);
          std::string sender = line.substr(1, line.find('!') - 1);

          if (message == "!ping") {
            std::string replyTarget = (target[0] == '#' || target[0] == '&') ? target : sender;
            sendCommand(g_sock, "PRIVMSG " + replyTarget + " :pong!");
          }
        }
      }
    }
  }

  if (g_sock != -1) {
    close(g_sock);
    g_sock = -1;
  }
  return 0;
}
