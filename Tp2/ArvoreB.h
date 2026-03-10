#ifndef ARVOREB_H
#define ARVOREB_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "struct.h"
#define MM 40

typedef struct TipoPagina* TipoApontador;

typedef struct TipoPagina {
    short n;
    regs r[MM];
    TipoApontador p[MM + 1];
} TipoPagina;

void Inicializa(TipoApontador *Arvore);
bool Pesquisa(regs *x, TipoApontador Ap, conts* quantify, int medir_tempo);
void InsereNaPagina(TipoApontador Ap, regs Reg, TipoApontador ApDir, conts* quantify);
void Insere(regs Reg, TipoApontador *Ap, conts* quantify);
void InsereRec(regs Reg, TipoApontador Ap, short *Cresceu, regs *RegRetorno, TipoApontador *ApRetorno, conts* quantify);
bool ArvoreB(regs *registro, FILE *arq, conts* performance); 

#endif
