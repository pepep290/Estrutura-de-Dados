#ifndef BESTRELA_H
#define BESTRELA_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "struct.h" // Inclui regs e conts

#define M 40 // Ordem da árvore B Estrela

typedef enum { Interna, Externa } TipoPaginaa;

typedef struct TipoPaginaEstrela* TipoApEst;

typedef struct TipoPaginaEstrela {
    TipoPaginaa Pt; // Tipo da página: Interna ou Externa (Folha)
    union {
        // Estrutura para PÁGINAS INTERNAS (somente chaves e ponteiros)
        struct {
            int ni; // Número de chaves na página interna
            int ri[M]; // Chaves
            TipoApEst pi[M + 1]; // Ponteiros (filhos)
        } U0;
        // Estrutura para PÁGINAS EXTERNAS (registros completos)
        struct {
            int ne; // Número de registros na página externa
            regs re[M]; // Registros completos
            TipoApEst prox_folha; // Ponteiro para a próxima página folha
        } U1;
    } UU;
} TipoPaginaEstrela;

// Protótipos das funções
void InicializaEstrela(TipoApEst *Arvore);
void PesquisaEstrela(regs *x, TipoApEst Ap, conts* performance);
void InsereEstrela(regs Reg, TipoApEst *Ap, conts* performance);
void InsereRecEstrela(regs Reg, TipoApEst Ap, short *Cresceu, 
                      regs *RegRetorno, TipoApEst *ApRetorno, conts* performance);
void InsereNaPaginaEstrela(TipoApEst Ap, regs Reg, conts* performance);

bool ArvoreBEstrela(regs *registro, FILE *arq, conts performance[2]); 

#endif
