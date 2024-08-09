#ifndef __UDS_MESSAGE_H__
#define __UDS_MESSAGE_H__
#include <stdint.h>
#include "can_frame.h"

// this is only appreciable for can frames on the CAN bus!!!

class UDS_Frame
{
public:
  enum Service: uint8_t
  {
    //Diagnostic and Communications Management
    DiagnosticSessionControl        = 0x10,    ECUReset                        = 0x10,
    SecurityAccess                  = 0x27,    CommunicationControl            = 0x28,
    Authentication                  = 0x29,    TesterPresent                   = 0x3E,
    AccessTimingParameter           = 0x83,    SecuredDataTransmission         = 0x84,
    ControlDTCSetting               = 0x85,    ResponseOnEvent                 = 0x86,
    LinkControl                     = 0x87,
    /*--------------------------Data Transmission--------------------------*/
    ReadDataByIdentifier            = 0x22,    ReadMemoryByAddress             = 0x23,
    ReadScalingDataByIdentifier     = 0x24,    ReadDataByPeriodicIdentifier    = 0x2A,
    DynamicallyDefineDataIdentifier = 0x2C,    WriteDataByIdentifier           = 0x2E,
    WriteMemoryByAddress            = 0x3D,
    /*-----------------------Stored Data Transmission----------------------*/
    ClearDiagnosticUInformation     = 0x14,    ReadDTCInformation              = 0x19,
    /*------------------------Input/output control-------------------------*/
    InputOutputControlByIdentifier  = 0x2F,
    /*--------------------Remote Activation of Routine---------------------*/
    RoutineControl                  = 0x31,
    /*--------------------------Upload / Download--------------------------*/
    RequestDownload                 = 0x34,    RequestUpload                   = 0x35,
    TransferData                    = 0x36,    RequestTransferExit             = 0x37,
    RequestFileTransfer             = 0x38,
    /*--------------------------Negative response--------------------------*/
    NegativeResponse                = 0x7F
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
  enum PCI//ProtocolControlInformation
  {
    PCI_SingleFrame      = 0x0,
    PCI_FirstFrame       = 0x1,
    PCI_ConsecutiveFrame = 0x2,
    PCI_FlowControlFrame = 0x3
  };
  enum FCF_Flag
  {
    FCF_Flag_CTS      = 0x00,
    FCF_Flag_Wait     = 0x01,
    FCF_Flag_Overflow = 0x02
  };


  UDS_Frame();
  ~UDS_Frame();
  
  void SetSource(const CAN_Frame::Source a_source);
  CAN_Frame::Source GetSource() const;
  void SetCanID(const uint32_t a_can_id);
  uint32_t GetCanID();

  void SetProtocolInformation(UDS_Frame::PCI a_protocol_information);
  UDS_Frame::PCI GetProtocolInformation();

  void SetDataLength(const uint16_t a_data_length);
  uint16_t Getdatalength();

  void SetConsecutiveFrameNumber(const uint8_t a_consecutive_frame_number);
  uint8_t GetConsecutiveFrameNumner();

  void SetSID(const Service a_service_id);
  Service GetSID();

  void SetSubfunctionByte(uint8_t a_subfunction_byte);
  uint8_t GetSubfunctionByte();

  void SetData(const uint8_t* a_payload_array, uint32_t a_payload_size, const uint32_t a_offset);
  const uint8_t* GetData();

  void SetFlagOfFCF(const FCF_Flag a_flag);
  FCF_Flag GetFlagOfFCF() const;
private:
  CAN_Frame::Source m_source;
  uint32_t          m_can_id;
  PCI               m_protocol_information;
  uint16_t          m_data_length;
  uint8_t           m_consecutive_frame_number;
  Service           m_sid;
  uint8_t           m_subfunction_byte;
  uint8_t           m_data[4096];
  bool              is_ready; //at the end of transmission sets to true
  FCF_Flag          m_fcf_flag;
  //rdp (including did - data identifier) request data parameters
  //padding
};

#endif //__UDS_MESSAGE_H__