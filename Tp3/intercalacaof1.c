#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "intercalacaof1.h" 
#include "consolidado.h"

// Estrutura para controlar o estado de cada fita durante a intercalação
typedef struct {
    FILE *fptr;
    Registro reg;
    int ativa;      // 1 se a fita ainda tem dados (não chegou no EOF)
    int runAtivo;   // 1 se a fita ainda está contribuindo para o bloco atual
} ControleFita;

// geração de blocos ordenados por seleção por substituição

int gerarRunsIniciais(char *nomeArquivo, Estatisticas *stats) {
    FILE *arq = fopen(nomeArquivo, "rb");
    if (!arq) return 0;

    FILE *fitas[F_ENTRADA];
    char nome[30];
    for (int i = 0; i < F_ENTRADA; i++) {
        sprintf(nome, "fita_%d.bin", i);
        fitas[i] = fopen(nome, "wb");
    }

    ElementoHeap memoria[TAM_MEMORIA];
    int n = 0; // ocupação atual do heap
    Registro regLido;

    // encher a memória inicialmente
    while (n < TAM_MEMORIA && lerRegistroBin(arq, &regLido)) {
        stats->leituras++;
        memoria[n].reg = regLido;
        memoria[n].congelado = 0;
        n++;
    }
    construirMinHeap(memoria, n, stats);

    int idxFitaAtual = 0;
    int heapSize = n; // elementos não congelados
    int totalRuns = 0;
    if (n > 0) totalRuns = 1;

    Registro ultimoEscrito;
    ultimoEscrito.nota = -1.0; 

    while (n > 0) {
        if (heapSize == 0) {
            for (int i = 0; i < n; i++) memoria[i].congelado = 0;
            heapSize = n;
            construirMinHeap(memoria, heapSize, stats);
            
            idxFitaAtual = (idxFitaAtual + 1) % F_ENTRADA;
            totalRuns++;
        }

        Registro menor = memoria[0].reg;
        escreverRegistroBin(fitas[idxFitaAtual], &menor);
        stats->escritas++;
        ultimoEscrito = menor;

        if (lerRegistroBin(arq, &regLido)) {
            stats->leituras++;
            stats->comparacoes++;
            
            if (regLido.nota < ultimoEscrito.nota) {
                memoria[0] = memoria[heapSize - 1];
                memoria[heapSize - 1].reg = regLido;
                memoria[heapSize - 1].congelado = 1;
                heapSize--;
                if (heapSize > 0) refazerMinHeap(memoria, 0, heapSize, stats);
            } else {
                memoria[0].reg = regLido;
                memoria[0].congelado = 0;
                refazerMinHeap(memoria, 0, heapSize, stats);
            }
        } else {
            memoria[0] = memoria[heapSize - 1];
            if (heapSize < n) {
                ElementoHeap temp = memoria[heapSize]; 
                memoria[heapSize] = memoria[n - 1];    
                memoria[n - 1] = temp;                 
            }
            heapSize--;
            n--; 
            if (heapSize > 0) refazerMinHeap(memoria, 0, heapSize, stats);
        }
    }

    fclose(arq);
    for (int i = 0; i < F_ENTRADA; i++) fclose(fitas[i]);
    
    return totalRuns;
}


// intercalação (19 fitas -> 1 fita)
int intercalarParaSaida(Estatisticas *stats) {
    ControleFita entrada[F_ENTRADA];
    char nome[30];
    int ativas = 0;

    for (int i = 0; i < F_ENTRADA; i++) {
        sprintf(nome, "fita_%d.bin", i);
        entrada[i].fptr = fopen(nome, "rb");
        if (entrada[i].fptr && lerRegistroBin(entrada[i].fptr, &entrada[i].reg)) {
            stats->leituras++;
            entrada[i].ativa = 1;
            entrada[i].runAtivo = 1; 
            ativas++;
        } else {
            entrada[i].ativa = 0;
            entrada[i].runAtivo = 0;
            if (entrada[i].fptr) { fclose(entrada[i].fptr); entrada[i].fptr = NULL; }
        }
    }

    if (ativas == 0) return 0;

    sprintf(nome, "fita_%d.bin", F_SAIDA_IDX);
    FILE *saida = fopen(nome, "wb");
    int runsGerados = 0;

    while (ativas > 0) {
        runsGerados++;
        int fitasNoRunAtual = 0;

        for (int i = 0; i < F_ENTRADA; i++) {
            if (entrada[i].ativa) {
                entrada[i].runAtivo = 1;
                fitasNoRunAtual++;
            }
        }

        Registro ultimo;

        while (fitasNoRunAtual > 0) {
            int menorIdx = -1;

            for (int i = 0; i < F_ENTRADA; i++) {
                if (entrada[i].ativa && entrada[i].runAtivo) {
                    if (menorIdx == -1) {
                        menorIdx = i;
                    } else {
                        stats->comparacoes++;
                        if (entrada[i].reg.nota < entrada[menorIdx].reg.nota) {
                            menorIdx = i;
                        }
                    }
                }
            }
            
            if (menorIdx == -1) break; 

            escreverRegistroBin(saida, &entrada[menorIdx].reg);
            stats->escritas++;
            ultimo = entrada[menorIdx].reg;

            Registro prox;
            if (lerRegistroBin(entrada[menorIdx].fptr, &prox)) {
                stats->leituras++;
                stats->comparacoes++;
                if (prox.nota < ultimo.nota) {
                    entrada[menorIdx].runAtivo = 0;
                    fitasNoRunAtual--;
                }
                entrada[menorIdx].reg = prox;
            } else {
                entrada[menorIdx].ativa = 0;
                entrada[menorIdx].runAtivo = 0;
                fclose(entrada[menorIdx].fptr); entrada[menorIdx].fptr = NULL;
                fitasNoRunAtual--;
                ativas--;
            }
        }
    }

    fclose(saida);
    return runsGerados;
}

//redistruibuição (1 fita -> 19 fitas)
void distribuirFitas(Estatisticas *stats) {
    char nome[30];
    sprintf(nome, "fita_%d.bin", F_SAIDA_IDX);
    FILE *origem = fopen(nome, "rb");
    if (!origem) return;

    FILE *destinos[F_ENTRADA];
    for (int i = 0; i < F_ENTRADA; i++) {
        sprintf(nome, "fita_%d.bin", i);
        destinos[i] = fopen(nome, "wb");
    }

    Registro reg, anterior;
    int fitaAtual = 0;

    if (lerRegistroBin(origem, &reg)) {
        stats->leituras++;
        escreverRegistroBin(destinos[fitaAtual], &reg);
        stats->escritas++;
        anterior = reg;

        while (lerRegistroBin(origem, &reg)) {
            stats->leituras++;
            stats->comparacoes++;
            if (reg.nota < anterior.nota) {
                fitaAtual = (fitaAtual + 1) % F_ENTRADA;
            }
            escreverRegistroBin(destinos[fitaAtual], &reg);
            stats->escritas++;
            anterior = reg;
        }
    }

    fclose(origem);
    for (int i = 0; i < F_ENTRADA; i++) fclose(destinos[i]);
    
    sprintf(nome, "fita_%d.bin", F_SAIDA_IDX);
    remove(nome); 
}

// ordenação externa por intercalação balanceada f+1
void intercalacaoBalanceadaFmais1(char *nomeArquivo, Estatisticas *stats) {
    int totalRuns = gerarRunsIniciais(nomeArquivo, stats);
    
    while (totalRuns > 1) {
        totalRuns = intercalarParaSaida(stats);
        if (totalRuns > 1) {
            distribuirFitas(stats);
        }
    }

    char nomeFinal[30];
    sprintf(nomeFinal, "fita_%d.bin", F_SAIDA_IDX);
    
    remove("fita_0.bin");
    rename(nomeFinal, "fita_0.bin");

    for (int i = 1; i < TOTAL_FITAS; i++) {
        sprintf(nomeFinal, "fita_%d.bin", i);
        remove(nomeFinal);
    }
}