#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib> // For system()
using namespace std;

// Function to play MP3 files from a playlist
void playSongsFromFile(const std::string &filePath) {
    std::ifstream playlistFile(filePath);

    // Check if the file can be opened
    if (!playlistFile.is_open()) {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return;
    }

    std::string songPath;
    bool songPlayed = false;

    // Read each line from the file
    while (std::getline(playlistFile, songPath)) {
        if (songPath.empty()) {
            continue; // Skip empty lines
        }
        
        std::cout << "Found file: " << songPath << std::endl;

        // Validate the file type
        if (songPath.substr(songPath.find_last_of('.') + 1) != "mp3") {
            std::cerr << "Skipping non-MP3 file: " << songPath << std::endl;
            continue;
        }

        // Play the song
        std::cout << "Now Playing: " << songPath << std::endl;
        std::string command = "mpg123 \"" + songPath + "\"";
        int result = system(command.c_str());

        std::cout << "System return value: " << result << std::endl;
        if (result != 0) {
            std::cerr << "Error: Failed to play " << songPath << std::endl;
        } else {
            songPlayed = true;
        }
    }

    // Close the playlist file
    playlistFile.close();

    if (!songPlayed) {
        std::cerr << "No valid songs were played. Please check your playlist." << std::endl;
    }
}

int main() {
    std::string filePath;
    std::cout << "Enter the path to your playlist file (e.g., playlist.txt): ";
    std::cin >> filePath;

    // Play songs from the specified playlist
    playSongsFromFile(filePath);

    std::cout << "Program ended. Thank you for using the music player!" << std::endl;

    return 0;
}
// ../home/aman/Desktop/songs
