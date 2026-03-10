#ifndef func_h
#define func_h

//Pedro Vítor Silva Cruz 24.2.4086
//Rafael Araújo Granato 24.2.4085

typedef struct tabuleiro Tab;
//  1.LeituraTabuleiro: inicializa tabuleiro a partir de dados do terminal.
//  2. TabuleiroEhValido: retorna se o tabuleiro é válido ou não.
//  3. Venceu: verifica se alguém já venceu.
//  4. Empate: verifica se houve empate no jogo.
//  5. JogadaMestre: retorna as possíveis jogadas mestres disponíveis

void LeituraTabuleiro(Tab* tabuleiro);
int TabuleiroEhValido(Tab* tabuleiro);
char Venceu(Tab* tabuleiro);
int Empate(Tab* tabuleiro);
int JogadaMestre(Tab* tabuleiro, int coordernadas[2][2]);
char proximoJogador(Tab* tabuleiro);

#endif // func_h