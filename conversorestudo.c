#include <stdio.h>
#include <stdlib.h>
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"

// Definições de pinos
const int VRX = 26;
const int VRY = 27;
const int ADC_CHANNEL_0 = 0;
const int ADC_CHANNEL_1 = 1;
const int SW = 22;          // Botão SW (joystick)
const int BTN_A = 5;        // Botão A
const int BTN_B = 6;        // Botão B
const int LED_B = 12;
const int LED_R = 13;
const int LED_G = 11;
const float DIVIDER_PWM = 16.0;
const uint16_t PERIOD = 4096;
uint16_t led_b_level = 100, led_r_level = 100;
uint slice_led_b, slice_led_r;
volatile int led_b_state = 1, led_r_state = 1, led_g_state = 0;
volatile int pwm_enabled = 1;

// Variáveis para controle de debounce
volatile uint32_t last_interrupt_time_a = 0, last_interrupt_time_sw = 0, last_interrupt_time_b = 0;
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
    adc_gpio_init(VRX);
    adc_gpio_init(VRY);
}

void setup_buttons() {
    // Configuração do botão A
    gpio_init(BTN_A);
    gpio_set_dir(BTN_A, GPIO_IN);
    gpio_pull_up(BTN_A);

    // Configuração do botão SW (Joystick)
    gpio_init(SW);
    gpio_set_dir(SW, GPIO_IN);
    gpio_pull_up(SW);

    // Configuração do botão B
    gpio_init(BTN_B);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_B);

    // Configuração de interrupção para todos os botões
    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_FALL, true, button_interrupt_handler);
    gpio_set_irq_enabled_with_callback(SW, GPIO_IRQ_EDGE_FALL, true, button_interrupt_handler);
    gpio_set_irq_enabled_with_callback(BTN_B, GPIO_IRQ_EDGE_FALL, true, button_interrupt_handler);
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
    adc_select_input(ADC_CHANNEL_1);
    *vrx_value = adc_read();
    
    adc_select_input(ADC_CHANNEL_0);
    *vry_value = adc_read();
}

void button_interrupt_handler(uint gpio, uint32_t events) {
    uint32_t current_time = time_us_32();

    // Filtro de debounce
    if (gpio == BTN_A && current_time - last_interrupt_time_a > DEBOUNCE_DELAY * 1000) {
        last_interrupt_time_a = current_time;
        pwm_enabled = !pwm_enabled;
        if (!pwm_enabled) {
            pwm_set_gpio_level(LED_B, 0);
            pwm_set_gpio_level(LED_R, 0);
        }
        printf("Botão A pressionado! PWM %s\n", pwm_enabled ? "ativado" : "desativado");
    }

    if (gpio == SW && current_time - last_interrupt_time_sw > DEBOUNCE_DELAY * 1000) {
        last_interrupt_time_sw = current_time;
        led_g_state = !led_g_state;
        gpio_put(LED_G, led_g_state); // Alteração apenas no botão SW
        printf("Botão SW pressionado! LED Verde %s\n", led_g_state ? "ligado" : "desligado");
    }

    if (gpio == BTN_B && current_time - last_interrupt_time_b > DEBOUNCE_DELAY * 1000) {
        last_interrupt_time_b = current_time;
        printf("Botão B pressionado!\n");
    }
}

int main() {
    uint16_t vrx_value, vry_value;
    setup();
    printf("Joystick Iniciado\n");

    while (true) {
        // Leitura dos eixos do joystick
        joystick_read_axis(&vrx_value, &vry_value);

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
        printf("X: %u, Y: %u, Botão A: %d, Botão B: %d, LED Verde: %d\n", vrx_value, vry_value, gpio_get(BTN_A), gpio_get(BTN_B), led_g_state);
    }
}
