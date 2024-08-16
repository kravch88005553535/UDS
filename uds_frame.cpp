 #include <iostream>
#include "uds_frame.h"

UDS_Frame::UDS_Frame()
  : m_is_frame_valid{true}
{}

UDS_Frame::UDS_Frame(const UDS_Frame& a_other)
{
  m_source = a_other.m_source;
  m_protocol_information = a_other.m_protocol_information;
  m_data_length = a_other.m_data_length;
  m_sid = a_other.m_sid;
  for(auto i{0}; i < 4096; ++i)
    m_data[i] = a_other.m_data[i];
  m_is_frame_valid = a_other.m_is_frame_valid;
  m_fcf_flag = a_other.m_fcf_flag;
}
UDS_Frame::~UDS_Frame(){}

void UDS_Frame::SetSource(const CAN_Frame::Source a_source)
{
  m_source = a_source;
}
CAN_Frame::Source UDS_Frame::GetSource() const
{
  return m_source;
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
uint16_t UDS_Frame::GetDataLength()
{
  return m_data_length;
}

void UDS_Frame::SetSID(const Service a_service_id)
{
  m_sid = a_service_id;
}

UDS_Frame::Service UDS_Frame::GetSID()
{
  return m_sid;
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

void UDS_Frame::SetframeValidity(bool a_validity)
{
  m_is_frame_valid = a_validity;
}
bool UDS_Frame::IsFrameValid()
{
  return m_is_frame_valid;
}

void UDS_Frame::SetFlagOfFCF(const FCF_Flag a_flag)
{
  m_fcf_flag = a_flag;
}
UDS_Frame::FCF_Flag UDS_Frame::GetFlagOfFCF() const
{
  return m_fcf_flag;
}