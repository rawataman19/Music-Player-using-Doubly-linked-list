#include <iostream>
#include <string>
#include <filesystem>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <fcntl.h>

using namespace std;
namespace fs = filesystem;

struct SongNode {
    string songPath;
    SongNode* next;
    SongNode* prev;
    
    SongNode(const string& path) : songPath(path), next(nullptr), prev(nullptr) {}
};

class DoublyLinkedList {
private:
    SongNode* head;
    SongNode* tail;
    SongNode* current;
    int size;
    pid_t playerPID;
    bool isPaused;

   

    
public:
    DoublyLinkedList() : head(nullptr), tail(nullptr), current(nullptr), size(0), playerPID(-1), isPaused(false) {}
    
    ~DoublyLinkedList() {
        killCurrentPlayer();
        clear();
        
    }
    
    void addSong(const string& songPath) {
        SongNode* newNode = new SongNode(songPath);
        
        if (!head) {
            head = tail = current = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
        size++;
    }
    
    void killCurrentPlayer() {
        if (playerPID > 0) {
            kill(playerPID, SIGTERM);
            waitpid(playerPID, nullptr, 0);
            playerPID = -1;
        }
    }
    
    void loadFromDirectory(const string& dirPath) {
        clear();
        cout << "\nLoading songs from: " << dirPath << endl;
        
        try {
            for (const auto& entry : fs::directory_iterator(dirPath)) {
                if (entry.path().extension() == ".mp3") {
                    addSong(entry.path().string());
                    cout << "Added: " << entry.path().filename() << endl;
                }
            }
        } catch (const fs::filesystem_error& e) {
            cerr << "Error reading directory: " << e.what() << endl;
        }
        
        current = head;  // Set current to first song
        cout << "\nTotal songs loaded: " << size << endl;
    }
    
    void displayPlaylist() {
        if (!head) {
            cout << "Playlist is empty!" << endl;
            return;
        }
        
        cout << "\nPlaylist (Current song marked with ▶):" << endl;
        cout << "------------------------------------" << endl;
        
        SongNode* temp = head;
        int index = 1;
        while (temp) {
            cout << (temp == current ? "▶ " : "  ");
            cout << index++ << ". " << fs::path(temp->songPath).filename() << endl;
            temp = temp->next;
        }
    }
    
    void playCurrent() {
        if (!current) {
            cout << "No song to play!" << endl;
            return;
        }
        
        killCurrentPlayer();
        
        playerPID = fork();
        if (playerPID == 0) {
            execlp("mpg123", "mpg123", "-q", current->songPath.c_str(), nullptr);
            exit(1);
        }
        
        cout << "\nNow playing: " << fs::path(current->songPath).filename() << endl;
        isPaused = false;
    }
    
    void playNext() {
        if (current && current->next) {
            current = current->next;
            playCurrent();
        } else if (current) {
            // Wrap around to the beginning
            current = head;
            playCurrent();
        }
    }
    
    void playPrevious() {
        if (current && current->prev) {
            current = current->prev;
            playCurrent();
        } else if (current) {
            // Wrap around to the end
            current = tail;
            playCurrent();
        }
    }
    
    void togglePause() {
        if (playerPID > 0) {
            if (!isPaused) {
                kill(playerPID, SIGSTOP);
                cout << "\nPaused playback" << endl;
                isPaused = true;
            } else {
                kill(playerPID, SIGCONT);
                cout << "\nResumed playback" << endl;
                isPaused = false;
            }
        }
    }
    
    void clear() {
        killCurrentPlayer();
        
        SongNode* current = head;
        while (current) {
            SongNode* temp = current;
            current = current->next;
            delete temp;
        }
        
        head = tail = this->current = nullptr;
        size = 0;
    }
    
    void startPlayback() {
        if (!head) {
            cout << "No songs in playlist!" << endl;
            return;
        }
        
        current = head;
        playCurrent();
        
        cout << "\nPlayback Controls:" << endl;
        cout << "SPACE - Pause/Resume" << endl;
        cout << "n     - Next song" << endl;
        cout << "p     - Previous song" << endl;
        cout << "l     - List songs" << endl;
        cout << "q     - Return to menu" << endl;
        
        while (true) {
            char input;
            if (read(STDIN_FILENO, &input, 1) > 0) {
                switch (input) {
                    case ' ':
                        togglePause();
                        break;
                    case 'n':
                        playNext();
                        break;
                    case 'p':
                        playPrevious();
                        break;
                    case 'l':
                        displayPlaylist();
                        break;
                    case 'q':
                        killCurrentPlayer();
                        return;
                }
            }
            
            // Check if current song has finished
            if (playerPID > 0) {
                int status;
                pid_t result = waitpid(playerPID, &status, WNOHANG);
                if (result > 0) {
                    // Song finished, play next
                    playNext();
                }
            }
            
            usleep(100000); // Sleep for 100ms
        }
    }
    
    int getSize() const { return size; }
};

int main() {
    cout << "=== Doubly Linked List MP3 Player ===" << endl;
    
    if (system("which mpg123 > /dev/null 2>&1") != 0) {
        cerr << "Error: mpg123 is not installed! Please install it using:\n"
             << "sudo apt-get install mpg123" << endl;
        return 1;
    }
    
    DoublyLinkedList playlist;
    string dirPath;
    
    cout << "Enter the path to your songs directory: ";
    getline(cin, dirPath);
    playlist.loadFromDirectory(dirPath);
    
    while (true) {
        cout << "\nMenu:" << endl;
        cout << "1. Play songs" << endl;
        cout << "2. Display playlist" << endl;
        cout << "3. Reload songs" << endl;
        cout << "4. Exit" << endl;
        cout << "Enter choice (1-4): ";
        
        char choice;
        cin >> choice;
        cin.ignore();
        
        switch (choice) {
            case '1':
                playlist.startPlayback();
                break;
            case '2':
                playlist.displayPlaylist();
                break;
            case '3':
                cout << "Enter directory path: ";
                getline(cin, dirPath);
                playlist.loadFromDirectory(dirPath);
                break;
            case '4':
                cout << "Goodbye!" << endl;
                return 0;
            default:
                cout << "Invalid choice!" << endl;
        }
    }
    
    return 0;
}