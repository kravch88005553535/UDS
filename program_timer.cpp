#include <iostream>
#include <chrono>

#include "program_timer.h"

Program_timer::Program_timer(Type a_type)
  : m_type{a_type}
  , m_check_time{}
  , m_interval_us{}
  , m_status{Status_is_stopped}
{}
Program_timer::Program_timer(Type a_type, uint32_t a_interval_us)
  : m_type{a_type}
  , m_interval_us{a_interval_us}
  , m_check_time{std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()}
  , m_status{Status_is_started}
{
  m_check_time += m_interval_us;
}
Program_timer::~Program_timer()
{}

void Program_timer::SetInterval_us(uint32_t a_interval_us)
{
  m_interval_us = a_interval_us;
}
uint32_t Program_timer::GetInterval_us() const
{
  return m_interval_us;
}
void Program_timer::SetInterval_ms(uint32_t a_interval_ms)
{
  constexpr auto us_in_ms{1000};
  m_interval_us = a_interval_ms * us_in_ms;
}
void Program_timer::SetInterval_sec(uint32_t a_interval_sec)
{
  constexpr auto us_in_sec{1'000'000};
  m_interval_us = a_interval_sec * us_in_sec;
}
void Program_timer::SetInterval_min(uint32_t a_interval_min)
{
  constexpr auto sec_in_min{60};
  constexpr auto us_in_sec{1'000'000};
  m_interval_us = a_interval_min * sec_in_min * us_in_sec;
}
void Program_timer::Start()
{
  if(!m_interval_us or IsStarted())
    return;

  Reload();
  m_status = Status_is_started;
}
void Program_timer::Stop()
{
  m_status = Status_is_stopped;
}
bool Program_timer::Check()
{
  if(!IsStarted())
    return false;

  volatile int64_t current_time{std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()};
  //std::cout << "The system clock is currently at " << current_time << std::endl;

  if(current_time >= m_check_time)
  {
    switch (m_type)
    {
    case Type_one_pulse:
      m_status = Status_is_stopped;
      return true;
    break; //fake, never executes
    
    case Type_loop:
      m_check_time = current_time + m_interval_us;
      return true;
    break; //fake, never executes
    }
  }
  else
    return false;

  std::cout << "program timer check error" << '\n';
  return false;
}
void Program_timer::Reload()
{
  int64_t current_time{std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()};
  m_check_time = current_time + m_interval_us;
}
bool Program_timer::IsStarted() const
{
  return static_cast<bool>(m_status);
}
Program_timer::Status Program_timer::GetTimerStatus() const
{
  return m_status;
}