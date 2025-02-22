#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"

#define limiar_conducao_baixo 2120
#define limiar_conducao_alto 2020
#define pino_nivel_baixo 26
#define pino_nivel_alto 27

int pino_nivel_para_canal_adc(int pino) {
    switch (pino) {
        case pino_nivel_baixo:
            return 0;
        case pino_nivel_alto:
            return 1;
        default:
            return -1;
    }
}

typedef enum {
    NIVEL_CRITICO,
    NIVEL_BAIXO,
    NIVEL_ALTO,
    NIVEL_ERRO
} estado_nivel_t;


estado_nivel_t checar_nivel(uint16_t nivel_baixo, uint16_t nivel_alto, uint16_t limiar_baixo, uint16_t limiar_alto) {
    printf("Nível baixo: %d\n", nivel_baixo);
    printf("Nível alto: %d\n", nivel_alto);
    printf("Limiar baixo: %d\n", limiar_baixo);
    printf("Limiar alto: %d\n", limiar_alto);
    if (nivel_baixo > limiar_baixo && nivel_alto > limiar_alto) {
        return NIVEL_CRITICO;
    } else if (nivel_baixo < limiar_baixo && nivel_alto > limiar_alto) {
        return NIVEL_BAIXO;
    } else if (nivel_baixo < limiar_baixo && nivel_alto < limiar_alto) {
        return NIVEL_ALTO;
    } else {
        return NIVEL_ERRO;
    }
}

int main()
{
    stdio_init_all();

    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    // Example to turn on the Pico W LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    adc_init();
    adc_gpio_init(pino_nivel_baixo);
    adc_gpio_init(pino_nivel_alto);



    while (true) {
        adc_select_input(pino_nivel_para_canal_adc(pino_nivel_baixo));
        uint16_t nivel_baixo = adc_read();


        adc_select_input(pino_nivel_para_canal_adc(pino_nivel_alto));
        uint16_t nivel_alto = adc_read();

        estado_nivel_t estado = checar_nivel(nivel_baixo, nivel_alto, limiar_conducao_baixo, limiar_conducao_alto);
        if (estado == NIVEL_CRITICO) {
            printf("Nível crítico\n");
        } else if (estado == NIVEL_BAIXO) {
            printf("Nível baixo\n");
        } else if (estado == NIVEL_ALTO) {
            printf("Nível alto\n");
        } else {
            printf("Nível erro\n");
        }

        sleep_ms(1000);
    }
}
