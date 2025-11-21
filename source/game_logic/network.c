#ifndef _WIN32
#include <arpa/inet.h>
#else
#include <winsock2.h>
#define socklen_t int
#endif
#include <errno.h>
#include <unistd.h>
#include <zot.h>

bool get_socket(int *sock, char *address, int port)
{
  *sock = socket(PF_INET, SOCK_DGRAM, 0);
  if (*sock < 0)
  {
    LOG_ERROR("Could not open UDP socket: %s", strerror(errno));
    return false;
  }

  char yes = 1;
  if (setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
  {
    LOG_ERROR("Can not reuse UDP address: %s", strerror(errno));
  }

  struct sockaddr_in name = {.sin_family = AF_INET,
                             .sin_port = htons(port),
                             .sin_addr.s_addr = inet_addr(address)};

  if (bind(*sock, (struct sockaddr *)&name, sizeof(name)) < 0)
  {
    LOG_ERROR("Could not open UDP socket: %s", strerror(errno));
    return false;
  }

  return true;
}

#define ADDR "127.0.0.1"
#define PORT 8098
void init_server()
{
  int socket;
  if (!get_socket(&socket, ADDR, PORT))
  {
    return;
  }

  struct sockaddr_in client_addr;
  socklen_t addr_len = sizeof(client_addr);
  uint32_t buff_len = 1024;
  char buffer[buff_len];
  int recvlen = recvfrom(socket, buffer, buff_len, MSG_WAITALL,
                         (struct sockaddr *)&client_addr, &addr_len);
  buffer[recvlen] = '\0';

  int flag;
#ifndef MSG_CONFIRM
  flag = 0;
#else
  flag = MSG_CONFIRM;
#endif
  sendto(socket, buffer, strlen(buffer), flag,
         (const struct sockaddr *)&client_addr, addr_len);
}

void init_client()
{
  int socket;
  if (!get_socket(&socket, ADDR, PORT))
  {
    return;
  }

  struct sockaddr_in server_addr;
  socklen_t addr_len = sizeof(server_addr);
  uint32_t buff_len = 1024;
  char buffer[buff_len];

  int flag;
#ifndef MSG_CONFIRM
  flag = 0;
#else
  flag = MSG_CONFIRM;
#endif

  sendto(socket, buffer, strlen(buffer), flag,
         (const struct sockaddr *)&server_addr, addr_len);
  int recvlen = recvfrom(socket, buffer, buff_len, MSG_WAITALL,
                         (struct sockaddr *)&server_addr, &addr_len);
  buffer[recvlen] = '\0';

  close(socket);
}