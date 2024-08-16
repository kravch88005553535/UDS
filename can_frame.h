#ifndef __CAN_FRAME_H__
#define __CAN_FRAME_H__

#include <cstdint>
#include "can_frame.h"

class CAN_Frame
{
public:
  enum DataPos
  {
    DataPos_0 = 0, DataPos_1,
    DataPos_2, DataPos_3,
    DataPos_4, DataPos_5,
    DataPos_6, DataPos_7,
  };

  enum Source
  {
    Source_CAN1     = 1,
    Source_CAN2     = 2,
    Source_CAN3     = 3,
    Source_Unknown  = 0xFF
  };

public:
  CAN_Frame();
  CAN_Frame(const Source a_source, const uint32_t a_id, const uint8_t* ap_data);
  ~CAN_Frame();
  
  void     SetID(const uint32_t a_id);
  uint32_t GetID() const;
  void     SetData(const DataPos a_data_pos, const uint8_t a_data);
  uint8_t  GetData(const DataPos a_data_pos) const;
  const uint8_t* GetDataPtr() const;
  const uint8_t* GetDataPtr(const DataPos datapos) const;
  void     SetSource(const Source a_source);
  Source   GetSource();
  void     Fill(const uint8_t a_fill_with);
private:
  uint32_t  m_id;
  Source    m_source;
  uint8_t   m_data[8];
  //  edit length of packet in case of uds always 8
};

#endif //__CAN_FRAME_H__