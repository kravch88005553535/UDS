#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <deque>
#include <queue>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/fcntl.h>
#include "can_frame.h"
#include "uds.h"
   
class Application
{
public:
  Application(const uint32_t a_ecu_rx_can_id, const uint32_t a_ecu_functional_can_id, const uint32_t a_ecu_tx_can_id);
  ~Application();

  bool Execute();
private:
  std::queue<std::string> m_rx_socket_queue;
  std::queue<std::string> m_tx_socket_queue;
  std::deque <CAN_Frame*> m_rx_can_deque;
  std::deque <CAN_Frame*> m_tx_can_deque;

  int                     m_uds_socket;
  sockaddr_un             m_uds_socket_address;
  
  int                     m_diagmesg_socket;
  sockaddr_un             m_diagmesg_socket_address;

  int                     m_cmd_socket;
  sockaddr_un             m_cmd_socket_address;

  UDSOnCAN&               mref_uds;
  DID_Repository&         m_did_repository;

  const uint32_t          m_ecu_rx_can_id;
  const uint32_t          m_ecu_functional_can_id;
  const uint32_t          m_ecu_tx_can_id;

  void CreateSocketUDS();
  void CreateSocketDiagMesg();
  void CheckModifiedDids();
  void RecieveDataFromDiagSocket();
  void CheckSocketForNewRxData();
  void TransmitCanFrameToSocket();
};

#endif //__APPLICATION_H__
