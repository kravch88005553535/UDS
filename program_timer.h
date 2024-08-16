#ifndef __PROGRAM_TIMER_H__
#define __PROGRAM_TIMER_H__

#include <cstdint>

class Program_timer
{
public:
  enum TimerType: uint8_t
  {
    TimerType_one_pulse,
    TimerType_loop
  };
  
  enum TimerStatus: uint8_t
  {
    TimerStatus_is_stopped = 0,
    TimerStatus_is_started = 1
  };
  
  explicit Program_timer(TimerType a_timertype);
  explicit Program_timer(TimerType a_timertype, uint32_t a_interval_ms);
  ~Program_timer();
  
  void SetInterval_ms(uint32_t a_interval_ms);
  uint32_t GetInterval_ms() const;
  void SetInterval_sec(uint32_t a_interval_sec);
  void SetInterval_min(uint32_t a_interval_min);
  
  void Start();
  void Stop();
  bool Check() const;
  bool IsStarted() const;
  TimerStatus GetTimerStatus() const;
private:
  Program_timer() = delete;

  TimerType    m_timertype;
  TimerStatus  m_timerstatus;
  uint32_t     m_interval_ms;

  
};

#endif //__PROGRAM_TIMER_H__