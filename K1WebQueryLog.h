/* 
 * File:   WebQueryLog.h
 * Author: Raymond Burkholder
 *         rburkholder@quovadis.bm
 *
 * Created on July 21, 2015, 9:58 AM
 */

#pragma once

#include <string>
#include <fstream>
#include <sstream>

#include <boost/date_time/posix_time/ptime.hpp>

#include <boost/shared_ptr.hpp>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include "StateStatus.h"

class K1WebQueryLog {
public:
  
  struct QueryData {
    typedef boost::shared_ptr<QueryData> pQueryData_t;
    boost::posix_time::ptime dt; // setting filename
    std::string p;
    std::string s;
    std::string v;
    std::string d;
    std::string sServerName;  // server name
    pQueryData_t pNext; // maintains a linked list
    QueryData( void ) { Reset(); }
    void Reset( void ) { dt = boost::posix_time::not_a_date_time; }
  };
  
  typedef QueryData::pQueryData_t pQueryData_t;
  
  K1WebQueryLog( StateStatus&, const std::string& sFileNamePrefix );
  virtual ~K1WebQueryLog();
  
  void GetContainer( pQueryData_t& ); // get a string from free list 
  void Log( pQueryData_t& ); // send string to log 
  void PutContainer( pQueryData_t& );
  
private:
  
  StateStatus& m_status;
  
  const std::string m_sFileNamePrefix;
  
  std::ofstream m_ofs;
  std::ostringstream m_ssLogNameDateTime;
  std::ostringstream m_ssLogEntryDateTime;
  
  boost::posix_time::ptime m_dtLast;
  
  boost::mutex m_mutexQDAvail;
  pQueryData_t m_pQDAvailHead;  // no concern for order
  
  boost::mutex m_mutexQDToLog;
  pQueryData_t m_pQDToLogFirst; // Fifo head
  pQueryData_t m_pQDToLogLast;  // Fifo tail
  
  bool m_bAllowThread;
  
  boost::thread m_thread;
  
  boost::mutex m_mutexWait;  // pause thread while no work available
  boost::condition_variable m_cvWriteLog;
  
  void File( boost::posix_time::ptime, const std::string& sServerName );
  void Thread(void); // stuff running in the thread
  
};
