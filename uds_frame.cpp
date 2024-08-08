#include <iostream>
#include "uds_frame.h"


UDS_Frame::UDS_Frame(){}
UDS_Frame::~UDS_Frame(){}


void UDS_Frame::SetSource(const CAN_Frame::Source a_source)
{
  m_source = a_source;
}
CAN_Frame::Source UDS_Frame::GetSource() const
{
  return m_source;
}
void UDS_Frame::SetCanID(const uint32_t a_can_id)
{
  m_can_id = a_can_id;
}
uint32_t UDS_Frame::GetCanID()
{
  return m_can_id;
}
void UDS_Frame::SetProtocolInformation(UDS_Frame::PCI a_protocol_information)
{
  m_protocol_information = a_protocol_information;
}
UDS_Frame::PCI UDS_Frame::GetProtocolInformation()
{
  return m_protocol_information;
}
void UDS_Frame::SetDataLength(const uint16_t a_data_length)
{
  m_data_length = a_data_length;
}
uint16_t UDS_Frame::Getdatalength()
{
  return m_data_length;
}

void UDS_Frame::SetConsecutiveFrameNumber(const uint8_t a_consecutive_frame_number)
{
  m_consecutive_frame_number = a_consecutive_frame_number;
}
uint8_t UDS_Frame::GetConsecutiveFrameNumner()
{
  return m_consecutive_frame_number;
}

void UDS_Frame::SetSID(const Service a_service_id)
{
  m_sid = a_service_id;
}

UDS_Frame::Service UDS_Frame::GetSID()
{
  return m_sid;
}

void UDS_Frame::SetSubfunctionByte(uint8_t a_subfunction_byte)
{
  m_subfunction_byte = a_subfunction_byte;
}
uint8_t UDS_Frame::GetSubfunctionByte()
{
  return m_subfunction_byte;
}

void UDS_Frame::SetData(const uint8_t* a_payload_array, uint32_t a_payload_size, const uint32_t a_offset)
{
  if(!a_payload_size)
  {
    std::cout << "Set payload error! Size of payload is zero!" << '\n';
    return;
  }

  uint8_t* m_write_to_ptr{&m_data[0 + a_offset]};
  while(a_payload_size--)
  {
    *m_write_to_ptr++ = *a_payload_array++;
  }
}
const uint8_t* UDS_Frame::GetData()
{
  return &m_data[0];
}
void UDS_Frame::SetFlagOfFCF(const FCF_Flag a_flag)
{
  m_fcf_flag = a_flag;
}
UDS_Frame::FCF_Flag UDS_Frame::GetFlagOfFCF() const
{
  return m_fcf_flag;
}