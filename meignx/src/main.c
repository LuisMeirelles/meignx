#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

void begin_request()
{
}

int main()
{
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(9000),
    .sin_addr = {
      .s_addr = 0x220A8C0,
    },
  };

  int error_connect = connect(fd, (struct sockaddr *)&addr, sizeof(addr));

  if (error_connect != 0) {
    perror("connect");
    return 1;
  }

  printf("funcionou\n");

  return 0;
}
