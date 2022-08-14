/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

// WS2812 LED
#define IS_RGBW false
#define NUM_PIXELS 1
#define WS2812_PIN 3

// UART
#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define CHAR_M 0x4D
#define CHAR_R 0x52
#define CHAR_G 0x47
#define CHAR_B 0x42

#define NUM_MODES 3

static uint32_t rx_counter = 0;
static uint8_t rx_buffer[32];
static uint32_t mode;
static uint32_t mode_changed;

static uint32_t manual_px;
static uint8_t manual_px_update;

// PIO handle struct
typedef struct {
   PIO pio;
   uint32_t sm;
} pio_handle_t;

// RX interrupt handler
void on_uart_rx() {
   while (uart_is_readable(UART_ID)) {
      rx_buffer[rx_counter] = uart_getc(UART_ID);
      rx_counter++;
   }
}

void parse_uart_message() {
   // First character indicates the type of mode setting
   uint8_t rx_px;
   char mode_type = rx_buffer[0];
   uint32_t num_chars = rx_counter;
   rx_counter--;
   
   // Decoder a mode change
   if (mode_type == CHAR_M) {
      char mode_set = rx_buffer[1];
      if ((mode_set > 0x2F) && (mode_set < 0x3A)) {
         mode = (mode_set - 0x30);
         mode_changed = 1;
         manual_px_update = 0x01;
      }
   }
   
   rx_px = rx_buffer[1] & 0xFF;
   rx_counter--;
   
   if (mode_type == CHAR_G) {
      manual_px_update = 0x01;
      manual_px &= 0x0000FFFF;
      manual_px |= (0x00FFFFFF & rx_px << 16);
   }
   if (mode_type == CHAR_R) {
      manual_px_update = 0x01;
      manual_px &= 0x00FF00FF;
      manual_px |= (0x00FFFFFF & (rx_px << 8));
   }
   if (mode_type == CHAR_B) {
      manual_px_update = 0x01;
      manual_px &= 0x00FFFF00;
      manual_px |= (0x00FFFFFF & rx_px);
   }
   
   rx_counter = 0;
}

// LED functions
static inline void put_pixel(uint32_t pixel_grb, pio_handle_t pio_handle) {
   pio_sm_put_blocking(pio_handle.pio, pio_handle.sm, pixel_grb << 8u);
}

uint32_t rgb_test(uint32_t t) {
   uint32_t pixel_val;
   t &= 0x03;
   switch (t) {
      case 0 : {
            pixel_val = 0x000000FF;
            break;
      }
      case 1 : {
            pixel_val = 0x0000FF00;
            break;
      }
      case 2 : {
            pixel_val = 0x00FF0000;
            break;
      }
      case 3 : {
            pixel_val = 0xFF000000;
            break;
      }
      default : {
            pixel_val = 0xFFFFFFFF;
            break;
      }
   }
   return pixel_val;
}

uint32_t rgb_fade(uint32_t current_px_val) {
   uint8_t g = ((current_px_val & 0x00FF0000) >> 16);
   uint8_t r = ((current_px_val & 0x0000FF00) >> 8);
   uint8_t b = (current_px_val & 0x000000FF);
   
   if ((r == 0) && (g < 255)) {
      g++;
      b--;
   }
   else if ((b == 0) && (r < 255)) {
      r++;
      g--;
   }
   else if ((g == 0) && (b < 255)) {
      b++;
      r--;
   }
   
   return ((g << 16) + (r << 8) + b);
}

int main() {
   stdio_init_all();
   
   // Setup PIO
   PIO pio = pio0;
   uint sm = pio_claim_unused_sm(pio, true);
   uint offset = pio_add_program(pio, &ws2812_program);
   pio_handle_t pio_handle;
   pio_handle.pio = pio;
   pio_handle.sm = sm;
   
   ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
   
   // Setup UART physical
   uart_init(UART_ID, BAUD_RATE);
   gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
   gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
   uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
   
   // Turn off flow control and FIFO
   uart_set_hw_flow(UART_ID, false, false);
   uart_set_fifo_enabled(UART_ID, false);
   
   // Configure RX only interrupt
   int UART_IRQ = UART0_IRQ;
   irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
   irq_set_enabled(UART_IRQ, true);
   uart_set_irq_enables(UART_ID, true, false);
   
   uart_puts(UART_ID, "\nWS2812 RGB LED Test\n\r");
   
   // Initialise LED
   uint32_t current_px;
   uint32_t t = 0;
   uint32_t rgb_px = 0x00FF0000;
   uint32_t rgb_next_px;
   put_pixel(rgb_px, pio_handle);
   mode = 0;
   mode_changed = 0;
   
   manual_px = 0x00808080;
   
   while (1) {
      
      if (rx_counter != 0) {
         parse_uart_message();
      }
      
      if (mode_changed) {
         mode_changed = 0;
         switch (mode) {
            case 0 : uart_puts(UART_ID, "Mode 0: RGB Fade\n\r"); break;
            case 1 : uart_puts(UART_ID, "Mode 1: Colour Test\n\r"); break;
            case 2 : uart_puts(UART_ID, "Mode 2: Manual\n\r"); break;
            case 3 : uart_puts(UART_ID, "Mode 3: Blink\n\r"); break;
            case 4 : uart_puts(UART_ID, "Mode 4: Off\n\r"); break;
         }
      }
      
      switch (mode) {
         case 0 : {
            // RGB fade
            rgb_next_px = rgb_fade(rgb_px);
            current_px = rgb_next_px;
            put_pixel(current_px, pio_handle);
            rgb_px = rgb_next_px;
            sleep_ms(10);
            break;
         }
         
         case 1 : {
            // Colour test mode
            current_px = rgb_test(t);
            put_pixel(current_px, pio_handle);
            t++;
            sleep_ms(5000);
            break;
         }
         
         case 2 : {
            // Manual mode
            if (manual_px_update) {
               manual_px_update = 0x00;
               current_px = manual_px;
               put_pixel(current_px, pio_handle);
            }
            sleep_ms(10);
            break;
         }
         
         case 3 : {
            // Blink
            put_pixel(0, pio_handle);
            sleep_ms(250);
            put_pixel(current_px, pio_handle);
            sleep_ms(250);
            break;
         }
         
         case 4 : {
            // Off
            put_pixel(0, pio_handle);
            sleep_ms(10);
            break;
         }
         
         default : {
            // Default to off
            put_pixel(0, pio_handle);
            sleep_ms(10);
            break;
         }
      }
   }
}
