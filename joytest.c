#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"    // Biblioteca para o display SSD1306
#include "inc/font.h"
#include <stdio.h>

// Definições dos pinos para o ADC (joystick)
#define ADC_PIN_X 26   // GP26 -> ADC0 (eixo X)
#define ADC_PIN_Y 27   // GP27 -> ADC1 (eixo Y)
#define BOTJ  22      // Pino do botão do joystick
#define BOTA 5        // Pino do botão A

// Definições dos pinos para os LEDs
#define LED_RED_PIN 13   // LED vermelho (PWM) no GPIO13
#define LED_BLUE_PIN 12  // LED azul (PWM) no GPIO12
#define LED_G 11         // Pino do LED Verde 

#define DEBOUNCE_DELAY_US 350000  // Tempo de debounce em microssegundos

// Configurações para o display SSD1306 via I2C
#define I2C_PORT i2c1
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15
#define SSD1306_I2C_ADDRESS 0x3C   // Endereço comum para o SSD1306
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT   64

// Estados possíveis da borda
typedef enum {
    NO_BORDER = 0,
    SINGLE_BORDER,
    DOUBLE_BORDER,
    BORDER_COUNT  // Número total de estilos de borda
} border_style_t;

// Flags e variáveis globais
volatile bool botj_pressed = false;  // Flag para o botão do joystick
volatile bool bota_pressed = false;  // Flag para o botão A
border_style_t current_border = NO_BORDER;  // Estilo da borda atual
bool ledg_state = false;            // Estado do LED Verde
bool leds_enabled = true;           // Estado dos LEDs PWM (vermelho e azul)
ssd1306_t display;                  // Instância global do display SSD1306

// Função de interrupção para os botões (BOTJ e BOTA)
void button_callback(uint gpio, uint32_t events) {
    // Desativa a interrupção temporariamente para debouncing
    gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, false);
    static uint64_t last_interrupt_time = 0;
    uint64_t current_time = time_us_64();

    if ((current_time - last_interrupt_time) > DEBOUNCE_DELAY_US) {
        if (gpio == BOTJ) {
            botj_pressed = true;
        } else if (gpio == BOTA) {
            bota_pressed = true;
        }
        last_interrupt_time = current_time;
    }
    // Reativa a interrupção
    gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, true);
}

int main() {
    stdio_init_all();
    sleep_ms(100); // Aguarda para estabilizar

    // Configuração dos botões BOTJ e BOTA com a mesma callback
    gpio_init(BOTJ);
    gpio_set_dir(BOTJ, GPIO_IN);
    gpio_pull_up(BOTJ);
    gpio_set_irq_enabled_with_callback(BOTJ, GPIO_IRQ_EDGE_FALL, true, &button_callback);

    gpio_init(BOTA);
    gpio_set_dir(BOTA, GPIO_IN);
    gpio_pull_up(BOTA);
    gpio_set_irq_enabled_with_callback(BOTA, GPIO_IRQ_EDGE_FALL, true, &button_callback);

    // Configuração do LED Verde
    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_put(LED_G, ledg_state);

    // Inicializa o ADC e configura os pinos
    adc_init();
    adc_gpio_init(ADC_PIN_X);
    adc_gpio_init(ADC_PIN_Y);

    // Configura os pinos dos LEDs para PWM
    gpio_set_function(LED_RED_PIN, GPIO_FUNC_PWM);
    gpio_set_function(LED_BLUE_PIN, GPIO_FUNC_PWM);
    uint slice_red = pwm_gpio_to_slice_num(LED_RED_PIN);
    uint slice_blue = pwm_gpio_to_slice_num(LED_BLUE_PIN);
    pwm_set_wrap(slice_red, 4095);
    pwm_set_wrap(slice_blue, 4095);
    pwm_set_enabled(slice_red, true);
    pwm_set_enabled(slice_blue, true);

    // Configuração do I2C para o display
    i2c_init(I2C_PORT, 400 * 1000);  // I2C a 400 kHz
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // Inicializa o display SSD1306
    ssd1306_init(&display, SCREEN_WIDTH, SCREEN_HEIGHT, false, SSD1306_I2C_ADDRESS, I2C_PORT);
    ssd1306_config(&display);
    ssd1306_fill(&display, false);
    ssd1306_send_data(&display);

    while (true) {
        // Leitura dos canais ADC para os eixos do joystick
        adc_select_input(0);
        uint16_t adc_x = adc_read();
        adc_select_input(1);
        uint16_t adc_y = adc_read();

        // Mapeamento dos valores do ADC para a posição de um quadrado (8x8 pixels)
        int square_size = 8;
        int max_x_pos = 64 - square_size;
        int max_y_pos = 128 - square_size;
        int pos_x = max_x_pos - ((adc_x * max_x_pos) / 4095);
        int pos_y = (adc_y * max_y_pos) / 4095;



        

        //int pos_x = (adc_x * max_x_pos) / 4095; 
        //int pos_y = max_y_pos - ((adc_y * max_y_pos) / 4095); // Corrigido para inverter o eixo Y
        //int pos_x = max_x_pos - ((adc_x * max_x_pos) / 4095); // Corrigido para inverter o eixo X
        //int pos_y = (adc_y * max_y_pos) / 4095;







        // Atualiza os níveis do PWM dos LEDs vermelho e azul somente se estiverem habilitados
        if (leds_enabled) {
            pwm_set_gpio_level(LED_RED_PIN, adc_x);
            pwm_set_gpio_level(LED_BLUE_PIN, adc_y);
        } else {
            pwm_set_gpio_level(LED_RED_PIN, 0);
            pwm_set_gpio_level(LED_BLUE_PIN, 0);
        }

        // Processa o evento do botão do joystick (BOTJ)
        if (botj_pressed) {
            botj_pressed = false;
            // Alterna o estado do LED Verde
            ledg_state = !ledg_state;
            gpio_put(LED_G, ledg_state);
            // Alterna o estilo da borda
            current_border = (current_border + 1) % BORDER_COUNT;
        }

        // Processa o evento do botão A (BOTA) para ligar/desligar os LEDs vermelho e azul
        if (bota_pressed) {
            bota_pressed = false;
            leds_enabled = !leds_enabled;
        }

        // Atualiza o display:
        // 1. Limpa o display
        ssd1306_fill(&display, false);
        // 2. Desenha o quadrado que representa o joystick
        ssd1306_rect(&display, pos_x, pos_y, square_size, square_size, true, true);
        // 3. Desenha a borda, se o estilo não for NO_BORDER
        if (current_border == SINGLE_BORDER) {
            ssd1306_rect(&display, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, true, false);
        } else if (current_border == DOUBLE_BORDER) {
            ssd1306_rect(&display, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, true, false);
            ssd1306_rect(&display, 2, 2, SCREEN_WIDTH - 4, SCREEN_HEIGHT - 4, true, false);
        }
        ssd1306_send_data(&display);

        sleep_ms(50); // Delay para reduzir flickering e permitir o debouncing
    }

    return 0;
}
