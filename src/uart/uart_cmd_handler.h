#ifndef uart_cmd_handler_H
#define uart_cmd_handler_H_

#include <atomic>
#include <hardware/uart.h>
#include <string>

class UARTCommandHandler {
public:
  explicit UARTCommandHandler(uart_inst_t *uart);
  ~UARTCommandHandler();

  // Check if command is ready (call from main loop)
  [[nodiscard]] static bool has_command();

  // Retrieve and clear the command (call after has_command() returns true)
  std::string get_command();

  // Delete copy/move constructors (singleton pattern for ISR)
  UARTCommandHandler(const UARTCommandHandler &)            = delete;
  UARTCommandHandler &operator=(const UARTCommandHandler &) = delete;

private:
  uart_inst_t *uart_;
  int          irq_;

  // Static members for ISR access
  static std::string       command_buffer_;
  static std::atomic<bool> command_ready_;
  static uart_inst_t      *uart_instance_; // Store for ISR

  // Static ISR handler
  static void on_uart_rx();
};

#endif // uart_cmd_handler.h_
