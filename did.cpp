#include "did.h"


//Use std::move!!!
// DID_Instance::DID_Instance(const DID_Instance& aref_did_instance)
//     : m_did{aref_did_instance.m_did}
//     , m_size{aref_did_instance.m_size}
//     , m_datatype{aref_did_instance.m_datatype}
//     , m_rw{a_rw}
//     , mp_data_first_byte{nullptr}
// {}
DID_Instance::DID_Instance(const DID a_did, const uint32_t a_size, const DID_Datatype a_datatype, DID_RW a_rw)
  : m_did{a_did}
  , m_size{a_size}
  , m_datatype{a_datatype}
  , m_rw{a_rw}
  , m_islocked{false}
  , mp_data_first_byte{nullptr}
  , m_is_modified{false}
{
  if(a_datatype == DID_Datatype_c_string || a_datatype == DID_Datatype_std_string)
    m_size++;
  
  mp_data_first_byte = new uint8_t[m_size];
  if (a_rw == Readonly)
    Lock();
}
DID_Instance::~DID_Instance()
{
  delete mp_data_first_byte;
}
DID DID_Instance::GetDID() const
{
  return m_did;
}
uint32_t DID_Instance::GetDataSize() const
{
  return m_size;
}
DID_Instance::DID_Datatype DID_Instance::GetDataType() const
{
  return m_datatype;
}
DID_Instance::DID_RW DID_Instance::IsRW() const
{
  return m_rw;
}
const uint8_t* DID_Instance::GetPtrToConstData() const
{
  return mp_data_first_byte;
}
uint8_t* DID_Instance::GetPtrToData()
{
  if(m_rw == ReadWrite)
    return mp_data_first_byte;
  else return nullptr;
}
bool DID_Instance::IsLocked()
{
  return m_islocked;
}
void DID_Instance::Lock()
{
  m_islocked = true;
}
void DID_Instance::Unlock()
{
  m_islocked = false;
}
void DID_Instance::SetModifyFlag(bool a_flag)
{
  m_is_modified = a_flag;
}
bool DID_Instance::IsModified()
{
  return m_is_modified;
}


DID_Repository::DID_Repository(){}
DID_Repository::~DID_Repository(){}
void DID_Repository::AddDataIdentifier(DID_Instance* a_did_instance)
{
  for(auto it{m_dids_list.begin()}; it != m_dids_list.end();)
  {
    if((*it)->GetDID() == a_did_instance->GetDID())
    {
      delete (*it);
      it = m_dids_list.erase(it);
    }
    else  ++it;
  }
  m_dids_list.push_back(a_did_instance);
}
void DID_Repository::AddDataIdentifier(const DID a_did, const uint32_t a_size_bytes, const DID_Instance::DID_Datatype a_datatype, const DID_Instance::DID_RW a_rw)
{
  DID_Instance* temp_did{new DID_Instance(a_did, a_size_bytes, a_datatype, a_rw)};
  for(auto it{m_dids_list.begin()}; it != m_dids_list.end();)
  {
    if((*it)->GetDID() == temp_did->GetDID())
    {
      delete (*it);
      it = m_dids_list.erase(it);
    }
    else  ++it;
  }
  m_dids_list.push_back(temp_did);
}
bool DID_Repository::RemoveDataIdentifier(const DID a_did)
{
  for(auto it{m_dids_list.begin()}; it != m_dids_list.end(); ++it)
  {
    if((*it)->GetDID() == a_did)
    {
      delete (*it);
      m_dids_list.erase(it);
      return true;
    }
  }
  return false;
}
const DID_Instance* const DID_Repository::GetInstanceOfDID(const DID a_did)
{
  for(auto it{m_dids_list.begin()}; it != m_dids_list.end(); ++it)
  {
    if((*it)->GetDID() == a_did)
      return *it;
  }
  return nullptr;
}
bool DID_Repository::ReadDataIdentifier(const DID a_did, uint8_t* ap_read_to, uint8_t a_size_bytes)
{
  for(auto it{m_dids_list.begin()}; it != m_dids_list.end(); ++it)
  {
    if((*it)->GetDID() == a_did)
    {
      if (a_size_bytes > (*it)->GetDataSize())
      {
        std::cout << "[   ERROR   ] Can not read data by ID 0x" << std::hex << std::uppercase << a_did << ". Size of output buffer is too high!" << '\n';
        return false;
      }
      uint8_t* read_to_ptr = ap_read_to;
      const uint8_t* read_from_ptr = (*it)->GetPtrToConstData(); 
      while (a_size_bytes--)
        *read_to_ptr++ = *read_from_ptr++;
      return true;
    }

  }
  std::cout << "[ WARNING ] Data by ID 0x" << std::hex << std::uppercase << a_did << " does not exist! (Inccorrect DID)" << '\n';
  return false;
}
std::string DID_Repository::ReadDataIdentifier(const DID a_did)
{
  for(auto it{m_dids_list.begin()}; it != m_dids_list.end(); ++it)
  {
    if((*it)->GetDID() == a_did)
    {
      bool condition{(*it)->GetDataType() == DID_Instance::DID_Datatype_c_string || (*it)->GetDataType() == DID_Instance::DID_Datatype_std_string};
      if(condition)
      {
        uint32_t size_of_data_to_read{(*it)->GetDataSize()};
        const char* ptr_read_from{(const char*)(*it)->GetPtrToConstData()};
        char* temp_string_index_ptr{new char[size_of_data_to_read]};  //???? CHECK FOR C STRINGS (NULL CHARACTERS) !!!
        const char* temp_string{temp_string_index_ptr};
        while (size_of_data_to_read--)
        {
            *temp_string_index_ptr++ = *ptr_read_from++;
        }
        std::string ret (temp_string);
        delete [] temp_string;
        return ret;
      }
      else
      {
        std::cout << "[   ERROR   ] Can not read data correctly by ID 0x" << std::hex << std::uppercase << a_did << ". The datatype mismatches!" << '\n';
        return std::string("");
      }
    }
  }
  std::cout << "[ WARNING ] Data by ID 0x" << std::hex << std::uppercase << a_did << " does not exist! (Inccorrect DID)" << '\n';
  return std::string("");
}
bool DID_Repository::FindDataIdentifier(const DID a_did) const
{
  bool is_operation_successful{false};
  for(auto it{m_dids_list.begin()}; it != m_dids_list.end(); ++it)
  {
    if((*it)->GetDID() == a_did)
      is_operation_successful = true;
  }
  return is_operation_successful;
}
uint32_t DID_Repository::GetDataIdentifierSize(const DID a_did) const
{
  for(auto it{m_dids_list.begin()}; it != m_dids_list.end(); ++it)
  {
    if((*it)->GetDID() == a_did)
      return (*it)->GetDataSize();
  }
  return 0;
}
bool DID_Repository::WriteDataIdentifier(const DID a_did, const uint8_t* ap_read_from, uint8_t a_size_bytes)
{
  bool is_operation_successful{false};
  for(auto it{m_dids_list.begin()}; it != m_dids_list.end(); ++it)
  {
    bool is_size_appropriate{(*it)->GetDataSize() >= a_size_bytes};
    bool is_rw{(*it)->IsRW() == DID_Instance::DID_RW::ReadWrite};
    bool is_not_locked{!(*it)->IsLocked()};

    /* if you do not need std::cout messages, use this equation:
    if( (*it)->GetDID() == a_did && (*it)->GetDataSize() >= a_size_bytes 
      && (*it)->IsRW() == DID_Instance::DID_RW::ReadWrite && !(*it)->IsLocked() )
    */

    if((*it)->GetDID() == a_did)
    {   
      if(!is_size_appropriate)
        std::cout << "[   ERROR   ] Can not write data by ID 0x" << std::hex << std::uppercase << a_did << ". Size of input buffer is too high!" << '\n';
      if(!is_rw)
        std::cout << "[   ERROR   ] Can not write data by ID 0x" << std::hex << std::uppercase <<  a_did << ". Data by this ID is READ ONLY!" << '\n';
      if(!is_not_locked)
        std::cout << "[   ERROR   ] Can not write data by ID 0x" << std::hex << std::uppercase << a_did << ". Lock is set!" << '\n';

      if(is_size_appropriate && is_rw && is_not_locked)
      {
        const uint8_t* read_from_ptr{ap_read_from};
        uint8_t* write_to_ptr{(*it)->GetPtrToData()};
        
        auto write_to_data_size{(*it)->GetDataSize()};
        auto read_from_data_size{a_size_bytes};

        while(write_to_data_size--) // !! never use "read_from_data_size--"" !!
        {
          if(read_from_data_size)
          {
            *write_to_ptr++ = *read_from_ptr++;
            --read_from_data_size;
          }
          else 
            *write_to_ptr++ = 0;
        }
        (*it)->SetModifyFlag(true);
        is_operation_successful = true;
      }
    }
  }
  return is_operation_successful; 
}
bool DID_Repository::WriteDataIdentifier(const DID a_did, const char* str)
{
  return WriteDataIdentifier(a_did, (const uint8_t*)str, strlen(str)+1);
}
bool DID_Repository::WriteDataIdentifier(const DID a_did, std::string str)
{
  return WriteDataIdentifier(a_did, str.c_str());
}