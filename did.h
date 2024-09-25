#ifndef __DID_H__
#define __DID_H__

#include <cstdint>
#include <list>
#include <string>
#include <cstring>
#include <iostream>

enum DID: uint16_t
{
  DID_VehicleManufacturerECUSoftwareConfigurationNumber         = 0xF1A0,//odx ok
  DID_VehicleManufacturerECUSoftwareConfigurationVersionNumber  = 0xF1A1,//odx ok
  DID_FirmwareUpdateMode                                        = 0xF1A2,//odx ok
  DID_MapsUpdateMode                                            = 0xF1A3,//odx ok
  DID_RS232_1_BaudrateSetup                                     = 0xF1A4,//odx ok
  DID_RS232_2_BaudrateSetup                                     = 0xF1A5,//odx ok
  DID_RS485_BaudrateSetup                                       = 0xF1A6,//odx ok
  DID_WiFiPassword                                              = 0xF1A7,//odx ok
  DID_DiagData                                                  = 0xF1A8,//odx ok
  DID_VehicleManufacturerECUSoftwareNumber                      = 0xF188,//odx ok
  DID_VehicleManufacturerECUSoftwareVersionNumber               = 0xF189,//odx ok
  DID_SystemSupplierIdentifierDataIdentifier                    = 0xF18A,//odx ok
  DID_ECUManufacturingDate                                      = 0xF18B,//odx ok
  DID_ECUSerialNumber                                           = 0xF18C,//odx ok need test
  DID_VIN                                                       = 0xF190,//odx ok 
  DID_VehicleManufacturerECUHardwareNumber                      = 0xF191,//odx ok
  DID_SystemSupplierECUHardwareNumber                           = 0xF192,//odx ok
  DID_SystemSupplierECUHardwareVersionNumber                    = 0xF193,//odx ok need test
  DID_SystemSupplierECUSoftwareNumber                           = 0xF194,//odx ok
  DID_SystemSupplierECUSoftwareVersionNumber                    = 0xF195,//odx ok
  DID_ExhaustRegulationOrTypeApprovalNumber                     = 0xF196,//odx ok
  DID_SystemNameOrEngineType                                    = 0xF197,//odx ok
  DID_RepairShopCodeOrTesterSerialNumber                        = 0xF198,//odx ok
  DID_ProgrammingDate                                           = 0xF199,//odx ok
  DID_ECUInstallationDate                                       = 0xF19D,//odx ok
  DID_SystemSupplierBaseConfiguration                           = 0xF1F0,//odx ok
  DID_BaseSoftwareVersion                                       = 0xFD10,//odx ok
  DID_FirmwareUpdateStatus                                      = 0xF1A9,//odx ok
  DID_MapsUpdateStatus                                          = 0xF1AA,//odx ok
  DID_RestartFromWatchdogError                                  = 0xF010,
  DID_HighVoltageError                                          = 0xF011,
  DID_LowVoltageError                                           = 0xF012,
  DID_GNSSModuleError                                           = 0xF013,
  DID_GyroAccelAccessError                                      = 0xF014,
  DID_BTPFirmwareUpdateError                                    = 0xF015,
  DID_KamazReliefMapLoadError                                   = 0xF016,
  DID_MCUFirmwareIntegrityError                                 = 0xF017,
  DID_BipError                                                  = 0xF018
};

class DID_Instance
{
public:
  enum DID_Datatype
  {
    DID_Datatype_std_string,
    DID_Datatype_c_string,
    DID_Datatype_float,
    DID_Datatype_double,
    DID_Datatype_integer,
    DID_Datatype_unsigned_integer,
    DID_Datatype_bool,
    DID_Datatype_raw_data
  };
  enum DID_RW
  {
    ReadWrite,
    Readonly
  };
  DID_Instance(const DID a_did, const uint32_t a_size, const DID_Datatype a_datatype, DID_RW a_rw);
  //have to use std::move!!!
  //DID_Instance(const DID_Instance& aref_did_instance);
  ~DID_Instance();
  DID            GetDID() const;
  uint32_t       GetDataSize() const;
  DID_Datatype   GetDataType() const;
  DID_RW         IsRW() const;
  const uint8_t* GetPtrToConstData() const;
  uint8_t*       GetPtrToData();
  bool           IsLocked();
  void           Lock();
  void           Unlock();
  void           SetModifyFlag(bool a_flag);
  bool           IsModified();
  
private:
  const DID      m_did;
  uint32_t       m_size;
  DID_Datatype   m_datatype;
  const DID_RW   m_rw;
  bool           m_islocked;
  uint8_t*       mp_data_first_byte;
  bool           m_is_modified;
};

class DID_Repository
{
public:
  DID_Repository();
  ~DID_Repository();
  const DID_Instance* const  GetInstanceOfDID(const DID a_did);
  bool                       ReadDataIdentifier(const DID a_did, uint8_t* ap_read_to, uint8_t a_size_bytes);
  bool                       LE_ReadDataIdentifier(const DID a_did, uint8_t* ap_read_to, uint8_t a_size_bytes);
  std::string                ReadDataIdentifier(const DID a_did);
  bool                       FindDataIdentifier(const DID a_did) const;
  uint32_t                   GetDataIdentifierSize(const DID a_did) const; 
  bool                       WriteDataIdentifier(const DID a_did, const uint8_t* ap_read_from, uint8_t a_size_bytes);
  bool                       LE_WriteDataIdentifier(const DID a_did, const uint8_t* ap_read_from, uint8_t a_size_bytes); //read from data is LE (use only for non string data types)
  bool                       WriteDataIdentifier(const DID a_did, const char* str);
  bool                       WriteDataIdentifier(const DID a_did, std::string str);
  void                       AddDataIdentifier(DID_Instance* ap_did);
  void                       AddDataIdentifier(const DID a_did, const uint32_t a_size_bytes, const DID_Instance::DID_Datatype a_datatype, const DID_Instance::DID_RW a_rw);
  bool                       RemoveDataIdentifier(const DID a_did);
  std::vector<DID_Instance*> GetListOfModifiedDIDs();
private:
  std::list<DID_Instance*> m_dids_list;    
};

#endif //__DID_H__ 