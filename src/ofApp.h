#pragma once

#include "ofMain.h"

#include <glm/gtx/intersect.hpp> 
//  General Purpose Ray class 
//
class Ray {
public:
	Ray(glm::vec3 p, glm::vec3 d) { this->p = p; this->d = d; }
	void draw(float t) { ofDrawLine(p, p + t * d); }

	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}

	glm::vec3 p, d;
};

//  Base class for any renderable object in the scene
//
class SceneObject {
public:
	virtual void draw() = 0;    // pure virtual funcs - must be overloaded
	virtual bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { cout << "SceneObject::intersect" << endl; return false; }

	// any data common to all scene objects goes here
	glm::vec3 position = glm::vec3(0, 0, 0);

	// material properties (we will ultimately replace this with a Material class - TBD)
	//
	ofColor diffuseColor = ofColor::grey;    // default colors - can be changed.
	ofColor specularColor = ofColor::lightGray;
	int intensity = 0;
	virtual ofColor getColor(glm::vec3 contactPt = glm::vec3(0, 0, 0), ofImage* t = &ofImage("wood.jpg")) {
		cout << "SceneObject->getColor()\n";
		return diffuseColor;
	}
	bool isSelectable = true;
};

//  General purpose sphere  (assume parametric)
//
class Sphere : public SceneObject {
public:
	Sphere(glm::vec3 p, float r, ofColor diffuse = ofColor::lightGray) { position = p; radius = r; diffuseColor = diffuse; }
	Sphere() {}
	ofColor getColor(glm::vec3 contactPt = glm::vec3(0,0,0), ofImage* t = &ofImage("wood.jpg")){
		//cout << "Sphere->getColor()\n";
		return diffuseColor;
	}
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	}
	void draw() {
		ofSetColor(diffuseColor);
		ofDrawSphere(position, radius);
	}
	bool isSelectable = true;
	float radius = 1.0;
};

//  Mesh class (will complete later- this will be a refinement of Mesh from Project 1)
//
class Mesh : public SceneObject {
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { return false; }
	void draw() { }
};


//  General purpose plane 
//
class Plane : public SceneObject {
public:
	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::darkOliveGreen, float w = 100, float h = 100) {
		position = p; normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;
		if (normal == glm::vec3(0, 1, 0)) plane.rotateDeg(90, 1, 0, 0);
		isSelectable = false;
	}
	Plane() {
		normal = glm::vec3(0, 1, 0);
		plane.rotateDeg(-90, 1, 0, 0);
		isSelectable = false;
	}

	bool intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normal);
	float sdf(const glm::vec3 & p);
	glm::vec3 getNormal(const glm::vec3 &p) { return this->normal; }
	void draw() {
		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);
		ofSetColor(diffuseColor);
		plane.drawWireframe();
	}
	ofColor getColor(glm::vec3 contactPt = glm::vec3(0, 0, 0), ofImage* t = NULL);
	ofPlanePrimitive plane;
	glm::vec3 normal;
	
	float width = 20;
	float height = 20;
	vector<SceneObject *> scene;
	vector<SceneObject *> lights;
};

class  ViewPlane : public Plane {
public:
	ViewPlane(glm::vec2 p0, glm::vec2 p1) { min = p0; max = p1; }

	ViewPlane() {                         // create reasonable defaults (6x4 aspect)
		min = glm::vec2(-3, -2);
		max = glm::vec2(3, 2);
		position = glm::vec3(0, 0, 5);
		normal = glm::vec3(0, 0, 1);      // viewplane currently limited to Z axis orientation
	}

	void setSize(glm::vec2 min, glm::vec2 max) { this->min = min; this->max = max; }
	float getAspect() { return width() / height(); }

	glm::vec3 toWorld(float u, float v);   //   (u, v) --> (x, y, z) [ world space ]

	void draw() {
		ofDrawRectangle(glm::vec3(min.x, min.y, position.z), width(), height());
	}


	float width() {
		return (max.x - min.x);
	}
	float height() {
		return (max.y - min.y);
	}

	// some convenience methods for returning the corners
	//
	glm::vec2 topLeft() { return glm::vec2(min.x, max.y); }
	glm::vec2 topRight() { return max; }
	glm::vec2 bottomLeft() { return min; }
	glm::vec2 bottomRight() { return glm::vec2(max.x, min.y); }

	//  To define an infinite plane, we just need a point and normal.
	//  The ViewPlane is a finite plane so we need to define the boundaries.
	//  We will define this in terms of min, max  in 2D.  
	//  (in local 2D space of the plane)
	//  ultimately, will want to locate the ViewPlane with RenderCam anywhere
	//  in the scene, so it is easier to define the View rectangle in a local'
	//  coordinate system.
	//
	glm::vec2 min, max;
};


//  render camera  - currently must be z axis aligned (we will improve this in project 4)
//
class RenderCam : public SceneObject {
public:
	RenderCam() {
		position = glm::vec3(0, 0, 10);
		aim = glm::vec3(0, 0, -1);
	}
	Ray getRay(float u, float v);
	void draw() { ofDrawBox(position, 1.0); };
	void drawFrustum();

	glm::vec3 aim;
	ViewPlane view;          // The camera viewplane, this is the view that we will render 
};

class Light : public SceneObject {
public:
	Light(glm::vec3 p, float i) {
		position = p;
		intensity = i;
	}

	void draw() { 
		ofSetColor(ofColor::white);
		ofDrawSphere(position, .1); 
	}

	glm::vec3 getPosition() { return position; }

	float intensity;
};

class SpotLight : public SceneObject {
public:
	SpotLight(glm::vec3 p, float i, float cA, glm::vec3 cD) {
		position = p;
		intensity = i;
		coneAngle = cA;
		coneDirection = cD;
	}

	void draw() {
		ofSetColor(ofColor::white);
		ofDrawSphere(position, .1);
		ofDrawArrow(position, position + coneDirection);
	}

	float intensity;
	float coneAngle;
	glm::vec3 coneDirection;
};

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void drawAxis();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	bool mouseToDragPlane(int x, int y, glm::vec3 & point);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	void rayTrace();
	ofColor lambert(Light & light, const glm::vec3 & p, const glm::vec3 & norm, const ofColor diffuse, float r);
	ofColor phong(Light light, const glm::vec3 & p, const glm::vec3 & norm, const ofColor diffuse, const ofColor specular, glm::vec3 & viewRay, float power, float r);
	ofColor spotLambert(SpotLight & spotlight, const glm::vec3 & p, const glm::vec3 & norm, const ofColor diffuse, float r);
	ofColor spotPhong(SpotLight spotlight, const glm::vec3 & p, const glm::vec3 & norm, const ofColor diffuse, const ofColor specular, glm::vec3 & viewRay, float power, float r);
	void drawGrid();
	void drawAxis(glm::vec3 position);
	bool objSelected() { return (selected.size() ? true : false); };

	bool bHide = true;
	bool bShowImage = false;
	ofColor *newColor;
	ofEasyCam  easyCam;
	ofCamera sideCam;
	ofCamera previewCam;
	ofCamera  *theCam;    // set to current camera either mainCam or sideCam

	// set up one render camera to render image throughn
	//
	RenderCam renderCam;
	ofImage image;
	
	ofImage *texture = &ofImage("leaf.jpg");

	vector<SceneObject *> scene;
	vector<Light *> lights;
	vector<SpotLight *> spotlights;
	vector<SceneObject *> selected;

	bool bDrag = false;
	glm::vec3 lastPoint;

	int imageWidth = 600;
	int imageHeight = 400;

	Sphere *current;

	Plane table = Plane(glm::vec3(0,-2,0), glm::vec3(0, 1, 0), ofColor::lightGray, 100, 100);
	ViewPlane vp = ViewPlane(glm::vec2(-3, -2), glm::vec2(3, 2));
	Sphere sphere0 = Sphere(glm::vec3(4, 4,-17.5), 1, ofColor::gold);
	Sphere sphere1 = Sphere(glm::vec3(0, 5, -20), 2, ofColor::silver);
	Sphere sphere2 = Sphere(glm::vec3(-4, 4, -17.5), 1, ofColor::fireBrick);
	Sphere sphere3 = Sphere(glm::vec3(6, 3, -15), 1, ofColor::green);
	Sphere sphere4 = Sphere(glm::vec3(-6, 3, -15), 1, ofColor::red);

	Light light0 = Light(glm::vec3(0, 8, -10), 2);
	Light light1 = Light(glm::vec3(-8, 6, -12), 2);
	Light light2 = Light(glm::vec3(8, 6, -12), 2);

	SpotLight spotlight0 = SpotLight(glm::vec3(10, 10, -12), 3, 15, glm::vec3(1,-1, -1));
	SpotLight spotlight1 = SpotLight(glm::vec3(-10, 10, -12), 2, 15, glm::vec3(1, -1, -1));
	SpotLight spotlight2 = SpotLight(glm::vec3(10, 10, 0), 5, 30, glm::vec3(-1, -1, -1));
};