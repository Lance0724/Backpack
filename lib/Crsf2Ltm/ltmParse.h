#include "config.h"

#define TLM_UART_BAUD               115200

bool ltm_encodeTargetData(uint8_t c);
bool parseLTM_GFRAME(uint32_t now);