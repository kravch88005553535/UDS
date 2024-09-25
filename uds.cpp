#include <iostream>
#include <random>
#include <chrono>
#include <cstdint>
#include "uds.h"
#include "iso15765-2.h"
#include "uds_config.h"

UDS::UDS()
  : m_is_busy{false}
  , m_sessiontype{DSC_Type_DefaultSession}
  , m_sa_security_level_unlocked{SecurityAccessLevel_NONE}
  , m_sa_requestsequenceerror{true}
  , m_programmingsession_number_of_attempts{5}
  , m_extendeddiagnosticsession_number_of_attempts{5}
  , m_safetysystemdiagnosticsession_number_of_attempts{5}
  , m_seed_size{Seedsize_4_byte}
  , m_seed{}
  , m_key{}
  , m_p2_timer{Program_timer::Type_one_pulse}
  , m_s3_timer{Program_timer::Type_one_pulse}
  , m_separation_time_min_this_device_timer{Program_timer::Type_one_pulse}
  , m_separation_time_min_tester_timer{Program_timer::Type_one_pulse}
  , m_separation_time_this_device{0x64} //100ms
  , m_separation_time_min_tester{}
  , m_block_size_this_device{0} //all frames without fcf
  , m_block_size_tester{}
{
  m_s3_timer.SetInterval_sec(5);
  m_p2_timer.SetInterval_ms(50);
  m_separation_time_min_this_device_timer.SetInterval_ms(m_separation_time_this_device);
}

bool UDS::SetSessionType(const SessionType a_sessiontype)
{
  /*If the client has requested a diagnostic session, which is already running, then the server shall send a positive
  response message and behave as shown in Figure 7 that describes the server internal behaviour when
  transitioning between sessions.*/
  if(m_sessiontype == a_sessiontype)
  {
    /*
    default session: When the server is in the defaultSession and the client requests to start the defaultSession then the
    server shall re-initialize the defaultSession completely. The server shall reset all activated/initiated/changed
    settings/controls during the activated session. This does not include long term changes programmed into non-volatile
    memory.
    */
    if(a_sessiontype == DSC_Type_DefaultSession)
    {
      m_s3_timer.Stop();
      m_sa_requestsequenceerror = true;
      m_sa_security_level_unlocked = SecurityAccessLevel_NONE;
    }
    else
    /*same or other session: When the server transitions from any diagnostic session other than the defaultSession to
    another session other than the defaultSession (including the currently active diagnostic session) then the server shall
    (re-) initialize the diagnostic session, which means that:
    i) Each event that has been configured in the server via the ResponseOnEvent (0x86) service shall be stopped.
    ii) Security shall be relocked. Note that the locking of security access shall reset any active diagnostic functionality that
    was dependent on security access to be unlocked (e.g., active inputOutputControl of a DID).
    iii) All other active diagnostic functionality that is supported in the new session and is not dependent upon security
    access shall be maintained. For example, any configured periodic scheduler shall remain active when transitioning
    from one non-defaultSession to another or the same non-DefaultSession and the states of the CommunicationControl
    and ControlDTCSetting services shall not be affected, which means that normal communication shall remain disabled
    when it is disabled at the point in time the session is switched.*/
    {
      m_s3_timer.Reload();
      m_s3_timer.Start(); //optional, but not need for this string here
      m_sa_requestsequenceerror = true;
      m_sa_security_level_unlocked = SecurityAccessLevel_NONE;
    }
    return true;
  }

  if(m_sessiontype != DSC_Type_DefaultSession)
  {
    if(a_sessiontype != DSC_Type_DefaultSession)
      return false;
    else
    {
      /*default session: When the server transitions from any diagnostic session other than the default session to the
      defaultSession then the server shall stop each event that has been configured in the server via the
      ResponseOnEvent (0x86) service and security shall be enabled. Any other active diagnostic functionality that is not
      supported in the defaultSession shall be terminated. For example, any configured periodic scheduler or output control
      shall be disabled and the states of the CommunicationControl and ControlDTCSetting services shall be reset, which
      means that normal communication shall be re-enabled when it was disabled at the point in time the session is
      switched to the defaultSession. The server shall reset all activated/initiated/changed settings/controls during the
      activated session. This does not include long term changes programmed into non-volatile memory
      */
      m_sessiontype = a_sessiontype;
      m_s3_timer.Stop();
      m_sa_requestsequenceerror = true;
      m_sa_security_level_unlocked = SecurityAccessLevel_NONE;
      return true;
    }
  }
  else
  {
    m_sessiontype = a_sessiontype;
    m_s3_timer.Reload();
    m_s3_timer.Start();
    m_sa_requestsequenceerror = true;
    m_sa_security_level_unlocked = SecurityAccessLevel_NONE;
    return true;
  }
  return false;
}
UDS::SessionType UDS::GetSessiontype()
{
  return m_sessiontype;
}
bool UDS::CheckNumberOfSecurityAccessAttempts(const uint8_t a_subfunction)
{
  auto requestseed{a_subfunction};
  switch(requestseed)
  {
    case 0x02:
    {
      if(m_programmingsession_number_of_attempts) 
        return m_programmingsession_number_of_attempts--;
    }
    break;
    case 0x04:
    {
      if(m_extendeddiagnosticsession_number_of_attempts)
        return m_extendeddiagnosticsession_number_of_attempts--;
    }
    break;
    case 0x06:
    {
      if(m_safetysystemdiagnosticsession_number_of_attempts)
        return m_safetysystemdiagnosticsession_number_of_attempts--;
    }
    break;
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
  m_sa_requestsequenceerror = true;
}
void UDS::SetCommunicationControl(CommunicationControl a_communication_control)
{
  switch (a_communication_control)
  {
    case CC_EnableRxEnableTx:
      m_is_rx_enabled = true;
      m_is_tx_enabled = true;
    break;
    
    case CC_EnableRxDisableTx:
      m_is_rx_enabled = true;
      m_is_tx_enabled = false;
    break;
    
    case CC_DisableRxEnableTx:
      m_is_rx_enabled = false;
      m_is_tx_enabled = true;
    break;
    
    case CC_DisableRxDisableTx:
      m_is_rx_enabled = false;
      m_is_tx_enabled = false;
    break;
  }
}
bool UDS::IsECUTXEnabled()
{
  return m_is_tx_enabled;
}
bool UDS::IsECURXEnabled()
{
  return m_is_rx_enabled;
}
void UDS::CheckS3Timer()
{
  if(m_sessiontype != DSC_Type_DefaultSession and m_s3_timer.Check())
    SetSessionType(DSC_Type_DefaultSession);
}
void UDS::SetSeparationTimeForTester(uint8_t a_stmin)
{
  // STmin values:
  //       0x00-0x7F - Separation Time minimum range 0-127 ms
  //       0x80-0xF0 - Reserved
  //       0xF1-0xF9 - Separation Time minimum range 100-900 μs
  //       0xFA-0xFF - Reserved
  if(a_stmin >= 0xF1 and a_stmin <= 0xF9)
    m_separation_time_min_tester_timer.SetInterval_us((a_stmin - 0xF0) * 100);

  if(a_stmin <= 0x7F)
    m_separation_time_min_tester_timer.SetInterval_ms(a_stmin);
  
  m_separation_time_min_tester = a_stmin;
}
uint8_t UDS::GetSeparationTimeTester() const
{
  return m_separation_time_min_tester;
}

void UDS::GenerateAndUpdateSecurityAccessSeed(UDS::SeedSize a_seed_size)
{
  auto mt_seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
  std::mt19937_64 mt(mt_seed);
  uint64_t bitmask{1};
  uint64_t seed{};
  m_seed = 0;
  do
  {
    bitmask *= 256;
    a_seed_size = static_cast<UDS::SeedSize>(a_seed_size-1);
  } while(a_seed_size);
  bitmask -=1;

  do
  {
    seed = mt();        //The server shall never send an all zero seed
    seed &= bitmask;    //for a given security level that is currently locked.
  } while (seed == 0);
  
  m_seed = seed;
  // std::cout << std::hex << "seed: "<< m_seed << '\n';
  
  CalculateSecurityAccessFullKey();
  m_key = m_key & bitmask;
  // std::cout << std::hex << "key: "<< m_key << '\n';
}
void UDS::CalculateSecurityAccessFullKey()
{
  //using XTEA Algorhytm
  uint64_t temp_seed{m_seed};
  constexpr uint32_t number_of_rounds{35};
  uint32_t* v{(uint32_t*) &temp_seed};
  uint32_t* k{new uint32_t[4]};
  k[0] = 0xEA27E48F;
  k[1] = 0xDD1356EB;
  k[2] = 0xA9E9B738;
  k[3] = 0xFDC15287;

  uint32_t v0{v[0]};
  uint32_t v1{v[1]};
  uint32_t sum {0};
  uint32_t delta {0x9E3779B9};
  for (auto i {0}; i < number_of_rounds; ++i)
  {
    v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + k[sum & 3]);
    sum += delta;
    v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + k[(sum>>11) & 3]);
  }
  v[0]=v0; v[1]=v1;

  //std::cout << "calculated sa full key: " << std::hex << temp_seed << '\n';
  
  m_key = temp_seed;
  delete[] k;
}
bool UDS::CompareSecurityAccessKey(uint64_t a_key)
{
  return a_key == m_key;
}
uint64_t UDS::GetSecurityAccessKey()
{
  return  m_key;
}
DID_Repository& UDS::GetDIDRepository()
{
  return m_did_repository;
}


UDSOnCAN::UDSOnCAN(const uint32_t a_ecu_functional_can_id)
  : UDS{}
  , m_status{Status_ok}
  , m_ecu_functional_can_id{a_ecu_functional_can_id}
{
  m_did_repository.AddDataIdentifier(DID_VehicleManufacturerECUSoftwareConfigurationNumber,        18,               DID_Instance::DID_Datatype_c_string,         DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_VehicleManufacturerECUSoftwareConfigurationVersionNumber, 32,               DID_Instance::DID_Datatype_c_string,         DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_FirmwareUpdateMode,                                       sizeof(bool),     DID_Instance::DID_Datatype_bool,             DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_MapsUpdateMode,                                           sizeof(bool),     DID_Instance::DID_Datatype_bool,             DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_RS232_1_BaudrateSetup,                                    sizeof(uint32_t), DID_Instance::DID_Datatype_unsigned_integer, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_RS232_2_BaudrateSetup,                                    sizeof(uint32_t), DID_Instance::DID_Datatype_unsigned_integer, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_RS485_BaudrateSetup,                                      sizeof(uint32_t), DID_Instance::DID_Datatype_unsigned_integer, DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_WiFiPassword,                                             64,               DID_Instance::DID_Datatype_c_string,         DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_DiagData,                                                 2,                DID_Instance::DID_Datatype_raw_data,         DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_VehicleManufacturerECUSoftwareNumber,                     20,               DID_Instance::DID_Datatype_c_string,         DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_VehicleManufacturerECUSoftwareVersionNumber,              32,               DID_Instance::DID_Datatype_c_string,         DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_SystemSupplierIdentifierDataIdentifier,                   32,               DID_Instance::DID_Datatype_c_string,         DID_Instance::Readonly);
  m_did_repository.AddDataIdentifier(DID_ECUManufacturingDate,                                     16,               DID_Instance::DID_Datatype_c_string,         DID_Instance::Readonly);
  m_did_repository.AddDataIdentifier(DID_ECUSerialNumber,                                          253,              DID_Instance::DID_Datatype_c_string,         DID_Instance::Readonly);
  m_did_repository.AddDataIdentifier(DID_VIN,                                                      17,               DID_Instance::DID_Datatype_c_string,         DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_VehicleManufacturerECUHardwareNumber,                     32,               DID_Instance::DID_Datatype_c_string,         DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_SystemSupplierECUHardwareNumber,                          32,               DID_Instance::DID_Datatype_c_string,         DID_Instance::Readonly);
  m_did_repository.AddDataIdentifier(DID_SystemSupplierECUHardwareVersionNumber,                   16,               DID_Instance::DID_Datatype_c_string,         DID_Instance::Readonly);
  m_did_repository.AddDataIdentifier(DID_SystemSupplierECUSoftwareNumber,                          128,              DID_Instance::DID_Datatype_c_string,         DID_Instance::Readonly);
  m_did_repository.AddDataIdentifier(DID_SystemSupplierECUSoftwareVersionNumber,                   128,              DID_Instance::DID_Datatype_c_string,         DID_Instance::Readonly);
  m_did_repository.AddDataIdentifier(DID_ExhaustRegulationOrTypeApprovalNumber,                    64,               DID_Instance::DID_Datatype_c_string,         DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_SystemNameOrEngineType,                                   64,               DID_Instance::DID_Datatype_c_string,         DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_RepairShopCodeOrTesterSerialNumber,                       32,               DID_Instance::DID_Datatype_c_string,         DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_ProgrammingDate,                                          16,               DID_Instance::DID_Datatype_c_string,         DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_ECUInstallationDate,                                      16,               DID_Instance::DID_Datatype_c_string,         DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_SystemSupplierBaseConfiguration,                          18,               DID_Instance::DID_Datatype_c_string,         DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_BaseSoftwareVersion,                                      128,              DID_Instance::DID_Datatype_c_string,         DID_Instance::Readonly);
  m_did_repository.AddDataIdentifier(DID_FirmwareUpdateStatus,                                     100,              DID_Instance::DID_Datatype_c_string,         DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_MapsUpdateStatus,                                         100,              DID_Instance::DID_Datatype_c_string,         DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_RestartFromWatchdogError,                                 1,                DID_Instance::DID_Datatype_bool,             DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_HighVoltageError,                                         1,                DID_Instance::DID_Datatype_bool,             DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_LowVoltageError,                                          1,                DID_Instance::DID_Datatype_bool,             DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_GNSSModuleError,                                          1,                DID_Instance::DID_Datatype_bool,             DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_GyroAccelAccessError,                                     1,                DID_Instance::DID_Datatype_bool,             DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_BTPFirmwareUpdateError,                                   1,                DID_Instance::DID_Datatype_bool,             DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_KamazReliefMapLoadError,                                  1,                DID_Instance::DID_Datatype_bool,             DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_MCUFirmwareIntegrityError,                                1,                DID_Instance::DID_Datatype_bool,             DID_Instance::ReadWrite);
  m_did_repository.AddDataIdentifier(DID_BipError,                                                 1,                DID_Instance::DID_Datatype_bool,             DID_Instance::ReadWrite);

  uint32_t default_async_interfaces_speed_kbaud{1};
  m_did_repository.LE_WriteDataIdentifier(DID_RS232_1_BaudrateSetup, (uint8_t*)&default_async_interfaces_speed_kbaud, sizeof(default_async_interfaces_speed_kbaud));
  m_did_repository.LE_WriteDataIdentifier(DID_RS232_2_BaudrateSetup, (uint8_t*)&default_async_interfaces_speed_kbaud, sizeof(default_async_interfaces_speed_kbaud));
  m_did_repository.LE_WriteDataIdentifier(DID_RS485_BaudrateSetup, (uint8_t*)&default_async_interfaces_speed_kbaud, sizeof(default_async_interfaces_speed_kbaud));
  m_did_repository.WriteDataIdentifier(DID_FirmwareUpdateStatus, " WX");
  m_did_repository.WriteDataIdentifier(DID_VIN, "ABCDE GFBD");
  m_did_repository.LE_ReadDataIdentifier(DID_RS232_1_BaudrateSetup, (uint8_t*)&default_async_interfaces_speed_kbaud, sizeof(default_async_interfaces_speed_kbaud));
  
  std::string temp{m_did_repository.ReadDataIdentifier(DID_VIN)};

  std::vector<DID_Instance*> vector{m_did_repository.GetListOfModifiedDIDs()};
  for(auto it{vector.begin()}; it != vector.end(); ++it)
  {
    if((*it)->IsModified())
      (*it)->SetModifyFlag(false);
  }
}

UDSOnCAN::~UDSOnCAN(){}

void UDSOnCAN::Execute()
{
  CheckS3Timer();
  if(!m_uds_rx_buffer.empty())
  {
    UDS_Frame* uds_frame{m_uds_rx_buffer.front()};
    m_uds_rx_buffer.pop_front();

    if(m_s3_timer.IsStarted())
      m_s3_timer.Reload();

    UDS::Service sid{uds_frame->GetSID()};
    CAN_Frame::Source source{uds_frame->GetSource()};
    #ifdef UDS_DEBUG_FRAME_VALIDITY_ENABLED
    if(!uds_frame->IsFrameValid())
    {
      MakeNegativeResponse(sid, UDS::NRC_ConditionsNotCorrect, source);
      delete uds_frame;
      return;
    }
    #endif //UDS_DEBUG

    switch(sid)
    {
      case UDS::Service_DiagnosticSessionControl:
      {
        // #ifdef UDS_DEBUG_SESSIONTYPE_VALIDITY_ENABLED
        // if(GetSessiontype() < UDS::DSC_Type_ExtendedDiagnosticSession)
        // {
        //   MakeNegativeResponse(sid, UDS::NRC_ServiceNotSupportedInActiveSession, source);
        //   break;
        // }
        // #endif //UDS_DEBUG_SESSIONTYPE_VALIDITY_ENABLED
        
        if(uds_frame->GetDataLength() > 1)
        {
          MakeNegativeResponse(sid, UDS::NRC_IncorrectMessageLengthOrInvalidFormat, source);
          break;
        }
        
        uint8_t sessiontype = *uds_frame->GetData() & 0x7F;
        if(sessiontype > DSC_Type_ExtendedDiagnosticSession)
        {
          MakeNegativeResponse(sid, UDS::NRC_Subfunctionnotsupported, source);
          break;
        }

        if(SetSessionType(static_cast<UDS::SessionType>(sessiontype)))
        {
          const bool pr_supression = *uds_frame->GetData() & 0x80;
          if(pr_supression == 0)
          {
            constexpr auto subfunction_size{1};
            constexpr auto p2_timings_size{4};
            const auto response_array_size{subfunction_size+p2_timings_size};
            uint8_t* response_data{new uint8_t[response_array_size]};  
            response_data[0] = sessiontype;
            response_data[1] = 0x00;
            response_data[2] = 0x32; //50ms
            response_data[3] = 0x01;
            response_data[4] = 0xF4; //5000ms
            MakePositiveResponse(sid, &response_data[0], response_array_size, source);
            delete [] response_data;
          }
        }
        else
        {
          MakeNegativeResponse(sid, UDS::NRC_ConditionsNotCorrect, source);
          break;
        }
      }
      break;

      case UDS::Service_SecurityAccess:
      {
        #ifdef UDS_DEBUG_FUNCTIONAL_ADDRESSING_CHECK_ENABLED
        if(uds_frame->IsAddressungFunctional())
        {
          //MakeNegativeResponse(sid, UDS::NRC_RequestCorrectlyReceived_ResponsePending, source);
          break;
        }
        #endif //UDS_DEBUG_FUNCTIONAL_ADDRESSING_CHECK_ENABLED

        #ifdef UDS_DEBUG_SESSIONTYPE_VALIDITY_ENABLED
        if(GetSessiontype() < UDS::DSC_Type_ProgrammingSession)
        {
          MakeNegativeResponse(sid, UDS::NRC_ServiceNotSupportedInActiveSession, source);
          break;
        }
        #endif //UDS_DEBUG_SESSIONTYPE_VALIDITY_ENABLED

        constexpr auto subfunction_size{1};
        const uint8_t  secutityaccess_type = *uds_frame->GetData() & 0x7F;
        const bool     pr_supression = *uds_frame->GetData() & 0x80;

        if(secutityaccess_type > SecurityAccessLevel_3_Response or secutityaccess_type == SecurityAccessLevel_NONE)
        {
          MakeNegativeResponse(sid, UDS::NRC_Subfunctionnotsupported, source);
          break;
        }
        //check current diagnostic session
        //if session is not correct
        //MakeNegativeResponse(sid, UDS_Frame::NRC_ConditionsNotCorrect);

        if(secutityaccess_type % 2 == 0x01)
        {
          if(uds_frame->GetDataLength() > subfunction_size)
          {
            MakeNegativeResponse(sid, UDS::NRC_IncorrectMessageLengthOrInvalidFormat, source);
            break;
          }

          uint8_t* response_data{new uint8_t[subfunction_size+m_seed_size]};
          response_data[0] = secutityaccess_type;
          if(m_sa_security_level_unlocked == secutityaccess_type)
          {
            for (auto i{0}; i < m_seed_size; ++i)
              response_data[m_seed_size-i] = 0;  
          }
          else
          {
            GenerateAndUpdateSecurityAccessSeed(m_seed_size);
            for (auto i{0}; i < m_seed_size; ++i)
              response_data[m_seed_size-i] = *((uint8_t*)&m_seed+i);
          }
          if(pr_supression == 0)
            MakePositiveResponse(sid, response_data, subfunction_size+m_seed_size, source);
          
          delete[] response_data;
          m_sa_requestsequenceerror = false;
        }
        else if (secutityaccess_type % 2 == 0x00)
        {
          const auto message_length{subfunction_size + m_seed_size};
          if(uds_frame->GetDataLength() > message_length)
          {
            MakeNegativeResponse(sid, UDS::NRC_IncorrectMessageLengthOrInvalidFormat, source);
            break;
          }
          if(m_sa_requestsequenceerror)
          {
            MakeNegativeResponse(sid, UDS::NRC_RequestSequenceError, source);
            break;
          }
          if(!CheckNumberOfSecurityAccessAttempts(secutityaccess_type))
          {
            MakeNegativeResponse(sid, UDS::NRC_ExceededNumberOfAttempts, source);
            break;
          }
          // if(!sa key delay required time timer)
          // {
          //   MakeNegativeResponse(sid, UDS_Frame::NRC_RequiredTimeDelayNotExpired);
          //   break;
          // }
          uint64_t recieved_key{};
          uint8_t* rkey_ptr{reinterpret_cast<uint8_t*>(&recieved_key)};
          const uint8_t* key_ptr{(uds_frame->GetData() + m_seed_size)};
          for (auto i{0}; i < m_seed_size; ++i)
            *rkey_ptr++ = *key_ptr--;

          if(CompareSecurityAccessKey(recieved_key))
          {
            m_sa_security_level_unlocked = static_cast<SecurityAccessLevel>(secutityaccess_type - 1);
            uint8_t* response_data{new uint8_t[subfunction_size]};
            response_data[0] = secutityaccess_type;
            if(pr_supression == 0)
              MakePositiveResponse(sid, response_data, subfunction_size, source);
            delete[] response_data;
            /*
             *  In case the server supports this delay timer then after a successful
             *  SecurityAccess service 'sendKey' execution the server internal indication information for a delay timer
             *  invocation on a power up/reset shall be cleared by the server.
             */
          }
          else
            MakeNegativeResponse(sid, UDS::NRC_InvalidKey, source); //start sa key delay required time timer

          m_sa_requestsequenceerror = true;
        }
        else
        {
          MakeNegativeResponse(sid, UDS::NRC_IncorrectMessageLengthOrInvalidFormat, source);
          break;
        }
      }
      break;

      case UDS::Service_CommunicationControl:
      {
        #ifdef UDS_DEBUG_SESSIONTYPE_VALIDITY_ENABLED
        if(GetSessiontype() < UDS::DSC_Type_ExtendedDiagnosticSession)
        {
          MakeNegativeResponse(sid, UDS::NRC_ServiceNotSupportedInActiveSession, source);
          break;
        }
        #endif //UDS_DEBUG_SESSIONTYPE_VALIDITY_ENABLED

        auto data_length{uds_frame->GetDataLength()};
        if(data_length == 0 or data_length > 4)
        {
          MakeNegativeResponse(sid, UDS::NRC_IncorrectMessageLengthOrInvalidFormat, source);
          break;
        }
        
        const uint8_t* ptr{(uds_frame->GetData())};
        const UDS::CommunicationControl communication_control{static_cast<UDS::CommunicationControl>(*ptr++)};
        const UDS::CommunicationType communication_type{static_cast<UDS::CommunicationType>(*ptr)};

        if(communication_control > UDS::CC_DisableRxDisableTx)
        {
          MakeNegativeResponse(sid, UDS::NRC_Subfunctionnotsupported, source);
          break;
        }
        SetCommunicationControl(communication_control);

        //add here communicationType , nodeIdentificationNumber (high byte), nodeIdentificationNumber (low byte) if necessary & edit condition of data length
        constexpr auto respone_data_size{1};
        MakePositiveResponse(sid, (const uint8_t*)&communication_control, respone_data_size, source); // positive response allowed
      }
      break;

      case UDS::Service_ReadDataByIdentifier:
      {
        #ifdef UDS_DEBUG_FUNCTIONAL_ADDRESSING_CHECK_ENABLED
        if(uds_frame->IsAddressungFunctional())
        {
          //MakeNegativeResponse(sid, UDS::NRC_RequestCorrectlyReceived_ResponsePending, source);
          break;
        }
        #endif //UDS_DEBUG_FUNCTIONAL_ADDRESSING_CHECK_ENABLED
        
        const uint32_t data_length{uds_frame->GetDataLength()};
        if(data_length % 2 != 0)
        {
          MakeNegativeResponse(sid, UDS::NRC_IncorrectMessageLengthOrInvalidFormat, source);
          break;
        }
        
        const uint8_t* ptr{uds_frame->GetData()};
        const DID did {static_cast<DID>((*ptr << 8) | (*(++ptr)))};

        if(m_did_repository.FindDataIdentifier(did))
        {
          auto did_size{m_did_repository.GetDataIdentifierSize(did)};
          uint32_t total_data_size{uint32_t(did_size + sizeof(DID))};
          uint8_t* response_data{new uint8_t[total_data_size]};
          response_data[0] = (did & 0xFF00) >> 8;
          response_data[1] = did & 0xFF;
          m_did_repository.ReadDataIdentifier(did, &response_data[2], did_size);
          MakePositiveResponse(sid,response_data, total_data_size, source);
          delete[] response_data;
        }
        else
          MakeNegativeResponse(sid, UDS::NRC_RequestOutOfRange, source);
      }
      break;

      case UDS::Service_WriteDataByIdentifier:
      {
        #ifdef UDS_DEBUG_FUNCTIONAL_ADDRESSING_CHECK_ENABLED
        if(uds_frame->IsAddressungFunctional())
        {
          //MakeNegativeResponse(sid, UDS::NRC_RequestCorrectlyReceived_ResponsePending, source);
          break;
        }
        #endif //UDS_DEBUG_FUNCTIONAL_ADDRESSING_CHECK_ENABLED

        #ifdef UDS_DEBUG_SESSIONTYPE_VALIDITY_ENABLED
        if(GetSessiontype() < UDS::DSC_Type_ExtendedDiagnosticSession)
        {
          MakeNegativeResponse(sid, UDS::NRC_ServiceNotSupportedInActiveSession, source);
          break;
        }
        if(m_sa_security_level_unlocked < SecurityAccessLevel_2)
        {
          MakeNegativeResponse(sid, UDS::NRC_SecurityAccessDenied, source);
          break;
        }
        #endif //UDS_DEBUG_SESSIONTYPE_VALIDITY_ENABLED

        const uint8_t* ptr{uds_frame->GetData()};
        auto  data_length{uds_frame->GetDataLength()};
        const DID did {static_cast<DID>((*ptr << 8) | (*(++ptr)))};
        ++ptr;
        data_length -= 2;
        if(m_did_repository.FindDataIdentifier(did))//look for did
        {
          auto did_size{m_did_repository.GetDataIdentifierSize(did)};

          if(did_size < data_length)
          {
            MakeNegativeResponse(sid, UDS::NRC_GeneralReject, source);//if size not compares make negative response
          }
          else
          {
            m_did_repository.WriteDataIdentifier(did, ptr, data_length);
            auto total_data_size{sizeof(DID)};
            uint8_t* response_data{new uint8_t[total_data_size]};
            response_data[0] = (did & 0xFF00) >> 8;
            response_data[1] = did & 0xFF;
            MakePositiveResponse(sid,response_data, total_data_size, source);
            delete[] response_data;
          }
        }
        else
          MakeNegativeResponse(sid, UDS::NRC_GeneralReject, source);//if !found transmit negative response
      }
      break;
      
      case UDS::Service_TesterPresent:
      {
        const uint8_t sprmi{*(uds_frame->GetData())}; // SPRMI = SuppressPosResMsgIndication
        if(sprmi == 0x80)
          asm("nop");//no response transmission needed
        else if(sprmi == 0x00)
        {
          constexpr auto respone_data_size{1};
          MakePositiveResponse(sid, &sprmi, respone_data_size, source); // positive response allowed
        }
      }
      break;

      default:
        MakeNegativeResponse(sid, UDS::NRC_ServiceNotSupported, source);
      break;
    }
    delete uds_frame;
  }
}
bool UDSOnCAN::ConvertCANFrameToUDS(const CAN_Frame* const ap_can_frame)
{
  static UDS_Frame static_uds_frame{};
  static uint32_t  cf_data_remaining{};
  static uint8_t   next_consecutive_frame_index{};

  volatile ISO_15765_2_PCI pci{(ISO_15765_2_PCI)((ap_can_frame->GetData(CAN_Frame::DataPos_0) & 0xF0) >> 4)};

  switch(pci)
  {
    case ISO_15765_2_PCI::PCI_SingleFrame:
    {
      UDS_Frame* rx_frame{new UDS_Frame};
      const uint8_t payload {uint8_t(ap_can_frame->GetData(CAN_Frame::DataPos_0) & 0x0F)};
      rx_frame->SetProtocolInformation(pci);
      rx_frame->SetSID(static_cast<UDS::Service>(ap_can_frame->GetData(CAN_Frame::DataPos_1)));
      rx_frame->SetData(ap_can_frame->GetDataPtr(CAN_Frame::DataPos_2), payload-1, 0);
      rx_frame->SetDataLength(payload-1);
      rx_frame->SetSource(ap_can_frame->GetSource());
      rx_frame->SetFunctionalAddressingFlag(ap_can_frame->GetID() == m_ecu_functional_can_id);
      
      m_uds_rx_buffer.push_back(rx_frame);
      return true;  
    }
    break;

    case ISO_15765_2_PCI::PCI_FirstFrame:
    {
      constexpr auto ff_data_size{3};
      constexpr auto ff_sid_size{1};
      constexpr auto ff_did_size{2};
      cf_data_remaining = (ap_can_frame->GetData(CAN_Frame::DataPos_0) & 0x0F) << 4 | ap_can_frame->GetData(CAN_Frame::DataPos_1);
      static_uds_frame.SetDataLength(cf_data_remaining-ff_sid_size);
      static_uds_frame.SetSID(static_cast<UDS::Service>(ap_can_frame->GetData(CAN_Frame::DataPos_2)));
      static_uds_frame.SetData(ap_can_frame->GetDataPtr(CAN_Frame::DataPos_3), ff_data_size+ff_did_size, 0);
      static_uds_frame.SetframeValidity(true);
      static_uds_frame.SetSource(ap_can_frame->GetSource());
      static_uds_frame.SetFunctionalAddressingFlag(ap_can_frame->GetID() == m_ecu_functional_can_id);
      cf_data_remaining -= ff_sid_size + ff_did_size + ff_data_size;
      next_consecutive_frame_index = 1;

      UDS_Frame* flowcontrolframe{new UDS_Frame};
      flowcontrolframe->SetProtocolInformation(ISO_15765_2_PCI::PCI_FlowControlFrame);
      flowcontrolframe->SetSource(ap_can_frame->GetSource());
      flowcontrolframe->SetFlagOfFCF(ISO_15765_2_PCI_FCF_Flag::FCF_Flag_CTS);
      m_uds_tx_buffer.push_back(flowcontrolframe);
      return true;
    }
    break;

    case ISO_15765_2_PCI::PCI_ConsecutiveFrame:
    {
      //if(!(--m_frames_remaining_until_next_fcf))
      //  SetStatus(UDSOnCAN::Status_is_waiting_fcf);

      const uint8_t consecutive_frame_index{(uint8_t)(ap_can_frame->GetData(CAN_Frame::DataPos_0) & 0x0F)};
      constexpr auto cf_data_size{7};
      const uint32_t offset{static_uds_frame.GetDataLength() - cf_data_remaining};
      if(cf_data_remaining == 0)
        break;

      if(consecutive_frame_index != next_consecutive_frame_index)
        static_uds_frame.SetframeValidity(false);

      if(cf_data_remaining <= cf_data_size)
      {
        static_uds_frame.SetData(ap_can_frame->GetDataPtr(CAN_Frame::DataPos_1), cf_data_remaining, offset);
        cf_data_remaining = 0;
        UDS_Frame* rx_frame{new UDS_Frame(static_uds_frame)};
        m_uds_rx_buffer.push_back(rx_frame);
        return true; 
      }
      else
      {
        static_uds_frame.SetData(ap_can_frame->GetDataPtr(CAN_Frame::DataPos_1), cf_data_size, offset);
        cf_data_remaining -= cf_data_size;
        if(++next_consecutive_frame_index > 0x0F)
          next_consecutive_frame_index = 1;
        //reload cf_wait timer
      }
    //check last frame & check frame sid
    //else select offset by frame number
    //if data finished perform actions in execute
    //bits 3..0 of byte 1 is sequence number
      return true;
    }
    break;

    case ISO_15765_2_PCI::PCI_FlowControlFrame:
    {
      // here no conversion to uds frame needed, 
      // just get parameters of UDS & transmit to can tx queue
      const ISO_15765_2_PCI_FCF_Flag flag{(ISO_15765_2_PCI_FCF_Flag)(ap_can_frame->GetData(CAN_Frame::DataPos_0) & 0x0F)};
      if(flag == ISO_15765_2_PCI_FCF_Flag::FCF_Flag_CTS)
      {
        m_frames_remaining_until_next_fcf = ap_can_frame->GetData(CAN_Frame::DataPos_1);
        const uint8_t separation_time{ap_can_frame->GetData(CAN_Frame::DataPos_1)};
        SetSeparationTimeForTester(separation_time);
      }
      else if(flag == ISO_15765_2_PCI_FCF_Flag::FCF_Flag_Wait)
      {
      }
      else if(flag == ISO_15765_2_PCI_FCF_Flag::FCF_Flag_Overflow)
      {
      }
      //if(flag == overflow/abort)

      //IN RESPONSE:
      //SET FLOWSTATUS
      //SET BLOCKSIZE
      //SET STMIN
      //SET SOURCE
      // IN REQUEST EXECURE ALL THIS PARAMETERS


      //bits 3..0 of byte 1 is flow status
      // byte 2 is block size
      // byte 3 is ST_min (Separation Time minimum)
      // bytes 4..8 are not N/A
        m_status = Status_recieved_fcf;
        return true;
    }
    break;

    default:
    break;
  }
  // if(!check valid values)
  //  return false;
  while(1);
  return false;
}
bool UDSOnCAN::IsRXBufferOfUDSEmpty()
{
  return m_uds_rx_buffer.size() == 0;
}
bool UDSOnCAN::IsTXBufferOfUDSEmpty()
{
  return m_uds_tx_buffer.size() == 0;
}
std::vector<CAN_Frame*> UDSOnCAN::ConvertUDSFrameToCAN()
{
  std::vector<CAN_Frame*>frames{};
  UDS_Frame* uds_frame{m_uds_tx_buffer.front()};
  m_uds_tx_buffer.pop_front();
    
  CAN_Frame* tx_frame{nullptr};
  constexpr uint8_t padding{0xAA};  //request padding 0x55, response padding 0xAA
  static uint16_t  ff_cf_remaining_data_bytes{}; // ff = first frame, cf = consecutive frame

  switch(uds_frame->GetProtocolInformation())
  {
    case ISO_15765_2_PCI::PCI_SingleFrame:
    {
      const uint8_t pci{static_cast<uint8_t>(uds_frame->GetProtocolInformation())};
      const uint8_t pci_shifted = pci << 4;
      const uint8_t size{static_cast<uint8_t>(uds_frame->GetDataLength()+1)};
      const uint8_t sid{static_cast<uint8_t>(uds_frame->GetSID())};
      const uint8_t* data{uds_frame->GetData()};
      tx_frame = {new CAN_Frame};
      tx_frame->Fill(padding);
      tx_frame->SetData(CAN_Frame::DataPos_0, pci_shifted | size);
      tx_frame->SetData(CAN_Frame::DataPos_1, sid);
      CAN_Frame::DataPos data_pos{CAN_Frame::DataPos_2};
      for(auto remaining_bytes{uds_frame->GetDataLength()}; remaining_bytes > 0; --remaining_bytes)
      {
        tx_frame->SetData(data_pos, *data);
        data_pos = static_cast<CAN_Frame::DataPos>(data_pos + 1);
        data++;
      }
      tx_frame->SetSource(uds_frame->GetSource());
      frames.push_back(tx_frame);
      tx_frame = nullptr;
    }
    break;
    case ISO_15765_2_PCI::PCI_FirstFrame:
    {
      constexpr auto data_bytes_total_in_ff{5};
      constexpr auto sid_size{1};
      const uint8_t pci{static_cast<uint8_t>(uds_frame->GetProtocolInformation())};
      const uint8_t pci_shifted = pci << 4;
      ff_cf_remaining_data_bytes = uds_frame->GetDataLength() + 1;
      volatile uint8_t size_l{static_cast<uint8_t>(ff_cf_remaining_data_bytes & 0xFF)};
      volatile uint8_t size_h{static_cast<uint8_t>((ff_cf_remaining_data_bytes & 0x0F00) >> 8)};
      const uint8_t sid{static_cast<uint8_t>(uds_frame->GetSID())};
      ff_cf_remaining_data_bytes -= sid_size;
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
      tx_frame->SetSource(uds_frame->GetSource());
      frames.push_back(tx_frame);
      tx_frame = nullptr;
    }
    break;
    case ISO_15765_2_PCI::PCI_ConsecutiveFrame:
    {
      constexpr auto cf_index_first{0x01}, cf_index_last{0x0F}, data_bytes_total_in_cf{7};
      uint8_t index{cf_index_first};
      const uint8_t pci{static_cast<uint8_t>(uds_frame->GetProtocolInformation())};
      const uint8_t pci_shifted = pci << 4;
      const uint8_t* data{uds_frame->GetData()};

      while(ff_cf_remaining_data_bytes >= data_bytes_total_in_cf)
      {
        tx_frame = {new CAN_Frame};
        tx_frame->Fill(0x55);
        tx_frame->SetData(CAN_Frame::DataPos_0, pci_shifted | index++);
        for(auto i{0}; i < data_bytes_total_in_cf; ++i)
        {
          CAN_Frame::DataPos data_pos = static_cast<CAN_Frame::DataPos>(CAN_Frame::DataPos_1 + i);
          tx_frame->SetData(data_pos, *data++);
        }
        tx_frame->SetSource(uds_frame->GetSource());
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
        tx_frame->SetSource(uds_frame->GetSource());
        frames.push_back(tx_frame);
        tx_frame = nullptr;
      }
    }
    break;
    case ISO_15765_2_PCI::PCI_FlowControlFrame:
    {
      const uint8_t pci{static_cast<uint8_t>(uds_frame->GetProtocolInformation())};
      const uint8_t pci_shifted = pci << 4;
      const ISO_15765_2_PCI_FCF_Flag fcf_flag{uds_frame->GetFlagOfFCF()};
      tx_frame = {new CAN_Frame};
      tx_frame->Fill(padding);
      tx_frame->SetData(CAN_Frame::DataPos_0, pci_shifted | (uint8_t)fcf_flag);
      tx_frame->SetData(CAN_Frame::DataPos_1, m_block_size_this_device);
      tx_frame->SetData(CAN_Frame::DataPos_2, m_separation_time_this_device);
      tx_frame->SetSource(uds_frame->GetSource());
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
void UDSOnCAN::SetStatus(const Status a_status)
{
  m_status = a_status;
}
UDSOnCAN::Status UDSOnCAN::GetStatus()
{
  return m_status;
}
void UDSOnCAN::MakePositiveResponse(const UDS::Service a_sid, const uint8_t* a_data_ptr, const uint32_t a_data_size, const CAN_Frame::Source a_source)
{
  if(a_data_size <= 6)
  {
    UDS_Frame* single_frame{new UDS_Frame};
    single_frame->SetSID(static_cast<UDS::Service>(a_sid + 0x40));
    single_frame->SetDataLength(a_data_size);
    single_frame->SetProtocolInformation(ISO_15765_2_PCI::PCI_SingleFrame);
    single_frame->SetData(a_data_ptr,a_data_size,0);
    single_frame->SetSource(a_source);
    m_uds_tx_buffer.push_back(single_frame);
  }
  else
  {
    UDS_Frame* first_frame{new UDS_Frame};
    uint32_t ff_data_size{5};
    first_frame->SetSID(static_cast<UDS::Service>(a_sid + 0x40));
    first_frame->SetDataLength(a_data_size);
    first_frame->SetProtocolInformation(ISO_15765_2_PCI::PCI_FirstFrame);
    first_frame->SetData(a_data_ptr,ff_data_size,0);
    first_frame->SetSource(a_source);
    m_uds_tx_buffer.push_back(first_frame);

    a_data_ptr += ff_data_size;
    UDS_Frame* consecutive_frames{new UDS_Frame};
    uint32_t cf_data_size{a_data_size - ff_data_size};
    consecutive_frames->SetDataLength(cf_data_size);
    consecutive_frames->SetProtocolInformation(ISO_15765_2_PCI::PCI_ConsecutiveFrame);
    consecutive_frames->SetData(a_data_ptr,cf_data_size,0);
    consecutive_frames->SetSource(a_source);
    m_uds_tx_buffer.push_back(consecutive_frames);

    m_status = Status_is_executing_rx_ff;
  }
}
void UDSOnCAN::MakeNegativeResponse(UDS::Service a_rejected_sid, UDS::NegativeResponseCode a_nrc, const CAN_Frame::Source a_source)
{
  constexpr auto payload_size{2};
  UDS_Frame* negativeresponse_frame{new UDS_Frame};
  negativeresponse_frame->SetProtocolInformation(ISO_15765_2_PCI::PCI_SingleFrame);
  negativeresponse_frame->SetSID(UDS::Service_NegativeResponse);
  negativeresponse_frame->SetDataLength(payload_size);
  uint8_t* data{new uint8_t[payload_size]};
  data[0] = (uint8_t)a_rejected_sid;
  data[1] = (uint8_t)a_nrc;
  negativeresponse_frame->SetData(data, payload_size, 0);
  negativeresponse_frame->SetSource(a_source);
  m_uds_tx_buffer.push_back(negativeresponse_frame);
  delete[] data;
}
