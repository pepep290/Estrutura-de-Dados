#ifndef ABIN_H
#define ABIN_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "struct.h"

typedef struct {
    regs valor;
    long int esq;   // filho esquerdo
    long int dir;   // filho direito
} Nodo;

void insereArvore(FILE* binTree, regs* novoDado, conts* performance);
FILE* criaArvore(FILE* arq, conts* performance);
bool ArvoreBinariaExt(regs* registro, FILE* arq, conts* performance);

#endif
