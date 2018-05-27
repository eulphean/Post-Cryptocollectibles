#include "SubsectionBody.h"

void SubsectionBody::setup(ofxBox2d &box2d, glm::vec2 meshOrigin, SoftBodyProperties softBodyProperties) {
  setupMeshPlane(meshOrigin, softBodyProperties); // Create a mesh.
  createBox2DSprings(box2d, softBodyProperties); // Create box2d structure.
  addForce();
}

void SubsectionBody::update() {
  // Update each point in the mesh according to the
  // box2D vertex.
  auto meshPoints = mesh.getVertices();
  
  for (int j = 0; j < meshPoints.size(); j++) {
    // Get the box2D vertex position.
    glm::vec2 pos = vertices[j] -> getPosition();
    
    // Update mesh point's position with the position of
    // the box2d vertex.
    auto meshPoint = meshPoints[j];
    meshPoint.x = pos.x;
    meshPoint.y = pos.y;
    mesh.setVertex(j, meshPoint);
  }
  
  // Check if these are in bound.
  isOutside = true;
  for (auto &v : vertices) {
    auto boundingRect = ofRectangle(0, 0, ofGetWidth(), ofGetHeight());
    isOutside = isOutside && !boundingRect.inside(v -> getPosition());
  }
  
  if (isOutside == true) {
    // Remove all the vertices.
    ofRemove(vertices, [&](std::shared_ptr<ofxBox2dCircle> circle) {
      return true;
    });
  }
}

void SubsectionBody::draw(bool showSoftBody) {
  // Draw the soft bodies.
  if (showSoftBody) {
    ofPushStyle();
      for(auto v: vertices) {
        ofNoFill();
        ofSetColor(ofColor::red);
        v->draw();
      }

      for(auto j: joints) {
        ofSetColor(ofColor::blue);
        j->draw();
      }
    ofPopStyle();
  }
  
  // Draw the meshes.
  mesh.draw();
}

// Use TRIANGLE mode to setup a mesh.
void SubsectionBody::setupMeshPlane(glm::vec2 meshOrigin, SoftBodyProperties softBodyProperties) {
  mesh.clear();
  mesh.setMode(OF_PRIMITIVE_TRIANGLES);
  
  // Create a mesh for the grabber.
  int nRows = softBodyProperties.meshDimensions.x;
  int nCols = softBodyProperties.meshDimensions.y;
  
  // Width, height for mapping the correct texture coordinate.
  int w = softBodyProperties.subsectionSize.x;
  int h = softBodyProperties.subsectionSize.y;
  
  // Create the mesh.
  for (int y = 0; y < nRows; y++) {
    for (int x = 0; x < nCols; x++) {
      float ix = meshOrigin.x + w * x / (nCols - 1);
      float iy = meshOrigin.y + h * y / (nRows - 1);
      mesh.addVertex({ix, iy, 0});
      
      // Since, we have ofDisableArbTex, we map the coordinates from 0 - 1. 
      float texX = ofMap(ix, 0, softBodyProperties.textureDimensions.x, 0, 1, true); // Map the calculated x coordinate from 0 - 1
      float texY = ofMap(iy, 0, softBodyProperties.textureDimensions.y, 0, 1, true); // Map the calculated y coordinate from 0 - 1
      mesh.addTexCoord(glm::vec2(texX, texY));
    }
  }

  // We don't draw the last row / col (nRows - 1 and nCols - 1) because it was
  // taken care of by the row above and column to the left.
  for (int y = 0; y < nRows - 1; y++)
  {
      for (int x = 0; x < nCols - 1; x++)
      {
          // Draw T0
          // P0
          mesh.addIndex((y + 0) * nCols + (x + 0));
          // P1
          mesh.addIndex((y + 0) * nCols + (x + 1));
          // P2
          mesh.addIndex((y + 1) * nCols + (x + 0));

          // Draw T1
          // P1
          mesh.addIndex((y + 0) * nCols + (x + 1));
          // P3
          mesh.addIndex((y + 1) * nCols + (x + 1));
          // P2
          mesh.addIndex((y + 1) * nCols + (x + 0));
      }
  }
}

void SubsectionBody::createBox2DSprings(ofxBox2d &box2d, SoftBodyProperties softBodyProperties) {
  auto meshVertices = mesh.getVertices();
  
  vertices.clear();
  joints.clear();

  // Create mesh vertices as Box2D elements.
  for (int i = 0; i < meshVertices.size(); i++) {
    auto vertex = std::make_shared<ofxBox2dCircle>();
    vertex -> setPhysics(softBodyProperties.vertexPhysics.x, softBodyProperties.vertexPhysics.y, softBodyProperties.vertexPhysics.z); // bounce, density, friction
    vertex -> setup(box2d.getWorld(), meshVertices[i].x, meshVertices[i].y, softBodyProperties.meshVertexRadius);
    vertices.push_back(vertex);
  }
  
  int meshRows = softBodyProperties.meshDimensions.x;
  int meshColumns = softBodyProperties.meshDimensions.y;
  
  // Create Box2d joints for the mesh.
  for (int y = 0; y < meshRows; y++) {
    for (int x = 0; x < meshColumns; x++) {
      int idx = x + y * meshColumns;
      
      // Do this for all columns except last column.
      // NOTE: Connect current vertex with the next vertex in the same row.
      if (x != meshColumns - 1) {
        auto joint = std::make_shared<ofxBox2dJoint>();
        int rightIdx = idx + 1;
        joint -> setup(box2d.getWorld(), vertices[idx] -> body, vertices[rightIdx] -> body, softBodyProperties.jointPhysics.x, softBodyProperties.jointPhysics.y); // frequency, damping
        joints.push_back(joint);
      }
      
      
      // Do this for each row except the last row. There is no further joint to
      // be made there.
      if (y != meshRows - 1) {
        auto joint = std::make_shared<ofxBox2dJoint>();
        int downIdx = x + (y + 1) * meshColumns;
        joint -> setup(box2d.getWorld(), vertices[idx] -> body, vertices[downIdx] -> body, softBodyProperties.jointPhysics.x, softBodyProperties.jointPhysics.y);
        joints.push_back(joint);
      }
    }
  }
}

void SubsectionBody::addForce() {
  int increment = 2;
  for (int i = 0; i < vertices.size(); i+=increment) {
    auto v = vertices[i];
    auto pos = v -> getPosition();
    v -> addForce(glm::vec2(ofRandom(-5, 5), ofRandom(-5, 5)), 1);
  }
}
