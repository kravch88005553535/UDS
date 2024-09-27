#ifndef __DTC_H__
#define __DTC_H__

#include <chrono>
#include "program_timer.h"

class DTC
{
public:
  enum Status
  {
    Status_Inactive = 0b00,
    Status_Saved= 0b01,
    Status_Active = 0b10,
    Status_ActiveAndSaved = 0b10,
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

  DTC(const Letter a_letter, const Standard a_standard, const Subsystem a_subsystem, const uint8_t a_fault_description);
  DTC(const char* a_dtc);
  ~DTC();
  
  bool IsActive() const;
  bool IsSaved() const;
  const char* GetDTC();
private:
  DTC() = delete;
  void CheckAgingCounter();
  void SetActiveFlag(const bool a_flag);
  void SetSavedFlag(const bool a_flag);
  void SetStatus(const Status a_status);
  std::time_t    GetCurrentDateTime();
  Program_timer  m_checking_interval_timer;
  uint32_t       m_aging_counter;
  uint32_t       m_aging_counter_threshold;
  Status         m_status;
  std::time_t    m_detection_timestamp;
  std::time_t    m_active_time;

  const Letter    m_letter;
  const Standard  m_standard;
  const Subsystem m_subsystem;
  const uint8_t   m_fault_description;
};

#endif //__DTC_H__