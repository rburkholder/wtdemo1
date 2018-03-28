/* 
 * File:   SessionK2.cpp
 * Author: Raymond Burkholder
 *         rburkholder@quovadis.bm
 * 
 * Created on July 20, 2015, 2:49 PM
 */

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/gregorian/gregorian_io.hpp>

#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <Wt/Http/Response>

#include "ResourceK1.h"

ResourceK1::ResourceK1( StateStatus& status, K1WebQueryLog& logK1 )
: Wt::WResource( ),
  m_status( status ), m_logK1( logK1 )
{
  
  // default from wt example, not used
  static const unsigned char gifData[] 
    =  { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x01, 0x00, 0x01, 0x00,
         0x80, 0x00, 0x00, 0xdb, 0xdf, 0xef, 0x00, 0x00, 0x00, 0x21,
         0xf9, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00,
         0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x02, 0x02, 0x44,
         0x01, 0x00, 0x3b };
  //setData(gifData, 43);
  
  // oxbio data k1 supplied gif
  static const std::string sGifData( "R0lGODlhAQABAJAAAP8AAAAAACH5BAUQAAAALAAAAAABAAEAAAICBAEAOw==");
  m_gifDecoded = Util::base64_decode(sGifData);
  
  // http://www.boost.org/doc/libs/1_58_0/doc/html/boost_random/tutorial.html  
  boost::random::random_device rng;
  boost::random::uniform_int_distribution<> dist;
  
  // preseed sha1 with random bits for session id
  int cntBytes( 0 );
  while ( 128 > cntBytes ) {
    auto rand = dist(rng);
    m_sha1.process_bytes( &rand, sizeof( rand ) );
    cntBytes += sizeof( rand );
  }
  
}

ResourceK1::~ResourceK1() {
  beingDeleted();  // required by wt infrastructure
}

void ResourceK1::handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response) {
  
  union BYTES {
    unsigned int digest[5];  // as delivered by the sha1 library
    Util::BYTE byte; // as required by the base64 encode process
  };
  
  boost::posix_time::ptime dt( boost::posix_time::microsec_clock::universal_time() );
  
  // need to log or not, otherwise is lost
  K1WebQueryLog::pQueryData_t p;
  m_logK1.GetContainer( p );
  
  bool bAcceptable( true );
  
  const Wt::Http::ParameterMap& map( request.getParameterMap() );
  if ( 2 != map.size() ) {
    bAcceptable = false;
  }
  else {
    for ( Wt::Http::ParameterMap::const_iterator iterPM = map.begin(); map.end() != iterPM; ++iterPM ) {
      //std::cerr << iterPM->first << "=";
      const Wt::Http::ParameterValues& values( iterPM->second );
      if ( 1 != values.size() ) {
        bAcceptable = false;
      }
      else {
        if ( "d" == iterPM->first ) {
          p->d = iterPM->second[ 0 ];
        }
        else {
          if ( "v" == iterPM->first ) {
            p->v = iterPM->second[ 0 ];
          }
          else {
            bAcceptable = false;
          }
        }
      }
    }
  }
  
  if ( !bAcceptable ) {
    m_logK1.PutContainer( p );
    std::cerr << "Improper Request: " << request.queryString() << std::endl;
  }
  else {
    static const std::string sSessionId( "K1SessionId" );
    const std::string* psCookie = request.getCookieValue( sSessionId );
    if ( 0 == psCookie ) {
      boost::uuids::detail::sha1 sha1( m_sha1 );  // copy prepped sha1 digest
      sha1.process_bytes( &dt, sizeof( dt ) );
      sha1.process_bytes( request.clientAddress().c_str(), request.clientAddress().size() );
      BYTES bytes;
      sha1.get_digest( bytes.digest );
      p->s = Util::base64_encode( &bytes.byte, sizeof( bytes ) );
      //std::cerr << "new cookie: " << p->s << std::endl;
    }
    else {
      p->s = *psCookie;
      //std::cerr << "old cookie: " << p->s << std::endl;
    }
    response.addHeader( "Set-Cookie", sSessionId + "=" + p->s );
    response.addHeader( "Pragma", "no-cache" );
    
    //response.addHeader( "Connection", "Keep-Alive" );
    
    response.setContentLength( m_gifDecoded.size() );
    response.setMimeType( "image/gif" );
    for ( std::vector<Util::BYTE>::const_iterator iter = m_gifDecoded.begin(); m_gifDecoded.end() != iter; ++iter ) {
      response.bout().put( *iter );
    }
    
    p->dt = dt;
    
    p->p = request.clientAddress();

    p->sServerName = request.serverName() + "-" + request.serverPort();
    
    m_logK1.Log( p );  // logs and returns the container
  }
   
}
