#include <iostream>
#include <cstring>
#include "dtc.h"

// DTC::DTC(const Standard a_standard, const Subsystem a_subsystem, const uint8_t a_individual_fault)
//   : m_individual_fault{0}
//   , m_is_active{false}
// {
  
// }

DTC::DTC(const char* a_dtc) 
  : m_is_active{false}
{
  if(strlen(a_dtc) != 5)
    std::cout << "Can not create DTC. " << a_dtc << " has invalid format!" << std::endl;
}

// DTC::DTC(const uint16_t a_dtc_id)
//   : m_dtc_id{a_dtc_id}
//   , m_is_active{false}
//   , m_timestamp{}
// {}

DTC::~DTC()
{}

bool DTC::IsActive()
{
  return m_is_active;
}

std::time_t GetCurrentDateTime()
{
  const auto now = std::chrono::system_clock::now();
  const std::time_t t_c = std::chrono::system_clock::to_time_t(now);
  std::cout << "The system clock is currently at " << std::ctime(&t_c);
  return t_c;
}
