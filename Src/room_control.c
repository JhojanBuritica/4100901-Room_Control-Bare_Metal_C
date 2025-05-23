#include "room_control.h"

#include "gpio.h"
#include "systick.h"
#include "uart.h"
#include "tim.h"

#define EXTERNAL_LED_PORT GPIOA
#define EXTERNAL_LED_PIN  7

static uint8_t led_active = 0;
static uint32_t led_start_tick = 0;

void room_control_app_init(void)
{
    // Asegurarse de que el LED esté apagado al inicio
    gpio_write_pin(EXTERNAL_LED_PORT, EXTERNAL_LED_PIN, 0);
}

void room_control_on_button_press(void)
{
    uart2_send_string("Boton B1: Presionado.\r\n");

    // Encender LED y guardar tiempo
    gpio_write_pin(EXTERNAL_LED_PORT, EXTERNAL_LED_PIN, 1);
    led_active = 1;
    led_start_tick = systick_get_tick();
}

void room_control_on_uart_receive(char received_char)
{
     static uint8_t pwm_state = 1; // 1 = ON, 0 = OFF

    switch (received_char) {
        case 'h':
        case 'H':
            tim3_ch1_pwm_set_duty_cycle(100); // LED al 100%
            pwm_state = 1;
            uart2_send_string("LED al 100%\r\n");
            break;

        case 'l':
        case 'L':
            tim3_ch1_pwm_set_duty_cycle(0);   // LED apagado
            pwm_state = 0;
            uart2_send_string("LED apagado\r\n");
            break;

        case 't':
            pwm_state = !pwm_state;
            tim3_ch1_pwm_set_duty_cycle(pwm_state ? 100 : 0);
            uart2_send_string(pwm_state ? "LED ON (toggle)\r\n" : "LED OFF (toggle)\r\n");
            break;

        default:
            uart2_send_string("Comando no reconocido\r\n");
            break;
    }
    }


void room_control_update(void)
{
    if (led_active) {
        if (systick_get_tick() - led_start_tick >= 3000) { // 3 segundos
            gpio_write_pin(EXTERNAL_LED_PORT, EXTERNAL_LED_PIN, 0);
            led_active = 0;
            uart2_send_string("LED externo apagado después de 3s.\r\n");
        }
    }
}
