#ifndef __UDS_MESSAGE_H__
#define __UDS_MESSAGE_H__
#include <stdint.h>
#include "can_frame.h"

// this is only appreciable for can frames on the CAN bus!!!

class UDS_Frame
{
public:
  enum Service: uint8_t
  {
    //Diagnostic and Communications Management
    DiagnosticSessionControl        = 0x10,    ECUReset                        = 0x10,
    SecurityAccess                  = 0x27,    CommunicationControl            = 0x28,
    Authentication                  = 0x29,    TesterPresent                   = 0x3E,
    AccessTimingParameter           = 0x83,    SecuredDataTransmission         = 0x84,
    ControlDTCSetting               = 0x85,    ResponseOnEvent                 = 0x86,
    LinkControl                     = 0x87,
    /*--------------------------Data Transmission--------------------------*/
    ReadDataByIdentifier            = 0x22,    ReadMemoryByAddress             = 0x23,
    ReadScalingDataByIdentifier     = 0x24,    ReadDataByPeriodicIdentifier    = 0x2A,
    DynamicallyDefineDataIdentifier = 0x2C,    WriteDataByIdentifier           = 0x2E,
    WriteMemoryByAddress            = 0x3D,
    /*-----------------------Stored Data Transmission----------------------*/
    ClearDiagnosticUInformation     = 0x14,    ReadDTCInformation              = 0x19,
    /*------------------------Input/output control-------------------------*/
    InputOutputControlByIdentifier  = 0x2F,
    /*--------------------Remote Activation of Routine---------------------*/
    RoutineControl                  = 0x31,
    /*--------------------------Upload / Download--------------------------*/
    RequestDownload                 = 0x34,    RequestUpload                   = 0x35,
    TransferData                    = 0x36,    RequestTransferExit             = 0x37,
    RequestFileTransfer             = 0x38,
    /*--------------------------Negative response--------------------------*/
    NegativeResponse                = 0x7F
  };
  enum PCI//ProtocolControlInformation
  {
    PCI_SingleFrame      = 0x0,
    PCI_FirstFrame       = 0x1,
    PCI_ConsecutiveFrame = 0x2,
    PCI_FlowControlFrame = 0x3
  };
  enum FCF_Flag
  {
    FCF_Flag_CTS      = 0x00,
    FCF_Flag_Wait     = 0x01,
    FCF_Flag_Overflow = 0x02
  };

  UDS_Frame();
  UDS_Frame(const UDS_Frame& a_other);
  ~UDS_Frame();
  
  void SetSource(const CAN_Frame::Source a_source);
  CAN_Frame::Source GetSource() const;

  void SetProtocolInformation(UDS_Frame::PCI a_protocol_information);
  UDS_Frame::PCI GetProtocolInformation();

  void SetDataLength(const uint16_t a_data_length);
  uint16_t GetDataLength();

  void SetSID(const Service a_service_id);
  Service GetSID();

  void SetData(const uint8_t* a_payload_array, uint32_t a_payload_size, const uint32_t a_offset);
  const uint8_t* GetData();

  void SetframeValidity(bool a_validity);
  bool IsFrameValid();

  void SetFlagOfFCF(const FCF_Flag a_flag);
  FCF_Flag GetFlagOfFCF() const;
private:
  CAN_Frame::Source m_source;
  PCI               m_protocol_information;
  uint16_t          m_data_length;
  Service           m_sid;
  uint8_t           m_data[4096];
  bool              m_is_frame_valid;
  FCF_Flag          m_fcf_flag;
};

#endif //__UDS_MESSAGE_H__at the end of transmission sets to true