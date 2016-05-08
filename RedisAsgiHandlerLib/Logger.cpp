#include <string>

#define WIN32_LEAN_AND_MEAN
#include <httpserv.h>
#include <evntprov.h>

#include "Logger.h"


namespace {
    // {B057F98C-CB95-413D-AFAE-8ED010DB73C5}
    static const GUID EtwGuid = { 0xb057f98c, 0xcb95, 0x413d,{ 0xaf, 0xae, 0x8e, 0xd0, 0x10, 0xdb, 0x73, 0xc5 } };
} // end anonymous namespace


Logger::Logger()
{
    ::EventRegister(&EtwGuid, nullptr, nullptr, &m_etw_handle);
}


Logger::~Logger()
{
    ::EventUnregister(m_etw_handle);
}


void Logger::Log(const std::wstring& msg) const
{
    ::EventWriteString(m_etw_handle, 0, 0, msg.c_str());
}