#ifndef ASI_H
#define ASI_H
#include <stdio.h>
#include <stdbool.h>
#include "struct.h"
#define ITENSPAGINA 400
// muda com a quantidade de registros no arquivo
#define MAXTABELA 1200

// definição de uma entrada da tabela de índice das páginas
typedef struct {
    int posicao;
    int chave;
} tipoindice;

bool sequencialIndexado(regs *registro, FILE *arq, conts* performance);
int criaTabela(FILE *arq, tipoindice tabela[], conts* performance);
#endif


