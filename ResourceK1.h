/* 
 * File:   SessionK1.h
 * Author: Raymond Burkholder
 *         rburkholder@quovadis.bm
 *
 * Created on July 20, 2015, 2:49 PM
 */

#pragma once

// https://gist.github.com/jhasse/990731
#include <boost/uuid/sha1.hpp>

#include <Wt/WResource>

#include "UtilBase64.h"
#include "StateStatus.h"
#include "K1WebQueryLog.h"

// note: needs to be re-entrant, typically statically defined

class ResourceK1: public Wt::WResource {
public:
  ResourceK1( StateStatus&, K1WebQueryLog& );
  virtual ~ResourceK1();
protected:
  virtual void handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response);
private:
  StateStatus& m_status;
  K1WebQueryLog& m_logK1;
  boost::uuids::detail::sha1 m_sha1; // pre-seeded, then remains static and is copied for each use
  std::vector<Util::BYTE> m_gifDecoded;    // assigned once and re-used re-entrantly
};
