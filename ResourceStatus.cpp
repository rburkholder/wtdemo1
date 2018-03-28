/* 
 * File:   ResourceStatus.cpp
 * Author: Raymond Burkholder
 *         rburkholder@quovadis.bm
 * 
 * Created on August 1, 2015, 8:33 AM
 */

#include <Wt/Http/Response>

#include "ResourceStatus.h"

ResourceStatus::ResourceStatus(StateStatus& status )
  : Wt::WResource( ),
    m_status( status )
{
  //suggestFileName( "status.json" );
}

ResourceStatus::~ResourceStatus() {
  beingDeleted();  // required by wt infrastructure
}

void ResourceStatus::handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response) {
  
  response.addHeader( "Pragma", "no-cache" );
  response.addHeader( "Content-Disposition", "attachment; filename=\"status.json\"" );
  
  response.setMimeType( "plain/text" );
  
  if ( "127.0.0.1" == request.clientAddress() ) {
    m_status.Out( response.out() );
  }

  //response.addHeader( "Connection", "Keep-Alive" );

  //response.setContentLength( m_gifDecoded.size() );
  //response.setMimeType( "image/gif" );
  //for ( std::vector<Util::BYTE>::const_iterator iter = m_gifDecoded.begin(); m_gifDecoded.end() != iter; ++iter ) {
  //  response.bout().put( *iter );
  //}

}