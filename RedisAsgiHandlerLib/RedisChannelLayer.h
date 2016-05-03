#pragma once

#include <functional>
#include <iostream>
#include <cvt/wstring>
#include <codecvt>
#include <random>

#include <ppltasks.h>

#include <hiredis.h>


typedef std::unique_ptr<redisReply, std::function<void(void*)>> RedisReply;

class RedisChannelLayer
{
public:

    RedisChannelLayer(std::string ip = "127.0.0.1", int port = 6379, std::string prefix = "asgi:");
    ~RedisChannelLayer();

    std::string NewChannel(std::string prefix) const;

    template<typename T>
    concurrency::task<void> Send(const std::string& channel, T& msg);

    std::tuple<std::string, std::string> ReceiveMany(const std::vector<std::string>& channels, bool blocking = false);

private:

    std::string GenerateUuid();

    template<typename... Args>
    RedisReply ExecuteRedisCommand(std::string format_string, Args... args)
    {
        auto reply = static_cast<redisReply*>(redisCommand(m_redis_ctx, format_string.c_str(), args...));
        RedisReply wrapped_reply(reply, freeReplyObject);
        return wrapped_reply;
    }

    std::wstring_convert<std::codecvt_utf8<wchar_t>> m_utf8_conv;
    std::string m_prefix;
    int m_expiry; // seconds
    redisContext *m_redis_ctx;
    std::default_random_engine m_random_engine;
};

template<typename T>
inline concurrency::task<void> RedisChannelLayer::Send(const std::string& channel, T& msg)
{
    return concurrency::create_task([this, channel, &msg]() {
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, msg);

        // asgi_redis chooses to store the data in a random key, then add the key to
        // the channel. (This allows us to set the data to expire, which we do).
        std::string data_key = m_prefix + GenerateUuid();
        ExecuteRedisCommand("SET %s %b", data_key.c_str(), buffer.data(), buffer.size());
        ExecuteRedisCommand("EXPIRE %s %i", data_key.c_str(), m_expiry);

        // We also set expiry on the channel. (Subsequent Send()s will bump this expiry
        // up). We set to +10, because asgi_redis does... presumably to workaround the
        // fact that time has passed since we put the data into the data_key?
        std::string channel_key = m_prefix + channel;
        ExecuteRedisCommand("RPUSH %s %b", channel_key.c_str(), data_key.c_str(), data_key.length());
        ExecuteRedisCommand("EXPIRE %s %i", channel_key.c_str(), m_expiry + 10);
    });
}
