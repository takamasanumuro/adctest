// Programa para leitura de uma placa de sensor de nível transistorizada com a Raspberry Pi Pico W

#include <stdio.h> // Inclui as funções de entrada e saída padrão como printf()
#include "pico/stdlib.h" // Inclui as funções essenciais para o Pico como stdio_init_all() e sleep_ms()
#include "hardware/adc.h" // Inclui as funções para o uso do ADC

/*Atenção: garantir que hardware_adc e pico_stdlib estejam incluídos no arquivo CMakeLists.txt em target_link_libraries,
caso contrário o VSCode não achará os caminhos de inclusão das bibliotecas
*/


/* Definição dos limiares obtidos experimentalmente para a detecção do nível do líquido
Valores abaixo do limiar indicam condutividade alta, enquanto valores acima do limiar indicam condutividade baixa 
*/

#define limiar_conducao_baixo 2120 // Para o sensor de nível baixo
#define limiar_conducao_alto 2020  // Para o sensor de nível alto

// Definição dos pinos conectados aos sensores de nível
#define pino_nivel_baixo 26
#define pino_nivel_alto 27

// Função para mapear pinos GPIO para os canais ADC correspondentes
int pino_nivel_para_canal_adc(int pino) {
    switch (pino) {
        case pino_nivel_baixo:
            return 0; // ADC0
        case pino_nivel_alto:
            return 1; // ADC1
        default:
            return -1; // Caso inválido
    }
}

// Função para calcular a média de várias leituras do ADC para reduzir ruído
uint16_t media_adc(int canal, int amostras) {
    uint32_t soma = 0;
    for (int i = 0; i < amostras; i++) {
        adc_select_input(canal);
        soma += adc_read();
        sleep_us(500);  // Pequeno atraso entre leituras
    }
    return soma / amostras;
}

// Enumeração para representar os diferentes estados do nível do líquido
typedef enum {
    NIVEL_CRITICO, // Nível muito alto
    NIVEL_BAIXO,   // Nível baixo
    NIVEL_ALTO,    // Nível normal
    NIVEL_ERRO     // Erro na leitura
} estado_nivel_t;

// Função para verificar o nível do líquido com base nos valores lidos
estado_nivel_t checar_nivel(uint16_t nivel_baixo, uint16_t nivel_alto, uint16_t limiar_baixo, uint16_t limiar_alto) {
    // Debug: imprimir valores lidos
    printf("Nível baixo: %d\n", nivel_baixo);
    printf("Nível alto: %d\n", nivel_alto);
    printf("Limiar baixo: %d\n", limiar_baixo);
    printf("Limiar alto: %d\n", limiar_alto);
    
    // Lógica de classificação do nível do líquido
    if (nivel_baixo > limiar_baixo && nivel_alto > limiar_alto) {
        return NIVEL_CRITICO; // Ambos os sensores estão sem contato com o líquido
    } else if (nivel_baixo < limiar_baixo && nivel_alto > limiar_alto) {
        return NIVEL_BAIXO; // Apenas o sensor de nível baixo está em contato com o líquido
    } else if (nivel_baixo < limiar_baixo && nivel_alto < limiar_alto) {
        return NIVEL_ALTO; // Ambos os sensores estão em contato com o líquido
    } else {
        return NIVEL_ERRO; // Estado inesperado
    }
}

int main()
{
    stdio_init_all(); // Inicializa a comunicação serial para debug

    adc_init(); // Inicializa o ADC
    adc_gpio_init(pino_nivel_baixo); // Configura o pino do sensor de nível baixo
    adc_gpio_init(pino_nivel_alto);  // Configura o pino do sensor de nível alto

    while (true) {

        #define NUMERO_AMOSTRAS 10
        uint16_t nivel_baixo = media_adc(pino_nivel_para_canal_adc(pino_nivel_baixo), NUMERO_AMOSTRAS);
        uint16_t nivel_alto = media_adc(pino_nivel_para_canal_adc(pino_nivel_alto), NUMERO_AMOSTRAS);

        // Determina o estado do nível do líquido
        estado_nivel_t estado = checar_nivel(nivel_baixo, nivel_alto, limiar_conducao_baixo, limiar_conducao_alto);
        
        // Exibe o estado no console
        if (estado == NIVEL_CRITICO) {
            printf("Nível crítico\n");
        } else if (estado == NIVEL_BAIXO) {
            printf("Nível baixo\n");
        } else if (estado == NIVEL_ALTO) {
            printf("Nível alto\n");
        } else {
            printf("Nível erro\n");
        }

        sleep_ms(1000); // Aguarda 1 segundo antes da próxima leitura
    }
}
