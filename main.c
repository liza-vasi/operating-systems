
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#define PORT 8080

int main(void) {
    int server = socket(AF_INET, SOCK_STREAM, 0);

    if (server == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(server, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("bind");
        close(server);
        return 1;
    }

    if (listen(server, 5) == -1) {
        perror("listen");
        close(server);
        return 1;
    }

    printf("Сервер запущен на порту %d\n", PORT);
    printf("Ожидание подключения...\n");
    fflush(stdout);

  
    int client = -1;

    while (1) {
        fd_set read_fds;

        FD_ZERO(&read_fds);
        FD_SET(server, &read_fds);

        int max_fd = server;

        if (client != -1) {
            FD_SET(client, &read_fds);

            if (client > max_fd)
                max_fd = client;
        }

        int result = pselect(max_fd + 1, &read_fds,
                             NULL, NULL, NULL, NULL);

        if (result == -1) {
            perror("pselect");
            break;
        }

        // Новое подключение
        if (FD_ISSET(server, &read_fds)) {
            int new_client = accept(server, NULL, NULL);

            if (new_client == -1) {
                perror("accept");
            } else {
                printf("Новое подключение\n");

                if (client == -1) {
                    client = new_client;
                    printf("Клиент сохранён\n");
                } else {
                    close(new_client);
                    printf("Лишнее подключение закрыто\n");
                }
            }
        }

        // Получение данных
        if (client != -1 && FD_ISSET(client, &read_fds)) {
            char buffer[1024];

            ssize_t bytes = recv(client, buffer, sizeof(buffer), 0);

            if (bytes > 0) {
                printf("Получено байтов: %zd\n", bytes);
            } else {
                if (bytes < 0)
                    perror("recv");
                else
                    printf("Клиент отключился\n");

                close(client);
                client = -1;
            }
        }
    }

    if (client != -1)
        close(client);

    close(server);
    return 0;
}

