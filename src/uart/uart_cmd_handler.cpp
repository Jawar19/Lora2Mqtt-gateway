#include "uart_cmd_handler.h"
#include <cstdio>
#include <hardware/irq.h>
#include <pico/stdio.h>

// Static member initialization
std::string       UARTCommandHandler::command_buffer_;
std::atomic<bool> UARTCommandHandler::command_ready_{false};
uart_inst_t      *UARTCommandHandler::uart_instance_ = nullptr;

UARTCommandHandler::UARTCommandHandler(uart_inst_t *uart) : uart_(uart) {

  // Store instance for ISR
  uart_instance_ = uart_;

  // Setup UART interrupts
  irq_ = (uart_ == uart0) ? UART0_IRQ : UART1_IRQ;

  irq_set_exclusive_handler(irq_, on_uart_rx);
  irq_set_enabled(irq_, true);
  uart_set_irq_enables(uart_, true, false); // RX only

  printf("UART command handler initialized\n");
}

UARTCommandHandler::~UARTCommandHandler() {
  irq_set_enabled(irq_, false);
  uart_set_irq_enables(uart_, false, false);
  uart_tx_wait_blocking(uart_);
  printf("UART command handler shutdown\n");
}

bool UARTCommandHandler::has_command() {
  return command_ready_.load(std::memory_order_acquire);
}

std::string UARTCommandHandler::get_command() {
  std::string cmd = command_buffer_;
  command_buffer_.clear();
  command_ready_.store(false, std::memory_order_release);
  return cmd;
}

// Static ISR - called from interrupt context
void UARTCommandHandler::on_uart_rx() {
  while (uart_is_readable(uart_instance_)) {
    char c = uart_getc(uart_instance_);

    if (c == '\n' || c == '\r') {
      if (!command_buffer_.empty()) {
        command_ready_.store(true, std::memory_order_release);
      }
    } else if (c >= 32 && c < 127) { // Printable ASCII
      command_buffer_ += c;
      uart_putc(uart_instance_, c);  // Echo
    } else if (c == 127 || c == 8) { // Backspace
      if (!command_buffer_.empty()) {
        command_buffer_.pop_back();
        uart_puts(uart_instance_, "\b \b");
      }
    }
  }
}
