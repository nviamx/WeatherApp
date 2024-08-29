#include <iostream>
#include <thread>
#include "CommonObject.h"
#include "DrawThread.h"
#include "DownloadThread.h"

int main()
{
    CommonObjects common; // object of type common objects
  
    DrawThread draw;
    auto draw_th = std::jthread([&] {draw(common); });

    DownloadThread down;
    auto down_th = std::jthread([&] {down(common); });
    /*DrawThread: Manages GUI rendering and user interaction with ImGui, handles displaying weather data and favorites.
    DownloadThread: Fetches weather data from an external API, processes the data, and updates the shared state.
    Both threads interact through shared CommonObjects, using mutexes to synchronize access and ensure data consistency.*/

    std::cout << "Welcome to the Weather App \n";
    draw_th.join();
    down_th.join();

    return 0;
}
