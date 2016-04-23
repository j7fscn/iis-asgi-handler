#include <iostream>

#include <msgpack.hpp>

#include "RedisChannelLayer.h"
#include "AsgiHttpRequestMsg.h"


struct RequestDict {
    std::string reply_channel;
    std::string http_version;
    std::string method;
    std::string scheme;
    std::string path;
    std::string query_string;
    std::string root_path;
    std::list<std::tuple<std::string, std::string>> headers;

    // body
    // body_channel

    MSGPACK_DEFINE_MAP(
        reply_channel, http_version, method, scheme,
        path, query_string, root_path, headers
    );
};


void main()
{
	RedisChannelLayer channels;

	struct RequestDict request;
	request.reply_channel = "blah";

	channels.Send("http.request", request);
	channels.Send("http.request", request);

    auto one = channels.ReceiveMany({ "asgi:http.request" });
    auto two = channels.ReceiveMany({ "asgi:http.request" });
	std::cout << "Received " << std::get<0>(one) << " " << std::get<1>(one).get() << std::endl;
	std::cout << "Received " << std::get<0>(two) << " " << std::get<1>(two).get() << std::endl;

    std::cin.get();
}