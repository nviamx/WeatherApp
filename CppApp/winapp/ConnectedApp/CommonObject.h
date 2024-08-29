#pragma once
#include <atomic>
#include <string>
#include <vector>
#include <mutex>
struct WeatherData {
	std::string name;
	std::string country;
	std::string base;
	double temp; // Ensure this is a double
	std::string main;
};


struct CommonObjects
{
	std::atomic_bool exit_flag = false;
	std::atomic_bool start_download = false;
	std::atomic_bool data_ready = false;
	/*represent a boolean value that can be safely accessed
	and modified by multiple threads simultaneously.
	It is part of the <atomic> header, 
	which provides various atomic types for safe concurrent programming.*/


	std::string cityName;
	std::vector<WeatherData> WeatherData;
	std::mutex mtx; // Mutex for thread safety
};
