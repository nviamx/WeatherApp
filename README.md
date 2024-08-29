# Weather App Documentation

## Overview
This weather app is a multithreaded application that fetches weather data for cities and displays it using a graphical user interface (GUI). The application consists of several components:

- **CommonObjects**: A shared data structure used by different threads.
- **DrawThread**: Manages GUI rendering and user interaction.
- **DownloadThread**: Fetches weather data from an API.
- **ConnectedApp**: Main application entry point that initializes and runs the threads.

## Components

### 1. CommonObjects.h
**Header File**: `CommonObjects.h`

This file defines the `WeatherData` structure and the `CommonObjects` structure used to share data between threads.

- **WeatherData**: Represents weather information for a city.
  - `std::string name`: City name.
  - `std::string country`: Country of the city.
  - `std::string base`: Base information (not used in the current implementation).
  - `double temp`: Temperature in Celsius.
  - `std::string main`: Weather description.

- **CommonObjects**: Contains shared data and synchronization primitives.
  - `std::atomic_bool exit_flag`: Indicates when the application should exit.
  - `std::atomic_bool start_download`: Signals that a download should start.
  - `std::atomic_bool data_ready`: Indicates that data has been fetched and is ready.
  - `std::string cityName`: Stores the name of the city for which weather data is being fetched.
  - `std::vector<WeatherData> WeatherData`: Holds the weather data for display.
  - `std::mutex mtx`: Mutex used to synchronize access to shared data.

### 2. DrawThread.h and DrawThread.cpp
**Header File**: `DrawThread.h`

Defines the `DrawThread` class which handles GUI drawing.

- **DrawThread**:
  - **Public Method**: `void operator()(CommonObjects& common)`: Invoked to start the GUI rendering process.

**Source File**: `DrawThread.cpp`

Implements the GUI drawing logic using the ImGui library.

- **GUI Drawing Function (DrawAppWindow)**:
  - Sets the color palette and styles for the ImGui window.
  - Provides an input field for entering city names and a button to trigger weather data download.
  - Displays favorite cities and their temperatures.
  - Shows weather data in a table if available.
  - Allows users to mark cities as favorites and save them to a file.

- **Operator Method**:
  - Loads favorite cities from `data.txt`.
  - Fetches weather data for all favorite cities using `fetchFavoritesWeather`.
  - Calls `GuiMain` with `DrawAppWindow` to render the GUI.
  - Sets the `exit_flag` to true to signal application exit.

### 3. DownloadThread.h and DownloadThread.cpp
**Header File**: `DownloadThread.h`

Defines the `DownloadThread` class which handles data downloading.

- **DownloadThread**:
  - **Public Method**: `void operator()(CommonObjects& common)`: Invoked to start the data download process.
  - **Private Member**: `std::string _download_url`: Stores the URL for downloading weather data (not used in the current implementation).

**Source File**: `DownloadThread.cpp`

Implements the logic for fetching weather data from the OpenWeatherMap API.

- **Download Logic**:
  - Uses the httplib library to send HTTP GET requests to fetch weather data.
  - Parses the JSON response using `nlohmann::json` to extract weather information.
  - Updates the `CommonObjects` structure with the fetched weather data.

### 4. ConnectedApp.cpp
**Source File**: `ConnectedApp.cpp`

The main entry point of the application.

- **Main Function**:
  - Creates a `CommonObjects` instance to share data between threads.
  - Initializes `DrawThread` and `DownloadThread` objects.
  - Launches `DrawThread` and `DownloadThread` in separate threads.
  - Waits for both threads to finish by calling `join` on them.

## How It Works

### Initialization
- The `ConnectedApp` initializes the `CommonObjects` and creates instances of `DrawThread` and `DownloadThread`.
- Threads are launched to handle GUI drawing and data downloading concurrently.

### Data Download
- The `DownloadThread` periodically checks if `start_download` is set.
- It fetches weather data from the OpenWeatherMap API based on the city name provided in `CommonObjects`.
- The fetched data is parsed and stored in `CommonObjects`.

### GUI Rendering
- The `DrawThread` continuously renders the GUI using ImGui.
- It displays the current weather data and allows users to search for new cities and mark favorites.
- The GUI updates based on the data provided by `DownloadThread`.

### Synchronization
- `std::mutex` is used to synchronize access to shared data between the threads.
- `std::atomic` variables ensure safe concurrent modifications of flags like `exit_flag`, `start_download`, and `data_ready`.

### Termination
- The application runs until the `exit_flag` is set to true.
- Once set, the threads complete their execution and the main function ends, terminating the program.

## Purpose of the Threads

### DownloadThread
- **Function**: Fetches weather data from the API.
- **Role**: Handles network operations and updates the shared data structure with new weather information.

### DrawThread
- **Function**: Manages and renders the GUI using ImGui.
- **Role**: Provides an interface for the user to interact with, including displaying weather data and handling user inputs.

## Coordination Mechanism

The threads use the `CommonObjects` structure and synchronization primitives to coordinate their actions. Here's a breakdown of how they work together:

### Shared Data (CommonObjects)
- **cityName**: Holds the name of the city for which weather data is requested.
- **WeatherData**: Stores the fetched weather data.
- **start_download**: A flag that signals when the `DownloadThread` should start downloading data.
- **data_ready**: A flag that indicates when the data is ready to be displayed.
- **exit_flag**: A flag that signals when the application should exit.
- **mtx**: A mutex used to ensure thread-safe access to the shared data.

### How They Work Together

#### Starting the Download
- **Trigger**: When the user enters a city name and clicks the "Search" button in the GUI, the `DrawThread` sets the `start_download` flag to true and updates the `cityName`.
- **Action**: The `DownloadThread` continuously checks the `start_download` flag. When it detects that `start_download` is true, it starts the download process for the specified city.

#### Fetching Data
- **DownloadThread**: Makes an HTTP request to fetch weather data and updates the `WeatherData` in the `CommonObjects` structure.
- **Synchronization**: While the `DownloadThread` is fetching data, it uses the mutex (`mtx`) to safely access and modify shared data.

#### Notifying Data Readiness
- **Action**: After the `DownloadThread` completes fetching and parsing the weather data, it sets the `data_ready` flag to true to indicate that the data is available for display.

#### Updating the GUI
- **DrawThread**: Periodically checks the `data_ready` flag. When it detects that `data_ready` is true, it reads the updated `WeatherData` and renders the information in the GUI.
- **Synchronization**: The `DrawThread` uses the mutex to access and display the weather data safely.

#### Exiting the Application
- **Action**: When the `exit_flag` is set to true, both threads are notified to finish their operations and exit.

## Summary

- **Synchronization**: The `CommonObjects` structure and mutex ensure that both threads can safely access and modify shared data.
- **Communication**: The threads communicate via flags (`start_download`, `data_ready`) and shared data (`cityName`, `WeatherData`) to coordinate their actions.
- **Turn Management**: There is no explicit turn-based mechanism. Instead, the threads operate based on the state of shared flags and data, with the `DownloadThread` handling data fetching and the `DrawThread` managing GUI updates based on the availability of data.

This approach ensures that data fetching and GUI rendering are handled concurrently, providing a responsive user experience without blocking either operation.
