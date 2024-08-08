#ifndef __DID_H__
#define __DID_H__

#include <cstdint>
#include <list>
#include <string>
#include <cstring>
#include <iostream>

enum DID: uint16_t
{
  DID_VehicleManufacturerECUSoftwareConfigurationNumber         = 0xF1A0,
  DID_VehicleManufacturerECUSoftwareConfigurationVersionNumber  = 0xF1A1,
  DID_FirmwareUpdateMode                                        = 0x200,  //= 0xF1A2,
  DID_MapUpdateMode                                             = 0xF1A3,
  DID_RS232_1_BaudrateSetup                                     = 0xF1A4,
  DID_RS232_2_BaudrateSetup                                     = 0xF1A5,
  DID_RS485_BaudrateSetup                                       = 0xF1A6,
  DID_WiFiPassword                                              = 0xF1A7,
  DID_DiagData                                                  = 0xF1A8,
  DID_VehicleManufacturerECUSoftwareNumber                      = 0xF188,
  DID_VehicleManufacturerECUSoftwareVersionNumber               = 0xF189,
  DID_SystemSupplierIdentifierDataIdentifier                    = 0xF18A,
  DID_ECUManufacturingDate                                      = 0xF18B,
  DID_ECUSerialNumber                                           = 0xF18C,
  DID_VIN                                                       = 0xF190,
  DID_VehicleManufacturerECUHardwareNumber                      = 0xF191,
  DID_SystemSupplierECUHardwareNumber                           = 0xF192,
  DID_SystemSupplierECUHardwareVersionNumber                    = 0xF193,
  DID_SystemSupplierECUSoftwareNumber                           = 0xF194,
  DID_SystemSupplierECUSoftwareVersionNumber                    = 0xF195,
  DID_ExhaustRegulationOrTypeApprovalNumber                     = 0xF196,
  DID_SystemNameOrEngineType                                    = 0xF197,
  DID_RepairShopCodeOrTesterSerialNumber                        = 0xF198,
  DID_ProgrammingDate                                           = 0xF199,
  DID_ECUInstallationDate                                       = 0xF19D,
  DID_SystemSupplierBaseConfiguration                           = 0xF1F0,
  DID_BaseSoftwareVersion                                       = 0xFD10
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
    DID_Datatype_bytearray
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
private:
  const DID      m_did;
  uint32_t       m_size;
  DID_Datatype   m_datatype;
  const DID_RW   m_rw;
  bool           m_islocked;
  uint8_t*       mp_data_first_byte;
};

class DID_Repository
{
public:
  DID_Repository();
  ~DID_Repository();
  const DID_Instance* const GetInstanceOfDID(const DID a_did);
  bool                      ReadDataIdentifier(const DID a_did, uint8_t* ap_read_to, uint8_t a_size_bytes);
  std::string               ReadDataIdentifier(const DID a_did);
  bool                      FindDataIdentifier(const DID a_did) const;
  uint32_t                  GetDataIdentifierSize(const DID a_did) const; 
  bool                      WriteDataIdentifier(const DID a_did, const uint8_t* ap_read_from, uint8_t a_size_bytes);
  bool                      WriteDataIdentifier(const DID a_did, const char* str);
  bool                      WriteDataIdentifier(const DID a_did, std::string str);
  void                      AddDataIdentifier(DID_Instance* ap_did);
  void                      AddDataIdentifier(const DID a_did, const uint32_t a_size_bytes, const DID_Instance::DID_Datatype a_datatype, const DID_Instance::DID_RW a_rw);
  bool                      RemoveDataIdentifier(const DID a_did);
private:
  std::list<DID_Instance*> m_dids_list;    
};

#endif //__DID_H__ 