#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "begin_request.h"
#include "send_params.h"
#include "send_stdin.h"

static void handle_send_params_result(const SendParamsResult send_params_result)
{
    switch (send_params_result.tag)
    {
    case SEND_PARAMS_PROTOCOL_ERR:
        {
            const AddParamResult add_param_error = send_params_result.payload.protocol_error.previous;

            switch (add_param_error.tag)
            {
            case ADD_PARAM_LENGTH_TOO_LARGE:
                {
                    const FCGI_ParamComponent component = add_param_error.payload.length_error.component;

                    const char* component_name = component == FCGI_COMPONENT_NAME
                                                     ? "NAME"
                                                     : "VALUE";

                    fprintf(
                        stderr,
                        "[PROTOCOL ERROR] Failed to add FastCGI parameter.\n"
                        "  -> Reason: The %s length exceeds the maximum allowed limit of %zu bytes.\n",
                        component_name,
                        add_param_error.payload.length_error.max_size
                    );
                    break;
                }
            case ADD_PARAM_BUFFER_TOO_SMALL:
                fprintf(
                    stderr,
                    "[PROTOCOL ERROR] Insufficient internal buffer capacity while serializing parameters.\n"
                    "  -> Required: %zu bytes | Buffer capacity: %zu bytes.\n",
                    add_param_error.payload.buffer_error.min_buf_size,
                    add_param_error.payload.buffer_error.given_capacity
                );
                break;
            case ADD_PARAM_OK:
                break;
            }

            break;
        }
    case SEND_PARAMS_NETWORK_ERR:
        {
            const int sys_errno = send_params_result.payload.network_error.sys_errno;

            fprintf(
                stderr,
                "[NETWORK ERROR] Failed to send parameters through the socket.\n"
                "  -> Error Code: %d\n"
                "  -> Description: %s\n",
                sys_errno,
                strerror(sys_errno)
            );

            break;
        }
    case SEND_PARAMS_BODY_TOO_LARGE:
        {
            const size_t calculated_size = send_params_result.payload.body_error.calculated_size;
            const size_t max_size = send_params_result.payload.body_error.max_size;

            fprintf(
                stderr,
                "[LIMIT ERROR] The FCGI_PARAMS record body size is too large.\n"
                "  -> Calculated size: %zu bytes | Protocol maximum limit: %zu bytes.\n",
                calculated_size,
                max_size
            );

            break;
        }
    case SEND_PARAMS_OK:
        break;
    }
}

static void handle_request(const int fd)
{
    begin_request(fd);

    const SendParamsResult send_params_result = send_params(fd);

    handle_send_params_result(send_params_result);

    send_stdin(fd);
}

static void handle_response(const int fd)
{
    char buf[4096] = {0};

    const ssize_t recvd = recv(fd, buf, sizeof(buf), 0);

    if (recvd == -1)
    {
        perror("recv");
    }

    printf("recv returned: %zd\n", recvd);
}

int main()
{
    const int fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(9000),
        .sin_addr = {
            .s_addr = 0x220A8C0,
        },
    };

    const int error_connect = connect(fd, (struct sockaddr*)&addr, sizeof(addr));

    if (error_connect != 0)
    {
        perror("connect");
        return errno;
    }

    handle_request(fd);

    handle_response(fd);

    close(fd);

    return 0;
}
