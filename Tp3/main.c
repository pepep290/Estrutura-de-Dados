#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Inclusão dos cabeçalhos dos módulos auxiliares
#include "consolidado.h"
#include "intercalacao2f.h"
#include "intercalacaof1.h"
#include "qsortexterno.h"

// Função para preparar o arquivo binário de entrada com base na situação desejada
void prepararCenario(char *nomeOriginal, char *nomeDestino, int n, int situacao) {
    FILE *in = fopen(nomeOriginal, "r");
    if (!in) {
        fprintf(stderr, "Erro ao abrir arquivo original: %s\n", nomeOriginal);
        exit(1);
    }

    // Aloca buffer para leitura e ordenação em memória (simula o cenário)
    Registro *buffer = malloc(n * sizeof(Registro));
    if (!buffer) {
        fprintf(stderr, "Erro de alocacao de memoria.\n");
        fclose(in);
        exit(1);
    }

    for (int i = 0; i < n; i++) {
        lerRegistroTexto(in, &buffer[i]);
    }
    fclose(in);

    if (situacao == 1) {
        // Situação 1: Arquivo Ascendente
        qsort(buffer, n, sizeof(Registro), compararRegistros);
    } else if (situacao == 2) {
        // Situação 2: Arquivo Descendente
        qsort(buffer, n, sizeof(Registro), compararRegistros);
        // Inverte o vetor
        for (int i = 0; i < n / 2; i++) {
            Registro temp = buffer[i];
            buffer[i] = buffer[n - 1 - i];
            buffer[n - 1 - i] = temp;
        }
    }
    // Situação 3 (Desordenado) não requer qsort prévio, mantém a ordem de leitura

    FILE *out = fopen(nomeDestino, "wb");
    if (!out) {
        fprintf(stderr, "Erro ao criar arquivo binario de entrada.\n");
        free(buffer);
        exit(1);
    }

    for (int i = 0; i < n; i++) {
        escreverRegistroBin(out, &buffer[i]);
    }
    fclose(out);
    free(buffer);
}

int main(int argc, char *argv[]) {
    // Validação dos argumentos da linha de comando
    if (argc < 4) {
        printf("Uso: %s <metodo> <quantidade> <situacao> [-P]\n", argv[0]);
        printf("Metodos: 1-Intercalacao 2f | 2-Intercalacao 2f+1 | 3-Quicksort Externo\n");
        printf("Situacao: 1-Ascendente | 2-Descendente | 3-Desordenado\n");
        return 1;
    }

    int metodo = atoi(argv[1]);
    int quantidade = atoi(argv[2]);
    int situacao = atoi(argv[3]);
    int imprimir = (argc == 5 && strcmp(argv[4], "-P") == 0);

    char *arquivoProcessado = "entrada_preparada.bin";
    
    // Gera o arquivo binário de entrada conforme a quantidade e situação
    prepararCenario("PROVAO.TXT", arquivoProcessado, quantidade, situacao);

    // Inicialização das métricas de desempenho
    Estatisticas stats = {0, 0, 0, 0.0};

    clock_t inicio = clock();

    // Seleção do método de ordenação externa
    switch (metodo) {
        case 1:
            // Intercalação Balanceada 2f (f caminhos)
            intercalacaoBalanceada2f(arquivoProcessado, &stats);
            break;
        case 2:
            // Intercalação Balanceada f+1 (f+1 caminhos)
            intercalacaoBalanceadaFmais1(arquivoProcessado, &stats);
            break;
        case 3:
            // Quicksort Externo
            quicksortExterno(arquivoProcessado, &stats);
            break;
        default:
            printf("Metodo invalido.\n");
            return 1;
    }

    clock_t fim = clock();
    stats.tempo_execucao = (double)(fim - inicio) / CLOCKS_PER_SEC;

    // Relatório de Desempenho
    printf("\n--- RELATORIO DE DESEMPENHO ---\n");
    printf("Metodo: %d | Situacao: %d | N: %d\n", metodo, situacao, quantidade);
    printf("Transferencias de leitura: %ld\n", stats.leituras);
    printf("Transferencias de escrita: %ld\n", stats.escritas);
    printf("Comparacoes entre notas: %ld\n", stats.comparacoes);
    printf("Tempo de execucao: %.4f segundos\n", stats.tempo_execucao);

    // Impressão opcional dos resultados (Flag -P)
    if (imprimir) {
        Registro r;
        FILE *res = NULL;

        // Tenta abrir o arquivo resultante. 
        // Nota: O nome do arquivo final pode variar dependendo da implementação do método.
        // O código original sugeria "fita_0.bin" ou "fita_10.bin".
        // O arquivoProcessado geralmente contém o resultado no Quicksort ou na Intercalação se renomeado.
        
        // Verificação hierárquica comum em ordenação externa:
        if ((res = fopen(arquivoProcessado, "rb")) == NULL) { 
             // Se não estiver no original, tenta as fitas de saída comuns
             if ((res = fopen("fita_0.bin", "rb")) == NULL) {
                 res = fopen("fita_10.bin", "rb"); // Fita comum para saída em 2f de 20 fitas
             }
        }

        if (res) {
            printf("\n--- RESULTADO DA ORDENACAO (Primeiros registros) ---\n");
            
            // Lógica de impressão segura
            if (lerRegistroBin(res, &r)) {
                int count = 0;
                do {
                    printf("%08ld %05.1f %s %-50s %-30s\n", 
                           r.inscricao, r.nota, r.estado, r.cidade, r.curso);
                    count++;
                    // Opcional: limitar a impressão se for muito grande, mesmo com -P
                } while (lerRegistroBin(res, &r) && (!imprimir || count < quantidade)); 
            } else {
                printf("A fita de resultado esta vazia ou ilegivel.\n");
            }
            fclose(res);
        } else {
            printf("Erro: Arquivo de resultado nao encontrado. Verifique a logica de saida do metodo %d.\n", metodo);
        }
    }

    return 0;
}