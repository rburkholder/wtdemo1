/* 
 * File:   ResourceStatus.h
 * Author: Raymond Burkholder
 *         rburkholder@quovadis.bm
 *
 * Created on August 1, 2015, 8:33 AM
 */

#pragma once

#include <Wt/WResource>

#include "StateStatus.h"

class ResourceStatus: public Wt::WResource {
public:
  ResourceStatus( StateStatus& );
  virtual ~ResourceStatus();
protected:
  virtual void handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response);
private:
  StateStatus& m_status;
};



