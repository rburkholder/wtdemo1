/* 
 * File:   base64.h
 *
 * Created on July 21, 2015, 7:38 PM
 */

// courtesy of:
// http://stackoverflow.com/questions/180947/base64-decode-snippet-in-c

#pragma once

#include <vector>
#include <string>

namespace Util {

typedef unsigned char BYTE;

std::string base64_encode(BYTE const* buf, unsigned int bufLen);
std::vector<BYTE> base64_decode(std::string const&);

} // namespace util