#include <sstream>
#include "application.h"
#include "program_timer.h"

Application::Application(const uint32_t a_ecu_rx_can_id, const uint32_t a_ecu_tx_can_id)
  : mref_uds {*(new UDSOnCAN())} 
  , m_ecu_rx_can_id{a_ecu_rx_can_id}
  , m_ecu_tx_can_id{a_ecu_tx_can_id}
  , m_rx_socket_queue{}
  , m_tx_socket_queue{}
  , m_rx_can_deque{}
  , m_tx_can_deque{}
  , m_socket{}
  , m_addr{}
{
}

Application::~Application()
{
  if (close(m_socket) < 0)
    std::cout << "socket close error\n";
}

bool Application::Execute()
{
  printf("Creating socket... ");
  if ((m_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("Socket");
    return 1;
  }
  printf("Done\n");
  printf("Trying to connect...\n");

  m_addr.sun_family = AF_UNIX;
  auto socket_address{"/tmp/uds.sock"};
   strcpy(m_addr.sun_path, socket_address);

  volatile auto len{strlen(m_addr.sun_path) + sizeof(m_addr.sun_family)};

  if (connect(m_socket, (sockaddr*)&m_addr, len) == -1)
  {
    perror("connect");
    exit(1);
  }
  printf("Connected.\n");

  while (1)
  {
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
  return 0;
}

void Application::CheckSocketForNewRxData()
{  
  static std::string recieved_data{};
  char string[100];
  auto t{recv(m_socket, string, 100, 0)};
  if (t > 0) {
    recieved_data.append(string);
      string[t] = '\0';
      printf("RX>%s", string);
  } else {
      if (t < 0) perror("recv");
      else printf("Server closed connection\n");
      exit(1);
  }

  // recieved_data.append("001#A34E3FF8#thjdkb\n"); //only for test
  //recieved_data.append(m_rx_socket_queue.back());
  std::size_t newline_index{recieved_data.find('\n')};
  if(newline_index and recieved_data.size())
  {
    volatile bool is_frame_valid{true};

    std::string recieved_data_substring{recieved_data.substr(0, newline_index)};
    recieved_data = recieved_data.substr(newline_index+1);    
    
    std::size_t first_grid_index{recieved_data_substring.find('#')};

    CAN_Frame::Source source{};// источник 1 байт  CAN1..3
    source = static_cast<CAN_Frame::Source>(std::stoul(recieved_data_substring.substr(0,first_grid_index)));
    if(source < CAN_Frame::Source_CAN1 || source >= CAN_Frame::Source_CAN3)
    {
      source = CAN_Frame::Source_Unknown;
      is_frame_valid = false;
    }
    else
//          ---------------------------------//SOURCE OK//---------------------------------------------

    recieved_data_substring = recieved_data_substring.substr(first_grid_index+1);
    std::size_t second_grid_index{recieved_data_substring.find('#')};
    
    uint32_t rx_can_id{};
    if(second_grid_index >=3 && second_grid_index <=8)
    {
      std::string id{recieved_data_substring.substr(0,second_grid_index)};
      rx_can_id = std::stoul(id, nullptr, 16);
    }
//    else is_frame_valid = false;

//          ---------------------------------//RX_CAN_ID OK//------------------------------------------

    constexpr auto can_frame_data_size_bytes{8};
    recieved_data_substring = recieved_data_substring.substr(second_grid_index+1);
    uint8_t can_data[can_frame_data_size_bytes]{};
    uint8_t length{recieved_data_substring.length()};
    constexpr auto min_substring_size{3};
    constexpr auto max_substring_size{23};
    
    if(length >= min_substring_size and length <= max_substring_size)
    {
      const char* substring{recieved_data_substring.c_str()};
      for(auto i{0}; i < length; i+=3)
      {
        std::string byte{"0x"};
        byte.append(recieved_data_substring.substr(i,2));
        auto index{i/3};
        can_data[index] = std::stoul(byte, nullptr, 16);
      }
    }
    else is_frame_valid = false;
    //   -------------------------------------//DATA OK//-------------------------------------------
    if(is_frame_valid)
    {
      CAN_Frame* p_frame{new CAN_Frame(source, rx_can_id, &can_data[0])};
      m_rx_can_deque.push_back(p_frame);   
    }
  }
}
void Application::TransmitCanFrameToSocket()
{
  if(m_tx_can_deque.size() == 0)
    return;

  volatile int nbytes;
  CAN_Frame* can_frame = m_tx_can_deque.front();
  can_frame->SetID(m_ecu_tx_can_id); //set ECU ID to frame

  std::string transmit_data{};
  uint32_t source{static_cast<uint32_t>(can_frame->GetSource())};// источник 1 байт  CAN1..3
  std::stringstream stringstream;
  
  stringstream << std::hex << source << "#";

  uint32_t tx_can_id{can_frame->GetID()};

  stringstream << std::hex << tx_can_id << "#";
  constexpr auto can_frame_data_size{8};
  for(auto i{0}; i < can_frame_data_size; ++i)
  {
    CAN_Frame::DataPos datapos{static_cast<CAN_Frame::DataPos>(i)};
    uint32_t data{can_frame->GetData(datapos)};
    if(data < 0x10)
      stringstream << "0";

    stringstream << std::hex << data;
    if(datapos != CAN_Frame::DataPos_7)
      stringstream << ".";
  }
  
  transmit_data.append(stringstream.str());
  std::cout << "TX<";
  std::cout << transmit_data << "\n\n";
  auto string_length{strlen(transmit_data.c_str())+1};
  //const char * strf 
  if (send(m_socket, (transmit_data +'\n').c_str(), string_length, 0) == -1) {
    perror("send");
    exit(1);
  }
  
  m_tx_can_deque.pop_front(); 
}