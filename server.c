
#include <arpa/inet.h>  //inet_addr
#include <stdio.h>
#include <stdlib.h>     // atoi
#include <string.h>     // strlen
#include <sys/socket.h>
#include <unistd.h>     // write

#define MAX_FILENAME_LENGTH  4096

void byte_unstuffing(char *stuffed_str, char *unstuffed_str) {
    int i, j;
    for (i = 0, j = 0; stuffed_str[i] != '\0'; i++, j++) {
        // Verifica se encontrou a sequência "byeb"
        if (stuffed_str[i] == 'b' && stuffed_str[i + 1] == 'y' && stuffed_str[i + 2] == 'e' && stuffed_str[i + 3] == 'b') {
            // Substitui "byeb" por "bye"
            unstuffed_str[j++] = 'b';
            unstuffed_str[j++] = 'y';
            unstuffed_str[j++] = 'e';
            i += 3; // Pula os próximos 3 caracteres ('y', 'e', 'b')
        } else {
            // Caso contrário, copia o caractere normalmente
            unstuffed_str[j] = stuffed_str[i];
        }
    }
    unstuffed_str[j] = '\0'; // Termina a string
}


int main(int argc, char* argv[]) {
    int socket_desc, client_sock, c, read_size;
    struct sockaddr_storage client;  // Para suportar tanto IPv4 quanto IPv6
    char client_message[4096];

    struct sockaddr_in server_ipv4;
    struct sockaddr_in6 server_ipv6;




    // Check for required arguments
    if (argc != 3) {
        printf("Usage: %s <IP address> <Port>\n", argv[0]);
        return 1;
    }

    // Convert command-line arguments
    const char *ip_address = argv[1];
    int port = atoi(argv[2]);

    // Create socket
    socket_desc = socket(AF_INET6, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket\n");
        return 1;
    }
    puts("Socket created");

     // Habilita a reutilização de endereço/porta
    int optval = 1;
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(socket_desc);
        return 1;
    }

     // Configura o endereço do servidor IPv6
    memset(&server_ipv6, 0, sizeof(server_ipv6));
    server_ipv6.sin6_family = AF_INET6;
    server_ipv6.sin6_port = htons(port);
    server_ipv6.sin6_addr = in6addr_any;  // Aceitar qualquer endereço IPv6

    // Configura o endereço do servidor IPv4
    memset(&server_ipv4, 0, sizeof(server_ipv4));
    server_ipv4.sin_family = AF_INET;
    server_ipv4.sin_port = htons(port);
    server_ipv4.sin_addr.s_addr = INADDR_ANY;  // Aceitar qualquer endereço IPv4

    // Tenta fazer o bind em IPv6 primeiro (IPv6 + IPv4 suportados)
    if (bind(socket_desc, (struct sockaddr*)&server_ipv6, sizeof(server_ipv6)) < 0) {
        // Se o bind com IPv6 falhar, tenta com IPv4
        if (bind(socket_desc, (struct sockaddr*)&server_ipv4, sizeof(server_ipv4)) < 0) {
            perror("Bind failed. Error");
            return 1;
        }
    }
    puts("Bind done");
    // Listen
    listen(socket_desc, 1);

    // Accept an incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    // Accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&c);
    if (client_sock < 0) {
        perror("Accept failed");
        return 1;
    }
    puts("Connection accepted");

     // Obtém o endereço IP do cliente
    char ip_str[INET6_ADDRSTRLEN];
    if (client.ss_family == AF_INET6) {
        // Se o cliente usar IPv6
        struct sockaddr_in6 *client_in6 = (struct sockaddr_in6*)&client;
        inet_ntop(AF_INET6, &client_in6->sin6_addr, ip_str, sizeof(ip_str));
        printf("IPV6.");  // Mostra o IP do cliente

    } else if (client.ss_family == AF_INET) {
        // Se o cliente usar IPv4
        struct sockaddr_in *client_in4 = (struct sockaddr_in*)&client;
        inet_ntop(AF_INET, &client_in4->sin_addr, ip_str, sizeof(ip_str));
            printf("IPV4.");  // Mostra o IP do cliente

    }

    printf("Client IP: %s\n", ip_str);  // Mostra o IP do cliente

    char filename[MAX_FILENAME_LENGTH + 50];
    char directory_name[MAX_FILENAME_LENGTH]; // Variável para armazenar o nome do diretório

    FILE *file = NULL; // Inicializa o ponteiro do arquivo

    // Receive a message from client
    while ((read_size = recv(client_sock, client_message,  sizeof(client_message) - 1, 0)) > 0) {

        client_message[read_size] = '\0';
        // Send the message back to client
        if (strcmp(client_message, "ready") == 0){
            if (write(client_sock, "ready ack", strlen("ready ack")) < 0) {
                perror("Write failed");
                break; 
            }

        }else if (strcmp(client_message, "bye") == 0) {
            printf("Salvando arquivo e fechando a conexão.");
            if (file != NULL) { // Fecha o arquivo se ele foi aberto
                fclose(file);
            }
            //break; // Sai do loop while interno
        }else if (strncmp(client_message, "dir:", 4) == 0) { 
            // Verifica se a mensagem começa com "dir:"
            strcpy(directory_name, client_message + 4); // Copia o nome do diretório
        // Cria o nome do arquivo usando o nome do diretório recebido
            snprintf(filename, sizeof(filename), "%s_%s.txt", ip_str ,
                 directory_name); 

            file = fopen(filename, "w"); // Abre o arquivo para escrita
            if (file == NULL) {
                perror("Erro ao criar arquivo");
                close(client_sock);
                continue;
            }
            if (write(client_sock, "ok", strlen("ok")) < 0) {
                perror("arquivo não criado");
                break; 
            }
        }
        else {
         //Armazena a mensagem (lista de arquivos) no arquivo
            char unstuffed[2000];
            if (file != NULL) {
                byte_unstuffing(client_message, unstuffed);
                fprintf(file, "%s", unstuffed); // Grava a nova string no arquivo
            } 
            if (write(client_sock, "ok", strlen("ok")) < 0) {
                perror("arquivo não criado");
                break; 
            }
        }
    }

    if (read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if (read_size == -1) {
        perror("Receive failed");
    }
    
    close(client_sock);
    close(socket_desc);
    return 0;
}
