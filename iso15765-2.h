#ifndef __ISO_15765_2__
#define __ISO_15765_2__

  enum class ISO_15765_2_PCI : uint8_t
  {//ProtocolControlInformation
    PCI_SingleFrame      = 0x0,
    PCI_FirstFrame       = 0x1,
    PCI_ConsecutiveFrame = 0x2,
    PCI_FlowControlFrame = 0x3
  };
  enum class ISO_15765_2_PCI_FCF_Flag : uint8_t
  {
    FCF_Flag_CTS      = 0x00,
    FCF_Flag_Wait     = 0x01,
    FCF_Flag_Overflow = 0x02
  };


#endif //__ISO_15765_2__