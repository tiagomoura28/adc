#include <stdio.h>  
#include <stdlib.h>
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "ssd1306.h"
#include "pico/stdlib.h"
#include "font.h"

// Definições de pinos
#define I2C_PORT i2c1 
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define JOYSTICK_X_PIN 26  // GPIO para eixo X
#define JOYSTICK_Y_PIN 27  // GPIO para eixo Y
#define JOYSTICK_PB 22     // GPIO para botão do Joystick
#define Botao_A 5          // GPIO para botão A
#define LED_B 12
#define LED_R 13
#define LED_G 11
#define DIVIDER_PWM 16.0
#define PERIOD 4096

uint16_t led_b_level = 100, led_r_level = 100;
uint slice_led_b, slice_led_r;
volatile int led_b_state = 1, led_r_state = 1, led_g_state = 0;
volatile int pwm_enabled = 1;

// Variáveis para controle de debounce
volatile uint32_t last_interrupt_time_a = 0, last_interrupt_time_sw = 0;
const uint32_t DEBOUNCE_DELAY = 200;  // 200 ms de debounce

// Protótipos das funções
void setup_joystick();
void setup_pwm_led(uint led, uint *slice, uint16_t level);
void joystick_read_axis(uint16_t *vrx_value, uint16_t *vry_value);
void setup_buttons();

// Funções de interrupção
void button_interrupt_handler(uint gpio, uint32_t events);

void setup() {
    stdio_init_all();
    setup_joystick();
    setup_buttons();
    setup_pwm_led(LED_B, &slice_led_b, led_b_level);
    setup_pwm_led(LED_R, &slice_led_r, led_r_level);
    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_put(LED_G, led_g_state); // Garantir que o LED verde está no estado inicial
}

void setup_joystick() {
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
}

void setup_buttons() {
    // Configuração do botão A
    gpio_init(Botao_A);
    gpio_set_dir(Botao_A, GPIO_IN);
    gpio_pull_up(Botao_A);

    // Configuração do botão SW (Joystick)
    gpio_init(JOYSTICK_PB);
    gpio_set_dir(JOYSTICK_PB, GPIO_IN);
    gpio_pull_up(JOYSTICK_PB);

    // Configuração de interrupção para os botões
    gpio_set_irq_enabled_with_callback(Botao_A, GPIO_IRQ_EDGE_FALL, true, button_interrupt_handler);
    gpio_set_irq_enabled_with_callback(JOYSTICK_PB, GPIO_IRQ_EDGE_FALL, true, button_interrupt_handler);
}

void setup_pwm_led(uint led, uint *slice, uint16_t level) {
    gpio_set_function(led, GPIO_FUNC_PWM);
    *slice = pwm_gpio_to_slice_num(led);
    pwm_set_clkdiv(*slice, DIVIDER_PWM);
    pwm_set_wrap(*slice, PERIOD);
    pwm_set_gpio_level(led, level);
    pwm_set_enabled(*slice, true);
}

void joystick_read_axis(uint16_t *vrx_value, uint16_t *vry_value) {
    adc_select_input(1); // Eixo X
    *vrx_value = adc_read();
    
    adc_select_input(0); // Eixo Y
    *vry_value = adc_read();
}

void button_interrupt_handler(uint gpio, uint32_t events) {
    uint32_t current_time = time_us_32();

    // Filtro de debounce
    if (gpio == Botao_A && current_time - last_interrupt_time_a > DEBOUNCE_DELAY * 1000) {
        last_interrupt_time_a = current_time;
        pwm_enabled = !pwm_enabled;
        if (!pwm_enabled) {
            pwm_set_gpio_level(LED_B, 0);
            pwm_set_gpio_level(LED_R, 0);
        }
        printf("Botão A pressionado! PWM %s\n", pwm_enabled ? "ativado" : "desativado");
    }

    if (gpio == JOYSTICK_PB && current_time - last_interrupt_time_sw > DEBOUNCE_DELAY * 1000) {
        last_interrupt_time_sw = current_time;
        led_g_state = !led_g_state;
        gpio_put(LED_G, led_g_state); // Alteração apenas no botão SW
        printf("Botão SW pressionado! LED Verde %s\n", led_g_state ? "ligado" : "desligado");
    }
}

int main() {
    uint16_t vrx_value, vry_value;
    uint16_t square_x = 60, square_y = 28;  // Posição inicial do quadrado centralizada
    setup();
    printf("Joystick Iniciado\n");

    // I2C Inicialização. Usando a 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    // Limpa o display
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    char str_x[5];
    char str_y[5];
    bool cor = true;

    while (true) {
        // Limpa o display antes de atualizar
        ssd1306_fill(&ssd, !cor);

        // Desenha a borda externa (borda grossa ou normal)
        if (gpio_get(JOYSTICK_PB)) {
            // Borda grossa (quando o botão SW é pressionado)
            ssd1306_rect(&ssd, 1, 1, 126, 62, cor, false);  // Borda grossa
        } else {
            // Borda normal
            ssd1306_rect(&ssd, 0, 0, 128, 64, cor, false);  // Borda normal
        }

        // Desenha o quadrado que se move com o joystick
        ssd1306_rect(&ssd, square_y, square_x, 8, 8, cor, !cor);

        // Atualiza o display
        ssd1306_send_data(&ssd);

        // Leitura dos eixos do joystick
        joystick_read_axis(&vrx_value, &vry_value);

        // Atualiza a posição do quadrado
        square_x = (vrx_value / 4096.0) * (WIDTH - 8);  // Limita o movimento horizontal
        square_y = (4095 - vry_value) / 4096.0 * (HEIGHT - 8); // Limita o movimento vertical

       // square_y = (vry_value / 4096.0) * (HEIGHT - 8); // Limita o movimento vertical

        // Controle do brilho dos LEDs com PWM
        if (pwm_enabled) {
            int led_b_intensity = abs((int) vry_value - 2048) * 2;
            int led_r_intensity = abs((int) vrx_value - 2048) * 2;

            // Apagar LED se o valor estiver entre 2048 e 2088
            if (vrx_value >= 2048 && vrx_value <= 2128) {
                led_r_intensity = 0;
            }
            if (vry_value >= 2040 && vry_value <= 2090) {
                led_b_intensity = 0;
            }

            // Limitar os valores de PWM para não ultrapassarem o máximo
            if (led_b_intensity > 4095) led_b_intensity = 4095;
            if (led_r_intensity > 4095) led_r_intensity = 4095;

            pwm_set_gpio_level(LED_B, led_b_intensity);
            pwm_set_gpio_level(LED_R, led_r_intensity);
        }

        sleep_ms(100);
        printf("X: %u, Y: %u, Botão A: %d, LED Verde: %d\n", vrx_value, vry_value, gpio_get(Botao_A), led_g_state);
    }
}  
