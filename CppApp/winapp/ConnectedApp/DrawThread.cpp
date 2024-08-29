#include "DrawThread.h"
#include "GuiMain.h"
#include "../../shared/ImGuiSrc/imgui.h"
#include <iostream>
#include <mutex>
#include <fstream>
#include <unordered_map>

#define IMGUI_ENABLE_FREETYPE

// Use an unordered_map to store favorites and their associated temperatures
std::unordered_map<std::string, double> favorites;

    
// Function to save favorite cities to data.txt
void saveFavorites() {
    std::ofstream file("data.txt");
    if (!file.is_open()) {
        std::cerr << "Could not open the data.txt file for writing!" << std::endl;
        return;
    }
    for (const auto& [city, _] : favorites) {
        file << city << "\n";
    }
    file.close();
}

// Function to load favorite cities from data.txt
void loadFavorites() {
    std::ifstream file("data.txt");
    if (!file.is_open()) {
        std::cerr << "Could not open the data.txt file for reading!" << std::endl;
        return;
    }
    std::string city;
    while (std::getline(file, city)) {
        favorites[city] = 0.0; // Initialize with a dummy temperature
    }
    file.close();
}

// Function to fetch and update weather data for favorite cities
void fetchFavoritesWeather(CommonObjects& common) {
    for (const auto& [city, _] : favorites) {
        {
            std::lock_guard<std::mutex> lock(common.mtx);
            common.cityName = city;
            common.start_download = true;
        }
        // Wait for download to finish (this is a simple approach, may need adjustment for actual use)
        while (!common.data_ready) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        // Update favorite with the fetched temperature
        if (!common.WeatherData.empty()) {
            favorites[city] = common.WeatherData.back().temp; // Assuming the latest entry is the relevant one
        }
        common.data_ready = false; // Reset for next city
    }
}

// GUI Drawing Function

// GUI Drawing Function
void DrawAppWindow(void* common_ptr)
{
    auto common = (CommonObjects*)common_ptr;

    // Set the modern, contrasty color palette
    ImVec4 backgroundColor = ImVec4(0.98f, 0.98f, 0.5f, 1.0f); // More yellow, still pastel
    ImVec4 textColor = ImVec4(0.05f, 0.05f, 0.1f, 1.0f);       // Darker navy blue
    ImVec4 favoriteTextColor = ImVec4(1.0f, 0.6f, 0.3f, 1.0f); // Warm orange
    ImVec4 weatherDataTextColor = ImVec4(1.0f, 0.4f, 0.6f, 1.0f); // Vibrant pink
    ImVec4 buttonColor = ImVec4(1.0f, 0.4f, 0.6f, 1.0f);       // Vibrant pink
    ImVec4 checkboxColor = ImVec4(1.0f, 0.6f, 0.3f, 1.0f);     // Warm orange

    ImGui::Begin("Weather App", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

    // Set background color
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = backgroundColor;

    // Input text field
    static char buff[50];
    ImGui::InputText("City Name", buff, sizeof(buff));
    ImGui::SameLine();

    // Set button color
    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.5f, 0.9f, 1.0f)); // Lighter purple
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.3f, 0.7f, 1.0f));  // Darker purple
    if (ImGui::Button("Search")) {
        {
            std::lock_guard<std::mutex> lock(common->mtx);   // Prevent race conditions
            common->cityName = buff;
            common->start_download = true;  // Trigger download
        }
        std::cout << "Search triggered for: " << buff << std::endl;
    }
    ImGui::PopStyleColor(3); // Restore previous button styles

    // Display favorite cities
    if (!favorites.empty()) {
        ImGui::Separator();
        ImGui::TextColored(favoriteTextColor, "Favorite Cities:");
        for (const auto& [city, temp] : favorites) {
            ImGui::TextColored(favoriteTextColor, "%s: %.0f C", city.c_str(), temp);
        }
    }
    else {
        ImGui::Separator();
        ImGui::TextColored(textColor, "No favorite cities.");
    }

    // Display weather data if available
    if (common->data_ready) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTable("WeatherData", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Country");
            ImGui::TableSetupColumn("Temperature (Celsius)");
            ImGui::TableSetupColumn("Description");
            ImGui::TableSetupColumn("Favorite");
            ImGui::TableHeadersRow();

            for (const auto& wd : common->WeatherData) {
                ImGui::TableNextRow();

                // Display city name
                ImGui::TableSetColumnIndex(0);
                ImGui::TextColored(weatherDataTextColor, wd.name.c_str());

                // Display country
                ImGui::TableSetColumnIndex(1);
                ImGui::TextColored(weatherDataTextColor, wd.country.c_str());

                // Set temperature color based on the value
                ImVec4 tempColor;
                if (wd.temp > 30.0) {
                    tempColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
                }
                else if (wd.temp >= 20.0 && wd.temp <= 30.0) {
                    tempColor = ImVec4(1.0f, 0.5f, 0.0f, 1.0f); // Orange
                }
                else {
                    tempColor = ImVec4(0.0f, 0.0f, 1.0f, 1.0f); // Blue
                }

                // Display temperature with the appropriate color
                ImGui::TableSetColumnIndex(2);
                ImGui::TextColored(tempColor, "%.0f", wd.temp);

                // Display description
                ImGui::TableSetColumnIndex(3);
                ImGui::TextColored(weatherDataTextColor, wd.main.c_str());

                // Checkbox for favorite with warm orange color
                ImGui::TableSetColumnIndex(4);
                ImGui::PushStyleColor(ImGuiCol_CheckMark, checkboxColor);
                bool is_favorite = favorites.find(wd.name) != favorites.end();
                if (ImGui::Checkbox(("##favorite_" + wd.name).c_str(), &is_favorite)) {
                    if (is_favorite) {
                        // Add to favorites with temperature
                        favorites[wd.name] = wd.temp;
                    }
                    else {
                        // Remove from favorites
                        favorites.erase(wd.name);
                    }
                    saveFavorites(); // Save the updated favorites to file
                }
                ImGui::PopStyleColor(); // Restore previous checkbox styles
            }
            ImGui::EndTable();
        }
    }

    ImGui::End();
}




// DrawThread Operator
void DrawThread::operator()(CommonObjects& common)
{
    // Load favorites from file
    loadFavorites();

    // Fetch weather data for all favorite cities
    fetchFavoritesWeather(common);

    GuiMain(DrawAppWindow, &common);
    common.exit_flag = true;
}
