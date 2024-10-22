#ifndef __DTC_H__
#define __DTC_H__

#include <chrono>
#include "program_timer.h"

class DTC
{
public:
  enum Status: uint8_t
  {
    Status_Inactive = 0b00,
    Status_Saved= 0b01,
    Status_Active = 0b10,
    Status_ActiveAndSaved = 0b11,
  };
  enum Letter
  {
    P_Powertrain,
    C_Chassis,
    B_Body,
    U_VehicleOnboardComputers
  };
  enum Standard
  {
    Standard_SAE_EOBD = 0,
    Standard_VehicleManufacturerSpecific = 1
  };
  enum Subsystem
  {
    Subsystem_FuelAirMetering_AuxiliaryEmissionControls = 0,
    Subsystem_FuelAirMetering = 1,
    Subsystem_FuelAirMetering_InjectorCircuit = 2,
    Subsystem_IgnitionSystemOrMisfire = 3,
    Subsystem_AuxiliaryEmissionControls = 4,
    Subsystem_VehicleSpeedControls_IdleControlSystem= 5,
    Subsystem_ComputerOutputCircuit = 6,
    Subsystem_Transmission_1 = 7,
    Subsystem_Transmission_2 = 8
  };
  enum SaveToMemoryStatus
  {
    SaveToMemoryStatus_Unknown,
    SaveToMemoryStatus_LoadedFromFile,
    SaveToMemoryStatus_NeedSave,
    SaveToMemoryStatus_SavedToMemory
  };

  DTC(const Letter a_letter, const Standard a_standard, 
      const Subsystem a_subsystem, const uint8_t a_fault_description,
      const int32_t a_activeflag_threshold, const int32_t a_saveflag_threshold = 0);
  // DTC(const char* a_dtc);
  ~DTC();
  
  void SetConditionFailedFlag(const bool a_flag);
  bool IsConditionFailed() const;
  bool IsActive() const;
  bool IsSaved() const;
  void Check();
  bool SetActiveFlagThreshold(const uint32_t a_threshold);
  bool SetSaveFlagThreshold(const uint32_t a_threshold);
  std::string GetAbbreviation() const;
  SaveToMemoryStatus GetSaveToMemoryStatus() const;
  void SetSaveToMemoryStatus(const SaveToMemoryStatus a_savetomemory_status);
  static bool Check1msTimer();
private:
  DTC() = delete;
  void Test(); //Annex D
  void CheckFaultDetectionCounter();
  void SetActiveFlag(const bool a_flag);
  void SetSaveFlag(const bool a_flag);

  void                SetStatus(const Status a_status);
  void                SetStatus(const bool a_active_flag, const bool a_save_flag);
  std::time_t         GetCurrentDateTime();
  int32_t             m_fault_detection_counter;
  int32_t             m_activeflag_threshold;
  int32_t             m_saveflag_threshold;
  Status              m_status;
  bool                m_is_condition_failed;
  SaveToMemoryStatus  m_savetomemory_status;
  bool                m_testfailed_bit;
  std::time_t         m_detection_timestamp;
  std::time_t         m_active_time;
  
  bool s_active_flag{false}; //temporary
  bool s_save_flag  {false}; //temporary
  
  const Letter    m_letter;
  const Standard  m_standard;
  const Subsystem m_subsystem;
  const uint8_t   m_fault_description;

  static Program_timer  m_1ms_timer;
};

#endif //__DTC_H__