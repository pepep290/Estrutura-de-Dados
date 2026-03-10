#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "consolidado.h"

// Antigo "registros.c"
int lerRegistroTexto(FILE *arq, Registro *reg) {
    char linha[110];
    if (fgets(linha, sizeof(linha), arq) == NULL) return 0;

    char tempInsc[9], tempNota[6], tempEstado[3], tempCidade[51], tempCurso[31];

    strncpy(tempInsc, &linha[0], 8); tempInsc[8] = '\0';
    strncpy(tempNota, &linha[9], 5); tempNota[5] = '\0';
    strncpy(tempEstado, &linha[15], 2); tempEstado[2] = '\0';
    strncpy(tempCidade, &linha[18], 50); tempCidade[50] = '\0';
    strncpy(tempCurso, &linha[69], 30); tempCurso[30] = '\0';

    reg->inscricao = atol(tempInsc);
    reg->nota = atof(tempNota);
    strcpy(reg->estado, tempEstado);
    strcpy(reg->cidade, tempCidade);
    strcpy(reg->curso, tempCurso);

    return 1;
}

// Antigo "utils.c"
void refazerMinHeap(ElementoHeap *v, int esq, int dir, Estatisticas *stats) {
    int i = esq;
    int j = i * 2 + 1;
    ElementoHeap aux = v[i];

    while (j < dir) {
        if (j < dir - 1) {
            stats->comparacoes++;
            if (v[j].reg.nota > v[j + 1].reg.nota) {
                j++;
            }
        }
        stats->comparacoes++;
        if (aux.reg.nota <= v[j].reg.nota) {
            break;
        }
        v[i] = v[j];
        i = j;
        j = i * 2 + 1;
    }
    v[i] = aux;
}

void construirMinHeap(ElementoHeap *v, int n, Estatisticas *stats) {
    for (int esq = (n / 2) - 1; esq >= 0; esq--) {
        refazerMinHeap(v, esq, n, stats);
    }
}

void limparFitas(int totalFitas) {
    char nomeFita[20];
    for (int i = 0; i < totalFitas; i++) {
        sprintf(nomeFita, "fita_%d.bin", i);
        remove(nomeFita);
    }
}
