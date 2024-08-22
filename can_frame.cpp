#include "can_frame.h"

CAN_Frame::CAN_Frame()
  : m_source{Source_Unknown}
  , m_id{0}
  , m_data{0}
{
}

CAN_Frame::CAN_Frame(const Source a_source, const uint32_t a_id, const uint8_t* ap_data)
  : m_source{a_source}
  , m_id{a_id}
  , m_data{}
{
  for (int i = 0; i < 8; ++i)
    m_data[i] = *ap_data++;
}

CAN_Frame::~CAN_Frame()
{
}

void CAN_Frame::SetID(const uint32_t a_id)
{
  m_id = a_id;
}

uint32_t CAN_Frame::GetID() const
{
  return m_id;
}

void CAN_Frame::SetData(const DataPos a_data_pos, const uint8_t a_data)
{
  m_data[a_data_pos] = a_data;
}

uint8_t CAN_Frame::GetData(const DataPos a_data_pos) const
{
  return m_data[a_data_pos];
}

const uint8_t* CAN_Frame::GetDataPtr() const
{
  return &m_data[0];
}

const uint8_t* CAN_Frame::GetDataPtr(const CAN_Frame::DataPos datapos) const
{
  return &m_data[datapos];
}

void CAN_Frame::SetSource(const Source a_source)
{
  m_source = a_source;
}

CAN_Frame::Source CAN_Frame::GetSource() const
{
  return m_source;
}
void CAN_Frame::Fill(const uint8_t a_fill_with)
{
  for (int i = 0; i < 8; ++i)
    m_data[i] = a_fill_with;
}