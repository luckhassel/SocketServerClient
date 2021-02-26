#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// Define a porta do servidor
#define PORT 4242

// Define o tamanho do buffer para a troca de mensagens
#define BUFFER_LENGTH 4096


//Programa principal
int main(void) {

    //Criação de estruturas do tipo sockaddr_in para armazenar informações sobre o server-client socket
    struct sockaddr_in client, server;

    //File descriptors do cliente e servidor
    int serverfd, clientfd;

    //Declaracao da variavel buffer suportada, de tamanho do BUFFER_LENGTH (4096)
    char buffer[BUFFER_LENGTH];

    //Declaracao de variavel que definirá qual comando será executado
    char init_char[3] = {};
    //Declaração da ultima posição do char acima como /0 para indicar fim do elemento e evitar, assim, erros
    init_char[2] = '\0';

    //Declaração de ponteiro do tipo FILE que receberá a saída do comando popen
    FILE *fp;

    fprintf(stdout, "Starting server ...\n");

    // Criação do socker server. 
    // Domínio -> AF_INET significa que é um socket que se comunicará seguindo padrão TCP/IP.
    // Tipo -> Além disso, serão utilizados apenas endereços IPV4 (SOCK_STREAM)
    // Protocolo -> O valor zero significa que será utilizado o protocolo (IP) 
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverfd == -1) {
        perror("Can't create the server socket:");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Server socket created with fd: %d\n", serverfd);

    // Define algumas características do socker no elemento server da estrutura já criada
    // Ou seja, define os parâmetros como domínio e porta de comunicação   
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    //Seta os valores de sin zero para 0
    memset(server.sin_zero, 0x0, 8);

    //Verifica o problema da porta definida (4242) já estar em uso
    int yes = 1;
    if(setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR,
                  &yes, sizeof(int)) == -1) {
        perror("Socket options error:");
        return EXIT_FAILURE;
    }

    // Realiza o bind da porta ao servidor. Ou seja, quando o servidor receber uma mensagem
    // Ela entrará por essa porta
    if(bind(serverfd, (struct sockaddr*)&server, sizeof(server)) == -1 ) {
        perror("Socket bind error:");
        return EXIT_FAILURE;
    }

    // Lida com algum erro na criação do servidor
    if(listen(serverfd, 1) == -1) {
        perror("Listen error:");
        return EXIT_FAILURE;
    }

    fprintf(stdout, "Listening on port %d\n", PORT);

    socklen_t client_len = sizeof(client);

    // Lida com erro de tentativa de conexão vinda de algum cliente
    if ((clientfd=accept(serverfd,
        (struct sockaddr *) &client, &client_len )) == -1) {
        perror("Accept error:");
        return EXIT_FAILURE;
    }

    // Realiza a cópia da mensagem principal para o buffer. 
    // Ou seja, é aquela mensagem que é respondida ao cliente quando ele a mensagem de conexão
    strcpy(buffer, "Connected successfully! \n\0");

    // Realiza o envio da mensagem acima ao cliente, e entra no loop principal.
    // A partir daqui, enquando não for quebrada a conexão ou recebida a mensagem "Disconnect"
    // O server ficará ouvindo a porta 4242 por novas mensagens
    if (send(clientfd, buffer, strlen(buffer), 0)) {
        fprintf(stdout, "New client accepted. Waiting for commands...\n");

        // Fica verificando a entrada das mensagens
        do {

            // Zera o buffer de envio
            memset(buffer, 0x0, BUFFER_LENGTH);

            // Realiza a leitura da mensagem recebida pelo cliente
            int message_len;
            if((message_len = recv(clientfd, buffer, BUFFER_LENGTH, 0)) > 0) {
                buffer[message_len - 1] = '\0';
                printf("Command received: %s\n", buffer);
            }
            
            // init_char recebe as duas primeiras posição de buffer afim de diferenciar as mensagens
            init_char[0] = buffer[0];
            init_char[1] = buffer[1];

            // Verifica se a mensagem recebida foi "Disconnect"
            if(strcmp(buffer, "Disconnect") == 0) {
                send(clientfd, "Disconnected", 12, 0);                
            }

            // Verifica se os dois primeiros caracteres são "ls"
            else if (strcmp(init_char, "ls") == 0){
                // Cria-se duas variáveis auxiliares que serão utilizadas para envio do resultado
                // Do comando ao cliente
                char path[BUFFER_LENGTH] = {};
                char output[BUFFER_LENGTH] = {};

                // Realiza a abertura do comando. Popen realiza a abertura de um processo filho
                // A parte de "2>&1" se faz necessária para encaminhas os erros padrões do Linux ao cliente
                fp = popen(strcat(buffer, " 2>&1"), "r");
                if (fp == NULL) {
                    printf("Failed to run command\n" );
                }

                // Lê todas as linhas da saída do comando realizado no SERVIDOR
                while (fgets(path, sizeof(path), fp) != NULL) {
                    strcat(output, path);
                }
                
                // Realiza o envio da mensagem ao cliente. Ela pode, ou conter dados do comando ls
                // Ou conter alguma mensagem de erro padrão do Linux
                send(clientfd, output, sizeof(output), 0);
            }

            //Verifica se os dois primeiros caracteres são "mk"
            else if (strcmp(init_char, "mk") == 0){
                // Cria-se duas variáveis auxiliares que serão utilizadas para envio do resultado
                // Do comando ao cliente
                char path[BUFFER_LENGTH] = {};
                char output[BUFFER_LENGTH] = {};

                // Realiza a abertura do comando. Popen realiza a abertura de um processo filho
                // A parte de "2>&1" se faz necessária para encaminhas os erros padrões do Linux ao cliente
                fp = popen(strcat(buffer, " 2>&1"), "r");
                if (fp == NULL) {
                    printf("Failed to run command\n" );
                }

                // Lê todas as linhas da saída do comando realizado no SERVIDOR
                while (fgets(path, sizeof(path), fp) != NULL) {
                    strcat(output, path);
                }

                // Como esse comando não possui retorno, foi implementado um retorno sucesso caso haja sucesso
                // Em sua execução
                if (strcmp(output, "") == 0)
                    send(clientfd, "Success", 8, 0);

                // Caso ocorra algum erro, é retornado o erro padrão do Linux
                else
                    send(clientfd, output, sizeof(output), 0);
            }

            //Verifica se os dois primeiros caracteres são "cd"
            else if (strcmp(init_char, "cd") == 0){
                // Cria-se duas variáveis auxiliares que serão utilizadas para envio do resultado
                // Do comando ao cliente
                char path[BUFFER_LENGTH] = {};
                char output[BUFFER_LENGTH] = {};

                //Aqui é criada uma variável auxiliar que armazenará o restante do comando após o "cd"
                char aux[BUFFER_LENGTH] = {};
                int j = 0;

                for (int i=3; i<BUFFER_LENGTH; i++){
                    if (buffer[i] == '\0')
                        break;
                    aux[j] = buffer[i];
                    j += 1; 
                }

                // Como o popen utilizado nos outros comandos cria um processo filho, não funcionaria aqui
                // Isso porque, caso fosse mudado o diretório em um processo filho, não haveria efeito
                // Dessa forma, utiliza-se o chdir, que realiza o comando no processo principal
                // Caso ele encontre algum erro, é retornado o erro padrão do Linux ao comando errado cd
                if (chdir(aux) != 0)
                    send(clientfd, strcat(aux, ": No such file or directory"), sizeof(aux), 0);

                // Caso o comando seja realizado, retornamos "Success", uma vez que não há retorno padrão do Linux
                else
                    send(clientfd, "Success", 8, 0);
            }

            //Verifica se os dois primeiros caracteres são "re"
            else if (strcmp(init_char, "re") == 0){
                // Cria-se duas variáveis auxiliares que serão utilizadas para envio do resultado
                // Do comando ao cliente
                char path[BUFFER_LENGTH] = {};
                char output[BUFFER_LENGTH] = {};

                // Realiza a abertura do comando. Popen realiza a abertura de um processo filho
                // A parte de "2>&1" se faz necessária para encaminhas os erros padrões do Linux ao cliente
                fp = popen(strcat(buffer, " 2>&1"), "r");
                if (fp == NULL) {
                    printf("Failed to run command\n" );
                }

                // Lê todas as linhas da saída do comando realizado no SERVIDOR
                while (fgets(path, sizeof(path), fp) != NULL) {
                    strcat(output, path);
                }
                
                // Como esse comando não possui retorno, foi implementado um retorno sucesso caso haja sucesso
                // Em sua execução
                if (strcmp(output, "") == 0)
                    send(clientfd, "Success", 8, 0);

                // Caso ocorra algum erro, é retornado o erro padrão do Linux
                else
                    send(clientfd, output, sizeof(output), 0);
            }

            //Verifica se os dois primeiros caracteres são "rm"
            else if (strcmp(init_char, "rm") == 0){
                // Cria-se duas variáveis auxiliares que serão utilizadas para envio do resultado
                // Do comando ao cliente
                char path[BUFFER_LENGTH] = {};
                char output[BUFFER_LENGTH] = {};

                // Realiza a abertura do comando. Popen realiza a abertura de um processo filho
                // A parte de "2>&1" se faz necessária para encaminhas os erros padrões do Linux ao cliente
                fp = popen(strcat(buffer, " 2>&1"), "r");
                if (fp == NULL) {
                    printf("Failed to run command\n" );
                }

                // Lê todas as linhas da saída do comando realizado no SERVIDOR
                while (fgets(path, sizeof(path), fp) != NULL) {
                    strcat(output, path);
                }
                
                // Como esse comando não possui retorno, foi implementado um retorno sucesso caso haja sucesso
                // Em sua execução
                if (strcmp(output, "") == 0)
                    send(clientfd, "Success", 8, 0);
                
                // Caso ocorra algum erro, é retornado o erro padrão do Linux
                else
                    send(clientfd, output, sizeof(output), 0);
            }

            // Caso seja enviada alguma mensagem não conhecida pelo server, é retornada a mensagem "Error"
            else{
                send(clientfd, "Error", 5, 0);
            }

        } while(strcmp(buffer, "Disconnect"));
    }

    // Fecha a conexão com o cliente
    close(clientfd);

    // Fecha a conexão com o servidor
    close(serverfd);

    printf("Connection closed\n\n");

    return EXIT_SUCCESS;
} 