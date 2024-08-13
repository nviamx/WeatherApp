#include "data.h"
#include <iostream>
#include <unordered_map>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>
#include <mutex>
#include "httplib.h"
#include <nlohmann/json.hpp>

std::mutex mtx;

std::unordered_map<std::string, std::pair<std::string, std::string>> loadCityCoordinates(const std::string& filename) {
    std::unordered_map<std::string, std::pair<std::string, std::string>> cityCoords;
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string city, lat, lon;

        std::getline(ss, city, ',');
        std::getline(ss, lat, ',');
        std::getline(ss, lon, ',');

        cityCoords[city] = { lat, lon };
    }

    return cityCoords;
}

std::pair<std::string, std::string> getCoordinates(const std::string& cityName, const std::unordered_map<std::string, std::pair<std::string, std::string>>& cityCoords) {
    auto it = cityCoords.find(cityName);
    if (it != cityCoords.end()) {
        return it->second;
    }
    throw std::runtime_error("City not found");
}

std::string getWeather(const std::string& lat, const std::string& lon, const std::string& apiKey) {
    httplib::Client cli("http://api.openweathermap.org");
    std::string url = "/data/2.5/weather?lat=" + lat + "&lon=" + lon + "&appid=" + apiKey;

    auto res = cli.Get(url.c_str());

    if (res && res->status == 200) {
        return res->body;
    }
    else {
        throw std::runtime_error("Request failed with status: " + std::to_string(res ? res->status : 0));
    }
}

void searchCity(const std::string& cityName, const std::unordered_map<std::string, std::pair<std::string, std::string>>& cityCoords, std::vector<WeatherData>& history, const std::string& apiKey) {
    std::string lat, lon;

    // Use a thread to safely retrieve the coordinates
    std::thread coordThread([&]() {
        auto coords = getCoordinates(cityName, cityCoords);
        std::unique_lock<std::mutex> lock(mtx);
        lat = coords.first;
        lon = coords.second;
        });
    coordThread.join();

    // Fetch weather data
    std::string weatherData = getWeather(lat, lon, apiKey);

    // Parse JSON and update WeatherData struct
    auto json = nlohmann::json::parse(weatherData);
    float temp = json["main"]["temp"];
    float maxTemp = json["main"]["temp_max"];
    float minTemp = json["main"]["temp_min"];
    float hum = json["main"]["humidity"];
    float windSpd = json["wind"]["speed"];
    float pres = json["main"]["pressure"];
    std::string desc = json["weather"][0]["description"];

    // Create a WeatherData object and store it in history
    WeatherData weather(cityName, lat, lon);
    weather.setWeatherData(temp, maxTemp, minTemp, hum, windSpd, pres, desc);

    std::unique_lock<std::mutex> lock(mtx);
    history.push_back(weather);
}

int main() {
    std::vector<WeatherData> history;
    std::vector<WeatherData> favorites;
    std::unordered_map<std::string, std::pair<std::string, std::string>> cityCoords;
    std::string apiKey = "cf0ca4e5cc709128b297fcbd183eb72a";
    std::string citiesFile = "cities.txt";

    try {
        cityCoords = loadCityCoordinates(citiesFile);
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading city coordinates: " << e.what() << std::endl;
        return -1;
    }

    bool running = true;
    while (running) {
        std::string cityName;
        std::cout << "Enter city name (or 'exit' to quit): ";
        std::getline(std::cin, cityName);

        if (cityName == "exit") {
            running = false;
            continue;
        }

        try {
            searchCity(cityName, cityCoords, history, apiKey);

            // Display weather data for the searched city
            history.back().printData();

            std::string favChoice;
            std::cout << "Mark this city as favorite? (y/n): ";
            std::cin >> favChoice;
            std::cin.ignore();  // Clear the newline from input buffer

            if (favChoice == "y") {
                std::unique_lock<std::mutex> lock(mtx);
                history.back().markAsFavorite();
                favorites.push_back(history.back());
            }

            // Display favorites
            std::cout << "\nFavorite Cities:\n";
            for (const auto& favCity : favorites) {
                favCity.printData();
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        std::cout << "----------------------\n";
    }

    return 0;
}
