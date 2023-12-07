#include <iostream>
#include <string>
#include <fstream>
#include <tgbot/tgbot.h>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <zbar.h>
#include <chrono> // Adicione esta linha para usar std::this_thread::sleep_for
#include <thread>
#include <atomic>
#include <future>
// #include <ctime>
#include <iomanip>
#include <pthread.h>
// #include <stdio.h>
// #include <unistd.h>
#include <thread>


#define LOCK 0
#define UNLOCK 1

using namespace cv;
using namespace std;
using namespace TgBot;
using namespace zbar;

// typedef struct struct_aluno ALUNO;
struct Aluno {
    std::string nome;
    std::string matricula;
    // char nome[100]; 
    // char matricula[100];
};
char string_nome[100];
char string_mat[100];

TgBot::Bot bot("6423547833:AAE9xw6eJC2fz9X82MpAmW0WbUxAhzaAmU8");  // Substitua "SEU_TOKEN" pelo token do seu bot

/*
enum EstadoCadastro {
    AGUARDANDO_NOME,
    AGUARDANDO_MATRICULA,
    CONCLUIDO
};

int opcao = 0;

EstadoCadastro estadoCadastro = AGUARDANDO_NOME;
*/

std::atomic<bool> continuarExecucao(true);

bool encerrarChamada = false;
bool chamadaAtiva = false;
bool flagString = false;
bool cabecalho = false;
bool arquivoExiste = true;

std::vector<std::string> vetorDeStrings;

std::thread chamadaThread;


enum ExecutarComando {
    INICIO_CODIGO,
    CADASTRAR_NOME,
    CADASTRAR_MATRICULA,
    FAZER_LISTA,
    FAZER_CHAMADA,
    CONCLUIDO
};

ExecutarComando executarComando = INICIO_CODIGO;

Aluno aluno;

void listarPessoas(TgBot::Bot& bot, TgBot::Message::Ptr message);
void iniciarChamada();
void finalizarChamada();
void cadastrarAula();
void deletarAluno();
void gerarRelatorio();
void sendInlineKeyboard(TgBot::Bot& bot, TgBot::Message::Ptr message);
void* leituraQRCode(void* dummy_ptr);
void prepararRelatorio();


// Função para cadastrar um aluno
void cadastraAluno(TgBot::Bot& bot, TgBot::Message::Ptr message) {
    printf("Cadastra aluno\n"); // Debug

    executarComando = CADASTRAR_NOME;
    bot.getApi().sendMessage(message->chat->id, "Digite o nome do aluno:");
}

void listarPessoas(TgBot::Bot& bot, TgBot::Message::Ptr message) {
    printf("Listar pessoas\n");
    bot.getApi().sendMessage(message->chat->id, "Envie qualquer mensagem para listar pessoas:");
    // sleep(1);
    executarComando = FAZER_LISTA;
    
}
void iniciarChamada(TgBot::Bot& bot, TgBot::Message::Ptr message) {
    printf("Iniciar Chamada\n");
    executarComando = FAZER_CHAMADA;
    
    pthread_t thread_id1;
    pthread_create(&thread_id1, NULL, &leituraQRCode, NULL);
    // leituraQRCode();
    
}

void* leituraQRCode(void* dummy_ptr){
    chamadaAtiva = true;
    // Inicializa a c'âmera
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Erro ao abrir a câmera!" << endl;
        // return NULL;
        
    }
    cout << chamadaAtiva;
    // Inicializa o leitor ZBar
    ImageScanner scanner;
    scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
        // Coloque o código da chamada da câmera aqui
    
    

    while(chamadaAtiva){
    // Captura um frame da câmera
        Mat frame;
        cap >> frame;

        // Converte o frame para escala de cinza
        Mat gray;
        cvtColor(frame, gray, COLOR_BGR2GRAY);

        // Cria uma imagem ZBar a partir do frame em escala de cinza
        Image image(frame.cols, frame.rows, "Y800", gray.data, frame.cols * frame.rows);

        // Escaneia a imagem em busca de códigos de barras
        int n = scanner.scan(image);

        // Itera sobre todos os códigos de barras encontrados
        for (Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol) {
            // Imprime o valor do código de barras
            cout << "Código de Barras: " << symbol->get_data() << endl;
            
            // Você pode armazenar o valor em uma string aqui
            string codigoDeBarras = symbol->get_data();

            flagString = false;

            for (const std::string& str : vetorDeStrings) {
                if (codigoDeBarras == str){
                    flagString = true;
                    break;
                }
            }

            if (!flagString){
                vetorDeStrings.push_back(codigoDeBarras);
            }
        }

        // Exibe o frame original com a marcação do código de barras
        imshow("Câmera", frame);
        // std::cout << "Encerrar chamada: " << std::boolalpha << encerrarChamada << std::endl;
        // std::cout << "Chamada ativa: " << std::boolalpha << chamadaAtiva << std::endl;
        // Verifica se a tecla 'q' foi pressionada para sair do loop
        if (waitKey(1) == 'q' || encerrarChamada) {
            chamadaAtiva = false;
            break;
        }
    }
    
    // Libera os recursos
    cap.release();
    destroyAllWindows();
    // return NULL;
    prepararRelatorio();
    cout << "Chamada Encerrada";
    // Comentar no projeto final 
    std::cout << "Vetor de Strings:" << std::endl;
    for (const std::string& str : vetorDeStrings) {
        std::cout << str << std::endl;
    }
    // --------------------------------------

}

void prepararRelatorio(){
    // Obtendo a hora atual
    auto now = std::chrono::system_clock::now();

    // Convertendo para um tipo de tempo (std::time_t) que pode ser usado para obter a data
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    // Obtendo a estrutura tm para obter informações de data
    std::tm localTime = *std::localtime(&currentTime);

     // Criando uma string para armazenar a data formatada
    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d"); // Formato: Ano-Mês-Dia

    // Obtendo a string formatada
    std::string dataString = oss.str();

    // Imprimindo a string da data
    std::cout << "Data: " << dataString << std::endl;

    // Preparacao do arquivo
    std::fstream arquivo("alunos_relatorio.csv", std::ios::in | std::ios::out | std::ios::app);

    if (!arquivo.is_open()) {
        std::cerr << "Erro ao abrir o arquivo." << std::endl;
        return;
    }

    std::string primeiraLinha;
    std::getline(arquivo, primeiraLinha);

    // Modifique a primeira linha conforme necessário
    primeiraLinha += "," + dataString;

    // Move o ponteiro de arquivo para o início do arquivo
    arquivo.seekp(0, std::ios::beg);

    // Escreve a nova primeira linha no arquivo
    arquivo << primeiraLinha << std::endl;

    arquivo.close();
}
void finalizarChamada() {
    chamadaAtiva = false;
    // if (chamadaThread.joinable()) {
    //     chamadaThread.join();
    //     printf("Chamada encerrada\n");
    // }
}

void cadastrarAula(TgBot::Bot& bot, TgBot::Message::Ptr message) {
    printf("cadastrar aula\n");
    bot.getApi().sendMessage(message->chat->id, "Cadastrar aula:");
}

void deletarAluno(TgBot::Bot& bot, TgBot::Message::Ptr message) {
    printf("deletar aluno\n");
    bot.getApi().sendMessage(message->chat->id, "Deletar Aluno:");
}

void gerarRelatorio(TgBot::Bot& bot, TgBot::Message::Ptr message) {
    printf("relatorio\n");
    bot.getApi().sendMessage(message->chat->id, "Relatorio:");
}

bool matriculaExiste(const std::string& matricula) {
    std::ifstream arquivo("alunos.csv");
    std::string linha;

    while (std::getline(arquivo, linha)) {
        std::stringstream ss(linha);
        std::string nome, matriculaExistente;

        std::getline(ss, nome, ',');
        std::getline(ss, matriculaExistente);

        if (matriculaExistente == matricula) {
            // A matrícula já existe no arquivo
            return true;
        }
    }

    // A matrícula não foi encontrada
    return false;
}

// Função de callback para processar comandos do Telegram
void handleMessages(TgBot::Bot& bot, TgBot::Message::Ptr message) {
    // Verifica se a mensagem contém um comando
    if (!message->text.empty() && message->text[0] == '/') {
        // Extrai o comando sem a barra inicial '/'
        std::string command = message->text.substr(1);

        // Mapeia comandos para opções do menu
        if (command == "cadastrarAluno") {
            cadastraAluno(bot, message);
        } else if (command == "listarPessoas"){
            listarPessoas(bot, message);
        }else if (command == "iniciarChamada") {
            iniciarChamada(bot, message);
        } else if (command == "encerrarChamada"){
            finalizarChamada();
            
        } else if (command == "cadastrarAula") {
            cadastrarAula(bot, message);
        } else if (command == "deletarAluno") {
            deletarAluno(bot, message);
        } else if (command == "gerarRelatorio") {
            gerarRelatorio(bot, message);
        } else if (command == "sair") {
            bot.getApi().sendMessage(message->chat->id, "Programa encerrado");
        } else if (command == "menu") {
            sendInlineKeyboard(bot, message);
        } else {
            bot.getApi().sendMessage(message->chat->id, "Comando inválido. Por favor, use comandos válidos.");
        }
    } else {
        // Se não for um comando, trata como entrada para o cadastro
        //cadastraAluno(bot, message);
        switch (executarComando){
            case CADASTRAR_NOME: {
            // Aluno aluno; // Remova essa linha

            // Salve o texto de entrada na variável global string_nome
            strncpy(string_nome, message->text.c_str(), sizeof(string_nome)-1);
            string_nome[sizeof(string_nome)-1] = '\0';

            // Solicite a matrícula
            bot.getApi().sendMessage(message->chat->id, "Digite a matricula:");
            executarComando = CADASTRAR_MATRICULA;
            break;
            }

            case CADASTRAR_MATRICULA:{
            // Aluno aluno; // Remova essa linha
            // aluno.matricula = message->text; // Remova essa linha

            // Agora use string_nome para o nome do aluno
            // e message->text para a matrícula
            Aluno aluno;
            aluno.nome = string_nome;
            aluno.matricula = message->text;

            cout << "Nome: " << aluno.nome << ", Matricula: " << aluno.matricula << endl;

            if (!isdigit(message->text[0])) {
            // Se não for um número, pede novamente a matrícula
            bot.getApi().sendMessage(message->chat->id, "Matrícula inválida. Digite novamente:");
            return;
            }

            // Verifique se a matrícula já existe
            if (matriculaExiste(aluno.matricula)) {
                bot.getApi().sendMessage(message->chat->id, "Matrícula já cadastrada. Digite uma matrícula diferente:");
                return;
            }

            // // Nome do arquivo CSV
            // const char* arquivoCSV = "alunos.csv";
            // tenta abrir o arquivo CSV para leitura
            std::ifstream arquivo_leitura("alunos.csv");

            // Verificar se o arquivo foi aberto corretamente
            if (!arquivo_leitura.is_open()) {
                std::cerr << "Erro ao abrir o arquivo CSV." << std::endl;
                arquivoExiste = false;
             // Se não conseguir abrir o arquivo logo ele não existe;
            } 

            arquivo_leitura.close();

            ofstream arquivo("alunos.csv", ios::app);  // ios::app para adicionar ao final do arquivo
            
            // if (!cabecalho){
            //     arquivo << "Dont Care, Data: " << end1;
            //     cabecalho = true;
            // }

            // Verificando se o arquivo foi aberto com sucesso
            if (arquivo.is_open()) {
                if (!arquivoExiste){
                    arquivo << "Naoimporta" << "," << "Data" << "," << endl;
                    arquivoExiste = true;
                }
                // Escrevendo no arquivo CSV
                arquivo << aluno.nome << "," << aluno.matricula << endl;

                // Fechando o arquivo
                arquivo.close();

                cout << "Informações do aluno salvas com sucesso no arquivo alunos.csv." << endl;
            } else {
                cout << "Erro ao abrir o arquivo alunos.csv." << endl;
            }

                // Nome do arquivo de origem e destino .csv
            const char* arquivoOrigem = "alunos.csv";
            const char* arquivoDestino = "alunos_relatorio.csv";

            // Abrir o arquivo de origem para leitura
            std::ifstream arquivoEntrada(arquivoOrigem);

            // Verificar se o arquivo de origem foi aberto corretamente
            if (!arquivoEntrada.is_open()) {
                std::cerr << "Erro ao abrir o arquivo de origem." << std::endl;
                return;
            }

            // Abrir o arquivo de destino para escrita
            std::ofstream arquivoSaida(arquivoDestino);

            // Verificar se o arquivo de destino foi aberto corretamente
            if (!arquivoSaida.is_open()) {
                std::cerr << "Erro ao abrir o arquivo de destino." << std::endl;
                return;
            }

            // Ler e escrever o conteúdo do arquivo
            arquivoSaida << arquivoEntrada.rdbuf();

            // Fechar os arquivos
            arquivoEntrada.close();
            arquivoSaida.close();

            std::cout << "Arquivo .csv copiado com sucesso." << std::endl;

                
                executarComando = CONCLUIDO;
            break;
            }

            case FAZER_LISTA:{
                Aluno aluno;
                string linha;

                cout << "Lista de Alunos:" << endl;

                ifstream arquivo("alunos.csv");

                // Verificando se o arquivo foi aberto com sucesso
                if (!arquivo.is_open()) {
                    cout << "Erro ao abrir o arquivo alunos.csv." << endl;
                    return;
                }

                // Percorrendo cada linha do arquivo
                while (getline(arquivo, linha)) {
                    // Usando um stringstream para separar os campos
                    stringstream ss(linha);
                    getline(ss, aluno.nome, ',');
                    getline(ss, aluno.matricula);

                    // Exibindo as informações do aluno
                    cout << "Nome: " << aluno.nome << ", Matrícula: " << aluno.matricula << endl;
                    bot.getApi().sendMessage(message->chat->id, "Nome: " + aluno.nome + " Matrícula: " + aluno.matricula);
                }

                // Fechando o arquivo
                arquivo.close();
            break;
            }

            case FAZER_CHAMADA:{
                chamadaAtiva = true;
            }
        }   
    }
}

// Função para criar um menu inline
void sendInlineKeyboard(TgBot::Bot& bot, TgBot::Message::Ptr message) {
    bot.getApi().sendMessage(message->chat->id, "1) Cadastrar Aluno -> /cadastrarAluno"); 
    bot.getApi().sendMessage(message->chat->id, "2) Iniciar Chamada -> /iniciarChamada");  
    bot.getApi().sendMessage(message->chat->id, "3) Cadastrar Aula -> /cadastrarAula");  
    bot.getApi().sendMessage(message->chat->id, "4) Deletar aluno -> /deletarAluno");
    bot.getApi().sendMessage(message->chat->id, "5) Gerar Relatorio -> /gerarRelatorio");     
    // Crie uma mensagem com o teclado inline
    bot.getApi().sendMessage(message->chat->id, "Escolha uma opção:");
}


int main() {

    // pthread_t thread_id1;
    // Configura a função de callback para processar mensagens
    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
        handleMessages(bot, message);
    });

    // Inicia o loop para receber mensagens
    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        bot.getApi().deleteWebhook();
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            longPoll.start();

        }
    } catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
    }

    return 0;
}
