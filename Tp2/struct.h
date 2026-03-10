#ifndef STRUCT_H
#define STRUCT_H

#include <time.h>

// definição de um item do arquivo de dados
typedef struct Registros {
    int chave; // valor de pesquisa
    long int dado1; 
    char dado2[5000]; 
} regs;

// contadores para análise
typedef struct{
    long int ntranf;//numero de tranferencias
    long int ncomp;//numero de comparaçoes
    double tempo;//tempo
} conts;

// Função para calcular tempo
static inline void calcTemp(clock_t inicio, clock_t fim, conts *controle) {
    controle->tempo = (long double)(fim - inicio) / CLOCKS_PER_SEC;
}

#endif