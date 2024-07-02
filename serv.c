#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
const int N = 1024, thr = 5;
pthread_mutex_t st_mutex;
FILE* f;
int rec_msg(int sock)
{
    char buf[N];
    int i, n;
    bzero(buf, N);
    if ((n = recv(sock, buf, N, 0)) < 0)
    {
        perror(" Плохое получение потоком.");
        fclose(f);
        return 0;
    }
    buf[n] = '\0';
    printf(" Socket для клиента: %d\n", sock);
    printf(" Сообщение: %s\n\n", buf);
    //printf(" Длина сообщения: %d\n", n);

    pthread_mutex_lock(&st_mutex);
    f = fopen("file.txt", "a");
    fprintf(f, " Socket для клиента: %d\n", sock);
    fprintf(f, " Сообщение: %s\n\n", buf);
    //fprintf(f, " Длина сообщения: %d\n", n);
    fclose(f);
    pthread_mutex_unlock(&st_mutex);

    return 1;
}

int main()
{
    int sock_s, sock_c, len, child, i, start, stop;
    char* end;
    struct sockaddr_in servAddr;
    f = fopen("file.txt", "w");
    fclose(f);
    pthread_t th[thr];
    pthread_attr_t ta;
    if ((sock_s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror(" Сокет не открылся.");
        return 1;
    }
    bzero(&servAddr, sizeof(servAddr));

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = 0;
    if (bind(sock_s, (const struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
    {
        perror(" Сервер не удалось связать.");
        return 1;
    }

    len = sizeof(servAddr);
    if (getsockname(sock_s, (struct sockaddr*)&servAddr, &len))
    {
        perror(" Проблема с getsockname.");
        return 1;
    }
    printf(" Номер порта сервера: %d\n", ntohs(servAddr.sin_port));
    listen(sock_s, 5);

    pthread_attr_init(&ta);
    pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED);
    pthread_mutex_init(&st_mutex, 0);

    while (1)
    {
        if ((sock_c = accept(sock_s, 0, 0)) < 0)
        {
            perror(" Проблема с сокетом клиента.");
            return 1;
        }
        
        for (i = 0; i < thr; i++)
        {
            if (pthread_create(&th[i], &ta, (void*)rec_msg, (void*)sock_c) < 0)
            {
                perror(" Проблема с потоком.");
                return 1;
            }
        }

        /*for (i = 0; i < thr; i++)
        {
            if (pthread_join(th[i], NULL) < 0)
            {
                perror(" Проблема с потоком2.");
                return 1;
            }
        }*/
    }
    pthread_mutex_destroy(&st_mutex);
    return 0;
}