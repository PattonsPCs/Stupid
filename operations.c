#include <stdlib.h>
#include <time.h>
#include <windows.h>

// Path to your video file - change this to your video file path
#define VIDEO_FILE_PATH "C:\\Users\\antho\\Downloads\\Voicy_foxy jumpscare fnaf2.mp4"

// Check if we should play the video (1 in 100 chance)
// Returns 1 if video should play, 0 otherwise
int shouldPlayVideo(void) {
    static int initialized = 0;
    if (!initialized) {
        srand((unsigned int)time(NULL));
        initialized = 1;
    }
    
    int roll = rand() % 100;
    return (roll == 0);  // 1 in 100 chance (0-99, so 0 is 1/100)
}

// Play the video file using the default video player
void playVideo(void) {
    ShellExecuteA(NULL, "open", VIDEO_FILE_PATH, NULL, NULL, SW_SHOWNORMAL);
}
