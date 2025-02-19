#include <stdio.h>
#include <stdlib.h>
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "hardware/irq.h"

// Definições de pinos
const int VRX = 26;
const int VRY = 27;
const int SW = 22;   // Botão SW (Joystick)
const int BTN_A = 5; // Botão A
const int LED_B = 12;
const int LED_R = 13;
const int LED_G = 11;
const float DIVIDER_PWM = 16.0;
const uint16_t PERIOD = 4096;
uint16_t led_b_level = 100, led_r_level = 100;
uint slice_led_b, slice_led_r;
volatile int led_g_state = 0;
volatile int pwm_enabled = 1;

// Variáveis para debounce
volatile uint32_t last_interrupt_time_a = 0, last_interrupt_time_sw = 0;
const uint32_t DEBOUNCE_DELAY = 200;  // 200ms

// Protótipos das funções
void setup();
void setup_joystick();
void setup_pwm_led(uint led, uint *slice, uint16_t level);
void joystick_read_axis(uint16_t *vrx_value, uint16_t *vry_value);
void setup_buttons();
void button_a_callback(uint gpio, uint32_t events);
void joystick_button_callback(uint gpio, uint32_t events);

void setup() {
    stdio_init_all();
    setup_joystick();
    setup_buttons();
    setup_pwm_led(LED_B, &slice_led_b, led_b_level);
    setup_pwm_led(LED_R, &slice_led_r, led_r_level);
    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_put(LED_G, led_g_state);
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
    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_FALL, true, &button_a_callback);

    // Configuração do botão SW (Joystick)
    gpio_init(SW);
    gpio_set_dir(SW, GPIO_IN);
    gpio_pull_up(SW);
    gpio_set_irq_enabled_with_callback(SW, GPIO_IRQ_EDGE_FALL, true, &joystick_button_callback);
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
    adc_select_input(1);
    *vrx_value = adc_read();
    
    adc_select_input(0);
    *vry_value = adc_read();
}

// Interrupção do Botão A
void button_a_callback(uint gpio, uint32_t events) {
    uint32_t current_time = time_us_32();
    if (current_time - last_interrupt_time_a > DEBOUNCE_DELAY * 1000) {
        last_interrupt_time_a = current_time;
        pwm_enabled = !pwm_enabled;

        if (!pwm_enabled) {
            pwm_set_gpio_level(LED_B, 0);
            pwm_set_gpio_level(LED_R, 0);
        }

        printf("Botão A pressionado! PWM: %s\n", pwm_enabled ? "ON" : "OFF");
    }
}

// Interrupção do Botão SW (Joystick)
void joystick_button_callback(uint gpio, uint32_t events) {
    uint32_t current_time = time_us_32();
    if (current_time - last_interrupt_time_sw > DEBOUNCE_DELAY * 1000) {
        last_interrupt_time_sw = current_time;
        led_g_state = !led_g_state;
        gpio_put(LED_G, led_g_state);
        printf("Botão SW pressionado! LED Verde: %d\n", led_g_state);
    }
}

int main() {
    uint16_t vrx_value, vry_value;
    setup();
    printf("Joystick Iniciado\n");

    while (true) {
        joystick_read_axis(&vrx_value, &vry_value);

        if (pwm_enabled) {
            int led_b_intensity = abs((int) vry_value - 2048) * 2;
            int led_r_intensity = abs((int) vrx_value - 2048) * 2;

            // Limita os valores de PWM
            if (led_b_intensity > 4095) led_b_intensity = 4095;
            if (led_r_intensity > 4095) led_r_intensity = 4095;

            pwm_set_gpio_level(LED_B, led_b_intensity);
            pwm_set_gpio_level(LED_R, led_r_intensity);
        }

        sleep_ms(50);
        printf("X: %u, Y: %u, PWM: %d, LED Verde: %d\n", vrx_value, vry_value, pwm_enabled, led_g_state);
    }
}
