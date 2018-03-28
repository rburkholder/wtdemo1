/*
 * File:   WebQueryLog.cpp
 * Author: Raymond Burkholder
 *         rburkholder@quovadis.bm
 * Created on July 21, 2015, 9:58 AM
 */

#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/gregorian/gregorian_io.hpp>
#include <boost/bind.hpp>

#include "K1WebQueryLog.h"

K1WebQueryLog::K1WebQueryLog( StateStatus& status, const std::string& sFileNamePrefix ):
  m_status( status ),
  m_sFileNamePrefix( sFileNamePrefix ),
  m_bAllowThread( true ), //m_bThreadRunning(false),
  m_dtLast( boost::posix_time::not_a_date_time ),
  m_thread( boost::bind( &K1WebQueryLog::Thread, this ) )
{
  boost::posix_time::time_facet* facetLogNameDateTime = new boost::posix_time::time_facet("%Y%m%d%H%M");
  m_ssLogNameDateTime.imbue( std::locale(m_ssLogNameDateTime.getloc(), facetLogNameDateTime) );
  
  //m_ss.exceptions(std::ios_base::failbit);
  boost::posix_time::time_facet* facetLogEnryDateTime = new boost::posix_time::time_facet("%Y.%m.%d %H:%M:%S");
  m_ssLogEntryDateTime.imbue( std::locale(m_ssLogEntryDateTime.getloc(), facetLogEnryDateTime) );
}

K1WebQueryLog::~K1WebQueryLog() {
  m_bAllowThread = false; // prepare to finish off thread
  m_cvWriteLog.notify_one(); // finish off any outstanding logging
  m_thread.join();  // wait for thread to clear out
  std::cerr << "K1WebQueryLog dtor " << m_status.m_cntContainersCreated << std::endl;
  assert( 0 == m_status.m_cntContainersActive );
  assert( 0 == m_pQDToLogFirst.get() );
  assert( 0 == m_pQDToLogLast.get() );
  m_pQDAvailHead.reset();  // should cascade delete all QueryData containers
  m_ofs.close();
}

void K1WebQueryLog::GetContainer( pQueryData_t& p ) {
  assert( 0 == p.get() );
  boost::lock_guard<boost::mutex> lock( m_mutexQDAvail );
  if( 0 == m_pQDAvailHead.get() ) {
    p.reset( new QueryData );
    m_status.m_cntContainersCreated.fetch_add( 1, boost::memory_order_relaxed );
  }
  else {
    p = m_pQDAvailHead;
    m_pQDAvailHead = p->pNext;
    p->pNext.reset();
  }
  assert( 0 != p.get() );
  m_status.m_cntContainersActive.fetch_add( 1, boost::memory_order_relaxed );
}
  
void K1WebQueryLog::PutContainer( pQueryData_t& p ) {
  boost::lock_guard<boost::mutex> lock(m_mutexQDAvail);
  p->Reset();
  p->pNext = m_pQDAvailHead;
  m_pQDAvailHead = p;
  p.reset();
  m_status.m_cntContainersActive.fetch_sub( 1, boost::memory_order_relaxed );
}

// may need to get at process id for further segregation?
void K1WebQueryLog::File( boost::posix_time::ptime dt, const std::string& sServerName ) {
  if ( !( boost::posix_time::not_a_date_time != m_dtLast ) ||
    ( m_dtLast.time_of_day().minutes() != dt.time_of_day().minutes() )
          ) {
    m_ofs.close();
    m_dtLast = dt;
    m_ssLogNameDateTime.str( "" );
    m_ssLogNameDateTime << dt;
    std::string sName( m_sFileNamePrefix );
    sName += "-";
    sName += sServerName;
    sName += "-";
    sName += m_ssLogNameDateTime.str();
    sName += ".log";
    m_ofs.open( sName.c_str() );
  }
}
  
void K1WebQueryLog::Log( pQueryData_t& p ) {
  boost::lock_guard<boost::mutex> lock(m_mutexQDToLog);
  assert( m_bAllowThread );
  assert( 0 == p->pNext.get() );
  if ( 0 == m_pQDToLogFirst.get() ) {
    m_pQDToLogFirst = p;
    m_pQDToLogLast = p;
  }
  else {
    m_pQDToLogLast->pNext = p;
    m_pQDToLogLast = p;
  }
  
  m_cvWriteLog.notify_one();
}

void K1WebQueryLog::Thread(void) {
  //m_bThreadRunning = true;
  while ( m_bAllowThread ) {
    boost::unique_lock<boost::mutex> lockWait(m_mutexWait);
    m_cvWriteLog.wait( lockWait );
    pQueryData_t p;
    while ( 0 != m_pQDToLogFirst.get() ) {
      {
        boost::lock_guard<boost::mutex> lock( m_mutexQDToLog );
        p = m_pQDToLogFirst;
        m_pQDToLogFirst = p->pNext;
        if ( 0 == m_pQDToLogFirst.get() ) {
          m_pQDToLogLast.reset();
        }
      }
      
      QueryData& qd( *p.get() );
      
      File( qd.dt, qd.sServerName );

      m_ssLogEntryDateTime.str( "" );
      m_ssLogEntryDateTime << qd.dt;
      
      m_ofs
        << "t=" << m_ssLogEntryDateTime.str() << ";"
        << "p=" << qd.p << ";"
        << "s=" << qd.s << ";"
        << "v=" << qd.v << ";"
        << "d=" << qd.d << ";"
        << std::endl;
      PutContainer( p );
    }
  }
}
// todo:
 // change once a minute
 // need a configuration parameter for location, use process id in name

