#include <iostream>
#include <random>

#include "uds.h"

#include <chrono>

UDS::UDS()
  : m_status{Status_ok}
  , m_sessiontype{DSC_Type_DefaultSession}
  , m_seed_size{Seed_size_5_byte}
  , m_seed{0}
  , m_key{0}
{
}

void UDS::SetUDSStatus(const Status a_status)
{
  m_status = a_status;
}
UDS::Status UDS::GetUDSStatus()
{
  return m_status;
}
void UDS::SetSessionType(const SessionType a_sessiontype)
{
  if(m_sessiontype == a_sessiontype)
    return;
  
  m_sessiontype = a_sessiontype;
}
UDS::SessionType UDS::GetSessiontype()
{
  return m_sessiontype;
}

bool UDS::CheckNumberOfSecurityAccessAttempts(const uint8_t a_subfunction)
{
  switch(a_subfunction)
  {
    case 0x01:
    {
      return m_programmingsession_number_of_attempts--;
    }
    case 0x03:
    {
      return m_extendeddiagnosticsession_number_of_attempts--;
    }
    case 0x05:
    {
      return m_safetysystemdiagnosticsession_number_of_attempts--;
    }
    default:
      return false;
  }
  return false;
}

void UDS::ReloadNumberOfSecurityAccessAttempts()
{
  m_programmingsession_number_of_attempts = 5;
  m_extendeddiagnosticsession_number_of_attempts = 5;
  m_safetysystemdiagnosticsession_number_of_attempts = 5;
}


UDSOnCAN::UDSOnCAN()
  : UDS{}
{
  m_did_repository.AddDataIdentifier(DID_VehicleManufacturerECUSoftwareConfigurationNumber,        32, DID_Instance::DID_Datatype_c_string, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_VehicleManufacturerECUSoftwareConfigurationVersionNumber, 32, DID_Instance::DID_Datatype_c_string, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_FirmwareUpdateMode,                                       sizeof(uint8_t/*bool*/),  DID_Instance::DID_Datatype_bool, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_MapUpdateMode,                                            sizeof(uint8_t/*bool*/),  DID_Instance::DID_Datatype_bool, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_RS232_1_BaudrateSetup,                                    sizeof(unsigned),  DID_Instance::DID_Datatype_unsigned_integer, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_RS232_2_BaudrateSetup,                                    sizeof(unsigned),  DID_Instance::DID_Datatype_unsigned_integer, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_RS485_BaudrateSetup,                                      sizeof(unsigned),  DID_Instance::DID_Datatype_unsigned_integer, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_WiFiPassword,                                             64, DID_Instance::DID_Datatype_c_string, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_DiagData,                                                 2,   DID_Instance::DID_Datatype_bytearray, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_VehicleManufacturerECUSoftwareNumber,                     20,  DID_Instance::DID_Datatype_c_string, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_VehicleManufacturerECUSoftwareVersionNumber,              32,  DID_Instance::DID_Datatype_c_string, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_SystemSupplierIdentifierDataIdentifier,                   32,  DID_Instance::DID_Datatype_c_string, DID_Instance::Readonly);
  m_did_repository.AddDataIdentifier(DID_ECUManufacturingDate,                                     16,  DID_Instance::DID_Datatype_c_string, DID_Instance::Readonly);
  m_did_repository.AddDataIdentifier(DID_ECUSerialNumber,                                          253, DID_Instance::DID_Datatype_c_string, DID_Instance::Readonly);
  m_did_repository.AddDataIdentifier(DID_VIN,                                                      17, DID_Instance::DID_Datatype_c_string, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_VehicleManufacturerECUHardwareNumber,                     32, DID_Instance::DID_Datatype_c_string, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_SystemSupplierECUHardwareNumber,                          32, DID_Instance::DID_Datatype_c_string, DID_Instance::Readonly);
  m_did_repository.AddDataIdentifier(DID_SystemSupplierECUHardwareVersionNumber,                   16, DID_Instance::DID_Datatype_c_string, DID_Instance::Readonly);
  m_did_repository.AddDataIdentifier(DID_SystemSupplierECUSoftwareNumber,                          128, DID_Instance::DID_Datatype_c_string, DID_Instance::Readonly);
  m_did_repository.AddDataIdentifier(DID_SystemSupplierECUSoftwareVersionNumber,                   128, DID_Instance::DID_Datatype_c_string, DID_Instance::Readonly);
  m_did_repository.AddDataIdentifier(DID_ExhaustRegulationOrTypeApprovalNumber,                    64, DID_Instance::DID_Datatype_c_string, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_SystemNameOrEngineType,                                   64, DID_Instance::DID_Datatype_c_string, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_RepairShopCodeOrTesterSerialNumber,                       32, DID_Instance::DID_Datatype_c_string, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_ProgrammingDate,                                          16, DID_Instance::DID_Datatype_c_string, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_ECUInstallationDate,                                      16, DID_Instance::DID_Datatype_c_string, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_SystemSupplierBaseConfiguration,                          18, DID_Instance::DID_Datatype_c_string, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_BaseSoftwareVersion,                                      128, DID_Instance::DID_Datatype_c_string, DID_Instance::Readonly);
//lock necessary dids
  uint32_t default_async_interfaces_speed_kbaud{19200};
  m_did_repository.WriteDataIdentifier(DID_RS232_1_BaudrateSetup, (uint8_t*)&default_async_interfaces_speed_kbaud, sizeof(default_async_interfaces_speed_kbaud));
  m_did_repository.WriteDataIdentifier(DID_RS232_2_BaudrateSetup, (uint8_t*)&default_async_interfaces_speed_kbaud, sizeof(default_async_interfaces_speed_kbaud));
  m_did_repository.WriteDataIdentifier(DID_RS485_BaudrateSetup, (uint8_t*)&default_async_interfaces_speed_kbaud, sizeof(default_async_interfaces_speed_kbaud));
  {
      const char* string = "123dfklgdfskgds g";
      volatile uint8_t str_len = strlen(string);

      m_did_repository.WriteDataIdentifier(DID_VIN, string);
      // m_did_repository.WriteDataIdentifier(DID_VIN, std::string("123dfklgdfskgds"));//DID_ExhaustRegulationOrTypeApprovalNumber
      char str[50]{};
      std::string t2 = m_did_repository.ReadDataIdentifier(DID_VIN);
      m_did_repository.ReadDataIdentifier(DID_VIN, (uint8_t*)&str[0], str_len);
      volatile auto t(7);
    }
}
UDSOnCAN::~UDSOnCAN(){}

bool UDSOnCAN::ConvertCANFrameToUDS(const CAN_Frame* const ap_can_frame)
{
// //PHYSICAL ADRESSING OR FUNCTIONAL ADRESSING:
// if(ap_can_frame->GetID() =< 0X7FF)
// {}
// else
// {
//  normal fixed addressing. 29 bit CAN ID.
//  physical addressed messages CAN_ID = 0x18DATTSS
//  functional addressed messages CAN_ID = 0x18DBTTSS
// where:
//         CAN_ID - value of CAN Identifier
//         TT - two (hexadecimal) digits of a 8-bit Target Address value
//         SS - two (hexadecimal) digits of a 8-bit Source Address value
//}

  UDS_Frame* rx_frame{new UDS_Frame};
  volatile UDS_Frame::PCI pci{(UDS_Frame::PCI)((ap_can_frame->GetData(CAN_Frame::DataPos_0) & 0xF0) >> 4)};
    asm("nop");
  //if(ap_can_frame.dlc<8)//check dlc
  //uds frame only valid  if Single Frame, Flow Control and Consecutive Frame that use CAN frame data optimization

  switch(pci)
  {
    case UDS_Frame::PCI::PCI_SingleFrame:
    {
      uint8_t payload {uint8_t(ap_can_frame->GetData(CAN_Frame::DataPos_0) & 0x0F)};
      rx_frame->SetSID(static_cast<UDS_Frame::Service>(ap_can_frame->GetData(CAN_Frame::DataPos_1)));
      rx_frame->SetData(ap_can_frame->GetDataPtr(CAN_Frame::DataPos_2), payload-1, 0);
      rx_frame->SetDataLength(payload-1);
      //dlc < 8 ? single frame data length byte 1 bits 3..0 : single frame data length byte 2
    }
    break;

    case UDS_Frame::PCI::PCI_FirstFrame:
    {
      uint16_t data_length{(ap_can_frame->GetData(CAN_Frame::DataPos_0) & 0x0F) << 4 || ap_can_frame->GetData(CAN_Frame::DataPos_1)};
      rx_frame->SetDataLength(data_length);
    // byte 1 bits 3..0 != 0 ? ff_dl = bits 3..0 of byte 1
    // else if bits 3..0 of byte 1 == 0 && byte 2 == 00 ff_dl = byte 3
    }
    break;

    case UDS_Frame::PCI::PCI_ConsecutiveFrame:
    //check last frame & check frame sid
    //else select offset by frame number
    //if data finished perform actions in execute
    //bits 3..0 of byte 1 is sequence number
    break;

    case UDS_Frame::PCI::PCI_FlowControlFrame:
    //IN RESPONSE:
    //SET FLOWSTATUS
    //SET BLOCKSIZE
    //SET STMIN
    // IN REQUEST EXECURE ALL THIS PARAMETERS


    //bits 3..0 of byte 1 is flow status
    // byte 2 is block size
    // byte 3 is ST_min (Separation Time minimum)
    // bytes 4..8 are not N/A
    
      m_status = Status_ok;
    break;

    default:
    break;
  }

  // if(!check valid values)
  //  return false;
  m_uds_rx_buffer.push_back(rx_frame);
    return true;  
}
void UDSOnCAN::Execute()
{
  if(!m_uds_rx_buffer.empty())
  {
    UDS_Frame* uds_frame{m_uds_rx_buffer.front()};
    m_uds_rx_buffer.pop_front();

    switch(uds_frame->GetSID())
    {
      case UDS_Frame::Service::DiagnosticSessionControl:
      {
        uint8_t sessiontype{*(uds_frame->GetData())};
        if(sessiontype > DSC_Type_SafetySystemDiagnosticSession)
        {
          MakeNegativeResponse(uds_frame->GetSID(), UDS_Frame::NRC_IncorrectMessageLengthOrInvalidFormat);
          break;
        }
        std::cout << "sessiontype: " << sessiontype << '\n'; //<< "sessiontype: "
        //check session change condition
        //if ok
        SetSessionType(static_cast<UDS::SessionType>(sessiontype));
        std::cout << "m_sessiontype: " << m_sessiontype << '\n'; //<< "sessiontype: "

        MakePositiveResponse(uds_frame->GetSID(), &sessiontype, 1);
      }
      break;

      case UDS_Frame::Service::SecurityAccess:
      {
        constexpr auto subfunction_size{1};
        constexpr auto seed_size{5};
        const uint8_t subfunction{*(uds_frame->GetData())};
        //check current diagnostic session & how to jump to it
        //if session is !ok, transmit negative response

        //if session is not correct
        //MakeNegativeResponse(uds_frame->GetSID(), UDS_Frame::NRC_ConditionsNotCorrect);


          

        //check unmber of tries
        // if !ok, transmit negative response nrc = exceed number of tries
        //if ok, transmit seed
        //check key
        if(subfunction %2 == 0x01)
        {
          if(!CheckNumberOfSecurityAccessAttempts(subfunction))
          {
            MakeNegativeResponse(uds_frame->GetSID(), UDS_Frame::NRC_ExceededNumberOfAttempts);
          }            

          uint8_t* response_data{new uint8_t[subfunction_size+seed_size]};
          GenerateAndUpdateSecurityAccessSeed(UDS::Seed_size_5_byte);
          response_data[0] = subfunction;
          for (auto i{0}; i < UDS::Seed_size_5_byte; ++i)
            response_data[5-i] = *((uint8_t*)&m_seed+i);
          MakePositiveResponse(uds_frame->GetSID(), response_data, subfunction_size+seed_size);
          delete[] response_data;
        }
        else
        {
          // if(!sa key delay required time timer)
          // {
          //   MakeNegativeResponse(uds_frame->GetSID(), UDS_Frame::NRC_RequiredTimeDelayNotExpired);
          //   break;
          // }

          uint64_t key{1/*here init sequence*/};
          if(CompareSecurityAccessKey(/*m_*/key))
          {
            
            uint8_t* response_data{new uint8_t[subfunction_size]};
            response_data[0] = subfunction;
            MakePositiveResponse(uds_frame->GetSID(), response_data, subfunction_size);
            delete[] response_data;
          }
          else
          {
            MakeNegativeResponse(uds_frame->GetSID(), UDS_Frame::NRC_InvalidKey);
            //start sa key delay required time timer
          }
          
        }
        
        
      }
      break;

      case UDS_Frame::Service::CommunicationControl:
      {
      }
      break;

      case UDS_Frame::Service::ReadDataByIdentifier:
      {
        const uint8_t* ptr{uds_frame->GetData()};
        const DID did {static_cast<DID>((*ptr << 8) | (*(++ptr)))};
        if(m_did_repository.FindDataIdentifier(did))
        {
          auto did_size{m_did_repository.GetDataIdentifierSize(did)};
          uint32_t total_data_size{uint32_t(did_size + sizeof(DID))};
          uint8_t* response_data{new uint8_t[total_data_size]};
          //fill did
          response_data[0] = (did & 0xFF00) >> 8;
          response_data[1] = did & 0xFF;
          //fill did data
          m_did_repository.ReadDataIdentifier(did, &response_data[2], did_size);
          MakePositiveResponse(uds_frame->GetSID(),response_data, total_data_size);
          delete[] response_data;
        }
        else
          MakeNegativeResponse(uds_frame->GetSID(), UDS_Frame::NRC_GeneralReject);
      }
      break;

      case UDS_Frame::Service::WriteDataByIdentifier:
      {
        const uint8_t* ptr{uds_frame->GetData()};
        auto data_length{uds_frame->Getdatalength()};
        UDS_Frame::Service sid{uds_frame->GetSID()};
        const DID did{static_cast<DID>((*ptr << 8) | (*(++ptr)))};
        ++ptr;
        data_length -= 2;
        std::cout << std::hex << sid << " " << did << '\n';
        if(m_did_repository.FindDataIdentifier(did))//look for did
        {
          auto did_size{m_did_repository.GetDataIdentifierSize(did)};
          if(did_size < data_length)
          {
            MakeNegativeResponse(sid, UDS_Frame::NRC_GeneralReject);//if size not compares make negative response
          }
          else
          {
            m_did_repository.WriteDataIdentifier(did, ptr, data_length);
            //set size to write(data length)
            //writedataidentifier
            //response positively
            auto total_data_size{sizeof(DID)};
            uint8_t* response_data{new uint8_t[total_data_size]};
            response_data[0] = (did & 0xFF00) >> 8;
            response_data[1] = did & 0xFF;
            MakePositiveResponse(sid,response_data, total_data_size);
            delete[] response_data;
          }
        }
        else
          MakeNegativeResponse(sid, UDS_Frame::NRC_GeneralReject);//if !found transmit negative response
      }
      break;
      
      case UDS_Frame::Service::TesterPresent:
      {
        const uint8_t sprmi{*(uds_frame->GetData())}; // SPRMI = SuppressPosResMsgIndication
        if(sprmi == 0x80)
          //start s3 timer
          asm("nop");//no response transmission needed
        else if(sprmi == 0x00)
        {
          //start s3 timer
          constexpr auto respone_data_size{1};
          MakePositiveResponse(uds_frame->GetSID(), &sprmi, respone_data_size); // positive response allowed
        }
        asm("nop");
      }
      break;

      default:
         asm("nop");
      break;
    }
    //get last uds instance
    //if multiple frame - calculate frames, get data & execute it
    //add frame/frames to m_uds_tx_buffer

     delete uds_frame;
  }
}
void UDSOnCAN::MakePositiveResponse(const UDS_Frame::Service a_sid, const uint8_t* a_data_ptr, const uint32_t a_data_size)
{
  // if(a_sid == flowcontrol)
  // {
  //   UDS_Frame* flowcontrolframe{new UDS_Frame};
  //   flowcontrolframe->SetSID(static_cast<UDS_Frame::Service>(a_sid + 0x40));
  //   flowcontrolframe->SetDataLength();
  //   flowcontrolframe->SetProtocolInformation(UDS_Frame::PCI_FlowControlFrame;
  //   m_uds_tx_buffer.push_back(flowcontrolframe);
  // }
  // else
  if(a_data_size <= 6)
  {
    UDS_Frame* single_frame{new UDS_Frame};
    single_frame->SetSID(static_cast<UDS_Frame::Service>(a_sid + 0x40));
    single_frame->SetDataLength(a_data_size);
    single_frame->SetProtocolInformation(UDS_Frame::PCI_SingleFrame);
    single_frame->SetData(a_data_ptr,a_data_size,0);
    m_uds_tx_buffer.push_back(single_frame);
  }
  else
  {
    UDS_Frame* first_frame{new UDS_Frame};
    uint32_t ff_data_size{5};
    first_frame->SetSID(static_cast<UDS_Frame::Service>(a_sid + 0x40));
    first_frame->SetDataLength(a_data_size);
    first_frame->SetProtocolInformation(UDS_Frame::PCI_FirstFrame);
    first_frame->SetData(a_data_ptr,ff_data_size,0);
    m_uds_tx_buffer.push_back(first_frame);

    a_data_ptr += ff_data_size;
    UDS_Frame* consecutive_frames{new UDS_Frame};
    uint32_t cf_data_size{a_data_size - ff_data_size};
    consecutive_frames->SetDataLength(cf_data_size);
    consecutive_frames->SetProtocolInformation(UDS_Frame::PCI_ConsecutiveFrame);
    consecutive_frames->SetData(a_data_ptr,cf_data_size,0);
    m_uds_tx_buffer.push_back(consecutive_frames);

    m_status = Status_is_executing_rx_ff;
  }
}
void UDSOnCAN::MakeNegativeResponse(UDS_Frame::Service a_rejected_sid, UDS_Frame::NegativeResponseCode a_nrc)
{
  constexpr auto payload_size{2};
  UDS_Frame* negativeresponse_frame{new UDS_Frame};
  negativeresponse_frame->SetProtocolInformation(UDS_Frame::PCI_SingleFrame);
  negativeresponse_frame->SetSID(UDS_Frame::Service::NegativeResponse);
  negativeresponse_frame->SetDataLength(payload_size);
  uint8_t* data{new uint8_t[payload_size]};
  data[0] = (uint8_t)a_rejected_sid;
  data[1] = (uint8_t)a_nrc;
  negativeresponse_frame->SetData(data, payload_size, 0);
  m_uds_tx_buffer.push_back(negativeresponse_frame); //or push front
  delete[] data;
}
std::vector<CAN_Frame*> UDSOnCAN::ConvertUDSFrameToCAN()
{
  std::vector<CAN_Frame*>frames{};
  UDS_Frame* uds_frame{m_uds_tx_buffer.front()};
  m_uds_tx_buffer.pop_front();
  CAN_Frame* tx_frame{nullptr};
  constexpr uint8_t padding{0x55};
  static uint16_t  ff_cf_remaining_data_bytes{}; // ff = first frame, cf = consecutive frame

  switch(uds_frame->GetProtocolInformation())
  {
    case UDS_Frame::PCI_SingleFrame:
    {
      const uint8_t pci_shifted{static_cast<uint8_t>(uds_frame->GetProtocolInformation() << 4)};
      const uint8_t size{static_cast<uint8_t>(uds_frame->Getdatalength()+1)};
      const uint8_t sid{static_cast<uint8_t>(uds_frame->GetSID())};
      const uint8_t* data{uds_frame->GetData()};
      tx_frame = {new CAN_Frame};
      tx_frame->Fill(padding);
      tx_frame->SetData(CAN_Frame::DataPos_0, pci_shifted | size);
      tx_frame->SetData(CAN_Frame::DataPos_1, sid);
      CAN_Frame::DataPos data_pos{CAN_Frame::DataPos_2};
      for(auto remaining_bytes{uds_frame->Getdatalength()}; remaining_bytes > 0; --remaining_bytes)
      {
        tx_frame->SetData(data_pos, *data);
        data_pos = static_cast<CAN_Frame::DataPos>(data_pos + 1);
        data++;
      }
      frames.push_back(tx_frame);
      tx_frame = nullptr;
    }
    break;
    case UDS_Frame::PCI_FirstFrame:
    {
      constexpr auto data_bytes_total_in_ff{5};
      const uint8_t pci_shifted{static_cast<uint8_t>(uds_frame->GetProtocolInformation() << 4)};
      ff_cf_remaining_data_bytes = uds_frame->Getdatalength()+1;
      //assign data length to the frame
      volatile uint8_t size_l{static_cast<uint8_t>(ff_cf_remaining_data_bytes & 0xFF)};
      volatile uint8_t size_h{static_cast<uint8_t>((ff_cf_remaining_data_bytes & 0x0F00) >> 8)};
      const uint8_t sid{static_cast<uint8_t>(uds_frame->GetSID())};
      const uint8_t* data{uds_frame->GetData()};
      tx_frame = {new CAN_Frame};
      tx_frame->SetData(CAN_Frame::DataPos_0, pci_shifted | size_h);
      tx_frame->SetData(CAN_Frame::DataPos_1, size_l);
      tx_frame->SetData(CAN_Frame::DataPos_2, sid);
      for(auto i{0}; i < data_bytes_total_in_ff; ++i) //fill frame with data
      {
        CAN_Frame::DataPos data_pos = static_cast<CAN_Frame::DataPos>(CAN_Frame::DataPos_3 + i);
        tx_frame->SetData(data_pos, *data++);
      }
      //check for ff_cf_remaining_data_bytes > data_bytes_total_in_ff
      ff_cf_remaining_data_bytes -= data_bytes_total_in_ff;
    }
      frames.push_back(tx_frame);
      tx_frame = nullptr;
    break;
    case UDS_Frame::PCI_ConsecutiveFrame:
    {
      constexpr auto cf_index_first{0x01}, cf_index_last{0x0F}, data_bytes_total_in_cf{7};
      uint8_t index{cf_index_first};
      const uint8_t pci_shifted{static_cast<uint8_t>(uds_frame->GetProtocolInformation() << 4)};
      const uint8_t* data{uds_frame->GetData()};

      while(ff_cf_remaining_data_bytes >= data_bytes_total_in_cf)
      {
        tx_frame = {new CAN_Frame};
        tx_frame->Fill(0x55);
        tx_frame->SetData(CAN_Frame::DataPos_0, pci_shifted | index++); //set frame first byte
        for(auto i{0}; i < data_bytes_total_in_cf; ++i)                 //set frame data
        {
          CAN_Frame::DataPos data_pos = static_cast<CAN_Frame::DataPos>(CAN_Frame::DataPos_1 + i);
          tx_frame->SetData(data_pos, *data++);
        }
        frames.push_back(tx_frame);
        tx_frame = nullptr;
        ff_cf_remaining_data_bytes -= data_bytes_total_in_cf;
        index > cf_index_last ? index = cf_index_first : true;
      }
      if(ff_cf_remaining_data_bytes)
      {
        tx_frame = {new CAN_Frame};
        tx_frame->Fill(padding);
        tx_frame->SetData(CAN_Frame::DataPos_0, pci_shifted | index);
        CAN_Frame::DataPos data_pos = static_cast<CAN_Frame::DataPos>(CAN_Frame::DataPos_1);
        while(ff_cf_remaining_data_bytes--)
        {
          tx_frame->SetData(data_pos, *data++);
          data_pos = static_cast<CAN_Frame::DataPos>(data_pos + 1);
        }
        frames.push_back(tx_frame);
        tx_frame = nullptr;
      }
    }
    break;
    case UDS_Frame::PCI_FlowControlFrame:
    {
      const uint8_t pci_shifted{static_cast<uint8_t>(uds_frame->GetProtocolInformation() << 4)};
      const UDS_Frame::FCF_Flag fcf_flag{uds_frame->GetFlagOfFCF()};
      tx_frame = {new CAN_Frame};
      tx_frame->Fill(padding);
      tx_frame->SetData(CAN_Frame::DataPos_0, pci_shifted | fcf_flag);
      tx_frame->SetData(CAN_Frame::DataPos_1, m_BS_this_device);
      tx_frame->SetData(CAN_Frame::DataPos_2, m_STmin_this_device);
      frames.push_back(tx_frame);
      tx_frame = nullptr;
    }
    break;

    default:
    break;
  }
  delete uds_frame;
  return frames;
}

bool UDSOnCAN::IsRXBufferOfUDSEmpty()
{
  return !(bool)m_uds_rx_buffer.size();
}
bool UDSOnCAN::IsTXBufferOfUDSEmpty()
{
  return !(bool)m_uds_tx_buffer.size();
}
void UDSOnCAN::GenerateAndUpdateSecurityAccessSeed(UDS::SeedSize a_seed_size)
{
  m_seed_size = a_seed_size;
  auto mt_seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();

  std::mt19937_64 mt(mt_seed);
  uint64_t bitmask{1};
  uint64_t seed{};
  do
  {
    bitmask *= 256;
    a_seed_size = static_cast<UDS::SeedSize>(a_seed_size-1);
  } while(a_seed_size);
  bitmask -=1;

  seed  = mt();
  m_seed = seed & bitmask;
  std::cout << std::hex << m_seed << '\n';
  CalculateSecurityAccessKey();
}
void UDSOnCAN::CalculateSecurityAccessKey()
{
  //USE XTEA Algorhytm
  uint64_t calculated_key;
  //calculate new key here
  m_key = calculated_key;
}
bool UDSOnCAN::CompareSecurityAccessKey(uint64_t a_key)
{
  return a_key == m_key;
}
uint64_t UDSOnCAN::GetSecurityAccessKey()
{
  return  m_key;
}
void UDSOnCAN::SendFlowControlFrame()
{
}
void UDSOnCAN::SetSeparationTime(uint8_t a_STmin)
{

  // STmin values:
  //       0x00-0x7F - Separation Time minimum range 0-127 ms
  //       0x80-0xF0 - Reserved
  //       0xF1-0xF9 - Separation Time minimum range 100-900 μs
  //       0xFA-0xFF - Reserved

  m_STmin_this_device = a_STmin;
}
uint8_t UDSOnCAN::GetSeparationTime() const
{
  return m_STmin_this_device;
}   
uint16_t UDSOnCAN::GetSeparationTime_us() const
{
  // STmin values:
  //       0x00-0x7F - Separation Time minimum range 0-127 ms
  //       0x80-0xF0 - Reserved
  //       0xF1-0xF9 - Separation Time minimum range 100-900 μs
  //       0xFA-0xFF - Reserved
  return 0;
}
