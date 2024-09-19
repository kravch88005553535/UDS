#ifndef __UDS_MESSAGE_H__
#define __UDS_MESSAGE_H__
#include <stdint.h>
#include "can_frame.h"
#include "iso15765-2.h"
// this is only appreciable for can frames on the CAN bus!!!

class UDS_Frame
{
public:
  UDS_Frame();
  UDS_Frame(const UDS_Frame& a_other);
  ~UDS_Frame();
  
  void SetSource(const CAN_Frame::Source a_source);
  CAN_Frame::Source GetSource() const;

  void SetProtocolInformation(ISO_15765_2_PCI a_protocol_information);
  ISO_15765_2_PCI GetProtocolInformation();

  void SetDataLength(const uint16_t a_data_length);
  uint16_t GetDataLength();

  void SetSID(const uint8_t a_service_id);
  uint8_t GetSID();

  void SetData(const uint8_t* a_payload_array, uint32_t a_payload_size, const uint32_t a_offset);
  const uint8_t* GetData();

  void SetframeValidity(bool a_validity);
  bool IsFrameValid();

  void SetFlagOfFCF(const ISO_15765_2_PCI_FCF_Flag a_flag);
  ISO_15765_2_PCI_FCF_Flag GetFlagOfFCF() const;
  void SetFunctionalAddressingFlag(bool a_flag);
  bool GetFunctionalAddressingFlag() const;
private:
  CAN_Frame::Source        m_source;
  ISO_15765_2_PCI          m_protocol_information;
  uint16_t                 m_data_length;
  uint8_t                  m_sid;
  uint8_t                  m_data[4096];
  bool                     m_is_frame_valid;
  ISO_15765_2_PCI_FCF_Flag m_fcf_flag;
  bool                     m_is_addressing_functional;
};

#endif //__UDS_MESSAGE_H__at the end of transmission sets to true