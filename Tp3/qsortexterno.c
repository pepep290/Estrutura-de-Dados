#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "qsortexterno.h"
#include "consolidado.h"

#define TRUE 1
#define FALSE 0
#define TAMAREA 10

// ==================== ESTRUTURAS DA ÁREA ====================

typedef struct NoQS {
    Registro reg;
    struct NoQS *prox;
} NoQS;

typedef struct {
    NoQS *inicio;
    NoQS *fim;
    int tamanho;
    int capacidade;
} TipoAreaQS;

// ==================== FUNÇÕES DA ÁREA (LISTA ENCADEADA ORDENADA) ====================

void FAVazia(TipoAreaQS *Area) {
    Area->inicio = NULL;
    Area->fim = NULL;
    Area->tamanho = 0;
    Area->capacidade = TAMAREA;
}

int ObterNumCelOcupadas(TipoAreaQS *Area) {
    return Area->tamanho;
}

void inserirArea(TipoAreaQS *Area, Registro *UltLido, int *NRArea, Estatisticas *stats) {
    if (Area->tamanho >= Area->capacidade) {
        return; // Área cheia
    }
    
    NoQS *novo = (NoQS *)malloc(sizeof(NoQS));
    if (!novo) return;
    
    novo->reg = *UltLido;
    novo->prox = NULL;
    
    // Inserção ordenada (crescente por nota)
    if (Area->inicio == NULL) {
        // Lista vazia
        Area->inicio = novo;
        Area->fim = novo;
    } else if (novo->reg.nota <= Area->inicio->reg.nota) {
        // Inserir no início
        stats->comparacoes++;
        novo->prox = Area->inicio;
        Area->inicio = novo;
    } else if (novo->reg.nota >= Area->fim->reg.nota) {
        // Inserir no final
        stats->comparacoes += 2; // Comparou com início e fim
        Area->fim->prox = novo;
        Area->fim = novo;
    } else {
        // Inserir no meio (busca posição)
        stats->comparacoes += 2; // Comparações anteriores
        NoQS *ant = Area->inicio;
        NoQS *atual = ant->prox;
        
        while (atual != NULL) {
            stats->comparacoes++;
            if (novo->reg.nota <= atual->reg.nota) {
                break;
            }
            ant = atual;
            atual = atual->prox;
        }
        
        ant->prox = novo;
        novo->prox = atual;
    }
    
    Area->tamanho++;
    *NRArea = Area->tamanho;
}

void RetiraMin(TipoAreaQS *Area, Registro *R, int *NRArea, Estatisticas *stats) {
    if (Area->inicio == NULL) {
        return; // Lista vazia
    }
    
    NoQS *temp = Area->inicio;
    *R = temp->reg;
    Area->inicio = temp->prox;
    
    if (Area->inicio == NULL) {
        Area->fim = NULL;
    }
    
    free(temp);
    Area->tamanho--;
    *NRArea = Area->tamanho;
}

void RetiraMax(TipoAreaQS *Area, Registro *R, int *NRArea, Estatisticas *stats) {
    if (Area->inicio == NULL) {
        return; // Lista vazia
    }
    
    if (Area->inicio == Area->fim) {
        // Apenas um elemento
        *R = Area->inicio->reg;
        free(Area->inicio);
        Area->inicio = NULL;
        Area->fim = NULL;
    } else {
        // Encontrar o penúltimo nó
        NoQS *ant = NULL;
        NoQS *atual = Area->inicio;
        
        while (atual->prox != NULL) {
            ant = atual;
            atual = atual->prox;
        }
        
        *R = atual->reg;
        ant->prox = NULL;
        Area->fim = ant;
        free(atual);
    }
    
    Area->tamanho--;
    *NRArea = Area->tamanho;
}

// ==================== FUNÇÕES AUXILIARES ====================

void LeSup(FILE **ArqLEs, Registro *UltLido, int *Ls, short *OndeLer, Estatisticas *stats) {
    fseek(*ArqLEs, (*Ls - 1) * sizeof(Registro), SEEK_SET);
    fread(UltLido, sizeof(Registro), 1, *ArqLEs);
    stats->leituras++;
    (*Ls)--;
    *OndeLer = FALSE;
}

void Leinf(FILE **ArqLi, Registro *UltLido, int *Li, short *OndeLer, Estatisticas *stats) {
    fread(UltLido, sizeof(Registro), 1, *ArqLi);
    stats->leituras++;
    (*Li)++;
    *OndeLer = TRUE;
}

void EscreveMax(FILE **ArqLEs, Registro R, int *Es, Estatisticas *stats) {
    fwrite(&R, sizeof(Registro), 1, *ArqLEs);
    stats->escritas++;
    (*Es)--;
}

void EscreveMin(FILE **ArqEi, Registro R, int *Ei, Estatisticas *stats) {
    fwrite(&R, sizeof(Registro), 1, *ArqEi);
    stats->escritas++;
    (*Ei)++;
}

// ==================== PARTICIONAMENTO ====================
void Particao(FILE **ArqLi, FILE **ArqEi, FILE **ArqLEs, int Esq, int Dir, 
              TipoAreaQS *Area, int *i, int *j, Estatisticas *stats) {
    int Ls = Dir, Es = Dir, Li = Esq, Ei = Esq, NRArea = 0;
    float Linf = -1.0e30, Lsup = 1.0e30;
    short OndeLer = TRUE;
    Registro UltLido, R;
    
    fseek(*ArqLi, (Li - 1) * sizeof(Registro), SEEK_SET);
    fseek(*ArqEi, (Ei - 1) * sizeof(Registro), SEEK_SET);
    *i = Esq - 1;
    *j = Dir + 1;
    
    while (Ls >= Li) {
        if (NRArea < TAMAREA - 1) {
            if (OndeLer)
                LeSup(ArqLEs, &UltLido, &Ls, &OndeLer, stats);
            else
                Leinf(ArqLi, &UltLido, &Li, &OndeLer, stats);
            inserirArea(Area, &UltLido, &NRArea, stats);
            continue;
        }
        
        if (Ls == Es)
            LeSup(ArqLEs, &UltLido, &Ls, &OndeLer, stats);
        else if (Li == Ei)
            Leinf(ArqLi, &UltLido, &Li, &OndeLer, stats);
        else if (OndeLer)
            LeSup(ArqLEs, &UltLido, &Ls, &OndeLer, stats);
        else
            Leinf(ArqLi, &UltLido, &Li, &OndeLer, stats);
        
        stats->comparacoes++;
        if (UltLido.nota > Lsup) {
            *j = Es;
            EscreveMax(ArqLEs, UltLido, &Es, stats);
            continue;
        }
        
        stats->comparacoes++;
        if (UltLido.nota < Linf) {
            *i = Ei;
            EscreveMin(ArqEi, UltLido, &Ei, stats);
            continue;
        }
        
        inserirArea(Area, &UltLido, &NRArea, stats);
        
        if (Ei - Esq < Dir - Es) {
            RetiraMin(Area, &R, &NRArea, stats);
            EscreveMin(ArqEi, R, &Ei, stats);
            Linf = R.nota;
        } else {
            RetiraMax(Area, &R, &NRArea, stats);
            EscreveMax(ArqLEs, R, &Es, stats);
            Lsup = R.nota;
        }
    }
    
    while (Ei <= Es) {
        RetiraMin(Area, &R, &NRArea, stats);
        EscreveMin(ArqEi, R, &Ei, stats);
    }
}

// ==================== QUICKSORT EXTERNO RECURSIVO ====================

void QuicksortExterno(FILE **ArqLi, FILE **ArqEi, FILE **ArqLEs, int Esq, int Dir, Estatisticas *stats) {
    int i, j;
    TipoAreaQS Area;

    if (Dir - Esq < 1) return;
    
    FAVazia(&Area);
    Particao(ArqLi, ArqEi, ArqLEs, Esq, Dir, &Area, &i, &j, stats);
    
    if (i - Esq < Dir - j) {
        QuicksortExterno(ArqLi, ArqEi, ArqLEs, Esq, i, stats);
        QuicksortExterno(ArqLi, ArqEi, ArqLEs, j, Dir, stats);
    } else {
        QuicksortExterno(ArqLi, ArqEi, ArqLEs, j, Dir, stats);
        QuicksortExterno(ArqLi, ArqEi, ArqLEs, Esq, i, stats);
    }
}

// ==================== FUNÇÃO WRAPPER PARA O MAIN ====================

// Substitua a função executarQuicksortExterno inteira por esta:

void quicksortExterno(char *nomeArquivo, Estatisticas *stats) {
    // 1. Cria um arquivo temporário de trabalho ÚNICO
    // O Quicksort Externo precisa ler e escrever no mesmo lugar para funcionar
    char *nomeTrabalho = "qsort_trabalho.bin";
    
    FILE *origem = fopen(nomeArquivo, "rb");
    FILE *trabalho = fopen(nomeTrabalho, "wb"); // Cria/Limpa o arquivo de trabalho
    
    if (!origem || !trabalho) {
        printf("Erro ao criar arquivo de trabalho.\n");
        if (origem) fclose(origem);
        if (trabalho) fclose(trabalho);
        return;
    }

    // Copia o conteúdo original para o arquivo de trabalho
    Registro reg;
    while (lerRegistroBin(origem, &reg)) {
        escreverRegistroBin(trabalho, &reg);
    }
    fclose(origem);
    fclose(trabalho);

    // 2. Abre TRES manipuladores para o MESMO arquivo de trabalho
    // Modo "r+b": Leitura e Escrita binária, sem apagar o conteúdo
    FILE *ArqLi = fopen(nomeTrabalho, "r+b");
    FILE *ArqEi = fopen(nomeTrabalho, "r+b");
    FILE *ArqLEs = fopen(nomeTrabalho, "r+b");

    if (!ArqLi || !ArqEi || !ArqLEs) {
        printf("Erro ao abrir manipuladores do arquivo de trabalho.\n");
        return;
    }

    // Descobre o tamanho
    fseek(ArqLi, 0, SEEK_END);
    long tamanho = ftell(ArqLi);
    int numRegistros = tamanho / sizeof(Registro);
    rewind(ArqLi); // Volta pro começo

    // 3. Executa o algoritmo
    // Como todos apontam para "qsort_trabalho.bin", o que ArqEi escreve,
    // ArqLi consegue ler nas chamadas recursivas.
    QuicksortExterno(&ArqLi, &ArqEi, &ArqLEs, 1, numRegistros, stats);

    // Fecha os manipuladores de trabalho
    fclose(ArqLi);
    fclose(ArqEi);
    fclose(ArqLEs);

    // 4. Copia o resultado final (ordenado) de volta para o arquivo original
    trabalho = fopen(nomeTrabalho, "rb");
    FILE *saida = fopen(nomeArquivo, "wb"); // Sobrescreve o original

    if (trabalho && saida) {
        while (lerRegistroBin(trabalho, &reg)) {
            escreverRegistroBin(saida, &reg);
        }
    } else {
        printf("Erro ao salvar resultado final.\n");
    }

    if (trabalho) fclose(trabalho);
    if (saida) fclose(saida);

    // Remove o temporário
    remove(nomeTrabalho);
    
    // Remove lixos antigos se existirem
    remove("qsort_ei.tmp");
    remove("qsort_les.tmp");
}