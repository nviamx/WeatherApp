#include "DownloadThread.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "nlohmann/json.hpp"

void DownloadThread::operator()(CommonObjects& common)
{
	httplib::Client cli("https://dummyjson.com");

	auto res = cli.Get("/recipes?limit=10&skip=10&select=name,image");
	if (res->status == 200)
	{
		auto json_result = nlohmann::json::parse(res->body);

	}
}

void DownloadThread::SetUrl(std::string_view new_url)
{
	_download_url = new_url;
}
