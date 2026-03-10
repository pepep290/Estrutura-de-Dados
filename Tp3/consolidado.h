#ifndef CONSOLIDADO_H
#define CONSOLIDADO_H

#include <stdio.h>

// Antigo "medidores.h"
typedef struct {
    long leituras;
    long escritas;
    long comparacoes;
    double tempo_execucao;
} Estatisticas;

static inline void inicializarEstatisticas(Estatisticas *stats) {
    stats->leituras = 0;
    stats->escritas = 0;
    stats->comparacoes = 0;
    stats->tempo_execucao = 0.0;
}

// Antigo "registros.h"
typedef struct {
    long int inscricao;
    float nota;
    char estado[3];   // 2 caracteres + '\0'
    char cidade[51];  // 50 caracteres + '\0'
    char curso[31];   // 30 caracteres + '\0'
} Registro;

int lerRegistroTexto(FILE *arq, Registro *reg);
static inline int lerRegistroBin(FILE *arq, Registro *reg) {
    return fread(reg, sizeof(Registro), 1, arq) == 1;
}

static inline int escreverRegistroBin(FILE *arq, Registro *reg) {
    return fwrite(reg, sizeof(Registro), 1, arq) == 1;
}

static inline int compararRegistros(const void *a, const void *b) {
    Registro *regA = (Registro *)a;
    Registro *regB = (Registro *)b;
    if (regA->nota < regB->nota) return -1;
    if (regA->nota > regB->nota) return 1;
    return 0;
}

// Antigo "utils.h"
typedef struct {
    Registro reg;
    int congelado;
} ElementoHeap;

void refazerMinHeap(ElementoHeap *v, int esq, int dir, Estatisticas *stats);
void construirMinHeap(ElementoHeap *v, int n, Estatisticas *stats);
void limparFitas(int totalFitas);

#endif
