#include "ofApp.h"

void ofApp::setup()
{
    ofBackground(0);
    ofDisableArbTex();
    ofEnableSmoothing();
    ofEnableAlphaBlending();
    ofSetVerticalSync(true);
    ofSetFrameRate(100);
  
    // Setup box 2d.
    box2d.init();
    box2d.setGravity(-10, -10);
    box2d.setFPS(200);
    box2d.enableEvents();
    box2d.registerGrabbing(); // Enable grabbing the circles.
  
    // Default values.
    newSubsection = false;
    showSoftBody = false;
    hideGui = false;
    clear = false;
  
    // Setup GUI.
    gui.setup();
    gui.add(meshVertexRadius.setup("Mesh vertex radius", 5, 1, 30));
    gui.add(subsectionWidth.setup("Subsection width", 50, 10, 500));
    gui.add(subsectionHeight.setup("Subsection height", 50, 10, 500));
    gui.add(meshColumns.setup("Mesh columns", 20, 1, 100));
    gui.add(meshRows.setup("Mesh rows", 20, 1, 100));
    gui.add(vertexDensity.setup("Vertex density", 0.5, 0, 1));
    gui.add(vertexBounce.setup("Vertex bounce", 0.5, 0, 1));
    gui.add(vertexFriction.setup("Vertex friction", 0.5, 0, 1));
    gui.add(jointFrequency.setup("Joint frequency", 4.f, 0.f, 20.f ));
    gui.add(jointDamping.setup("Joint damping", 1.f, 0.f, 5.f));
    meshVertexRadius.addListener(this, &ofApp::meshRadiusUpdated);
    subsectionWidth.addListener(this, &ofApp::subsectionSizeUpdated);
    subsectionHeight.addListener(this, &ofApp::subsectionSizeUpdated);
  
    gui.loadFromFile("ReproductiveFeedback.xml");
  
    // Custom walls for this program.
    //createCustomWalls();
    //box2d.createBounds();
    box2d.createGround(0, ofGetHeight(), ofGetWidth(), ofGetHeight());
  
    cryptoPunks.load("punks.png");

  
    // Collection of filters.
    populateFilters();
    
    // Create subsection properties.
    createSubsectionProperties();
  
    // Create all the subsections of the image.
    createImageSubsections();

    // Setup a mesh and box2d body for an image subsection.
    createSubsectionBody();
}

void ofApp::update()
{
    // Update subsection properties every time.
    createSubsectionProperties();
  
    // Update box2d
    box2d.update();

    for (int i = 0; i < softBodies.size(); i++) {
      softBodies[i].update();
      if (softBodies[i].isOutside) { // Erase this element if it's outside the bounds.
        softBodies.erase(softBodies.begin() + i);
      }
    }
  
    unsigned long elapsedTime = ofGetElapsedTimeMillis() - trackTime;
    if (elapsedTime > 3 * 1000 && softBodies.size() <= 3) { // Every 5 seconds create a new one. No more than 3 soft bodies on the screen.
      newSubsection = true;
      trackTime = ofGetElapsedTimeMillis(); // Reset time.
    }
}

void ofApp::draw()
{
  std::cout<<"Frame rate: " << ofGetFrameRate() << endl;
  
  ofPushMatrix();
    ofPushMatrix();

      ofPushStyle();
      filters[0] -> begin();
      cryptoPunks.draw(0, 0, ofGetWidth(), ofGetHeight());
      filters[0] -> end();
      ofPopStyle();

      // Torn subsections.
      if (tornSubsections.size() > 0) {
        for (auto s: tornSubsections) {
          ofPushStyle();
            filters[s.filterIdx] -> begin();
            ofTexture tex = cryptoPunks.getTexture();
            tex.drawSubsection(s.origin.x, s.origin.y, subsectionWidth, subsectionHeight, s.origin.x, s.origin.y);
            filters[s.filterIdx] -> end();
          ofPopStyle();
        }
      }

      // Soft body.
      if (softBodies.size() > 0) {
        for (auto b: softBodies) {
          ofPushStyle();
            filters[b.filterIdx] -> begin();
            cryptoPunks.getTexture().bind();
              b.draw(showSoftBody);
            cryptoPunks.getTexture().unbind();
            filters[b.filterIdx] -> end();
          ofPopStyle();
        }
      }

      // Recreate the mesh.
      if (newSubsection) {
        // Recreate the mesh and box2DSprings
        createSubsectionBody();
        newSubsection = false;
      }

      // Clear everything, recreate subsections, recreate soft bodies.
      if (clear) {
        //createImageSubsections();
        softBodies.clear();
        //tornSubsections.clear();
        clear = false;
      }
    ofPopMatrix();
  ofPopMatrix();
  
  if (!hideGui) {
    gui.draw();
  }
}

void ofApp::keyPressed(int key) {
    switch (key) {
      case 'n': {
        newSubsection = !newSubsection;
        break;
      }
      
      case 's': {
        showSoftBody = !showSoftBody;
        break;
      }
      
      case 'h': {
        hideGui = !hideGui;
        break;
      }
      
      case 'c': {
        clear = !clear;
        break;
      }
      
      default: {
        break;
      }
    }
}

void ofApp::createCustomWalls() {
  // Get a handle to the ground.
  auto &ground = box2d.ground;
  
  b2EdgeShape shape;
  
  ofRectangle rec(0/OFX_BOX2D_SCALE, 0/OFX_BOX2D_SCALE, ofGetWidth()/OFX_BOX2D_SCALE, ofGetHeight()/OFX_BOX2D_SCALE);
  
  // Left wall
  shape.Set(b2Vec2(rec.x, rec.y), b2Vec2(rec.x, rec.y + rec.height));
  ground->CreateFixture(&shape, 0.0f);
  
  // Right wall.
  shape.Set(b2Vec2(rec.x + rec.width, rec.y), b2Vec2(rec.x + rec.width, rec.y));
  ground->CreateFixture(&shape, 0.0f);
}

void ofApp::populateFilters() {
  filters.push_back(new SketchFilter(cryptoPunks.getWidth(), cryptoPunks.getHeight()));
  
  filters.push_back(new PosterizeFilter(5));
  
  // Displacement
  filters.push_back(new DisplacementFilter("img/glass/3.jpg", cryptoPunks.getWidth(), cryptoPunks.getHeight(), 30.0));


  // Lookup
  filters.push_back(new LookupFilter(cryptoPunks.getWidth(), cryptoPunks.getHeight(), "img/lookup_miss_etikate.png"));
  
   // Perlin Pixellation
  filters.push_back(new PerlinPixellationFilter(cryptoPunks.getWidth(), cryptoPunks.getHeight()));
  
  
//  filters.push_back(new ZoomBlurFilter());
//
//
//  filters.push_back(new LaplacianFilter(cryptoPunks.getWidth(), cryptoPunks.getHeight(), ofVec2f(1, 1)));
}

// Recreate image subsections.
void ofApp::subsectionSizeUpdated(int &num) {
  createImageSubsections();
}

void ofApp::createSubsectionProperties() {
  // Create Soft Body payload to create objects.
  softBodyProperties.meshDimensions = ofPoint(meshRows, meshColumns);
  softBodyProperties.vertexPhysics = ofPoint(vertexBounce, vertexDensity, vertexFriction); // x (bounce), y (density), z (friction)
  softBodyProperties.jointPhysics = ofPoint(jointFrequency, jointDamping); // x (frequency), y (damping)
  softBodyProperties.meshVertexRadius = meshVertexRadius;
  softBodyProperties.subsectionSize = ofPoint(subsectionWidth, subsectionHeight); // x (width), y(height)
  softBodyProperties.textureDimensions = ofPoint(cryptoPunks.getTexture().getWidth(), cryptoPunks.getTexture().getHeight());
}

void ofApp::createImageSubsections() {
  // Clear previous subsections.
  imageSubsections.clear();
  
  // Build a collection of image subsections.
  for (int x = meshVertexRadius; x < cryptoPunks.getWidth() - meshVertexRadius; x+=subsectionWidth) {
    for (int y = meshVertexRadius; y < cryptoPunks.getHeight() - meshVertexRadius; y+=subsectionHeight) {
      Subsection s = Subsection(glm::vec2(x, y)); // Default subsection.
      imageSubsections.push_back(s);
    }
  }
}

void ofApp::meshRadiusUpdated(float &radius) {
  // Update bounding box.
  box2d.createBounds(ofRectangle(0, 0, ofGetWidth() + radius, ofGetHeight() + radius));
}

void ofApp::createSubsectionBody() {
  SubsectionBody body;
  Subsection &s = imageSubsections[ofRandom(imageSubsections.size())];
  
  // Setup the body. 
  body.setup(box2d, s.origin, softBodyProperties);
  
  // NOTE: Make the sure the filter index is correctly transferred from
  // subsection to subsection body.
  body.filterIdx = s.filterIdx; // Old filter index that this soft body should bind to.
  s.filterIdx = (s.filterIdx + 1) % filters.size(); // Increment the filter index as this has been torn now.
  
  // Push this new subsection body to our collection.
  softBodies.push_back(body);
  
  // If s.origin is in the subsection, then just edit that subsection.
  bool found = false;
  for (auto &tornSub: tornSubsections) {
    // Check if these origins are equal.
    if (tornSub.origin.x == s.origin.x && tornSub.origin.y == s.origin.y) {
      // Update filter index in the tornSub to the new subsection.
      tornSub.filterIdx = s.filterIdx;
      std::cout << "Updating an already existing torn subsection: " << tornSubsections.size() << ", " << tornSub.filterIdx <<  endl;
      found = true;
      break;
    }
  }
  
  if (!found) {
    // Create new torn subsection since it hasn't been torn yet.
    Subsection tornSub = Subsection(s.origin, s.filterIdx);
    tornSubsections.push_back(tornSub);
    std::cout << "No old torn subsection found. Adding a new one: " << tornSubsections.size() << endl;
  }
  
  trackTime = ofGetElapsedTimeMillis();
}

void ofApp::exit() {
  gui.saveToFile("ReproductiveFeedback.xml");
}
