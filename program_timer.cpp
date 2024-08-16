#include <iostream>
#include <chrono>

#include "program_timer.h"

Program_timer::Program_timer(TimerType a_timertype)
{}
Program_timer::Program_timer(TimerType a_timertype, uint32_t a_interval_ms)

{}
Program_timer::~Program_timer()
{}
  
void Program_timer::SetInterval_ms(uint32_t a_interval_ms)
{}
uint32_t Program_timer::GetInterval_ms() const
{
  return m_interval_ms;
}
void Program_timer::SetInterval_sec(uint32_t a_interval_sec){}
void Program_timer::SetInterval_min(uint32_t a_interval_min){}
void Program_timer::Start()
{}
void Program_timer::Stop(){}
bool Program_timer::Check() const
{
  /*
      const auto now = std::chrono::system_clock::now();
    std::time_t t_c = std::chrono::system_clock::to_time_t(now);
    
    std::cout << "The system clock is currently at " << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << std::endl;
  */
  volatile uint64_t t{std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count()};
  std::cout << "The system clock is currently at " << t << std::endl;
  return true;
}
bool Program_timer::IsStarted() const
{
  return static_cast<bool>(m_timerstatus);
}
Program_timer::TimerStatus Program_timer::GetTimerStatus() const
{
  return m_timerstatus;
}