#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "intercalacao2f.h"
#include "consolidado.h"

#define F 10

typedef struct {
    Registro reg;
    int fita_origem;
    int ativo;
    int run_terminado;
} ItemMerge;

void heapify(ItemMerge *v, int n, int i, Estatisticas *stats) {
    int menor = i;
    int esq = 2 * i + 1;
    int dir = 2 * i + 2;
    
    if (esq < n) {
        stats->comparacoes++;
        if (v[esq].reg.nota < v[menor].reg.nota) menor = esq;
    }
    if (dir < n) {
        stats->comparacoes++;
        if (v[dir].reg.nota < v[menor].reg.nota) menor = dir;
    }
    
    if (menor != i) {
        ItemMerge temp = v[i];
        v[i] = v[menor];
        v[menor] = temp;
        heapify(v, n, menor, stats);
    }
}

void gerarRuns(char *nomeArquivo, Estatisticas *stats, int *numRuns) {
    FILE *entrada = fopen(nomeArquivo, "rb");
    if (!entrada) return;
    
    FILE *fitas[F];
    char nomeFita[20];
    ElementoHeap memoria[F];
    int n = 0; // ocupação atual do heap
    int heapSize = 0; // elementos não congelados
    Registro regLido;
    
    for (int i = 0; i < F; i++) {
        sprintf(nomeFita, "fita_%d.bin", i);
        fitas[i] = fopen(nomeFita, "wb");
    }
    
    // Encher a memória inicialmente
    while (n < F && lerRegistroBin(entrada, &regLido)) {
        stats->leituras++;
        memoria[n].reg = regLido;
        memoria[n].congelado = 0;
        n++;
    }
    heapSize = n;
    construirMinHeap(memoria, heapSize, stats);
    
    int idxFitaAtual = 0;
    int totalRuns = 0;
    if (n > 0) totalRuns = 1;
    
    Registro ultimoEscrito;
    ultimoEscrito.nota = -1.0; 
    
    while (n > 0) {
        if (heapSize == 0) {
            // Descongelar todos para próximo run
            for (int i = 0; i < n; i++) memoria[i].congelado = 0;
            heapSize = n;
            construirMinHeap(memoria, heapSize, stats);
            
            idxFitaAtual = (idxFitaAtual + 1) % F;
            totalRuns++;
            ultimoEscrito.nota = -1.0;
        }

        Registro menor = memoria[0].reg;
        escreverRegistroBin(fitas[idxFitaAtual], &menor);
        stats->escritas++;
        ultimoEscrito = menor;

        if (lerRegistroBin(entrada, &regLido)) {
            stats->leituras++;
            stats->comparacoes++;
            
            if (regLido.nota < ultimoEscrito.nota) {
                // Congela o novo registro - lógica corrigida
                if (heapSize > 0) {
                    // Remove o menor do heap e move para posição de congelado
                    memoria[heapSize - 1].reg = regLido;
                    memoria[heapSize - 1].congelado = 1;
                    heapSize--;
                    if (heapSize > 0) {
                        refazerMinHeap(memoria, 0, heapSize, stats);
                    }
                }
            } else {
                // Substitui o menor elemento no heap
                memoria[0].reg = regLido;
                memoria[0].congelado = 0;
                refazerMinHeap(memoria, 0, heapSize, stats);
            }
        } else {
            // Remove o menor elemento
            memoria[0] = memoria[heapSize - 1];
            heapSize--;
            n--;
            if (heapSize > 0) refazerMinHeap(memoria, 0, heapSize, stats);
        }
    }
    
    for (int i = 0; i < F; i++) {
        fclose(fitas[i]);
    }
    fclose(entrada);
    
    *numRuns = totalRuns;
}

void intercalarRuns(int numRuns, Estatisticas *stats) {
    FILE *fitas[2 * F];
    char nomeFita[20];
    int baseEntrada = 0, baseSaida = F;
    
    while (numRuns > 1) {
        // Abrir fitas de entrada e saída
        for (int i = 0; i < F; i++) {
            sprintf(nomeFita, "fita_%d.bin", baseEntrada + i);
            fitas[baseEntrada + i] = fopen(nomeFita, "rb");
            sprintf(nomeFita, "fita_%d.bin", baseSaida + i);
            fitas[baseSaida + i] = fopen(nomeFita, "wb");
        }
        
        int runsProcessados = 0;
        int novosRuns = 0;
        
        // Processar grupos de F runs
        while (runsProcessados < numRuns) {
            ItemMerge heap[F];
            int nHeap = 0;
            int fitaDestino = novosRuns % F;
            int fitasAtivas = 0;
            Registro ultimoEscrito;
            ultimoEscrito.nota = -1.0;
            
            // Inicializar heap com primeiro registro de cada fita do grupo
            for (int i = 0; i < F && runsProcessados + i < numRuns; i++) {
                if (lerRegistroBin(fitas[baseEntrada + i], &heap[nHeap].reg)) {
                    stats->leituras++;
                    heap[nHeap].fita_origem = i;
                    heap[nHeap].ativo = 1;
                    heap[nHeap].run_terminado = 0;
                    nHeap++;
                    fitasAtivas++;
                }
            }
            
            // Intercalar enquanto houver elementos no heap
            int heap_construido = 0;
            while (nHeap > 0) {
                // Construir heap mínimo apenas uma vez no início
                // Depois manter propriedade com refazerMinHeap
                if (!heap_construido) {
                    for (int i = nHeap/2 - 1; i >= 0; i--) {
                        heapify(heap, nHeap, i, stats);
                    }
                    heap_construido = 1;
                }
                
                // Verificar se o menor elemento pode ser escrito no run atual
                if (heap[0].reg.nota >= ultimoEscrito.nota) {
                    escreverRegistroBin(fitas[baseSaida + fitaDestino], &heap[0].reg);
                    stats->escritas++;
                    ultimoEscrito = heap[0].reg;
                    
                    // Ler próximo registro da mesma fita
                    int origem = heap[0].fita_origem;
                    if (lerRegistroBin(fitas[baseEntrada + origem], &heap[0].reg)) {
                        stats->leituras++;
                        // Verificar se ainda está no mesmo run
                        if (heap[0].reg.nota < ultimoEscrito.nota) {
                            // Run terminou, marcar como inativo
                            heap[0].ativo = 0;
                            heap[0] = heap[--nHeap];
                            fitasAtivas--;
                            // Refazer heap apenas para a raiz
                            if (nHeap > 0) {
                                heapify(heap, nHeap, 0, stats);
                            }
                        } else {
                            // Manter propriedade do heap
                            heapify(heap, nHeap, 0, stats);
                        }
                    } else {
                        // Fita terminou
                        heap[0].ativo = 0;
                        heap[0] = heap[--nHeap];
                        fitasAtivas--;
                        // Refazer heap apenas para a raiz
                        if (nHeap > 0) {
                            heapify(heap, nHeap, 0, stats);
                        }
                    }
                } else {
                    // Run atual terminou, começar novo run
                    fitaDestino = (novosRuns + 1) % F;
                    novosRuns++;
                    ultimoEscrito.nota = -1.0;
                    heap_construido = 0; // Reconstruir heap para novo run
                }
            }
            
            runsProcessados += F;
        }
        
        // Fechar todas as fitas
        for (int i = 0; i < 2 * F; i++) {
            if (fitas[i]) fclose(fitas[i]);
        }
        
        // Limpar fitas de entrada antigas
        for (int i = 0; i < F; i++) {
            sprintf(nomeFita, "fita_%d.bin", baseEntrada + i);
            remove(nomeFita);
        }
        
        numRuns = novosRuns;
        int temp = baseEntrada;
        baseEntrada = baseSaida;
        baseSaida = temp;
    }
    
    // Mover resultado final para fita_0.bin
    for (int i = 0; i < F; i++) {
        sprintf(nomeFita, "fita_%d.bin", baseEntrada + i);
        if (rename(nomeFita, "fita_0.bin") == 0) break;
    }
}

void intercalacaoBalanceada2f(char *nomeArquivo, Estatisticas *stats) {
    int numRuns = 0;
    
    // Verificar se arquivo existe
    FILE *check = fopen(nomeArquivo, "rb");
    if (!check) {
        printf("ERRO: Arquivo de entrada %s não encontrado\n", nomeArquivo);
        return;
    }
    fclose(check);
    
    gerarRuns(nomeArquivo, stats, &numRuns);
    
    // Verificar se runs foram gerados
    if (numRuns <= 0) {
        printf("ERRO: Nenhum run foi gerado\n");
        return;
    }
    
    intercalarRuns(numRuns, stats);
    
    // Verificar se resultado final foi criado
    FILE *resultado = fopen("fita_0.bin", "rb");
    if (!resultado) {
        printf("AVISO: Arquivo de resultado fita_0.bin não foi criado\n");
    } else {
        fclose(resultado);
    }
}
