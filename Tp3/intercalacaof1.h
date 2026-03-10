#ifndef INTERCALACAOF1_H
#define INTERCALACAOF1_H

#include "consolidado.h"

//20 fitas no total (f+1), sendo 19 entrada e 1 saída.
//memória interna de 19 registros.

#define F_ENTRADA 19 
#define F_SAIDA_INDICE 19 
#define F_SAIDA_IDX 19
#define TOTAL_FITAS 20 
#define TAM_MEMORIA 19 

void intercalacaoBalanceadaFmais1(char *nomeArquivo, Estatisticas *stats);

#endif
