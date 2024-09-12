#ifndef __DTC_H__
#define __DTC_H__

#include <chrono>

class DTC
{
public:
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
  
  bool IsActive();
  const char* GetDTC();
private:
  DTC() = delete;
  std::time_t GetCurrentDateTime();
  // const uint8_t  m_individual_fault;
  bool           m_is_active;
  std::time_t    m_timestamp;
};

#endif //__DTC_H__