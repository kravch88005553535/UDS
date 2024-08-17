#include "application.h"
#include "program_timer.h"
#include <chrono>

Application::Application(const uint32_t a_ecu_rx_can_id, const uint32_t a_ecu_tx_can_id)
  //: mref_socket{*(new sock_unix{std::string("/tmp/can.sock"), std::string("CAN_SOCKET"), &m_rx_socket_queue, &m_tx_socket_queue})} ///tmp/uds.sock"
  : mref_uds {*(new UDSOnCAN())} 
  , m_ecu_rx_can_id{a_ecu_rx_can_id}
  , m_ecu_tx_can_id{a_ecu_tx_can_id}
  , m_rx_socket_queue{}
  , m_tx_socket_queue{}
  , m_rx_can_deque{}
  , m_tx_can_deque{}
  , m_socket{}
  , m_addr{}
  , m_ifr{}
  , m_frame{}
{
}

Application::~Application()
{
if (close(m_socket) < 0) {
    std::cout << "CAN socket close error\n";
}
}

bool Application::Execute()
{
  if ((m_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
    perror("Socket");
    return 1;
  }

  strcpy(m_ifr.ifr_name, "can0" );
  ioctl(m_socket, SIOCGIFINDEX, &m_ifr);

  m_addr.can_family = AF_CAN;
  m_addr.can_ifindex = m_ifr.ifr_ifindex;

  if (bind(m_socket, (struct sockaddr *)&m_addr, sizeof(m_addr)) < 0) {
    perror("Bind");
    return 1;
  }

  int flags = fcntl(m_socket, F_GETFL, 0);
  if (fcntl(m_socket, F_SETFL, flags | O_NONBLOCK) < 0)
  {
    std::cout << "Error setting socket on nonblocked mode" << std::endl;
  }

  static Program_timer pt(Program_timer::Type_loop);
  pt.SetInterval_us(2000000);
  pt.Start();
  while (1)
  {
    int64_t current_time{std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()};
    if(pt.Check())
      std::cout << "timer_hit! " << current_time << std::endl;
//    std::thread rx_socket_tread(&Application::CheckSocketForNewRxData, this);
//    std::cout << std::this_thread::get_id() << '\n';
//    rx_socket_tread.join();
    
    CheckSocketForNewRxData();
    if(!m_rx_can_deque.empty())
    {
      const CAN_Frame* const can_recieved_frame{m_rx_can_deque.front()};
      m_rx_can_deque.pop_front();
      mref_uds.ConvertCANFrameToUDS(can_recieved_frame);
      delete can_recieved_frame;
    }
    mref_uds.Execute();
    
    while(!mref_uds.IsTXBufferOfUDSEmpty())
    {
      std::vector<CAN_Frame*>frames{mref_uds.ConvertUDSFrameToCAN()}; // return array of can frames via std::vector or std::array
      for (auto it: frames)
      {
        m_tx_can_deque.push_back(it); //need check
      }
    }
    //  if tx queue is not empty && transmission is
    //  not active, transmit can frame via socket

    switch (mref_uds.GetUDSStatus())
    {
    case UDS::Status_is_executing_rx_ff:
      TransmitCanFrameToSocket();
      mref_uds.SetUDSStatus(UDS::Status_rx_waits_for_FCF);
    break;

    case UDS::Status_rx_waits_for_FCF:
    break;
    
    default:
      TransmitCanFrameToSocket();
      //std::cout << " " << m_tx_can_deque.size() << '\n';
    break;
    }
  }

  //rx_socket_tread.join();
  return 0;
}

void Application::CheckSocketForNewRxData()
{  
// sudo slcand -o -c -s5 /dev/ttyACM0 can0
// sudo ifconfig can0 up
  volatile int nbytes;
  socklen_t  len = sizeof(m_addr);
  nbytes = recvfrom(m_socket, &m_frame, sizeof(can_frame),
                    0, (struct sockaddr*)&m_addr, &len);

  if (nbytes < 0) {
      //perror("Read");
      return;
  }

  if(m_frame.can_id == m_ecu_rx_can_id)
  {
    //std::cout << "CAN ID HIT!" << '\n';
    printf("RX: 0x%03X [%d] ",m_frame.can_id, 
    m_frame.can_dlc);
    for (auto i = 0; i < m_frame.can_dlc; i++)
        printf("%02X ",m_frame.data[i]);
    printf("\r\n");

    auto source {CAN_Frame::Source_CAN1};
    auto rx_can_id {m_frame.can_id};
    CAN_Frame* p_frame{new CAN_Frame(source, rx_can_id, &m_frame.data[0])};
    m_rx_can_deque.push_back(p_frame); 
  }

// mref_socket.process();
//   static std::string recieved_data{};
//   // recieved_data.append("001#A34E3FF8#thjdkb\n"); //only for test

//   std::size_t newline_index{recieved_data.find('\n')};
//   if(newline_index && recieved_data.size())
//   {
//       bool is_frame_valid{true};

//       std::string recieved_data_substring{recieved_data.substr(0, newline_index)};
//       recieved_data = recieved_data.substr(newline_index+1);    
      
//       std::size_t first_grid_index{recieved_data_substring.find('#')};

//       CAN_Frame::Source source{};// источник 1 байт  CAN1..3
//       source = static_cast<CAN_Frame::Source>(std::stoul(recieved_data_substring.substr(0,first_grid_index)));
//       if(source < CAN_Frame::Source_CAN1 || source >= CAN_Frame::Source_CAN3)
//       {
//           source = CAN_Frame::Source_Unknown;
//           is_frame_valid = false;
//       }
//       else
// //          ---------------------------------//SOURCE OK//---------------------------------------------

//       recieved_data_substring = recieved_data_substring.substr(first_grid_index+1);
//       std::size_t second_grid_index{recieved_data_substring.find('#')};
      
//       uint32_t rx_can_id{};
//       if(second_grid_index >=4 && second_grid_index <=8)
//       {
//           std::string id{recieved_data_substring.substr(0,second_grid_index)};
//           rx_can_id = std::stoul(id, nullptr, 16);
//       }
//       else is_frame_valid = false;
// //          ---------------------------------//RX_CAN_ID OK//------------------------------------------

//       const auto can_frame_max_data_size_bytes{8};

//       recieved_data_substring = recieved_data_substring.substr(second_grid_index+1);
//       uint8_t can_data[can_frame_max_data_size_bytes]{};
//       if(recieved_data_substring.length() <= can_frame_max_data_size_bytes)
//       {
//           const char* substring{recieved_data_substring.c_str()};
//           for(auto i{0}; i < recieved_data_substring.length(); ++i)
//           {
//               if(substring[i] == '\n')
//                   break;
//               can_data[i] = substring[i];
//           }
//       }
//       else is_frame_valid = false;
//          -------------------------------------//DATA OK//-------------------------------------------
      // if(is_frame_valid)
      // {
      //     CAN_Frame* p_frame{new CAN_Frame(source, rx_can_id, &can_data[0])};
      //     m_rx_can_deque.push_back(p_frame);   
      // }
  // }
}
void Application::TransmitCanFrameToSocket()
{
  if(m_tx_can_deque.size() == 0)
    return;

  volatile int nbytes;

  CAN_Frame* can_frame = m_tx_can_deque.front();
  can_frame->SetID(m_ecu_tx_can_id); //set ECU ID to frame
  m_frame.can_id = can_frame->GetID();

  for (auto i = 0; i < 8; ++i)
    m_frame.data[i] = can_frame->GetData(static_cast<CAN_Frame::DataPos>(i));

  nbytes = write(m_socket, &m_frame, sizeof(m_frame));

  printf("TX: 0x%03X [%d] ",m_frame.can_id, m_frame.can_dlc);
  for (auto i = 0; i < m_frame.can_dlc; i++)
      printf("%02X ",m_frame.data[i]);
  printf("\r\n");
  
  m_tx_can_deque.pop_front(); 
}