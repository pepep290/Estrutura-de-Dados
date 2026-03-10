#include "func.h"
#include "proto.c"
#include <stdio.h>
#include <stdlib.h>

int main(){
    int n;
    int coordenadas[2][2], count = 0;

    scanf("%d", &n); // lê a quantidade de jogos que serão avaliados
    Tab *tabuleiro = AlocaTabuleiros(n); // Cria um vetor[n] com uma tabela em cada casa

    for (int i = 0; i < n; i++){
        LeituraTabuleiro(&tabuleiro[i]); // le todas as posições
    }

    printf("\n");
    for (int i = 0; i < n; i++){ // percorre todos os jogos verificando sua situação 

        if (TabuleiroEhValido(&tabuleiro[i])){ // se for 0(válido), pula o if, caso seja 1(inválido) imprime 
            printf("Tabuleiro %d invalido\n", i + 1);
        }
        else if (Venceu(&tabuleiro[i]) != ' '){ 
            printf("Tabuleiro %d com vitoria [%c]\n", i + 1, Venceu(&tabuleiro[i]));
        }
        else if (Empate(&tabuleiro[i])){ // // Se for 0(ainda tem espaços vazios), pula o if, caso seja 1(não possui mais espaços vazios), o jogo dá velha
            printf("Tabuleiro %d deu velha\n", i + 1);
        }
        else{
            printf("Tabuleiro %d em andamento ", i + 1);
            if (proximoJogador(&tabuleiro[i]) == 'I'){
                printf("[proximo jogador indefinido]\n");
            }
            else{
                count = JogadaMestre(&tabuleiro[i], coordenadas);
                if(count != 0){
                    printf("[%c: ", proximoJogador(&tabuleiro[i]));
                    for(int i = 0; i < count; i++){
                        printf("(%d,%d)", coordenadas[i][0]+1, coordenadas[i][1]+1);
                    }
                    printf("]\n");
                }
                else{
                    printf("[%c: sem jogada mestre]\n", proximoJogador(&tabuleiro[i]));
                }
            }
        }
    }

    DesalocaTabuleiros(tabuleiro, n); // libera toda a memória alocada
    return 0;
}

