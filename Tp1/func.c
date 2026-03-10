#include "func.h"
#include <stdio.h>
#include <stdlib.h>

//Pedro Vítor Silva Cruz 24.2.4086
//Rafael Araújo Granato 24.2.4085

struct tabuleiro {  // nome deve bater com typedef em func.h
    char **tabuleiro; // matriz 3x3 de chars
};

Tab* AlocaTabuleiros(int n) {
    Tab *tabs = malloc(sizeof(Tab) * n);

    for (int i = 0; i < n; i++){
        tabs[i].tabuleiro = malloc(sizeof(char*) * 3);
        for (int j = 0; j < 3; j++){
            tabs[i].tabuleiro[j] = malloc(sizeof(char) * 3);
        }
    }
    return tabs;
}

void DesalocaTabuleiros(Tab *tab, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 3; j++) {
            free(tab[i].tabuleiro[j]);
        }
        free(tab[i].tabuleiro);
    }
    free(tab);
}

void LeituraTabuleiro(Tab* tab){
    char c;
    for (int linha = 0; linha < 3; linha++){
        for (int coluna = 0; coluna < 3;){
            if (scanf(" %c", &c) != 1) 
            continue;
            if (c == 'X' || c == 'O' || c == 'V'){
                tab->tabuleiro[linha][coluna] = c;
                coluna++;// só incrementa quando válido
            }
            // caso contrário, ignora e lê o próximo
        }
    }
}

int TabuleiroEhValido(Tab* tab){ // retorna 1 se inválido
    int contX = 0, contO = 0;
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            if (tab->tabuleiro[i][j] == 'X')
                contX++;
            else if (tab->tabuleiro[i][j] == 'O')
                contO++;
        }
    }
    if (contX > contO + 1 || contO > contX + 1)
        return 1;
    return 0;
}

char Venceu(Tab* tab){
    // Verifica linhas
    for (int i = 0; i < 3; i++){
        if (tab->tabuleiro[i][0] != 'V' 
            && tab->tabuleiro[i][0] == tab->tabuleiro[i][1] 
            && tab->tabuleiro[i][1] == tab->tabuleiro[i][2])
            return tab->tabuleiro[i][0];
    }
    // Verifica colunas
    for (int j = 0; j < 3; j++){
        if (tab->tabuleiro[0][j] != 'V' 
            && tab->tabuleiro[0][j] == tab->tabuleiro[1][j] 
            && tab->tabuleiro[1][j] == tab->tabuleiro[2][j])
            return tab->tabuleiro[0][j];
    }
    // Diagonal principal
    if (tab->tabuleiro[0][0] != 'V' 
        && tab->tabuleiro[0][0] == tab->tabuleiro[1][1] 
        && tab->tabuleiro[1][1] == tab->tabuleiro[2][2])
        return tab->tabuleiro[0][0];
    // Diagonal secundária
    if (tab->tabuleiro[0][2] != 'V' 
        && tab->tabuleiro[0][2] == tab->tabuleiro[1][1] 
        && tab->tabuleiro[1][1] == tab->tabuleiro[2][0])
        return tab->tabuleiro[0][2];
    return ' ';
}

int Empate(Tab* tab){
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            if (tab->tabuleiro[i][j] == 'V')
                return 0;
        }
    }
    return 1;
}

char proximoJogador(Tab* tab){
    int contX = 0, contO = 0;
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            if (tab->tabuleiro[i][j] == 'X')
                contX++;
            else if (tab->tabuleiro[i][j] == 'O')
                contO++;
        }
    }
    if (contX < contO)
        return 'X';
    else if (contX > contO)
        return 'O';
    else if (contX == contO)
        return 'I';
    return 'I';
}

int JogadaMestre(Tab* tab, int coordenadas[2][2]){
    char jogador = proximoJogador(tab);
    char aux;
    int count = 0;

    for(int i = 0; i < 3 && count < 2; i++){
        for(int j = 0; j < 3 && count < 2; j++){
            if(tab->tabuleiro[i][j] == 'V'){
                aux = tab->tabuleiro[i][j];
                tab->tabuleiro[i][j] = jogador; 
                if(Venceu(tab) == jogador){
                    coordenadas[count][0] = i;
                    coordenadas[count][1] = j;
                    count++;
                }
                tab->tabuleiro[i][j] = aux; // restaura a posição original
            }
        }
    }

    return count; // número de jogadas mestres encontradas (0, 1 ou 2)
}
