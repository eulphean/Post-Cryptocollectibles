#include "ofApp.h"


// This is the "main" function. It is the entry point to the program. Every
// computer program has a starting point, and this is it!
int main()
{
    ofSetupOpenGL(1280, 720, OF_FULLSCREEN);
    return ofRunApp(std::make_shared<ofApp>());
}
