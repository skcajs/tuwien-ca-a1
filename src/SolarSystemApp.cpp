#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/ObjLoader.h"
#include "cinder/params/Params.h"
#include "cinder/Easing.h"
#include <glm/gtx/matrix_decompose.hpp>

#include <random>

using namespace ci;
using namespace ci::app;
using namespace std;

float speed = 0.25f;
float currentTime = 0.0f;
double lastTime = 0;
double deltaTime = 0.1;

// Comments at the bottom
// You might find these useful
std::random_device rndDev;
std::mt19937 rndGen(rndDev());
std::uniform_real_distribution<float> uniformRadianDistribution(0.0f, static_cast<float>(2.0f * M_PI));

class SphereObject {
public:
	float orbitRotationSpeed = 0.0f; // Speed of the orbit Rotation
	float orbitOffset = 0.0f;        // Relative Offset for this Object
	float orbitDistance = 0.0f;

	float satelliteRotationSpeed = 0.0f;
	float satelliteDistance = 0.0f;
	float satelliteOffset = 0.0f;

	float axisRotationSpeed = 0.0f;	// Speed of rotation
	float rotationAngle = 0.0f;		// Current angle of rotation
	float radius = 1.0;             // Radius of this sphere object

	gl::TextureRef textureRef;		// Reference to the Texture this object uses
	gl::BatchRef batchRef;			// Reference to the batch this geometry uses

	void draw() {
		rotationAngle += axisRotationSpeed * deltaTime;
		orbitOffset += orbitRotationSpeed * deltaTime * speed;
		satelliteOffset += satelliteRotationSpeed * deltaTime * speed;
		gl::ScopedModelMatrix scpModelMatrix;
		gl::translate(orbitDistance * sin(orbitOffset), 0, orbitDistance * cos(orbitOffset));
		gl::translate(satelliteDistance * sin(satelliteOffset), 0, satelliteDistance * cos(satelliteOffset));
		gl::scale(radius, radius, radius);
		gl::rotate(angleAxis(rotationAngle, vec3(0, 1, 0)));
		textureRef->bind();
		batchRef->draw();
	}
};

class SolarSystemApp : public App {
public:
	void setup() override;
	void mouseDown(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void mouseWheel(MouseEvent event) override;
	void update() override;
	void draw() override;
	void resize() override;

private:
	CameraPersp cam;

	vec3 cameraEye;

	params::InterfaceGlRef interfaceRef;

	float avgFPS = 0.f;

	float calculateOrbitOffset(float lastOrbitOffset, float lastRadius, float nextRadius);

	vec2 lastMousePos;
	vec3 camLookAtPos = vec3(0, 0, 0);

	SphereObject sun;
	SphereObject mercury;
	SphereObject venus;
	SphereObject earth;
	SphereObject moon;
	SphereObject mars;
	SphereObject jupiter;
	SphereObject saturn;
	SphereObject uranus;
	SphereObject neptune;
};

void SolarSystemApp::setup() {
	std::cout << "setup" << std::endl;

	// Camera Setup
	cameraEye = vec3(250.0, 0.0, -30.0);
	cam.setEyePoint(cameraEye);
	cam.lookAt(vec3(0, 0, -30));

	auto textureShader = gl::ShaderDef().texture().lambert();
	auto texShaderRef = gl::getStockShader(textureShader);

	sun.textureRef = gl::Texture::create(loadImage(loadAsset("sun.jpg")));
	sun.batchRef = gl::Batch::create(geom::Sphere().subdivisions(256), texShaderRef);
	sun.radius = 20.0f;
	sun.axisRotationSpeed = 0.04;

	mercury.textureRef = gl::Texture::create(loadImage(loadAsset("mercury.jpg")));
	mercury.batchRef = gl::Batch::create(geom::Sphere().subdivisions(256), texShaderRef);
	mercury.radius = 0.38f;
	mercury.orbitDistance = calculateOrbitOffset(sun.orbitDistance, sun.radius, mercury.radius + 5.0f);
	mercury.orbitOffset = uniformRadianDistribution(rndGen);
	mercury.orbitRotationSpeed = 4.15f;
	mercury.axisRotationSpeed = 0.017f;

	venus.textureRef = gl::Texture::create(loadImage(loadAsset("venus.jpg")));
	venus.batchRef = gl::Batch::create(geom::Sphere().subdivisions(256), texShaderRef);
	venus.radius = 0.95f;
	venus.orbitDistance = calculateOrbitOffset(mercury.orbitDistance, mercury.radius, venus.radius);
	venus.orbitOffset = uniformRadianDistribution(rndGen);
	venus.orbitRotationSpeed = 1.62f;
	venus.axisRotationSpeed = 0.004f;

	earth.textureRef = gl::Texture::create(loadImage(loadAsset("earth.jpg")));
	earth.batchRef = gl::Batch::create(geom::Sphere().subdivisions(256), texShaderRef);
	earth.orbitDistance = calculateOrbitOffset(venus.orbitDistance, venus.radius, earth.radius);
	earth.orbitOffset = uniformRadianDistribution(rndGen);
	earth.orbitRotationSpeed = 1.0f;
	earth.axisRotationSpeed = 1.0f;

	moon.textureRef = gl::Texture::create(loadImage(loadAsset("moon.jpg")));
	moon.batchRef = gl::Batch::create(geom::Sphere().subdivisions(256), texShaderRef);
	moon.radius = 0.27f;
	moon.orbitDistance = earth.orbitDistance + moon.satelliteDistance;
	moon.orbitOffset = earth.orbitOffset;
	moon.satelliteDistance = earth.radius + 1.0f;
	moon.satelliteRotationSpeed = 2.0f;
	moon.orbitRotationSpeed = 1.0f;
	moon.axisRotationSpeed = 1.0f;

	mars.textureRef = gl::Texture::create(loadImage(loadAsset("mars.jpg")));
	mars.batchRef = gl::Batch::create(geom::Sphere().subdivisions(256), texShaderRef);
	mars.radius = 0.53f;
	mars.orbitDistance = calculateOrbitOffset(earth.orbitDistance, earth.radius, mars.radius);
	mars.orbitOffset = uniformRadianDistribution(rndGen);
	mars.orbitRotationSpeed = 0.53f;
	mars.axisRotationSpeed = 0.96f;

	jupiter.textureRef = gl::Texture::create(loadImage(loadAsset("jupiter.jpg")));
	jupiter.batchRef = gl::Batch::create(geom::Sphere().subdivisions(256), texShaderRef);
	jupiter.radius = 10.97f;
	jupiter.orbitDistance = calculateOrbitOffset(mars.orbitDistance, mars.radius, jupiter.radius);
	jupiter.orbitOffset = uniformRadianDistribution(rndGen);
	jupiter.orbitRotationSpeed = 0.08f;
	jupiter.axisRotationSpeed = 2.4f;

	saturn.textureRef = gl::Texture::create(loadImage(loadAsset("saturn.jpg")));
	saturn.batchRef = gl::Batch::create(geom::Sphere().subdivisions(256), texShaderRef);
	saturn.radius = 9.14f;
	saturn.orbitDistance = calculateOrbitOffset(jupiter.orbitDistance, jupiter.radius, saturn.radius);
	saturn.orbitOffset = uniformRadianDistribution(rndGen);
	saturn.orbitRotationSpeed = 0.033f;
	saturn.axisRotationSpeed = 2.18f;

	uranus.textureRef = gl::Texture::create(loadImage(loadAsset("uranus.jpg")));
	uranus.batchRef = gl::Batch::create(geom::Sphere().subdivisions(256), texShaderRef);
	uranus.radius = 3.98f;
	uranus.orbitDistance = calculateOrbitOffset(saturn.orbitDistance, saturn.radius, uranus.radius);
	uranus.orbitOffset = uniformRadianDistribution(rndGen);
	uranus.orbitRotationSpeed = 0.012f;
	uranus.axisRotationSpeed = -1.41f;

	neptune.textureRef = gl::Texture::create(loadImage(loadAsset("neptune.jpg")));
	neptune.batchRef = gl::Batch::create(geom::Sphere().subdivisions(256), texShaderRef);
	neptune.radius = 3.86f;
	neptune.orbitDistance = calculateOrbitOffset(uranus.orbitDistance, uranus.radius, neptune.radius);
	neptune.orbitOffset = uniformRadianDistribution(rndGen);
	neptune.orbitRotationSpeed = 0.006f;
	neptune.axisRotationSpeed = -1.5f;

	// Text Window
	interfaceRef = params::InterfaceGl::create(getWindow(), "Solar System", toPixels(ivec2(230, 100)));
	interfaceRef->addParam("FPS", &avgFPS, true);

	interfaceRef->addSeparator();
	interfaceRef->addParam("Speed", &speed).min(0.0f).max(100.0f).step(0.01f);

	gl::enableDepthWrite();
	gl::enableDepthRead();
	gl::enableVerticalSync(false);
}

void SolarSystemApp::mouseDown(MouseEvent event) {
	vec2 mousePos = event.getPos();
	float u = mousePos.x / (float)getWindowWidth();
	float v = mousePos.y / (float)getWindowHeight();
	auto ray = cam.generateRay(u, 1.f - v, cam.getAspectRatio());

	if (event.isRight() && event.isRightDown()) {
		lastMousePos = event.getPos();
	}
	if (event.isMiddle() || event.isMiddleDown()) {
		lastMousePos = event.getPos();
	}
}

void SolarSystemApp::mouseDrag(MouseEvent event) {
	if (event.isLeft() || event.isLeftDown()) {
		vec3 right = cross(cam.getViewDirection(), cam.getWorldUp());
		vec3 up = cross(cam.getViewDirection(), right);
		float r = (event.getPos().x - lastMousePos.x) / 180.0f;
		vec3 newCamPos = rotate(camLookAtPos - cam.getEyePoint(), r, up);
		r = (event.getPos().y - lastMousePos.y) / 180.0f;
		float t = acosf(dot(vec3(0, -1, 0), cam.getViewDirection()));
		if (acosf(dot(vec3(0, -1, 0), cam.getViewDirection())) > M_PI - 0.1)
			if (r > 0)
				r = 0;
		if (acosf(dot(vec3(0, -1, 0), cam.getViewDirection())) < 0.1)
			if (r < 0)
				r = 0;
		newCamPos = rotate(newCamPos, r, right);
		cam.setEyePoint(camLookAtPos - newCamPos);
		cam.lookAt(camLookAtPos);

		lastMousePos = event.getPos();
	}
	if (event.isRight() || event.isRightDown()) {
		vec3 right = cross(cam.getViewDirection(), cam.getWorldUp());
		vec3 up = cross(cam.getViewDirection(), right);
		float deltaX = (lastMousePos.x - event.getPos().x) * 0.01f;
		float deltaY = (lastMousePos.y - event.getPos().y) * 0.01f;
		cam.setEyePoint(cam.getEyePoint() + right * deltaX + up * deltaY);
		camLookAtPos += right * deltaX + up * deltaY;
		lastMousePos = event.getPos();
	}
}

void SolarSystemApp::mouseWheel(MouseEvent event) {
	cam.setEyePoint(cam.getEyePoint() + normalize(cam.getViewDirection()) * event.getWheelIncrement() * 1.5f);
}

//This function is called every frame
void SolarSystemApp::update() {
	currentTime = static_cast<float>(getElapsedSeconds());
	deltaTime = currentTime - lastTime;

	lastTime = getElapsedSeconds();
	avgFPS = getAverageFps();
}

void SolarSystemApp::resize() {
	cam.setAspectRatio(getWindowAspectRatio());
}

//Called after update()
void SolarSystemApp::draw() {
	gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl::setMatrices(cam);

	sun.draw();
	mercury.draw();
	venus.draw();
	earth.draw();
	moon.draw();
	mars.draw();
	jupiter.draw();
	saturn.draw();
	uranus.draw();
	neptune.draw();

	interfaceRef->draw();
}

float SolarSystemApp::calculateOrbitOffset(float lastOrbitOffset, float lastRadius, float nextRadius) {
	return lastOrbitOffset - lastRadius - 5.0f - nextRadius;
}

CINDER_APP(SolarSystemApp, RendererGl)

/*
* Comments
* All sizes of the planets are relative to earth, taken from https://science.nasa.gov/resource/solar-system-sizes/
* For the distances from the sun, initially I set the scaled positions to the sun, which sort of worked, but Saturn, Uranus and Neptune were very spaced out.
* A quick fix would have been to adjust the positions for the Jovian (gas) planets so that they looked uniform, but then they would more or less be uniformly spaced out.  
* Next I thought of setting their positions to a logarithmic scale. 
* This ended up looking worse as the Terrestrial planets looked very unevenly placed. So I figured the best solution (and the easiest)
* was just to set a uniform distance between all the planets. This was actually convienient as I could create a simple function to work out all the distances relative 
* to the last. All the planets are all "roughly" uniformely spaced anyway w.r.t their planet types (so, the Terrestrial planets are very evenly spaced, 
* and so are the Jovian... to a slightly lesser degree), so uniformly spacing the planets seemed apropriate here.
* The spin of the planets are all based off an earth day. So for each planet, I found out how long a day is (in earth hours), and adjusted accordingly (by divinding by 24).
* Likewise, the rotation orbital period of a planet is based on the Earths orbit (in earth days), and adjusted accordingly (by divinding by 365.26).
* Just to note, the orbital periods and the planetary rotations aren't synchronised.
* The moon orbits the earth, which I set by adding an additional satellite rotation and orbit period.
*/