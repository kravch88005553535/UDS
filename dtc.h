#ifndef __DTC_H__
#define __DTC_H__

#include <chrono>

class DTC
{
public:
  enum Status
  {
    Status_Inactove = 0b00,
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
    FuelAirMetering_AuxiliaryEmissionControls = 0,
    FuelAirMetering = 1,
    FuelAirMetering_InjectorCircuit = 2,
    IgnitionSystemOrMisfire = 3,
    AuxiliaryEmissionControls = 4,
    VehicleSpeedControls_IdleControlSystem= 5,
    ComputerOutputCircuit = 6,
    Transmission_1 = 7,
    Transmission_2 = 8
  };

  //DTC(const Standard a_standard, const Subsystem a_subsystem, const uint8_t a_individual_fault);
  DTC(const char* a_dtc);
  ~DTC();
  
  bool IsActive() const;
  bool IsSaved() const  ;
  void SetStatus(const Status a_status);
  const char* GetDTC();
private:
  DTC() = delete;
  std::time_t GetCurrentDateTime();
  // const uint8_t  m_individual_fault;
  Status         m_status;
  std::time_t    m_detection_timestamp;
  std::time_t    m_active_time;
};

#endif //__DTC_H__