#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetBackgroundColor(ofColor::black);
	theCam = &easyCam;
	easyCam.setPosition(glm::vec3(0, 0, 10));
	easyCam.lookAt(glm::vec3(0, 0, -1));
	easyCam.setNearClip(.1);
	sideCam.setPosition(glm::vec3(5, 0, 0));
	sideCam.lookAt(glm::vec3(0, 0, 0));
	sideCam.setNearClip(.1);
	image.allocate(imageWidth, imageHeight, OF_IMAGE_COLOR);
	//scene.push_back(&vp);
	//scene.push_back(&sphere0);
	//scene.push_back(&sphere1);
	//scene.push_back(&sphere2);
	//scene.push_back(&sphere3);
	//scene.push_back(&sphere4);
	scene.push_back(&table);

	//lights.push_back(&light0);
	lights.push_back(&light1);
	lights.push_back(&light2);

	//spotlights.push_back(&spotlight0);
	//spotlights.push_back(&spotlight1);
	//spotlights.push_back(&spotlight2);
	
	//table.scene.push_back(&sphere0);
	//table.scene.push_back(&sphere1);
	//table.scene.push_back(&sphere2);
	//table.scene.push_back(&sphere3);
	//table.scene.push_back(&sphere4);

	//table.lights.push_back(&light0);
	table.lights.push_back(&light1);
	table.lights.push_back(&light2);

	//table.spotlights.push_back(&spotlight0);
	//table.spotlights.push_back(&spotlight1);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	theCam->begin();
	drawAxis();
	ofNoFill();
	
	table.draw();
	vp.draw();

	renderCam.draw();

	for (auto i : scene) {
		i->draw();
	}
	//sphere0.draw();
	//sphere1.draw();
	//sphere2.draw();
	//sphere3.draw();
	//sphere4.draw();

	//light0.draw();
	//light1.draw();

	for (auto j : lights) {
		j->draw();
	}
	//spotlight0.draw();
	//spotlight1.draw();
	//spotlight2.draw();

	theCam->end();
}

void ofApp::drawAxis() {
	ofSetColor(ofColor::red);
	ofDrawLine(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0));
	ofSetColor(ofColor::green);
	ofDrawLine(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	ofSetColor(ofColor::blue);
	ofDrawLine(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
	
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key) {
	case OF_KEY_F1:
		theCam = &easyCam;
		break;
	case OF_KEY_F2:
		theCam = &sideCam;
		break;
	case OF_KEY_F3:
		cout << "Running rayTrace\n";
		rayTrace();
	case 'c':
		if (easyCam.getMouseInputEnabled()) easyCam.disableMouseInput();
		else easyCam.enableMouseInput();
		break;
	case 'j':
		newColor = &ofColor(ofRandom(0, 255), ofRandom(0, 255), ofRandom(0, 255));
		current = new Sphere(glm::vec3(0,2,-5), 1, *newColor);
		scene.push_back(current);
		table.scene.push_back(current);
		break;
	case 'd':
		if (!selected.empty()) {
			scene.erase(std::find(scene.begin(), scene.end(), selected[0]));
			table.scene.erase(std::find(table.scene.begin(), table.scene.end(), selected[0]));
			selected.clear();
		}
		break;
	case 't':
		cout << scene.size() << '\n';
		for (auto i : scene) {
			cout << i->getColor() << '\n';
		}
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	if (objSelected() && bDrag) {
		glm::vec3 point;
		mouseToDragPlane(x, y, point);
		
		
		selected[0]->position += (point - lastPoint);
		
		lastPoint = point;
	}
}

bool ofApp::mouseToDragPlane(int x, int y, glm::vec3 &point) {
	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	float dist;
	glm::vec3 pos;
	if (objSelected()) {
		pos = selected[0]->position;
	}
	else pos = glm::vec3(0, 0, 0);
	if (glm::intersectRayPlane(p, dn, pos, glm::normalize(theCam->getZAxis()), dist)) {
		point = p + dn * dist;
		return true;
	}
	return false;
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

	// if we are moving the camera around, don't allow selection
	//
	if (easyCam.getMouseInputEnabled()) return;

	// clear selection list
	//
	selected.clear();

	//
	// test if something selected
	//
	vector<SceneObject *> hits;

	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	// check for selection of scene objects
	//
	for (int i = 0; i < scene.size(); i++) {

		glm::vec3 point, norm;

		//  We hit an object
		//
		if (scene[i]->isSelectable && scene[i]->intersect(Ray(p, dn), point, norm)) {
			hits.push_back(scene[i]);
			cout << "hit\n";
		}
		
	}


	// if we selected more than one, pick nearest
	//
	SceneObject *selectedObj = NULL;
	if (hits.size() > 0) {
		selectedObj = hits[0];
		float nearestDist = std::numeric_limits<float>::infinity();
		for (int n = 0; n < hits.size(); n++) {
			float dist = glm::length(hits[n]->position - theCam->getPosition());
			if (dist < nearestDist) {
				nearestDist = dist;
				selectedObj = hits[n];
			}
		}
	}
	if (selectedObj) {
		selected.push_back(selectedObj);
		bDrag = true;
		mouseToDragPlane(x, y, lastPoint);
		cout << "hit\n";
	}
	else {
		selected.clear();
		cout << "miss\n";
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	bDrag = false;
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

// Intersect Ray with Plane  (wrapper on glm::intersect*
//
bool Plane::intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normalAtIntersect) {
	float dist;
	bool insidePlane = false;
	bool hit = glm::intersectRayPlane(ray.p, ray.d, position, this->normal, dist);
	if (hit) {
		Ray r = ray;
		point = r.evalPoint(dist);
		normalAtIntersect = this->normal;
		glm::vec2 xrange = glm::vec2(position.x - width / 2, position.x + width / 2);
		glm::vec2 zrange = glm::vec2(position.z - height / 2, position.z + height / 2);
		if (point.x < xrange[1] && point.x > xrange[0] && point.z < zrange[1] && point.z > zrange[0]) {
			insidePlane = true;
		}
	}
	return insidePlane;
}

// Convert (u, v) to (x, y, z) 
// We assume u,v is in [0, 1]
//
glm::vec3 ViewPlane::toWorld(float u, float v) {
	float w = width();
	float h = height();
	return (glm::vec3((u * w) + min.x, (v * h) + min.y, position.z));
}

// Get a ray from the current camera position to the (u, v) position on
// the ViewPlane
//
Ray RenderCam::getRay(float u, float v) {
	glm::vec3 pointOnPlane = view.toWorld(u, v);
	return(Ray(position, glm::normalize(pointOnPlane - position)));
}

ofColor Plane::getColor(glm::vec3 contactPt, ofImage* t) {
	bool hit = false;
	float distance = std::numeric_limits<float>::infinity();
	glm::vec3 intersectPoint, intersectNormal;
	SceneObject* closestObj = NULL;
	glm::vec3* closestPoint = NULL;
	glm::vec3* closestNorm = NULL;

	glm::vec3 I = contactPt - glm::vec3(contactPt.x, 0, 10);
	glm::vec3 B = glm::vec3(0, contactPt.y, 0);
	glm::vec3 A = I - B;
	glm::vec3 R = A - B;

	ofColor color = diffuseColor;
	for (auto obj : scene) {
		if (obj->intersect(Ray(contactPt, normalize(R)), intersectPoint, intersectNormal) ){
			hit = true;
				if (glm::distance(contactPt, intersectPoint) < distance) {
					distance = glm::distance(contactPt, intersectPoint);
					closestObj = obj;
					closestPoint = &intersectPoint;
					closestNorm = &intersectNormal;
				}
		}
	}
	if (hit) {
		//fetching the color takes a while. find a way to speed that up

		//cout << contactPt << "\n";
		//cout << closestPoint->x << ", " << closestPoint->y << ", " << closestPoint->z << "\n\n";
		//cout << "getting texture color\n";
		//ofColor textureColor = closestObj->getColor();
		//color += textureColor;
		color = closestObj->getColor();
		
		for (auto light : lights) {
			//lambert
			glm::vec3 l = normalize(light->position - *closestPoint);
			glm::vec3 normOffset = glm::vec3(closestNorm->x, closestNorm->y, closestNorm->z) * 0.1;
			glm::vec3 ip, nrml;
			Ray shadRay = Ray(*closestPoint + normOffset, normalize(light->position - *closestPoint + normOffset));

			for (auto obj : scene) {
				if (obj->intersect(shadRay, ip, nrml)) {
					color += ofColor::black;
				}
				else {
					color += (ofColor::white * light->intensity)*std::max(float(0), glm::dot(normalize(nrml), l));
				}
			}

			//phong
			glm::vec3 v = normalize(glm::vec3(0,0,10) + *closestPoint);
			glm::vec3 h = (l + v) / (glm::length(l + v));

			for (auto obj : scene) {
				if (obj->intersect(shadRay, ip, nrml)) {
					color += ofColor::black;
				}
				else {
					color += (ofColor::white * light->intensity) * pow(std::max(float(0), glm::dot(normalize(nrml),normalize(h))),1);
				}
			}
		}
	}
	
	return color;
}

void ofApp::rayTrace() {
	for (int i = 0; i < imageWidth; i++) {
		//cout << (i / imageWidth) * 100 << "% complete\n";
		for (int j = 0; j < imageHeight; j++) {
			cout << i << ", " << j << "\n";
			double u = (i + .5) / imageWidth;
			double v = (j + .5) / imageHeight;
			
			Ray ray = renderCam.getRay(u, v);

			bool hit = false;

			float distance = std::numeric_limits<float>::infinity();

			glm::vec3 intersectPoint, normal;

			SceneObject* closestObj = NULL;

			glm::vec3* closestPoint = NULL;

			glm::vec3* closestNorm = NULL;

			for (auto obj : scene) {
				if (obj->intersect(ray, intersectPoint, normal)) {
					hit = true;
					if (glm::distance(glm::vec3(u, v, 0), intersectPoint) < distance) {
						distance = glm::distance(glm::vec3(u, v, 0), intersectPoint);
						closestObj = obj;
						closestPoint = &intersectPoint;
						closestNorm = &normal;
						//cout << "Found closest object\n";
						
					}
				}
			}
			if (hit) {
				ofColor color = ofColor::black;
				//cout << "getting texture color\n";
				ofColor textureColor = closestObj->getColor(*closestPoint, texture);
				for (auto light : lights) {
					color += lambert(*light, *closestPoint, *closestNorm, textureColor, 1);
					color += phong(*light, *closestPoint, *closestNorm, textureColor, ofColor::white, glm::vec3(u, v, 0), 25, 1);
				}
				for (auto spotlight : spotlights) {
					color += spotLambert(*spotlight, *closestPoint, *closestNorm, textureColor, 1);
					color += spotPhong(*spotlight, *closestPoint, *closestNorm, textureColor, ofColor::white, glm::vec3(u, v, 0), 25, 1);
				}
		
				image.setColor(i, j, color);
				image.update();
				
			}
			else
			{
				image.setColor(i, j, ofColor::black);
				image.update();
			}
			
		}
	}
	cout << "flipping image\n";
	image.mirror(true, false);
	image.update();
	image.save("projectpic.jpg");
	cout << "save complete\n";
}

ofColor ofApp::lambert(Light &light, const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse, float r = 1) {
	glm::vec3 l = normalize(light.position - p);
	glm::vec3 normOffset = norm * 0.1;
	glm::vec3 intersectpt, normal;
	Ray shadRay = Ray(p + normOffset , normalize(light.position-p+normOffset));
	for (auto obj : scene) {
		if (obj->intersect(shadRay, intersectpt, normal)) {
			//cout << "shadow generated at "<< p <<"\n";
			return ofColor::black;
		}
	}
	return (diffuse * (light.intensity / (r*r))*std::max(float(0), glm::dot(normalize(norm), l)));
}

ofColor ofApp::phong(Light light, const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse, const ofColor specular, glm::vec3 &viewRay, float power = 1, float r = 1) {
	glm::vec3 l = normalize(light.position - p);
	glm::vec3 v = normalize(p + renderCam.position);
	glm::vec3 h = (l + v) / (glm::length(l + v));
	glm::vec3 normOffset = norm * 0.1;
	glm::vec3 intersectpt, normal;
	Ray shadRay = Ray(p + normOffset, normalize(light.position - p + normOffset));
	for (auto obj : scene) {
		if (obj->intersect(shadRay, intersectpt, normal)) {
			//cout << "shadow generated at "<< p <<"\n";
			return ofColor::black;
		}
	}
	return (specular) * (light.intensity / (r*r)) * pow(std::max(float(0), glm::dot(normalize(norm), normalize(h))),power);
}

ofColor ofApp::spotLambert(SpotLight &spotlight, const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse, float r = 1) {
	glm::vec3 l = normalize(spotlight.position - p);
	glm::vec3 normOffset = norm * 0.1;
	glm::vec3 intersectpt, normal;
	Ray shadRay = Ray(p + normOffset, normalize(spotlight.position - p + normOffset));

	glm::vec3 coneDirection = normalize(spotlight.coneDirection);
	glm::vec3 rayDirection = -normalize(spotlight.position - p + normOffset);
	float lightToSurfaceAngle = glm::degrees(glm::acos(glm::dot(rayDirection, coneDirection)));

	if (lightToSurfaceAngle > spotlight.coneAngle) {
		return ofColor::black;
	}

	for (auto obj : scene) {
		if (obj->intersect(shadRay, intersectpt, normal)) {
			//cout << "shadow generated at "<< p <<"\n";
			return ofColor::black;
		}
	}
	return (diffuse * (spotlight.intensity / (r*r))*std::max(float(0), glm::dot(normalize(norm), l)));
}

ofColor ofApp::spotPhong(SpotLight spotlight, const glm::vec3 &p, const glm::vec3 &norm, const ofColor diffuse, const ofColor specular, glm::vec3 &viewRay, float power = 1, float r = 1) {
	glm::vec3 l = normalize(spotlight.position - p);
	glm::vec3 v = normalize(p + renderCam.position);
	glm::vec3 h = (l + v) / (glm::length(l + v));
	glm::vec3 normOffset = norm * 0.1;
	glm::vec3 intersectpt, normal;
	Ray shadRay = Ray(p + normOffset, normalize(spotlight.position - p + normOffset));
	
	glm::vec3 coneDirection = normalize(spotlight.coneDirection);
	glm::vec3 rayDirection = -normalize(spotlight.position - p + normOffset);
	float lightToSurfaceAngle = glm::degrees(glm::acos(glm::dot(rayDirection, coneDirection)));

	if (lightToSurfaceAngle > spotlight.coneAngle) {
		return ofColor::black;
	}
	
	for (auto obj : scene) {
		if (obj->intersect(shadRay, intersectpt, normal)) {
			//cout << "shadow generated at "<< p <<"\n";
			return ofColor::black;
		}
	}
	return (specular) * (spotlight.intensity / (r*r)) * pow(std::max(float(0), glm::dot(normalize(norm), normalize(h))), power);
}

