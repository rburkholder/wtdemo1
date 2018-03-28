/* 
 * File:   StateStatus.h
 * Author: Raymond Burkholder
 *         rburkholder@quovadis.bm
 *
 * Created on August 1, 2015, 8:59 AM
 */

#pragma once

#include <boost/atomic.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

struct StateStatus {
public:
  
  boost::atomic<int> m_cntContainersCreated;
  boost::atomic<int> m_cntContainersActive;
  
  StateStatus():
    m_cntContainersCreated( 0 ), 
    m_cntContainersActive( 0 ) 
  {}
  virtual ~StateStatus() {};
  
  template<typename Stream>
  void Out( Stream& stream ) {
    boost::property_tree::ptree tree;
    tree.put( "CntContainersCreated", m_cntContainersCreated );
    tree.put( "CntContainersActive", m_cntContainersActive );
    boost::property_tree::json_parser::write_json( stream, tree );
  }
  
private:

};

