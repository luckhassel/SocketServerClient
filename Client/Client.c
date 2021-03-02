#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Define a porta do servidor
#define PORT 4242

// Define o tamanho do buffer
#define LEN 4096

// Define o endereço do servidor. Como será rodado localmente, utiliza-se o endereço de callback
#define SERVER_ADDR "127.0.0.1"

// Função principal
int main(void) {

    // Declaração de uma estrutura do tipo sockaddr_in para armazenar informações do servidor
    struct sockaddr_in server;

    // File descriptor ao socket local (clientes)
    int sockfd;

    int len = sizeof(server);
    int slen;

    // Declaração do buffer de mensagens recebidas
    char buffer_in[LEN];

    // Declaração do buffer de mensagens enviadas
    char buffer_out[LEN];

    fprintf(stdout, "Starting Client ...\n");

    // Cria um socket para o cliente 
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error on client socket creation:");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Client socket created with fd: %d\n", sockfd);

    // Realiza o preenchimento dos dados definidos na estrutura do tipo sockaddr_in
    // AF_INET -> Padrão TCP/IP
    server.sin_family = AF_INET;
    // Configura a porta de entrada como a 4242
    server.sin_port = htons(PORT);
    // Define o endereço do servidor
    server.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    // Preenche o atributo sin_zero com zeros
    memset(server.sin_zero, 0x0, 8);


    // Tenta realizar a conexão com o servidor
    if (connect(sockfd, (struct sockaddr*) &server, len) == -1) {
        perror("Can't connect to server");
        return EXIT_FAILURE;
    }


    // Recebe a mensagem do servidor, que indica o início da comunicação
    if ((slen = recv(sockfd, buffer_in, LEN, 0)) > 0) {
        buffer_in[slen + 1] = '\0';
        fprintf(stdout, "Received message: %s\n", buffer_in);
    }


    // Entra no loop que fica verificando se é recebida a mensagem "Disconnected"
    while (true) {

        // Zera os buffers de saída e entrada
        memset(buffer_in, 0x0, LEN);
        memset(buffer_out, 0x0, LEN);


        fprintf(stdout, "Send command: ");
        fgets(buffer_out, LEN, stdin);

        // Realiza o envio da mensagem
        send(sockfd, buffer_out, strlen(buffer_out), 0);

        // Recebe a mensagem de acordo com o envio
        slen = recv(sockfd, buffer_in, LEN, 0);
        printf("Received message: %s\n", buffer_in);

        // Caso a mensagem recebida seja "Disconnected", finaliza a conexão
        if(strcmp(buffer_in, "Disconnected") == 0)
            break;
    }


    // Finaliza o socket
    close(sockfd);

    fprintf(stdout, "\nConnection closed\n\n");

    return EXIT_SUCCESS;
}