#ifndef __UDS_H__
#define __UDS_H__

#include <vector>
#include <deque>

#include "uds_frame.h"
#include "can_frame.h"
#include "did.h"

class UDS
{
public:
  enum Status
  {
    Status_ok,
    Status_is_executing_rx_ff,
    Status_rx_waits_for_FCF
  };

  enum SeedSize
  {
    Seed_size_1_byte = 1,
    Seed_size_2_byte = 2,
    Seed_size_3_byte = 3,
    Seed_size_4_byte = 4,
    Seed_size_5_byte = 5
  };

  enum SessionType
  {
    DSC_Type_DefaultSession = 0x01,
    DSC_Type_ProgrammingSession = 0x02,
    DSC_Type_ExtendedDiagnosticSession = 0x03,
    DSC_Type_SafetySystemDiagnosticSession = 0x04
  };

  virtual void        SetUDSStatus(const Status a_status);
  virtual Status      GetUDSStatus();
  virtual void        SetSessionType(const SessionType a_sessiontype);
  virtual SessionType GetSessiontype();
  virtual bool        CheckNumberOfSecurityAccessAttempts(const uint8_t a_subfunction);
  virtual void        ReloadNumberOfSecurityAccessAttempts();
protected:
  UDS();
  virtual ~UDS() = default;

  Status      m_status;
  SessionType m_sessiontype;
  uint8_t     m_programmingsession_number_of_attempts{1};
  uint8_t     m_extendeddiagnosticsession_number_of_attempts{1};
  uint8_t     m_safetysystemdiagnosticsession_number_of_attempts{1};
  SeedSize    m_seed_size;
  uint64_t    m_seed;
  uint64_t    m_key;
};

class UDSOnCAN : public UDS
{
public:
  UDSOnCAN();
  ~UDSOnCAN();
  
  void Execute();
  void MakePositiveResponse(const UDS_Frame::Service a_sid, const uint8_t* a_data_ptr, const uint32_t a_data_size);
  void MakeNegativeResponse(const UDS_Frame::Service a_rejected_sid, UDS_Frame::NegativeResponseCode a_nrc);
  
  bool IsRXBufferOfUDSEmpty();
  bool IsTXBufferOfUDSEmpty();
  bool ConvertCANFrameToUDS(const CAN_Frame* const ap_can_frame);
  std::vector<CAN_Frame*> ConvertUDSFrameToCAN();

  void GenerateAndUpdateSecurityAccessSeed(UDS::SeedSize a_seed_size);
  void CalculateSecurityAccessKey();
  uint64_t GetSecurityAccessKey();
  bool CompareSecurityAccessKey(uint64_t a_key);

private:

  void SendFlowControlFrame();
  void SetSeparationTime(uint8_t a_STmin);
  uint8_t GetSeparationTime() const;
  uint16_t GetSeparationTime_us() const;

  std::deque <UDS_Frame*> m_uds_tx_buffer;
  std::deque <UDS_Frame*> m_uds_rx_buffer;

  uint8_t m_STmin_this_device;
  uint8_t m_STmin_tester;
  uint8_t m_BS_this_device;
  uint8_t m_BS_tester;

  DID_Repository m_did_repository;
};
#endif //__UDS_H__