#include <arpa/inet.h>   // inet_addr
#include <stdio.h>       // printf
#include <stdlib.h>      // atoi
#include <string.h>      // strlen
#include <sys/socket.h>  // socket
#include <unistd.h>      // close
#include <dirent.h>
#include <sys/time.h>

void byte_stuffing(char *str, char *stuffed_str) {
  int i, j;
  for (i = 0, j = 0; str[i] != '\0'; i++, j++) {
    if (str[i] == 'b' && str[i + 1] == 'y' && str[i + 2] == 'e') {
      stuffed_str[j++] = 'b'; 
      stuffed_str[j++] = 'y';
      stuffed_str[j++] = 'e'; // Adiciona o 'b' extra
      stuffed_str[j++] = 'b'; // Adiciona o 'b' extra
      i += 2; // Pula os próximos dois caracteres ("ye")
    } else {
      stuffed_str[j] = str[i];
    }
  }
  stuffed_str[j] = '\0';
}

int main(int argc, char* argv[]) {
    int sock;
    struct sockaddr_in server_ipv4;
    struct sockaddr_in6 server_ipv6;
    char message[4096], server_reply[2000];
    struct timeval start, end;  // Variáveis para medir o tempo
    long totalByteSent = 0; // Quantidade de bytes enviados para o servidor

    // Check for required arguments
    if (argc != 3) {
        printf("Usage: %s <IP address> <Port>\n", argv[0]);
        return 1;
    }

    // Convert command-line arguments
    const char *ip_address = argv[1];
    int port = atoi(argv[2]);

    // Verifica se o IP fornecido é IPv4 ou IPv6
    struct sockaddr *server = NULL;
    int ip_type = AF_INET6;  // Default to IPv6

    if (inet_pton(AF_INET6, ip_address, &(server_ipv6.sin6_addr)) == 1) {
        ip_type = AF_INET6;  // It's an IPv6 address
        server = (struct sockaddr*)&server_ipv6;
        server_ipv6.sin6_family = AF_INET6;
        server_ipv6.sin6_port = htons(port);
    } else if (inet_pton(AF_INET, ip_address, &(server_ipv4.sin_addr)) == 1) {
        ip_type = AF_INET;  // It's an IPv4 address
        server = (struct sockaddr*)&server_ipv4;
        server_ipv4.sin_family = AF_INET;
        server_ipv4.sin_port = htons(port);
    } else {
        printf("Invalid IP address format.\n");
        return 1;
    }

    // Create socket based on IP type (IPv4 or IPv6)
    sock = socket(ip_type, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create socket\n");
        return 1;
    }
    puts("Socket created");

    // Conecta ao servidor
    if (connect(sock, server, (ip_type == AF_INET6 ? sizeof(server_ipv6) : sizeof(server_ipv4))) < 0) {
        perror("Connect failed. Error");
        return 1;
    }
    puts("Connected\n");

    // Loop de 7 envios
    for (int i = 0; i < 7; i++) {
        totalByteSent = 0;
        printf("***Teste : %d\n", (i+1));

        
        memset(message, 0, sizeof(message));
        memset(server_reply, 0, sizeof(server_reply));
        if (send(sock, "ready", strlen("ready"), 0) < 0) {
            puts("Send failed");
            close(sock);
            return 1;
        }

        
        gettimeofday(&start, NULL);
        
        // Receive a reply from the server
        if (recv(sock, server_reply,  strlen(server_reply) - 1, 0) < 0) {
            puts("Receive failed");
            close(sock);
        }
        server_reply[strcspn(server_reply, "\n")] = 0; // Remove the trailing newline from the server response (if any)
        printf("Server reply: ");
        puts(server_reply);

        if (strcmp(server_reply, "ready ack") == 0){

                char directory_path[256];

                strcpy(directory_path,"4096bytes"); // Limpa a mensagem

                DIR *dir = opendir(directory_path);
                if (dir == NULL) {
                    perror("Erro ao abrir diretório");
                    
                }

                struct dirent *entry;
                strcpy(message, ""); // Limpa a mensagem
                while ((entry = readdir(dir)) != NULL) {
                    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                        continue;
                    }
                    if (entry->d_type == DT_REG || entry->d_type == DT_DIR) { // Verifica se é um arquivo regular
                        if (strcmp(entry->d_name, "bye") == 0) {
                            byte_stuffing("bye", entry->d_name); // Marca que o arquivo "bye" foi encontrado
                        }
                        strcat(message, entry->d_name);
                        strcat(message, "\n");
                    }
                }

                closedir(dir);

                char dirMessage[1000];
                // Envia o nome do diretório para o servidor
                snprintf(dirMessage, (5 + strlen(directory_path)), "dir:%s", directory_path);
                if (send(sock, dirMessage, strlen(dirMessage), 0) < 0) {
                    puts("Send failed");
                    return 1;
                }
                totalByteSent += strlen(dirMessage);


                // RECEBEU O NOME DO DIRETÓRIO
                if (recv(sock, server_reply,  strlen(server_reply) - 1, 0) < 0) {
                    puts("Receive failed");
                    close(sock);
                }


                // Envia a lista de arquivos para o servidor
                if (send(sock, message, strlen(message), 0) < 0) {
                    puts("Send failed");
                    return 1;
                }
                totalByteSent += strlen(message);


                // RECEBEU O NOME DOS ARQUIVOS
                if (recv(sock, server_reply,  strlen(server_reply) - 1, 0) < 0) {
                    puts("Receive failed");
                    close(sock);
                }

                if (send(sock, "bye", 3,0) < 0) { // Envia bye e fecha a conexão
                    puts("Send failed");
                    //return 1;
                }
                totalByteSent += strlen("bye");

                // Calculando o tempo gasto
                gettimeofday(&end, NULL);
                long seconds = end.tv_sec - start.tv_sec;
                long microseconds = end.tv_usec - start.tv_usec;

                // Handle negative microseconds if necessary
                if (microseconds < 0) {
                    microseconds += 1000000;
                    seconds--;
                }



                // Calcular o throughput em bytes por segundo
                double total_time_seconds = seconds + (microseconds / 1000000.0);
                double throughput = totalByteSent / total_time_seconds;  // bytes por segundo
                printf("tempo em segundos %lf \n", total_time_seconds);
                printf("Bytes enviados %ld \n", totalByteSent);
                printf("throughput (bytes.s) %.6f \n", throughput);
                printf("\n");
        }
    }

    close(sock); // Fecha o socket ao final de todos os envios
    return 0;
}