#include <stdio.h>
#include <iostream>

#include "application.h"

int main(int argc, char *argv[]) 
{
  constexpr uint32_t ecu_rx_can_id{0x750};
  constexpr uint32_t ecu_tx_can_id{0x751};
  Application app(ecu_rx_can_id, ecu_tx_can_id);
  return app.Execute();
}
