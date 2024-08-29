#include "DownloadThread.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>

// Function to find the value associated with a key in a JSON string
std::string extractValue(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\":";
    size_t startPos = json.find(searchKey);
    if (startPos == std::string::npos) {
        return "";
    }

    startPos += searchKey.length();
    while (startPos < json.length() && (json[startPos] == ' ' || json[startPos] == '"')) {
        ++startPos;
    }

    size_t endPos = json.find_first_of(",}", startPos);
    std::string value = json.substr(startPos, endPos - startPos);

    // Remove any surrounding quotes
    if (!value.empty() && value.front() == '"') {
        value.erase(0, 1);
    }
    if (!value.empty() && value.back() == '"') {
        value.pop_back();
    }

    return value;
}

// Function to manually parse the JSON response
void parseWeatherData(CommonObjects& common, const std::string& json) {
    WeatherData wd;

    try {
        auto jsonData = nlohmann::json::parse(json);

        // Access JSON fields safely
        if (jsonData.contains("name")) {
            wd.name = jsonData["name"].get<std::string>();
        }
        if (jsonData.contains("sys") && jsonData["sys"].contains("country")) {
            wd.country = jsonData["sys"]["country"].get<std::string>();
        }
        if (jsonData.contains("base")) {
            wd.base = jsonData["base"].get<std::string>();
        }
        if (jsonData.contains("main") && jsonData["main"].contains("temp")) {
            wd.temp = static_cast<int>(jsonData["main"]["temp"].get<double>() - 273.15);

        }
        if (jsonData.contains("weather") && !jsonData["weather"].empty()) {
            wd.main = jsonData["weather"][0]["description"].get<std::string>();
        }

        common.WeatherData.push_back(wd);
    }
    catch (const nlohmann::json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }
}

void DownloadThread::operator()(CommonObjects& common)
{
    while (!common.exit_flag) {
        if (common.start_download) {
            std::string cityName;
            {
                std::lock_guard<std::mutex> lock(common.mtx);
                cityName = common.cityName;
            }

            if (cityName.empty()) {
                std::cerr << "City name not set!" << std::endl;
                common.start_download = false;
                continue;
            }

            // Construct API request using the city's name
            std::string api_key = "cf0ca4e5cc709128b297fcbd183eb72a"; // API key
            std::string api_url = "/data/2.5/weather?q=" + cityName + "&appid=" + api_key;

            httplib::Client cli("https://api.openweathermap.org");
            auto res = cli.Get(api_url.c_str());

                        // Initializes an HTTP client for the OpenWeatherMap API.
                      // Sends an HTTP GET request to fetch weather data.
          
            if (res && res->status == 200) {//200-400 success
                std::string jsonResponse = res->body;
                parseWeatherData(common, jsonResponse);
                std::cout << "Data fetched!" << std::endl;
                common.data_ready = true;
                // Checks if the response is successful (HTTP status 200).
//            // If successful, retrieves the JSON data from the response,
//            // parses it to extract weather information, updates the shared data object
//            // and marks the data as ready for use.
            }
            else {
                std::cerr << "Failed to fetch weather data!" << std::endl;
            }

            common.start_download = false;  // Reset the flag
        }
    }
}



