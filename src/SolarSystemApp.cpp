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

float currentTime = 0.0f;

// You might find these useful
std::random_device rndDev;
std::mt19937 rndGen(rndDev());
std::uniform_real_distribution<float> uniformRadianDistribution(0.0f, static_cast<float>(2.0f * M_PI));

class SphereObject {
public:
	float orbitRotationSpeed = 1.0f; // Speed of the orbit Rotation
	float orbitOffset = 0.0f;        // Relative Offset for this Object
	float orbitDistance = 0.0f;

	float axisRotationSpeed = 1.0f;	// Speed of rotation
	float radius = 1.0;             // Radius of this sphere object

	gl::TextureRef textureRef;		// Reference to the Texture this object uses
	gl::BatchRef batchRef;			// Reference to the batch this geometry uses

	void draw() {
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

	double lastTime = 0;
	double deltaTime = 0.1;
	vec3 cameraEye;

	params::InterfaceGlRef interfaceRef;

	float avgFPS = 0.f;
	float speed = 1.0;

	vec2 lastMousePos;
	vec3 camLookAtPos = vec3(0, 0, 0);

	// you can delete this
	SphereObject sphere;
};

void SolarSystemApp::setup() {
	std::cout << "setup" << std::endl;
	// Camera Setup
	cameraEye = vec3(5.0, 1.0, 5.0);
	cam.setEyePoint(cameraEye);
	cam.lookAt(vec3(0, 0, 0));

	auto textureShader = gl::ShaderDef().texture().lambert();
	auto texShaderRef = gl::getStockShader(textureShader);
	sphere.textureRef = gl::Texture::create(loadImage(loadAsset("sun.jpg")));
	sphere.batchRef = gl::Batch::create(geom::Sphere().subdivisions(256), texShaderRef);

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

	sphere.draw();

	interfaceRef->draw();
}

CINDER_APP(SolarSystemApp, RendererGl)