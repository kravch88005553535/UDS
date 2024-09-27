#include <iostream>
#include <cstring>
#include "dtc.h"

DTC::DTC(const Letter a_letter, const Standard a_standard, const Subsystem a_subsystem, const uint8_t a_fault_description)
  : m_letter{a_letter}
  , m_standard{a_standard}
  , m_subsystem{a_subsystem}
  , m_fault_description{a_fault_description}
  
  , m_checking_interval_timer{Program_timer(Program_timer::Type_loop, 1/*ms*/)}
{
}

DTC::DTC(const char* a_dtc)
  : m_letter{P_Powertrain}
  , m_standard{Standard_SAE_EOBD}
  , m_subsystem{Subsystem_FuelAirMetering_AuxiliaryEmissionControls}
  , m_fault_description{0}
  , m_checking_interval_timer{Program_timer(Program_timer::Type_loop, 1)}
  , m_aging_counter{0}
  , m_aging_counter_threshold{100}
  , m_status{Status_Inactive}
  , m_detection_timestamp{0}
  , m_active_time{0}
{
  if(strlen(a_dtc) != 5)
    std::cout << "Can not create DTC. String " << a_dtc << " has an invalid format! \n only 5 characters allowed (for example \"P01234\")" << std::endl;
    //throw exception
}

DTC::~DTC()
{}

void DTC::CheckAgingCounter()
{
  if(m_checking_interval_timer.Check())
  {
    if(m_aging_counter > m_aging_counter_threshold)
      SetActiveFlag(true);

    if(m_aging_counter > 2 * m_aging_counter_threshold)
      SetSavedFlag(true);
      //save to memory

    if(m_aging_counter <= m_aging_counter_threshold)
      SetActiveFlag(false);
  }
  else
    return;
}

bool DTC::IsActive() const
{
  return m_status & Status_Active;
}
void DTC::SetActiveFlag(const bool a_flag)
{
  m_status = static_cast<Status>(m_status | Status_Active);
}
bool DTC::IsSaved() const
{
  return m_status & Status_Saved;
}
void DTC::SetSavedFlag(const bool a_flag)
{
  m_status = static_cast<Status>(m_status | Status_Saved);
}
void DTC::SetStatus(const Status a_status)
{
  m_status = a_status;
}

const char* DTC::GetDTC()
{
  return "";
}

std::time_t DTC::GetCurrentDateTime()
{
  const auto now = std::chrono::system_clock::now();
  const std::time_t t_c = std::chrono::system_clock::to_time_t(now);
  std::cout << "The system clock is currently at " << std::ctime(&t_c);
  return t_c;
}
