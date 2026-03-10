#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "struct.h"
#include "ASI.h" 

int criaTabela(FILE* arq, tipoindice tabela[], conts* performance) {
    // marca tempo de início da criação da tabela
    clock_t comeco = clock();
    regs* x = malloc(sizeof(regs));
    int pos = 0; 

    while (pos < MAXTABELA) { // loop pra criar tabela de indices

        long int cont = pos * ITENSPAGINA * sizeof(regs);
        if (fseek(arq, cont, SEEK_SET) != 0) {//pula para a pagina correta
            // se fseek falhar ( pulou além do fim do arquivo), para
            break;
        }
        // conta o acesso a memoria secundaria na preparação
        performance[0].ntranf++;
        if (fread(x, sizeof(regs), 1, arq) != 1) {// le apenas a primeira chave de cada pagina
            // se não conseguir ler (provavelmente fim do arquivo), para
            break;
        }
        tabela[pos].chave = x->chave;
        tabela[pos].posicao = pos + 1; 
        pos++;
    }
    free(x);
    // marca tempo de fim da criação da tabela
    clock_t final = clock();
    performance[0].tempo = (long double)(final - comeco) / CLOCKS_PER_SEC;
    return pos;
}

bool sequencialIndexado(regs *registro, FILE *arq, conts* performance) {

    //declaração de variáveis necessárias
    regs pagina[ITENSPAGINA];
    tipoindice tab[MAXTABELA];
    int i = 0, quantreg, tam = criaTabela(arq, tab, performance);
    long desloc;
    clock_t comeco = clock();

    performance[1].ncomp++; // comparação da primeira iteração do while
    // procura pela página onde o registro pode se encontra
    while (i < tam && tab[i].chave <= registro->chave){
        i++;
        performance[1].ncomp++; // comparação da próxima iteração do while
    }
        
    if (i == 0){ // caso a chave desejada seja menor que a 1a chave, o registro não existe no arquivo
        return false;
    }   
    else {
        // a ultima página pode não estar completa
        if (i < tam)
            quantreg = ITENSPAGINA;
        else {
            // é a última página (i == tam), precisamos calcular seu tamanho real.
            fseek(arq, 0, SEEK_END);
            long total_regs = ftell(arq) / sizeof(regs);

            // calcula o resto
            quantreg = total_regs % ITENSPAGINA;

            // correção: se o resto é 0 E o arquivo não está vazio 
            // significa que a última página está cheia
            if (quantreg == 0 && total_regs > 0){
                quantreg = ITENSPAGINA;
            }
        }

        // lê a página desejada do arquivo
        desloc = (tab[i - 1].posicao - 1) * ITENSPAGINA * sizeof(regs);
        fseek(arq, desloc, SEEK_SET);
        performance[1].ntranf++; // conta o acesso a memoria secundaria na pesquisa
        fread(&pagina, sizeof(regs), quantreg, arq);
        
        // pesquisa BINARIA na página lida
        int inicio = 0;
        int fim = quantreg - 1;
        while (inicio <= fim) {
          int meio = inicio + (fim - inicio) / 2;
            
            performance[1].ncomp++; // conta a comparação de indice na pesquisa binaria
            if (pagina[meio].chave == registro->chave) {
                *registro = pagina[meio];
                // marca tempo de fim da pesquisa quando a chave é encontrada
                clock_t final = clock();
                performance[1].tempo += (long double)(final - comeco) / CLOCKS_PER_SEC;
                return true;
            }
            
            performance[1].ncomp++; // conta a comparação de indice na pesquisa binaria
            if (pagina[meio].chave < registro->chave){
                inicio = meio + 1;
            }
            else 
                fim = meio - 1;
        }
        // antiga -> 
        //  for (int j = 0; j < quantreg; j++)
        //     if (pagina[j].chave == registro->chave)
        //     {
        //         *registro = pagina[j];
        //         return true;
        //     }

        // marca tempo de fim da pesquisa quando a chave não é encontrada
        clock_t final = clock();
        performance[1].tempo = (long double)(final - comeco) / CLOCKS_PER_SEC;
        
        return false;
    }
} 
