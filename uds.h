#ifndef __UDS_H__
#define __UDS_H__

#include <vector>
#include <deque>

#include "program_timer.h"
#include "uds_frame.h"
#include "can_frame.h"
#include "did.h"

class UDS
{
public:
  enum Service: uint8_t
  {
    //Diagnostic and Communications Management
    Service_DiagnosticSessionControl        = 0x10,    Service_ECUReset                        = 0x10,
    Service_SecurityAccess                  = 0x27,    Service_CommunicationControl            = 0x28,
    Service_Authentication                  = 0x29,    Service_TesterPresent                   = 0x3E,
    Service_AccessTimingParameter           = 0x83,    Service_SecuredDataTransmission         = 0x84,
    Service_ControlDTCSetting               = 0x85,    Service_ResponseOnEvent                 = 0x86,
    Service_LinkControl                     = 0x87,
    /*--------------------------Data Transmission--------------------------*/
    Service_ReadDataByIdentifier            = 0x22,    Service_ReadMemoryByAddress             = 0x23,
    Service_ReadScalingDataByIdentifier     = 0x24,    Service_ReadDataByPeriodicIdentifier    = 0x2A,
    Service_DynamicallyDefineDataIdentifier = 0x2C,    Service_WriteDataByIdentifier           = 0x2E,
    Service_WriteMemoryByAddress            = 0x3D,
    /*-----------------------Stored Data Transmission----------------------*/
    Service_ClearDiagnosticUInformation     = 0x14,    Service_ReadDTCInformation              = 0x19,
    /*------------------------Input/output control-------------------------*/
    Service_InputOutputControlByIdentifier  = 0x2F,
    /*--------------------Remote Activation of Routine---------------------*/
    Service_RoutineControl                  = 0x31,
    /*--------------------------Upload / Download--------------------------*/
    Service_RequestDownload                 = 0x34,    Service_RequestUpload                   = 0x35,
    Service_TransferData                    = 0x36,    Service_RequestTransferExit             = 0x37,
    Service_RequestFileTransfer             = 0x38,
    /*--------------------------Negative response--------------------------*/
    Service_NegativeResponse                = 0x7F
  };
  enum NegativeResponseCode: uint8_t
  {
    NRC_GeneralReject                                   = 0x10,
    NRC_ServiceNotSupported                             = 0x11,
    NRC_Subfunctionnotsupported                         = 0x12,
    NRC_IncorrectMessageLengthOrInvalidFormat           = 0x13,
    NRC_ResponseTooLong                                 = 0x14,
    NRC_BusyRepeatRequest                               = 0x21,
    NRC_ConditionsNotCorrect                            = 0x22,
    NRC_RequestSequenceError                            = 0x24,
    NRC_NoResponseFromSubnetComponent                   = 0x25,
    NRC_FailurePreventsExecutionOfRequestedAction       = 0x26,
    NRC_RequestOutOfRange                               = 0x31,
    NRC_SecurityAccessDenied                            = 0x33,
    NRC_AuthenticationFailed                            = 0x34,
    NRC_InvalidKey                                      = 0x35,
    NRC_ExceededNumberOfAttempts                        = 0x36,
    NRC_RequiredTimeDelayNotExpired                     = 0x37,
    NRC_SecureDataTransmissionRequired                  = 0x38,
    NRC_SecureDataTransmissionNotAllowed                = 0x39,
    NRC_SecureDataVerificationFailed                    = 0x3A,
    NRC_CertificateValidationFailed_InvalidTimePeriod   = 0x50,
    NRC_CertificateValidationFailed_InvalidSignature    = 0x51,
    NRC_CertificateValidationFailed_InvalidChainOfTrust = 0x52,
    NRC_CertificateValidationFailed_InvalidType         = 0x53,
    NRC_CertificateValidationFailed_InvalidFormat       = 0x54,
    NRC_CertificateValidationFailed_InvalidContent      = 0x55,
    NRC_CertificateValidationFailed_InvalidScope        = 0x56,
    NRC_CertificateValidationFailed_InvalidCertificate  = 0x57,
    NRC_OwnershipVerificationFailed                     = 0x58,
    NRC_ChallengeCalculationFailed                      = 0x59,
    NRC_SettingAccessRightFailed                        = 0x5A,
    NRC_SessionKeyCreationDerivationFailed              = 0x5B,
    NRC_ConfigurationDataUsageFailed                    = 0x5C,
    NRC_DeauthenticationFailed                          = 0x5D,
    NRC_UploadDownloadNotAccepted                       = 0x70,
    NRC_TransferDataSuspended                           = 0x71,
    NRC_GeneralProgrammingFailure                       = 0x72,
    NRC_WrongBlockSequenceNumber                        = 0x73,
    NRC_RequestCorrectlyReceived_ResponsePending        = 0x78,
    NRC_SubfunctionNotSupportedInActiveSession          = 0x7E,
    NRC_ServiceNotSupportedInActiveSession              = 0x7F,
    NRC_TooHighRPM                                      = 0x81,
    NRC_TooLowRPM                                       = 0x82,
    NRC_EngineIsRunning                                 = 0x83,
    NRC_EngineIsNotRunning                              = 0x84,
    NRC_EngineRunTimeTooLow                             = 0x85,
    NRC_TemperatureTooHigh                              = 0x86,
    NRC_TemperatureTooLow                               = 0x87,
    NRC_VehicleSpeedTooHigh                             = 0x88,
    NRC_VehicleSpeedTooLow                              = 0x89,
    NRC_ThrottleOrPedalTooHigh                          = 0x8A,
    NRC_ThrottleOrPedalTooLow                           = 0x8B,
    NRC_TransmissionRangeNotInNeutral                   = 0x8C,
    NRC_TransmissionRangeNotInGear                      = 0x8D,
    NRC_BrakeSwitchNotClosed                            = 0x8F,
    NRC_ShifterLeverNotInPark                           = 0x90,
    NRC_TorqueConverterClutchLocked                     = 0x91,
    NRC_VoltageTooHigh                                  = 0x92,
    NRC_VoltageTooLow                                   = 0x93,
    NRC_ResourceTemporaryUnavailable                    = 0x94
  };

  enum SeedSize
  {
    Seedsize_1_byte = 1,
    Seedsize_2_byte = 2,
    Seedsize_3_byte = 3,
    Seedsize_4_byte = 4,
    Seedsize_5_byte = 5
  };
  enum SessionType
  {
    DSC_Type_DefaultSession = 0x01,
    DSC_Type_ProgrammingSession = 0x02,
    DSC_Type_ExtendedDiagnosticSession = 0x03,
    DSC_Type_SafetySystemDiagnosticSession = 0x04
  };
  enum CommunicationControl
  {
    CC_EnableRxEnableTx = 0x00,
    CC_EnableRxDisableTx = 0x01,
    CC_DisableRxEnableTx = 0x02,
    CC_DisableRxDisableTx = 0x03
    //EnableRxAndDisableTxWithEnhancedAddressInformation = 0x04,
    //EnableRxAndTxWithEnhancedAddressInformation = 0x05,
    //0x06 – 0x3F ISOSAEReserved
    //0x40 – 0x5F vehicleManufacturerSpecific
  };
  enum CommunicationType
  {
    //ISOSAERESERVED = 0x00
    NormalCommunicationMessages = 0x1,
    NetworkManagementCommunicationMessages = 0x2,
    NetworkManagementCommunicationMessagesAndNormalCommunicationMessages = 0x3
  };

  void        SetSessionType(const SessionType a_sessiontype);
  SessionType GetSessiontype();
  bool        CheckNumberOfSecurityAccessAttempts(const uint8_t a_subfunction);
  void        ReloadNumberOfSecurityAccessAttempts();
  void        SetCommunicationControl(CommunicationControl a_communication_control);
  bool        IsECURXEnabled();
  bool        IsECUTXEnabled();
  void        CheckS3Timer();
  void        SetSeparationTimeForTester(uint8_t a_stmin);
  uint8_t     GetSeparationTimeTester() const;
  void        GenerateAndUpdateSecurityAccessSeed(SeedSize a_seed_size);
  void        CalculateSecurityAccessFullKey();
  uint64_t    GetSecurityAccessKey();
  bool        CompareSecurityAccessKey(uint64_t a_key);
protected:
  UDS();
  virtual ~UDS() = default;

  bool          m_is_busy;
  
  SessionType   m_sessiontype;
  uint8_t       m_sa_security_level_unlocked;
  bool          m_sa_requestsequenceerror;
  uint8_t       m_programmingsession_number_of_attempts;
  uint8_t       m_extendeddiagnosticsession_number_of_attempts;
  uint8_t       m_safetysystemdiagnosticsession_number_of_attempts;
  SeedSize      m_seed_size;
  uint64_t      m_seed;
  uint64_t      m_key;

  Program_timer m_p2_timer;
  Program_timer m_s3_timer;
  Program_timer m_separation_time_min_this_device_timer;
  Program_timer m_separation_time_min_tester_timer;
  const uint8_t m_separation_time_this_device;
  uint8_t       m_separation_time_min_tester;
  const uint8_t m_block_size_this_device; 
  uint8_t       m_block_size_tester;

  bool          m_is_rx_enabled;
  bool          m_is_tx_enabled;
  //CommunicationControl m_cc_status;
};

class UDSOnCAN : public UDS
{
public:
  enum Status
  {
    Status_ok,
    Status_is_busy,
    Status_is_executing_rx_ff,
    Status_is_waiting_fcf,
    Status_recieved_fcf,
  };
  UDSOnCAN();
  ~UDSOnCAN();
  
  void Execute();

  bool IsRXBufferOfUDSEmpty();
  bool IsTXBufferOfUDSEmpty();
  bool ConvertCANFrameToUDS(const CAN_Frame* const ap_can_frame);
  std::vector<CAN_Frame*> ConvertUDSFrameToCAN();

  void        SetStatus(const Status a_status);
  Status      GetStatus();

  DID_Repository& GetDIDRepository();
private:
  void MakePositiveResponse(const Service a_sid, const uint8_t* a_data_ptr, const uint32_t a_data_size, const CAN_Frame::Source a_source);
  void MakeNegativeResponse(const Service a_rejected_sid, UDS::NegativeResponseCode a_nrc, const CAN_Frame::Source a_source);
  
  std::deque <UDS_Frame*> m_uds_tx_buffer;
  std::deque <UDS_Frame*> m_uds_rx_buffer;

  DID_Repository m_did_repository;
  Status         m_status;
  uint8_t        m_frames_remaining_until_next_fcf;
};
#endif //__UDS_H__