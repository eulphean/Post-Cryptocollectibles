#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"
#include "SubsectionBody.h"
#include "ofxGui.h"
#include "ofxFilterLibrary.h"

class ofApp: public ofBaseApp
{
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;
    void exit() override;
  
    // Mesh helper methods.
    void createImageSubsections();
    void meshRadiusUpdated(float &radius); // Callback function for mesh radius update.
    void subsectionSizeUpdated(int &num); // Recreate image subsections with new width or height.
    void createSubsectionBody();
    void createSubsectionProperties();
    void populateFilters();
    void createCustomWalls();
  
    // Grabber stuff.
    ofVideoGrabber grabber;
  
    // Physics engine.
    vector<ofMesh> meshes;
    ofxBox2d box2d;
  
    //ofImage cryptoPunks;
  
    // Checks to keep track of updated box2d object.
    bool newSubsection;
    bool showSoftBody;
    bool hideGui;
    bool clear;
  
    // Subsections
    vector<Subsection> imageSubsections; // Pool of original subsections.
    vector<Subsection> tornSubsections; // Subsections removed. To be redrawn.
    std::map<int, vector<Subsection>> tornSubsectionsAtFilter;
    vector<SubsectionBody> softBodies; // Mesh + Box2D body removed.
    std::map<int, vector<SubsectionBody>> softBodiesAtFilter;
  
    // App GUI.
    ofxPanel gui;
    ofxFloatSlider meshVertexRadius;
    ofxIntSlider meshColumns;
    ofxIntSlider meshRows;
    ofxIntSlider subsectionWidth;
    ofxIntSlider subsectionHeight;
  
    // Physics.
    ofxFloatSlider vertexDensity;
    ofxFloatSlider vertexBounce;
    ofxFloatSlider vertexFriction;
    ofxFloatSlider jointFrequency;
    ofxFloatSlider jointDamping;
  
    // Soft body UI properties. 
    SoftBodyProperties softBodyProperties;
  
    // Collection of filters. 
    vector<AbstractFilter *> filters;
  
    unsigned long trackTime;
  
    // Collection of kitties.
    vector<ofImage> kitties;
};

