#pragma once
#include <Windows.h>
#include <iostream>
#include <sstream>

#define DEBUG(s) { std::wostringstream os_; os_<< __FILE__ << "(" << __LINE__ << "): - "; os_ << "DEBUG::" << s << std::endl; OutputDebugStringW(os_.str().c_str());}
