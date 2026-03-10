#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "struct.h" // Inclui as definições de regs e calcTemp

#define TOTAL 2000000 // Total de registros a serem gerados

// Função para embaralhar um array (Fisher-Yates)
void shuffle(int *array, int n) {
    if (n > 1) {
        for (int i = 0; i < n - 1; i++) {
            // Garante que o índice j está entre i e n-1
            int j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

// Função principal para gerar os 3 arquivos (crescente, decrescente, aleatório)
void gerar_arquivos() {

    // Nomes dos arquivos ajustados para o esperado no main.c
    FILE *asc = fopen("crescente.bin", "wb");
    FILE *desc = fopen("decrescente.bin", "wb");
    FILE *randf = fopen("aleatorio.bin", "wb");

    if (!asc || !desc || !randf) {
        printf("Erro ao abrir arquivos para escrita.\n");
        // Tenta fechar os que foram abertos antes de sair
        if (asc) fclose(asc);
        if (desc) fclose(desc);
        if (randf) fclose(randf);
        exit(1);
    }

    // 1. Geração do array aleatório
    int *array = malloc(TOTAL * sizeof(int));
    if (!array) {
        printf("Erro de alocação de memória.\n");
        exit(1);
    }
    for (int i = 0; i < TOTAL; i++) array[i] = i + 1;
    
    srand(time(NULL));
    shuffle(array, TOTAL); // Array com chaves de 1 a TOTAL em ordem aleatória

    // 2. Preenchimento dos registros e escrita nos arquivos
    regs reg;
    // Preenche o campo de dados com uma string fixa (5000 bytes)
    memset(reg.dado2, 'A', sizeof(reg.dado2) - 1);
    reg.dado2[sizeof(reg.dado2) - 1] = '\0'; // Garantir terminação da string

    for (int i = 0; i < TOTAL; i++) {
        // Arquivo Crescente
        reg.chave = i + 1;
        reg.dado1 = reg.chave * 10;
        fwrite(&reg, sizeof(regs), 1, asc);

        // Arquivo Decrescente
        reg.chave = TOTAL - i;
        reg.dado1 = reg.chave * 10;
        fwrite(&reg, sizeof(regs), 1, desc);

        // Arquivo Aleatório
        reg.chave = array[i];
        reg.dado1 = reg.chave * 10;
        fwrite(&reg, sizeof(regs), 1, randf);
    }

    // 3. Limpeza
    fclose(asc);
    fclose(desc);
    fclose(randf);
    free(array);

    printf("Arquivos gerados com sucesso! (crescente.bin, decrescente.bin, aleatorio.bin)\n");
}

int main() {
    gerar_arquivos();
    printf("Execução concluída.\n");
    return 0;
}
