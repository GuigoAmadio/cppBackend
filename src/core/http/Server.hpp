#pragma once

#include <string>
#include <functional>
#include <memory>
#include <atomic>
#include "../threading/ThreadPool.hpp"

// Inclusões de headers de sistema e declarações de tipo de socket de forma cross-platform
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    // No Windows, socket é um tipo SOCKET (unsigned int)
    typedef SOCKET socket_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    // Em Unix/Linux, socket é só um int
    typedef int socket_t;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

// Com certeza! Vou detalhar mais.
//
// Em projetos grandes em C++, pode acontecer de diferentes pessoas, bibliotecas ou áreas do programa criarem funções ou classes com o mesmo nome, como "Server", "Connection", etc. 
// Se todo mundo colocar tudo no "mundo global" do C++, cedo ou tarde os nomes vão colidir e causar erros de compilação.
//
// O namespace é uma ferramenta do C++ que permite separar e organizar o código em "espaços de nomes" distintos;
// pense neles como pastas para nomes (tipos, funções, variáveis).
//
// Por exemplo, digamos que você tenha:
// - Um "Server" para rede (HTTP)
// - Um "Server" para banco de dados
// - Um "Server" em alguma biblioteca de terceiros
//
// Se todos são apenas "Server", o compilador não consegue diferenciar. Com namespace, você pode fazer:
//
//   Core::Http::Server         // O servidor HTTP do núcleo do seu projeto
//   Database::Server           // Um servidor relacionado ao banco de dados
//   LibKafka::Server           // Um servidor da biblioteca Kafka
//
// Dessa forma, cada nome é único dentro do seu "espaço", e você não precisa se preocupar com colisões.
// Além disso, isso ajuda na leitura: só de ver "Core::Http::Server" já entende que é algo do núcleo, módulo HTTP.
//
// Como funciona na prática? 
// O código entre "namespace Core { namespace Http { ... } }" fica dentro desse namespace.
// Para acessar do lado de fora, você usa o nome completo ("qualificado"):
//
//   Core::Http::Server server(8080);
//
// Assim, você pode ter quantas classes "Server" quiser em namespaces diferentes, tudo organizado e sem briga.
//
// Resumindo: namespace serve para organizar, tornar o código mais legível, seguro contra conflitos, 
// e facilita a manutenção e entendimento em projetos grandes e/ou com muitas bibliotecas externas.
namespace Core::Http {

// Encaminhamento de declarações para classes que aparecem como ponteiros nas interfaces
class Request;  // Representa um HTTP request já parseado
class Response; // Representa um HTTP response pronto para ser enviado
class Router;   // Responsável pelo roteamento de requests HTTP para handlers adequados

/**
 * Classe principal: Server
 * 
 * Esta classe é responsável por toda a cadeia de funcionamento básico de um servidor HTTP,
 * contemplando o ciclo de vida do socket, a criação de múltiplas threads para lidar com conexões,
 * o roteamento de requests, a realização do parsing e envio de responses.
 *
 * Como Funciona (detalhado):
 * 1. Ao criar um Server, especifica-se uma porta para abrir um socket TCP que será usado para aceitar conexões.
 * 2. O método start() inicializa (se necessário) o sistema de sockets da plataforma (como Winsock),
 *    cria o socket servidor, faz bind na porta, e passa a escutar por conexões. O método bloqueia enquanto o servidor está ativo.
 * 3. Quando uma conexão chega, ela é aceita e delegada para uma thread no ThreadPool processar (concorrência eficiente).
 * 4. Cada conexão é lida, seu request é parseado (parseRequest), roteado via um objeto Router (por setRouter()), e uma response produzida.
 * 5. O envio da response é feito pela função sendResponse(), garantindo que o protocolo HTTP seja respeitado.
 * 6. O servidor pode ser parado de forma segura via stop(), que seta um estado atômico e força o fechamento ordenado de sockets
 *    e threads, sem interrupções abruptas.
 * 
 * Recursos e restrições:
 * - Não permite cópia ou movimentação (possuem ownership de recursos do SO).
 * - Permite alterar o número de threads do pool (via setThreadCount()).
 * - Gerencia cross-platform os detalhes de socket, tornando o código portável.
 * 
 * Uso:
 *   Server s(8000);
 *   s.setRouter(router);
 *   s.setThreadCount(4);
 *   s.start();  // Bloqueia até chamada de stop()
 */
class Server {
public:
    /**
     * @brief Constrói um novo servidor HTTP na porta especificada (padrão: 8080)
     *        Não efetua bind/criação de socket ainda.
     * @param port Porta a escutar (ex: 80, 8080)
     */
    Server(int port = 8080);

    /**
     * @brief Destrutor; libera todos recursos do SO (socket, threads, etc).
     *        Garante parada ordenada se necessário.
     */
    ~Server();

    // O Server não pode ser copiado nem movido pois, internamente, controla recursos do sistema operacional,
    // como sockets e pools de threads. Isso significa que não é seguro criar "cópias" desse objeto na memória.
    // Para garantir que isso nunca aconteça por engano (ex: Server x = y;), nós "deletamos" explicitamente os métodos especiais
    // de cópia e movimento. Na prática, as linhas abaixo dizem ao compilador: "essas operações são proibidas".

    // --- O que cada linha significa: ---
    // Ao acrescentar = delete nessas funções especiais, estamos dizendo ao compilador que:
    // "Não gere automaticamente nem permita o uso desses métodos, emitindo erro de compilação se alguém tentar copiar ou mover".
    // Isso é especialmente importante quando a classe gerencia recursos que não podem ser duplicados com segurança (como sockets abertos).
    // Você pode aplicar esse padrão a qualquer classe que deva ser 'não copiável' ou 'não movível', basta fazer assim:
    //     MinhaClasse(const MinhaClasse&) = delete;
    //     MinhaClasse& operator=(const MinhaClasse&) = delete;
    Server(const Server&) = delete;             // Impede: Server b(a); ou Server b = a; => ERRO em tempo de compilação
    Server& operator=(const Server&) = delete;  // Impede: b = a; => ERRO em tempo de compilação


    // --- Exemplo do que é proibido por essas linhas: ---
    //    Server a;
    //    Server b = a;            // Erro! (cópia)
    //    Server c = std::move(a); // Erro! (movimento)


    /**
     * @brief Inicia o servidor, bloqueando a thread chamadora enquanto está ativo.
     *        Inicializa sockets, pool de threads e começa o acceptLoop().
     *        Só retorna ao parar o servidor (stop()).
     */
    void start();

    /**
     * @brief Solicita parada ordenada do servidor.
     *        Fecha novos accepts e permite finalizar conexões ativas.
     */
    void stop();

    /**
     * @brief Define qual objeto Router deve ser usado para decidir para onde enviar cada request.
     * @param router Ponteiro compartilhado para o Router do sistema
     */
    void setRouter(std::shared_ptr<Router> router);

    /**
     * @brief Configura o número de threads do ThreadPool usadas para processar conexões em paralelo.
     *        O servidor criará um pool desse tamanho ao iniciar.
     * @param count Número desejado de threads de worker
     */
    void setThreadCount(int count);

private:
    int port_; // Porta TCP na qual o servidor deve escutar (ex: 8080)
    socket_t serverSocket_; // Socket principal do servidor (criado em createSocket())
    std::atomic<bool> running_; // Indica se o servidor está ativo (usado para shutdown controlado)
    std::shared_ptr<Router> router_; // Router HTTP registrado para encaminhamento dos requests
    int threadCount_; // Quantidade de threads do ThreadPool
    std::unique_ptr<Threading::ThreadPool> threadPool_; // Pool de threads para execução concorrente dos handlers

    /**
     * @brief Cria, configura e faz bind do socket de escuta na porta especificada.
     *        Garante portabilidade entre Windows e UNIX.
     */
    void createSocket();

    /**
     * @brief Loop principal de aceitação de conexões.
     *        Enquanto running_ for true, aceita conexões novas e delega ao ThreadPool.
     *        Implementa tolerância a erros operacionais do SO.
     */
    void acceptLoop();

    /**
     * @brief Processa toda a vida útil de uma conexão: leitura do request, parsing, roteamento e envio do response.
     *        Cada chamada geralmente roda dentro de uma thread do pool.
     * @param clientSocket Socket conectado ao cliente
     */
    void handleConnection(socket_t clientSocket);

    /**
     * @brief Realiza o parsing do stream do socket para produzir um objeto Request.
     *        Pode bloquear até receber header completo. Implementa parsing do protocolo HTTP.
     * @param socket Socket cliente para leitura
     * @return Objeto Request parseado (ou nullptr se inválido)
     */
    std::unique_ptr<Request> parseRequest(socket_t socket);

    /**
     * @brief Envia uma resposta HTTP serializada ao cliente.
     *        Normaliza headers, status e corpo conforme HTTP.
     * @param socket Socket cliente para envio
     * @param response Objeto Response preenchido
     */
    void sendResponse(socket_t socket, const Response& response);

    /**
     * @brief Fecha um socket cliente de forma segura e portável, liberando
     *        quaisquer recursos associados.
     * @param socket Socket cliente a ser fechado
     */
    void closeSocket(socket_t socket);

    /**
     * @brief Inicializa a stack de sockets do Windows (Winsock). No Unix não faz nada.
     *        Necessário antes de criar qualquer socket em _WIN32.
     */
    void initWinsock();

    /**
     * @brief Realiza cleanup da stack Winsock (Windows), liberando recursos globais.
     *        Em Unix, função dummy.
     */
    void cleanupWinsock();
};

} // namespace Core::Http

