#ifndef __PROGRAM_TIMER_H__
#define __PROGRAM_TIMER_H__

#include <cstdint>

class Program_timer
{
public:
  enum Type: uint8_t
  {
    Type_one_pulse,
    Type_loop
  };
  enum Status: uint8_t
  {
    Status_is_stopped = 0,
    Status_is_started = 1
  };

  explicit Program_timer(Type a_type);
  explicit Program_timer(Type a_type, uint32_t a_interval_ms);
  ~Program_timer();
  
  void SetInterval_us(uint32_t a_interval_us);
  uint32_t GetInterval_us() const;
  void SetInterval_ms(uint32_t a_interval_ms);
  void SetInterval_sec(uint32_t a_interval_sec);
  void SetInterval_min(uint32_t a_interval_min);
  
  void Start();
  void Stop();
  bool Check();
  void Reload();
  bool IsStarted() const;
  Status GetTimerStatus() const;
private:
  Program_timer() = delete;
  int64_t   m_check_time;
  Type      m_type;
  Status    m_status;
  uint32_t  m_interval_us;
};

#endif //__PROGRAM_TIMER_H__