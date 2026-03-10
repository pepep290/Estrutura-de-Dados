#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "struct.h"
#include "abin.h"

void insereArvore(FILE *binTree, regs *novoDado, conts* performance)
{

    Nodo *nodoLido = malloc(sizeof(Nodo));
    if (nodoLido == NULL)
    {
        printf("Erro de malloc (nodoLido)\n");
        return;
    }

    long posAtual = 0; // começa na raiz (byte 0)
    long posPai = -1; // guarda quem é o pai do nó atual

    while (posAtual != -1)
    {
        posPai = posAtual; // atualiza o pai antes de descer
        fseek(binTree, posAtual, SEEK_SET);
        performance[0].ntranf++;
        fread(nodoLido, sizeof(Nodo), 1, binTree);

        // compara chaves e decide se vai para direita ou esquerda
        performance[0].ncomp++;
        if (novoDado->chave < nodoLido->valor.chave)
        {
            posAtual = nodoLido->esq;
        }
        else if (novoDado->chave > nodoLido->valor.chave)
        {
            performance[0].ncomp ++;
            posAtual = nodoLido->dir;
        }
        else // para chaves iguais
        {
            performance[0].ncomp ++;
            free(nodoLido); 
            return;
        }
    }

    // aloca o novo nó
    Nodo *novoNodo = malloc(sizeof(Nodo));
    if (novoNodo == NULL)
    {
        printf("Erro de malloc (novoNodo)\n");
        free(nodoLido); // Libera o buffer anterior
        return;
    }

    // cria o novo nó sem filhos
    novoNodo->valor = *novoDado;
    novoNodo->esq = -1;
    novoNodo->dir = -1;

    // calcula o novo final do arquivo
    fseek(binTree, 0, SEEK_END);
    long novaPos = ftell(binTree);

    // e escreve o nodo nesse final
    performance[0].ntranf++;
    fwrite(novoNodo, sizeof(Nodo), 1, binTree);
    
    free(novoNodo); 

    // atualizamos os campos filhos do nó
    performance[0].ncomp++;
    if (novoDado->chave < nodoLido->valor.chave)
    {
        nodoLido->esq = novaPos;
    }
    else
    {
        nodoLido->dir = novaPos;
    }

    // sobrescreve o pai com o nó atualizado
    fseek(binTree, posPai, SEEK_SET);
    performance[0].ntranf++;
    fwrite(nodoLido, sizeof(Nodo), 1, binTree);


    free(nodoLido);
}

// cria o arquivo da arvore binaria e o no raiz
FILE *criaArvore(FILE *arq, conts *performance)
{
    clock_t inicio, fim;
    inicio = clock();
    FILE *binTree = fopen("bintree.bin", "w+b");
    if (binTree == NULL)
    {
        printf("Erro ao criar o arquivo pra montar a arvore!\n");
        return NULL;
    }

    // le e escreve a raiz, o primeiro nodo da arvore binaria externa
    Nodo *raiz = malloc(sizeof(Nodo));
    performance[0].ntranf++;
    fread(&raiz->valor, sizeof(regs), 1, arq);
    // inicializa os filhos com -1
    raiz->esq = -1;
    raiz->dir = -1;
    performance[0].ntranf++;
    fwrite(raiz, sizeof(Nodo), 1, binTree);
    free(raiz);

    regs *filhos = malloc(sizeof(regs));

    performance[0].ntranf++;
    while (fread(filhos, sizeof(regs), 1, arq) == 1){ // loop para criar o arquivo e inserir os registros
        performance[0].ntranf++;
        insereArvore(binTree, filhos, performance);
    }

    free(filhos);
    // calcula o tempo final da criação da arvore binaria
    fim = clock();
    performance[0].tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;
    return binTree;
}

bool ArvoreBinariaExt(regs *registro, FILE *arq, conts *performance)
{
    FILE *binTree = criaArvore(arq, performance); // long int - sizeof(regs) - long int
    clock_t inicio, fim;
    inicio = clock();
    if (binTree == NULL)//caso o arq criado nao seja criado corretamente
    {
        printf("Nâo foi possível abrir o arquivo\n");
        return false;
    }

    Nodo busca;
    fseek(binTree, 0, SEEK_SET);
    long int pos = 0; // pos e a linha pra procurar(posicao do vetor)pra saber quantos pular
    while (pos != -1)
    {
        fseek(binTree, pos, SEEK_SET);

        performance[1].ntranf++;
        fread(&busca, sizeof(Nodo), 1, binTree);

        performance[1].ncomp++;
        if (busca.valor.chave == registro->chave)//retorna true se bater c a chave procurada
        {
            fclose(binTree);
            fim = clock();
            performance[1].tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;
            return true;
        }
        performance[1].ncomp++;
        if (registro->chave > busca.valor.chave)//se for maior anda pra direita da arvore
            {
            if (busca.dir == -1)
            { // se nao tiver nada pra andar p dir retorna falso
                fclose(binTree);
                fim = clock();
                performance[1].tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;
                return false;
            }
            pos = busca.dir;//atualiza a pos que vai olhar no proximo loop 
        }
        performance[1].ncomp++;
        if (registro->chave < busca.valor.chave)//anda pra esquerda se for menor
        {
            if (busca.esq == -1)
            { // se nao tiver nada pra andar p esq retorna falso
                fclose(binTree);
                fim = clock();
                performance[1].tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;
                return false;
            }
            pos = busca.esq;
        }
    }
    return false;
}
