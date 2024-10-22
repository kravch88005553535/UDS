#include "sstream"
#include <iostream>
#include <cstring>
#include "dtc.h"
#define DTC_DEBUG

Program_timer DTC::m_1ms_timer {Program_timer(Program_timer::Type_loop, 1000)};

DTC::DTC(const Letter a_letter, const Standard a_standard, 
    const Subsystem a_subsystem, const uint8_t a_fault_description,
    const int32_t a_activeflag_threshold, const int32_t a_saveflag_threshold)
  : m_letter{a_letter}
  , m_standard{a_standard}
  , m_subsystem{a_subsystem}
  , m_fault_description{a_fault_description}
  , m_fault_detection_counter{0}
  , m_activeflag_threshold{a_activeflag_threshold}
  , m_saveflag_threshold{a_saveflag_threshold}
  , m_savetomemory_status{SaveToMemoryStatus_Unknown}
  , m_status{Status_Inactive}
  , m_is_condition_failed{true} //false
  , m_detection_timestamp{0}
  , m_active_time{0}
{
}

// DTC::DTC(const char* a_dtc)
//   : m_letter{P_Powertrain}
//   , m_standard{Standard_SAE_EOBD}
//   , m_subsystem{Subsystem_FuelAirMetering_AuxiliaryEmissionControls}
//   , m_fault_description{0}
//   , m_fault_detection_counter{0}
//   , m_activeflag_threshold{1000}
//   , m_saveflag_threshold{5000}
//   , m_status{Status_Inactive}
//   , m_is_condition_failed{true} //false
//   , m_detection_timestamp{0}
//   , m_active_time{0}
// {
//   //throw exception if(a_fault_description >100)
//   if(strlen(a_dtc) != 5)
//     std::cout << "Can not create DTC. String " << a_dtc << " has an invalid format! \n only 5 characters allowed (for example \"P01234\")" << std::endl;
//     //throw exception
// }

DTC::~DTC()
{}

void DTC::SetConditionFailedFlag(const bool a_flag)
{
  m_is_condition_failed = a_flag;
}
void DTC::Check()
{ //only for test
  CheckFaultDetectionCounter();
}
bool DTC::SetActiveFlagThreshold(const uint32_t a_threshold)
{
  m_activeflag_threshold = a_threshold;
  return false;
}
bool DTC::SetSaveFlagThreshold(const uint32_t a_threshold)
{
  m_saveflag_threshold = a_threshold;
  return false;
}
bool DTC::IsConditionFailed() const
{
  return m_is_condition_failed;
}
void DTC::CheckFaultDetectionCounter()
{
  const bool active_flag{m_fault_detection_counter >= m_activeflag_threshold};
  bool save_flag{m_fault_detection_counter == m_saveflag_threshold};
  if(m_saveflag_threshold == 0)
  save_flag = false;
  SetActiveFlag(active_flag);
  SetSaveFlag(save_flag);

  if(IsConditionFailed())
  {
    if(m_fault_detection_counter < m_saveflag_threshold)
      m_fault_detection_counter++;
      //conditionfailed = false
  }
  else
  {
    if(m_fault_detection_counter > 0)
      m_fault_detection_counter--;    
  }
  SetStatus(active_flag, save_flag);
}
void DTC::SetActiveFlag(const bool a_flag)
{
  m_status = static_cast<Status>(m_status & ~Status_Active);
  if(a_flag)
    m_status = static_cast<Status>(m_status | Status_Active);
}
bool DTC::IsActive() const
{
  return m_status & Status_Active;
} 
bool DTC::IsSaved() const
{
  return m_status & Status_Saved;
}
void DTC::SetSaveFlag(const bool a_flag)
{
  m_status = static_cast<Status>(m_status & ~Status_Saved);
  if(a_flag)
  {
    m_status = static_cast<Status>(m_status | Status_Saved);
    if(m_savetomemory_status == SaveToMemoryStatus_LoadedFromFile or m_savetomemory_status == SaveToMemoryStatus_Unknown)
      m_savetomemory_status = SaveToMemoryStatus_NeedSave;
  }
}

DTC::SaveToMemoryStatus DTC::GetSaveToMemoryStatus() const
{
  return m_savetomemory_status;
}
void DTC::SetSaveToMemoryStatus(const SaveToMemoryStatus a_savetomemory_status)
{
  m_savetomemory_status = a_savetomemory_status;
}
void DTC::SetStatus(const Status a_status)
{
  m_status = a_status;
}
void DTC::SetStatus(const bool a_active_flag, const bool a_save_flag)
{
  #ifdef DTC_DEBUG
  if(s_active_flag != a_active_flag)
  {
    s_active_flag = a_active_flag;
    std::cout << "DTC " << GetAbbreviation() << (a_active_flag ? " is active" : " is not active" ) << std::endl;
  }
  if(s_save_flag != a_save_flag)
  {
    s_save_flag = a_save_flag;
    std::cout << "DTC " << GetAbbreviation() << (a_save_flag ? " needs save" : " do not need save" ) << std::endl;
  }
  #endif //DTC_DEBUG

  SetActiveFlag(a_active_flag);
  SetSaveFlag(a_save_flag);
}
std::string DTC::GetAbbreviation() const
{
  std::stringstream ss;

  switch (m_letter)
  {
    case P_Powertrain:
      ss << "P";
    break;

    case C_Chassis:
      ss << "C";
    break;

    case B_Body:
      ss << "B";
    break;

    case U_VehicleOnboardComputers:
      ss << "U";
    break;
  }

  m_standard == Standard_SAE_EOBD ? ss << "0" : ss << "1";

  switch (m_subsystem)
  {
    case Subsystem_FuelAirMetering_AuxiliaryEmissionControls:
      ss << "0";
    break;

    case Subsystem_FuelAirMetering:
      ss << "1";
    break;

    case Subsystem_FuelAirMetering_InjectorCircuit:
      ss << "2";
    break;

    case Subsystem_IgnitionSystemOrMisfire:
      ss << "3";
    break;

    case Subsystem_AuxiliaryEmissionControls:
      ss << "4";
    break;

    case Subsystem_VehicleSpeedControls_IdleControlSystem:
      ss << "5";
    break;

    case Subsystem_ComputerOutputCircuit:
      ss << "6";
    break;

    case Subsystem_Transmission_1:
      ss << "7";
    break;

    case Subsystem_Transmission_2:
      ss << "8";
    break;

    default: 
      ss << "x";
    break;
  }

  if(m_fault_description < 10)
    ss << "0";

  ss << (uint32_t)m_fault_description;
  return ss.str();
}
bool DTC::Check1msTimer()
{
  return m_1ms_timer.Check();
}
std::time_t DTC::GetCurrentDateTime()
{
  const auto now = std::chrono::system_clock::now();
  const std::time_t t_c = std::chrono::system_clock::to_time_t(now);
  std::cout << "The system clock is currently at " << std::ctime(&t_c);
  return t_c;
}
