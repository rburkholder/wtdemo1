/* 
 * File:   main.cpp
 * Author: Raymond Burkholder
 *         rburkholder@quovadis.bm
 *
 * Created on July 17, 2015, 4:54 PM
 */

// http://www.webtoolkit.eu/widgets

#include <string>

#include <Wt/WServer>
#include <Wt/WLogger>

//#include "ClientSession.h"
#include "StateStatus.h"
#include "K1WebQueryLog.h"
#include "ResourceStatus.h"
#include "ResourceK1.h"

// run the program with the following parameters:
// --docroot /var/www/wt --http-address 0.0.0.0 --http-port 8080  --threads 10

//  change /etc/wt/wt_config.xml from:
// <user-agents type="ajax" mode="black-list">
// to
// <user-agents type="ajax" mode="white-list">

// 2015/07/24 todo:
//   number in 'handleresponse'
//   queued log entries
//   time spent writing log
//   time spent writing response
//   expand size of block used for writing log

Wt::WApplication* CreateApplication( const Wt::WEnvironment& env ) {
//  return new ClientSession( env );
}

void StartWebApp( int argc, char** argv ) {
  
  try {
    Wt::WServer server(argv[0]);

    server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);
    //server.addEntryPoint(Wt::Application, CreateApplication);
    //server.addEntryPoint(Wt::Application, CreateApplication, "/k1");
    
    server.logger().configure( "* -info -debug" );
    //server.logger().configure( "-*" );
    
    StateStatus status;
    
    K1WebQueryLog logK1WebQuery( status, "/home/sysadmin/projects/obio/k1log" );
    
    // http://redmine.webtoolkit.eu/boards/2/topics/4358
    // http://www.webtoolkit.eu/widgets/media/resources   
    
    ResourceStatus resourceStatus( status );
    server.addResource( &resourceStatus, "/status" );
    
    ResourceK1 resourceK1( status, logK1WebQuery );
    server.addResource( &resourceK1, "/k1" );

    if (server.start()) {
      Wt::WServer::waitForShutdown();
      server.stop();
    }
  } 
  catch (Wt::WServer::Exception& e) {
    std::cerr << "exception (wt): " << e.what() << std::endl;
  } 
  catch (std::exception &e) {
    std::cerr << "exception (std): " << e.what() << std::endl;
  }
  
}

// http://www.webtoolkit.eu/wt/doc/reference/html/InstallationUnix.html
int main(int argc, char** argv) {

  StartWebApp( argc, argv );
  return 0;
}
