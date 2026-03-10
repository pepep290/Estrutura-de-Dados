#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "struct.h"
#include "abin.h"
#include "ASI.h"
#include "ArvoreB.h"
#include "Bestrela.h"
/*
    argv[0] = executavel
    argv[1] = metodo
    argv[2] = quantidadeq
    argv[3] = situação
    argv[4] = chave
    argv[5] = imprime opcional
*/

int main(int argc, char *argv[])
{

    FILE *arq; // abre o arquivo registros geral

    bool imprime = false, encontrado = false;
    int pesq_metodo, pesq_quantidade, pesq_situacao, pesq_chave;
    conts performance[2]; // 0 para preparação, 1 para pesquisa
    // inicializa contadores
    for (int i = 0; i < 2; i++)
    {
        performance[i].ntranf = 0;
        performance[i].ncomp = 0;
        performance[i].tempo = 0;
    }

    if (argc != 5 && argc != 6)
    {
        printf("ERRO\n Formato: executavel metodo(1 a 4) quantidade(100 a 2M) situacao(1 a 3) chave imprimir(opcional)\n");
        return 0;
    }
    else if (argc == 6)
    {
        imprime = true;
    }

    pesq_metodo = atoi(argv[1]);
    pesq_quantidade = atoi(argv[2]);
    pesq_situacao = atoi(argv[3]);
    pesq_chave = atoi(argv[4]);

    regs *x = malloc(sizeof(regs));
    x->chave = pesq_chave;

    printf("DEBUG: metodo=%d, quantidade=%d, situacao=%d, chave=%d\n", 
       pesq_metodo, pesq_quantidade, pesq_situacao, pesq_chave);

    // caso dos arquivos - completo
    switch (pesq_situacao)
    {
    case 1: // arq = arquivo crescente
        arq = fopen("crescente.bin", "rb");
        if (arq == NULL)
        {
            printf("Erro ao abrir o arquivo\n");
            return 1;
        }
        break;

    case 2: // arq = arquivo decrescente
        arq = fopen("decrescente.bin", "rb");
        if (arq == NULL)
        {
            printf("Erro ao abrir o arquivo\n");
            return 1;
        }
        break;

    case 3: // arq = arquivo aleatorio
        arq = fopen("aleatorio.bin", "rb");
        if (arq == NULL)
        {
            printf("Erro ao abrir o arquivo\n");
            return 1;
        }
        break;

    default:
        printf("ERRO\n Situacao de arquivo invalida");
        return 0;
        break;
    }

    regs temp;

    FILE *menor = fopen("reduzido.bin", "wb");
    if (menor == NULL)
    {
        printf("Erro ao criar arquivo reduzido.\n");
        return 1;
    }
    for (int i = 0; i < pesq_quantidade; i++)
    {
        if (fread(&temp, sizeof(regs), 1, arq) != 1)
            break;

        fwrite(&temp, sizeof(regs), 1, menor);
    }
    fclose(menor);
    
    menor = fopen("reduzido.bin", "rb");
    if (menor == NULL)
    {
        printf("Erro ao reabrir o arquivo reduzido.\n");
        return 1;
    }

    // caso de metodos - completo
    switch (pesq_metodo)
    {
    case 1:
        encontrado = sequencialIndexado(x, menor, performance);
        break;
    case 2:
        encontrado = ArvoreBinariaExt(x, menor, performance);
        break;
    case 3: // Arvore B
        encontrado = ArvoreB(x, menor, performance);
        break;
    case 4: // Arvore B*
        encontrado = ArvoreBEstrela(x, menor, performance);
        break;

    default:
        printf("ERRO\n Metodo de pesquisa invalido");
        return 0;
        break;
    }

    printf("\n========== Desempenho: ==========\n");
    printf("\nFase de preparacao\n");
    printf("Numero de transferencias: %ld\n", performance[0].ntranf);
    printf("Numero de comparacoes: %ld\n", performance[0].ncomp);
    printf("Tempo gasto: %.5lf segundos\n", performance[0].tempo);
    printf("\nFase de pesquisa\n");
    printf("Numero de transferencias: %ld\n", performance[1].ntranf);
    printf("Numero de comparacoes: %ld\n", performance[1].ncomp);
    printf("Tempo gasto: %.5lf segundos\n", performance[1].tempo);

    // chave foi encontrada?
    if (encontrado)
    {
        printf("\nA chave foi encontrada\n");
    }
    else
    {
        printf("\nA chave nao foi encontrada\n");
    }
    // caso opcional de impressao
    if (imprime)
    {
        printf("\nA chave pesquisada e %d!\n", pesq_chave);
    }
    fclose(menor);
    free(x);
    return 0;
}
