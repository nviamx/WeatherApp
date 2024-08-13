#pragma once
#include <atomic>
#include <string>

struct CommonObjects
{
	std::atomic_bool exit_flag = false;
	std::atomic_bool start_download = false;
	std::string url;
};
