#include <sstream>
#include <ios>
#include <fstream>
#include "application.h"
#include "program_timer.h"
#include "did.h"
Application::Application(const uint32_t a_ecu_rx_can_id, const uint32_t a_ecu_functional_can_id, const uint32_t a_ecu_tx_can_id)
  : mref_uds {*(new UDSOnCAN(a_ecu_functional_can_id))} 
  , m_ecu_rx_can_id{a_ecu_rx_can_id}
  , m_ecu_functional_can_id{a_ecu_functional_can_id}
  , m_ecu_tx_can_id{a_ecu_tx_can_id}
  , m_rx_socket_queue{}
  , m_tx_socket_queue{}
  , m_rx_can_deque{}
  , m_tx_can_deque{}
  , m_uds_socket{}
  , m_uds_socket_address{}
  , m_diagmesg_socket{}
  , m_diagmesg_socket_address{}
  , m_cmd_socket{}
  , m_cmd_socket_address{}
  , m_did_repository{mref_uds.GetDIDRepository()}
  , m_dtc_vector{}
{}
Application::~Application()
{
  //delete UDS socket
  if (close(m_uds_socket) < 0)
    std::cout << "socket close error\n";
  
  //delete diagmesg socket
  if (close(m_diagmesg_socket) < 0)
  std::cout << "socket close error\n";
}

bool Application::Execute()
{
  std::system("clear");
  static std::ios_base::fmtflags coutformatflags{std::cout.flags()};

  CreateSocketUDS();
  CreateSocketDiagMesg();

  GenerateDTC();

  while (1)
  {
    CheckDTCStates();
    UpdateDTC();
    std::cout.flags(coutformatflags);

    CheckSocketForNewRxData();
    if(!m_rx_can_deque.empty())
    {
      const CAN_Frame* const can_recieved_frame{m_rx_can_deque.front()};
      m_rx_can_deque.pop_front();
      mref_uds.ConvertCANFrameToUDS(can_recieved_frame);
      delete can_recieved_frame;
    }

    mref_uds.Execute();
 
    if(!mref_uds.IsTXBufferOfUDSEmpty())
    {
      std::vector<CAN_Frame*>frames{mref_uds.ConvertUDSFrameToCAN()};
      for (auto it: frames)
        m_tx_can_deque.push_back(it); 
    }

    auto status{mref_uds.GetStatus()};
    switch(status)
    {
      case UDSOnCAN::Status_is_executing_rx_ff:
        TransmitCanFrameToSocket();
        mref_uds.SetStatus(UDSOnCAN::Status_is_waiting_fcf);
      break;
      
      case UDSOnCAN::Status_recieved_fcf:
        if(!m_tx_can_deque.empty())// and timer.check()
          TransmitCanFrameToSocket();
        else
          mref_uds.SetStatus(UDSOnCAN::Status_ok);
      break;

      case UDSOnCAN::Status_is_waiting_fcf:
      break;
      
      default:
        TransmitCanFrameToSocket();
      break;
    }
    CheckModifiedDids();
    RecieveDataFromDiagSocket();
  }
  return 0;
}

void Application::GenerateDTC()
{
  constexpr auto dtc_file_full_path{"/root/dtc/dtc-information.csv"};
  std::fstream dtc_file(dtc_file_full_path, std::ios::in | std::ios::out);
  const bool file_exists{dtc_file.good()};
  if(!file_exists)
  {
    std::cout << "DTC description file does not exist.\n" <<
    "Creating a new one in /root/dtc directory.\nFilling DTC file with default DTC values.\n";
    std::system("mkdir /root/dtc && touch /root/dtc/dtc-information.csv");

    if(!dtc_file.is_open())
    {
      dtc_file.open(dtc_file_full_path, std::ios::in | std::ios::out| std::ios::trunc);
      std::cout << "DTC file_is " << (dtc_file.is_open()? "opened successfully " : "not opened ") << "after creating\n";
      //here check file for exceptions

      /* B1600*/ dtc_file << DTC::B_Body << ',' << DTC::Standard_VehicleManufacturerSpecific  << ',' << DTC::Subsystem_ComputerOutputCircuit  << ',' << "00,5000,15000\n";
      /* B1601*/ dtc_file << DTC::B_Body << ',' << DTC::Standard_VehicleManufacturerSpecific  << ',' << DTC::Subsystem_ComputerOutputCircuit  << ',' << "01,3000,15000\n";
      /* B1602*/ dtc_file << DTC::B_Body << ',' << DTC::Standard_VehicleManufacturerSpecific  << ',' << DTC::Subsystem_ComputerOutputCircuit  << ',' << "02,100,15000\n";
      /* B1603*/ dtc_file << DTC::B_Body << ',' << DTC::Standard_VehicleManufacturerSpecific  << ',' << DTC::Subsystem_ComputerOutputCircuit  << ',' << "03,1200,15000\n";
      /* B1604*/ dtc_file << DTC::B_Body << ',' << DTC::Standard_VehicleManufacturerSpecific  << ',' << DTC::Subsystem_ComputerOutputCircuit  << ',' << "04,6500,15000\n";
      /* B1605*/ dtc_file << DTC::B_Body << ',' << DTC::Standard_VehicleManufacturerSpecific  << ',' << DTC::Subsystem_ComputerOutputCircuit  << ',' << "05,5000,15000\n";
      /* B1606*/ dtc_file << DTC::B_Body << ',' << DTC::Standard_VehicleManufacturerSpecific  << ',' << DTC::Subsystem_ComputerOutputCircuit  << ',' << "06,50,15000\n";
      /* B1607*/ dtc_file << DTC::B_Body << ',' << DTC::Standard_VehicleManufacturerSpecific  << ',' << DTC::Subsystem_ComputerOutputCircuit  << ',' << "07,3100,15000\n";
      /* B1608*/ dtc_file << DTC::B_Body << ',' << DTC::Standard_VehicleManufacturerSpecific  << ',' << DTC::Subsystem_ComputerOutputCircuit  << ',' << "08,900,15000\n";
      /* B1609*/ dtc_file << DTC::B_Body << ',' << DTC::Standard_VehicleManufacturerSpecific  << ',' << DTC::Subsystem_ComputerOutputCircuit  << ',' << "09,6925,15000\n";
      //write default data in file here
      dtc_file.close();
      dtc_file.open(dtc_file_full_path, std::ios::in | std::ios::out);
    }
  }

  if(!dtc_file.is_open())
  {
    std::cout << "Can not read/create DTC file. Functionality of DTC module in this session is set to default values\n";
    m_dtc_vector.push_back(DTC (DTC::B_Body, DTC::Standard_VehicleManufacturerSpecific, DTC::Subsystem_ComputerOutputCircuit, 1, 1000, 0));
    m_dtc_vector.push_back(DTC (DTC::B_Body, DTC::Standard_VehicleManufacturerSpecific, DTC::Subsystem_ComputerOutputCircuit, 2, 2000, 0));
    m_dtc_vector.push_back(DTC (DTC::B_Body, DTC::Standard_VehicleManufacturerSpecific, DTC::Subsystem_ComputerOutputCircuit, 3, 3000, 0));
    m_dtc_vector.push_back(DTC (DTC::B_Body, DTC::Standard_VehicleManufacturerSpecific, DTC::Subsystem_ComputerOutputCircuit, 4, 4000, 0));
    m_dtc_vector.push_back(DTC (DTC::B_Body, DTC::Standard_VehicleManufacturerSpecific, DTC::Subsystem_ComputerOutputCircuit, 5, 5000, 0));
    m_dtc_vector.push_back(DTC (DTC::B_Body, DTC::Standard_VehicleManufacturerSpecific, DTC::Subsystem_ComputerOutputCircuit, 6, 5500, 0));
    m_dtc_vector.push_back(DTC (DTC::B_Body, DTC::Standard_VehicleManufacturerSpecific, DTC::Subsystem_ComputerOutputCircuit, 7, 3200, 0));
    m_dtc_vector.push_back(DTC (DTC::B_Body, DTC::Standard_VehicleManufacturerSpecific, DTC::Subsystem_ComputerOutputCircuit, 8, 2700, 0));
    m_dtc_vector.push_back(DTC (DTC::B_Body, DTC::Standard_VehicleManufacturerSpecific, DTC::Subsystem_ComputerOutputCircuit, 9, 470, 0));
  }
  else
  {
    std::cout << "Started parsing DTC file \n";
    std::string line{};
    
    while(!dtc_file.eof())
    {
      std::getline(dtc_file, line);
      
      constexpr auto max_line_length{2+2+2+3+7+6};
      constexpr auto min_line_length{2+2+2+2+2+2};
      if(line.length() > max_line_length or line.length() < min_line_length)
        std::cout << "Parsing error!\n";
      else
        std::cout << line << '\n';
      
      auto comma_index{line.find(',')};
      const volatile DTC::Letter letter{(DTC::Letter) std::strtoul(line.c_str(), nullptr, 10)};
      line.erase(line.begin(), line.begin() + comma_index + 1);
      
      comma_index = line.find(',');
      const volatile DTC::Standard standard{(DTC::Standard) std::strtoul(line.c_str(), nullptr, 10)};
      line.erase(line.begin(), line.begin() + comma_index + 1);

      comma_index = line.find(',');
      const volatile DTC::Subsystem subsystem{(DTC::Subsystem) std::strtoul(line.c_str(), nullptr, 10)};
      line.erase(line.begin(), line.begin() + comma_index + 1);
      
      comma_index = line.find(',');
      const volatile uint32_t fault_description{(uint32_t) std::strtoul(line.c_str(), nullptr, 10)};
      line.erase(line.begin(), line.begin() + comma_index + 1);

      comma_index = line.find(',');
      const volatile uint32_t activeflag_threshold{(uint32_t) std::strtoul(line.c_str(), nullptr, 10)};
      line.erase(line.begin(), line.begin() + comma_index + 1);

      const volatile uint32_t saveflag_threshold{(uint32_t) std::strtoul(line.c_str(), nullptr, 10)};


      //check end of string
      //if !end_of_string
      //cout error

      asm("nop");
    }
    
  }

  dtc_file.close();
}


void Application::CreateSocketUDS()
{
  printf("Creating UDS socket... ");
  if ((m_uds_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    perror("Socket");
  printf("Done\n");
  printf("Trying to connect...\n");
  m_uds_socket_address.sun_family = AF_UNIX;
  auto socket_address{"/tmp/uds.sock"};
  strcpy(m_uds_socket_address.sun_path, socket_address);
  
  auto len{strlen(m_uds_socket_address.sun_path) + sizeof(m_uds_socket_address.sun_family)};
  
  int status = fcntl(m_uds_socket, F_SETFL, fcntl(m_uds_socket, F_GETFL, 0) | O_NONBLOCK);
  if(status == -1)
    perror("calling fcntl");
  
  if(connect(m_uds_socket, (sockaddr*)&m_uds_socket_address, len) == -1)
    perror("connect");
  
  printf("Connected to UDS socket.\n\n");
}
void Application::CreateSocketDiagMesg()
{
  printf("Creating DiagMesg socket... ");
  if ((m_diagmesg_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("Socket");
    //return 1;
  }
  printf("Done\n");
  printf("Trying to connect...\n");
  m_diagmesg_socket_address.sun_family = AF_UNIX;
  auto socket_address{"/tmp/udscmd.sock"};
   strcpy(m_diagmesg_socket_address.sun_path, socket_address);
  volatile auto len{strlen(m_diagmesg_socket_address.sun_path) + sizeof(m_diagmesg_socket_address.sun_family)};
  int status = fcntl(m_diagmesg_socket, F_SETFL, fcntl(m_diagmesg_socket, F_GETFL, 0) | O_NONBLOCK);
  if (status == -1){
    perror("calling fcntl");
    // handle the error.  By the way, I've never seen fcntl fail in this way
  }
  if (connect(m_diagmesg_socket, (sockaddr*)&m_diagmesg_socket_address, len) == -1)
  {
    perror("connect");
    //exit(1);
  }
  printf("Connected to DiagMesg socket.\n\n");



  printf("Creating CmdTest socket... ");
  if ((m_cmd_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("Socket");
    //return 1;
  }
  printf("Done\n");
  printf("Trying to connect...\n");
  m_cmd_socket_address.sun_family = AF_UNIX;
  auto cmd_socket_address{"/tmp/cmd.sock"};
   strcpy(m_cmd_socket_address.sun_path, cmd_socket_address);
  volatile auto length{strlen(m_cmd_socket_address.sun_path) + sizeof(m_cmd_socket_address.sun_family)};
  int cmd_status = fcntl(m_cmd_socket, F_SETFL, fcntl(m_cmd_socket, F_GETFL, 0) | O_NONBLOCK);
  if (cmd_status == -1){
    perror("calling fcntl");
    // handle the error.  By the way, I've never seen fcntl fail in this way
  }
  if (connect(m_cmd_socket, (sockaddr*)&m_cmd_socket_address, length) == -1)
  {
    perror("connect");
    //exit(1);
  }
  printf("Connected to CmdTest socket.\n\n");

  //std::string tx_data{"DID.F1A3.BOOL.TRUE\n"};
  // auto string_length{strlen(tx_data.c_str())+1};
  // if(send(m_cmd_socket, tx_data.c_str(), string_length, 0) == -1)
  //   perror("send");
}
void Application::CheckModifiedDids()
{
  std::vector<DID_Instance*> modified_dids{m_did_repository.GetListOfModifiedDIDs()}; 
  for(auto it{modified_dids.begin()}; it != modified_dids.end(); ++it)
  {
    std::stringstream ss;
    const DID_Instance::DID_Datatype datatype{(*it)->GetDataType()};
    const DID did_number{(*it)->GetDID()};
    //did_number == 0X0200 ? did_number = (DID)0xF1A2: true;
    ss << "DID." << std::hex << (uint16_t)did_number  << ".";
    switch (datatype)
    {
      case DID_Instance::DID_Datatype_std_string:
      case DID_Instance::DID_Datatype_c_string:
      {
        volatile auto t{0};
        const char* str_value{reinterpret_cast<const char*>((*it)->GetPtrToConstData())};
        ss << "STR." << str_value; //<< '\n';
      }
      break;
      case DID_Instance::DID_Datatype_float:
      {
        float flt_value{*(float*)(*it)->GetPtrToConstData()};
        ss << "FLT." << flt_value;
      }
      break;
      case DID_Instance::DID_Datatype_double:
      {
        double dbl_value{*(double*)(*it)->GetPtrToConstData()};
        ss << "DBL." << dbl_value;
      }
      break;
      case DID_Instance::DID_Datatype_integer:
      {
        int32_t i_value{*(int32_t*)(*it)->GetPtrToData()};
        ss << "INT." << std::hex << i_value; 
      }
      break;
      case DID_Instance::DID_Datatype_unsigned_integer:
      {
        int64_t ui_value{*(uint32_t*)(*it)->GetPtrToData()};
        ss << "UINT." << std::hex << ui_value;   //UINT.
      }
      break;
      case DID_Instance::DID_Datatype_bool:
      {
        bool b_value{*(bool*)(*it)->GetPtrToData()};
        ss << "BOOL." << (b_value ? "TRUE" : "FALSE"); 
      }
      break;
    }
    std::string transmit_data{ss.str()};    
    auto string_length{strlen(transmit_data.c_str())+1};
    if(send(m_diagmesg_socket, (transmit_data +'\n').c_str(), string_length, 0) == -1)
      perror("send");
  }
}
void Application::RecieveDataFromDiagSocket()
{
  static std::string recieved_data("");
  char string[100];
  auto t{recv(m_diagmesg_socket, string, 100, 0)};
  if (t > 0)
  {
    recieved_data.append(string);
    string[t] = '\0';
    printf("DIAG RX>%s", string);
  }

  std::size_t newline_index{recieved_data.find('\n')};
  if(newline_index and recieved_data.size())
  {
    volatile bool is_string_valid{true};
    
    std::string recieved_data_substring{recieved_data.substr(0, newline_index)};
    recieved_data = recieved_data.substr(newline_index+1);

    std::size_t first_dot_index{recieved_data_substring.find('.')};
    std::string did_text{recieved_data_substring.substr(0,first_dot_index)};
    recieved_data_substring = recieved_data_substring.substr(first_dot_index+1);

    std::size_t second_dot_index{recieved_data_substring.find('.')};
    std::string did_substring{recieved_data_substring.substr(0,second_dot_index)};
    did_substring.insert(0, "0x");
    volatile DID did{static_cast<DID>(std::stoul(did_substring, nullptr, 16))};
    recieved_data_substring = recieved_data_substring.substr(second_dot_index+1);   //DID OK

    std::size_t third_dot_index{recieved_data_substring.find('.')};
    std::string datatype{recieved_data_substring.substr(0,third_dot_index)};
    recieved_data_substring = recieved_data_substring.substr(third_dot_index+1);

    if(datatype == "STR")
    {
      m_did_repository.WriteDataIdentifier(did,recieved_data_substring);
    }
    else if(datatype == "BOOL")
    {
      const bool value {recieved_data_substring == "TRUE" ? true : false};
      m_did_repository.WriteDataIdentifier(did,(const uint8_t*)&value, sizeof(value));
    }
    else if(datatype == "INT")
    {
      const int value {std::stoi(recieved_data_substring)};
      m_did_repository.WriteDataIdentifier(did,(const uint8_t*)&value, sizeof(value));
    }
    else if(datatype == "UINT")
    {
      const unsigned value {(unsigned)std::stoul(recieved_data_substring)};
      m_did_repository.WriteDataIdentifier(did,(const uint8_t*)&value, sizeof(value));
    }
    else if(datatype == "FLT")
    {
      const float value {std::stof(recieved_data_substring)};
      m_did_repository.WriteDataIdentifier(did,(const uint8_t*)&value, sizeof(value));
    }
    else if(datatype == "DBL")
    {
      const double value {std::stod(recieved_data_substring)};
      m_did_repository.WriteDataIdentifier(did,(const uint8_t*)&value, sizeof(value));
    }
  }  
}
void Application::CheckSocketForNewRxData()
{  
  static std::string recieved_data("");

  constexpr auto recieve_size{100};
  
  char string[recieve_size];
  auto t = recv(m_uds_socket, string, recieve_size, 0);

  if(t > 0)
  {
    string[t] = '\0';
    recieved_data.append(string);
    //std::cout << "recieved " << (int64_t)t << " bytes of data" << std::endl;
    //for(volatile auto i{0}; i < t; ++i)
    //std::cout << std::dec << (unsigned)string[i] << ' ';
    //std::cout << '\n';
  }
  
  const auto newline_index{recieved_data.find('\n')};
  if(recieved_data.size() and newline_index != std::string::npos)
  {
    volatile bool is_frame_valid{true};

    std::string recieved_data_substring{recieved_data.substr(0, newline_index)};
    const auto end_iterator{recieved_data.begin() + newline_index + 1};
    recieved_data.erase(recieved_data.begin(), end_iterator);
    
    std::size_t first_grid_index{recieved_data_substring.find('#')};
    if(first_grid_index == std::string::npos)
      is_frame_valid = false;

    CAN_Frame::Source source{};
    if(is_frame_valid)
    {
      source = static_cast<CAN_Frame::Source>(std::stoul(recieved_data_substring.substr(0,first_grid_index)));
      if(source < CAN_Frame::Source_CAN1 or source >= CAN_Frame::Source_CAN3)
      {
        source = CAN_Frame::Source_Unknown;
        is_frame_valid = false;
      }//          ---------------------------------//SOURCE OK//---------------------------------------------
    }

    std::size_t second_grid_index{0};
    if(is_frame_valid)
    {
      recieved_data_substring = recieved_data_substring.substr(first_grid_index+1);
      second_grid_index = recieved_data_substring.find('#');
      if(second_grid_index == std::string::npos)
        is_frame_valid = false;
    }
        
    uint32_t rx_can_id{};
    if(is_frame_valid and second_grid_index >= 1 and second_grid_index <= 8)
    {
      std::string id{recieved_data_substring.substr(0,second_grid_index)};
      rx_can_id = std::stoul(id, nullptr, 16);//          ---------------------------------//RX_CAN_ID OK//------------------------------------------
    }
    if(!(rx_can_id == m_ecu_rx_can_id or rx_can_id == m_ecu_functional_can_id))
      is_frame_valid = false; 

    constexpr auto can_frame_data_size_bytes{8};
    uint8_t can_data[can_frame_data_size_bytes]{};

    recieved_data_substring = recieved_data_substring.substr(second_grid_index+1); 
    uint8_t length{static_cast<uint8_t>(recieved_data_substring.length())};
    constexpr auto min_substring_size{3};
    constexpr auto max_substring_size{23};
    if(length >= min_substring_size and length <= max_substring_size)
    {
      const char* substring{recieved_data_substring.c_str()};
      for(auto i{0}; i < length; i+=3)
      {
        std::string byte("0x");
        byte.append(recieved_data_substring.substr(i,2));
        auto index{i/3};
        can_data[index] = std::stoul(byte, nullptr, 16);//   -------------------------------------//DATA OK//-------------------------------------------
      }
    }
    else is_frame_valid = false;

    if(is_frame_valid)
    {
      CAN_Frame* p_frame{new CAN_Frame(source, rx_can_id, &can_data[0])};
      m_rx_can_deque.push_back(p_frame);
      std::cout << std::hex << "RX> " << (int)source << '#' << rx_can_id << '#' << (int)can_data[0] << '.' << (int)can_data[1] << '.'
                                                                                        << (int)can_data[2] << '.' << (int)can_data[3] << '.'
                                                                                        << (int)can_data[4] << '.' << (int)can_data[5] << '.'
                                                                                        << (int)can_data[6] << '.' << (int)can_data[7] << std::endl;
    }
    else
      std::cout << std::hex << "[NOT VALID] RX> " << (int)source << '#' << rx_can_id << '#' << (int)can_data[0] << '.' << (int)can_data[1] << '.'
                                                                                        << (int)can_data[2] << '.' << (int)can_data[3] << '.'
                                                                                        << (int)can_data[4] << '.' << (int)can_data[5] << '.'
                                                                                        << (int)can_data[6] << '.' << (int)can_data[7] << std::endl;
  }
}
void Application::TransmitCanFrameToSocket()
{
  if(m_tx_can_deque.size() == 0)
    return;

  volatile int nbytes;
  CAN_Frame* can_frame = m_tx_can_deque.front();
  can_frame->SetID(m_ecu_tx_can_id); //set ECU ID to frame

  std::string transmit_data("");
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
  
  stringstream << "\n";
  transmit_data.append(stringstream.str());
  volatile auto string_length{strlen(transmit_data.c_str())};
  auto bts = send(m_uds_socket, transmit_data.c_str(), string_length, 0);
  if (bts == -1) 
    perror("send");
  else
    std::cout << "TX< " << transmit_data; //no endl

  m_tx_can_deque.pop_front(); 
}
void Application::UpdateDTC()
{

}
void Application::CheckDTCStates()
{
  if(DTC::Check1msTimer())
  {
    for(auto& dtc: m_dtc_vector)
      dtc.Check();
  }

  constexpr auto restart_from_watchdog_error{"B1600"};
  constexpr auto high_voltage_error {"B1601"};
  constexpr auto low_voltage_error {"B1602"};
  constexpr auto gnss_module_error {"B1603"};
  constexpr auto gyro_accel_access_error{"B1604"};
  constexpr auto btp_firmware_update_error{"B1605"};
  constexpr auto btp_kamaz_relief_map_load_error{"B1606"};
  constexpr auto mcu_firmwareintegrity_error{"B1607"};
  constexpr auto bip_error{"B1608"};
  
  for(auto& dtc: m_dtc_vector)
  {
    //if dtc personal timer is expired
    //instead of all dtc check move check procedure here

    const std::string abbreviation{dtc.GetAbbreviation()};

    if(abbreviation == restart_from_watchdog_error)
    {
      bool error_flag{false};
      m_did_repository.ReadDataIdentifier(DID_RestartFromWatchdogError, (uint8_t*)&error_flag, sizeof(error_flag));
      dtc.SetConditionFailedFlag(error_flag);

      constexpr uint16_t bitmask_for_diagdata_did{0x01};
      uint16_t did_diagdata{};
      m_did_repository.LE_ReadDataIdentifier(DID_DiagData, (uint8_t*)&did_diagdata, sizeof(did_diagdata));
      
      const bool current_state_in_diagdata = did_diagdata & bitmask_for_diagdata_did;
      const bool desirable_state_in_diagdata{dtc.IsActive()};
      if(current_state_in_diagdata != desirable_state_in_diagdata)
      {
        did_diagdata &= ~bitmask_for_diagdata_did;
        did_diagdata |= error_flag ? bitmask_for_diagdata_did : 0;
        m_did_repository.LE_WriteDataIdentifier(DID_DiagData, (uint8_t*)&did_diagdata, sizeof(did_diagdata));
      }
    }

    if(abbreviation == high_voltage_error)
    {
      bool error_flag{false};
      m_did_repository.ReadDataIdentifier(DID_HighVoltageError, (uint8_t*)&error_flag, sizeof(error_flag));
      dtc.SetConditionFailedFlag(error_flag);

      constexpr uint16_t bitmask_for_diagdata_did{0x02};
      uint16_t did_diagdata{};
      m_did_repository.LE_ReadDataIdentifier(DID_DiagData, (uint8_t*)&did_diagdata, sizeof(did_diagdata));
      
      const bool current_state_in_diagdata = did_diagdata & bitmask_for_diagdata_did;
      const bool desirable_state_in_diagdata{dtc.IsActive()};
      if(current_state_in_diagdata != desirable_state_in_diagdata)
      {
        did_diagdata &= ~bitmask_for_diagdata_did;
        did_diagdata |= error_flag ? bitmask_for_diagdata_did : 0;
        m_did_repository.LE_WriteDataIdentifier(DID_DiagData, (uint8_t*)&did_diagdata, sizeof(did_diagdata));
      }
    }

    if(abbreviation == low_voltage_error)
    {
      bool error_flag{false};
      m_did_repository.ReadDataIdentifier(DID_LowVoltageError, (uint8_t*)&error_flag, sizeof(error_flag));
      dtc.SetConditionFailedFlag(error_flag);

      constexpr uint16_t bitmask_for_diagdata_did{0x04};
      uint16_t did_diagdata{};
      m_did_repository.LE_ReadDataIdentifier(DID_DiagData, (uint8_t*)&did_diagdata, sizeof(did_diagdata));
      
      const bool current_state_in_diagdata = did_diagdata & bitmask_for_diagdata_did;
      const bool desirable_state_in_diagdata{dtc.IsActive()};
      if(current_state_in_diagdata != desirable_state_in_diagdata)
      {
        did_diagdata &= ~bitmask_for_diagdata_did;
        did_diagdata |= error_flag ? bitmask_for_diagdata_did : 0;
        m_did_repository.LE_WriteDataIdentifier(DID_DiagData, (uint8_t*)&did_diagdata, sizeof(did_diagdata));
      }
    }

    if(abbreviation == gnss_module_error)
    {
      bool error_flag{false};
      m_did_repository.ReadDataIdentifier(DID_GNSSModuleError, (uint8_t*)&error_flag, sizeof(error_flag));
      dtc.SetConditionFailedFlag(error_flag);

      constexpr uint16_t bitmask_for_diagdata_did{0x08};
      uint16_t did_diagdata{};
      m_did_repository.LE_ReadDataIdentifier(DID_DiagData, (uint8_t*)&did_diagdata, sizeof(did_diagdata));
      
      const bool current_state_in_diagdata = did_diagdata & bitmask_for_diagdata_did;
      const bool desirable_state_in_diagdata{dtc.IsActive()};
      if(current_state_in_diagdata != desirable_state_in_diagdata)
      {
        did_diagdata &= ~bitmask_for_diagdata_did;
        did_diagdata |= error_flag ? bitmask_for_diagdata_did : 0;
        m_did_repository.LE_WriteDataIdentifier(DID_DiagData, (uint8_t*)&did_diagdata, sizeof(did_diagdata));
      }
    }

    if(abbreviation == gyro_accel_access_error)
    {
      bool error_flag{false};
      m_did_repository.ReadDataIdentifier(DID_GyroAccelAccessError, (uint8_t*)&error_flag, sizeof(error_flag));
      dtc.SetConditionFailedFlag(error_flag);

      constexpr uint16_t bitmask_for_diagdata_did{0x10};
      uint16_t did_diagdata{};
      m_did_repository.LE_ReadDataIdentifier(DID_DiagData, (uint8_t*)&did_diagdata, sizeof(did_diagdata));
      
      const bool current_state_in_diagdata = did_diagdata & bitmask_for_diagdata_did;
      const bool desirable_state_in_diagdata{dtc.IsActive()};
      if(current_state_in_diagdata != desirable_state_in_diagdata)
      {
        did_diagdata &= ~bitmask_for_diagdata_did;
        did_diagdata |= error_flag ? bitmask_for_diagdata_did : 0;
        m_did_repository.LE_WriteDataIdentifier(DID_DiagData, (uint8_t*)&did_diagdata, sizeof(did_diagdata));
      }
    }

    if(abbreviation == btp_firmware_update_error)
    {
      constexpr uint16_t bitmask_for_diagdata_did{0x20};
    }

    if(abbreviation == btp_kamaz_relief_map_load_error)
    {
      constexpr uint16_t bitmask_for_diagdata_did{0x40};
    }

    if(abbreviation == mcu_firmwareintegrity_error)
    {
      constexpr uint16_t bitmask_for_diagdata_did{0x80};
    }

    if(abbreviation == bip_error)
    {
      bool error_flag{false};
      m_did_repository.ReadDataIdentifier(DID_BipError, (uint8_t*)&error_flag, sizeof(error_flag));
      dtc.SetConditionFailedFlag(error_flag);

      constexpr uint16_t bitmask_for_diagdata_did{0x100};
      uint16_t did_diagdata{};
      m_did_repository.LE_ReadDataIdentifier(DID_DiagData, (uint8_t*)&did_diagdata, sizeof(did_diagdata));
      
      const bool current_state_in_diagdata = did_diagdata & bitmask_for_diagdata_did;
      const bool desirable_state_in_diagdata{dtc.IsActive()};
      if(current_state_in_diagdata != desirable_state_in_diagdata)
      {
        did_diagdata &= ~bitmask_for_diagdata_did;
        did_diagdata |= error_flag ? bitmask_for_diagdata_did : 0;
        m_did_repository.LE_WriteDataIdentifier(DID_DiagData, (uint8_t*)&did_diagdata, sizeof(did_diagdata));
      }
    }
  }
}