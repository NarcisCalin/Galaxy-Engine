#include <iostream>
#include <vector>
#include "raylib.h"
#include <cmath>
#include <algorithm>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <stdexcept>

#include "slingshot.h"


int screenWidth = 1000;
int screenHeight = 1000;

int targetFPS = 144;

const double G = 6.674 * pow(10, -11);

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <stdexcept>

class ThreadPool {
public:
	// Constructor that spawns 'numThreads' worker threads.
	ThreadPool(size_t numThreads) : stop(false) {
		for (size_t i = 0; i < numThreads; ++i) {
			workers.emplace_back([this] {
				for (;;) {
					std::function<void()> task;

					// Lock and wait for a task.
					{
						std::unique_lock<std::mutex> lock(this->queueMutex);
						this->condition.wait(lock, [this] {
							return this->stop || !this->tasks.empty();
							});
						if (this->stop && this->tasks.empty())
							return; // Exit thread.
						task = std::move(this->tasks.front());
						this->tasks.pop();
					}

					// Execute the task.
					task();
				}
				});
		}
	}

	// Enqueue a new task and return a future to get the result.
	template<class F, class... Args>
	auto enqueue(F&& f, Args&&... args)
		-> std::future<typename std::result_of<F(Args...)>::type>
	{
		using return_type = typename std::result_of<F(Args...)>::type;

		// Wrap the function and its arguments.
		auto task = std::make_shared<std::packaged_task<return_type()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);

		std::future<return_type> res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(queueMutex);

			// Don't allow enqueueing after stopping the pool.
			if (stop)
				throw std::runtime_error("enqueue on stopped ThreadPool");

			tasks.emplace([task]() { (*task)(); });
		}
		condition.notify_one();
		return res;
	}

	// Destructor: Join all threads.
	~ThreadPool() {
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			stop = true;
		}
		condition.notify_all();
		for (std::thread& worker : workers)
			worker.join();
	}

private:
	// Vector of worker threads.
	std::vector<std::thread> workers;
	// Task queue.
	std::queue<std::function<void()>> tasks;

	// Synchronization.
	std::mutex queueMutex;
	std::condition_variable condition;
	bool stop;
};


class MouseTrailDot {
public:
	Vector2 pos;
	int size = 5;
	MouseTrailDot(float x, float y, int z) {

		pos.x = x;
		pos.y = y;
		size = z;

	}
};

class Planet {
public:
	Vector2 pos;
	float size;
	Vector2 velocity;
	double mass;
	Color color;
	bool enableBlur = true;
	Vector2 acceleration;
	Vector2 prevAcceleration;
	bool customColor;
	float pressureRadius;




	Planet(float x, float y, float z, float vx, float vy, double w, int r, int g, int b, int a, float accX, float accY, bool customColor) {
		pos.x = x;
		pos.y = y;
		size = z;
		velocity.x = vx;
		velocity.y = vy;
		mass = w;
		color.r = r;
		color.g = g;
		color.b = b;
		color.a = a;
		acceleration.x = accX;
		acceleration.y = accY;
		prevAcceleration = acceleration;
		this->customColor = customColor;
		pressureRadius = size;

	}
};






bool rectanglesIntersect(const Rectangle& r1, const Rectangle& r2) {
	return!(r1.x > r2.x + r2.width ||
		r1.x + r1.width < r2.x ||
		r1.y > r2.y + r2.height ||
		r1.y + r1.height < r2.y);
}

class Quadtree {
public:
	Vector2 pos;
	float size;
	Color color;
	std::vector<Quadtree> subGrids;
	std::vector<Planet> myPlanets;
	float gridMass;
	Vector2 centerOfMass;
	Quadtree* parent;
	int depth;



	Quadtree(float posX, float posY, float size,
		int r, int g, int b, int a,
		std::vector<Planet> planets,
		Quadtree* parent = nullptr) {

		this->pos.x = posX;
		this->pos.y = posY;
		this->size = size;
		this->color.r = r;
		this->color.g = g;
		this->color.b = b;
		this->color.a = a;
		this->gridMass = 0;
		this->centerOfMass = { 0,0 };
		this->parent = parent;
		depth = (parent == nullptr) ? 0 : parent->depth + 1; // Set depth here
		for (auto& planet : planets) {
			if (planet.pos.x >= pos.x && planet.pos.x < pos.x + size &&
				planet.pos.y >= pos.y && planet.pos.y < pos.y + size) {
				myPlanets.push_back(planet);
			}
		}

		if (myPlanets.size() > 1) {
			subGridMaker();
		}
	}

	void subGridMaker() {
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) {
				subGrids.emplace_back(
					this->pos.x + i * (this->size / 2),
					this->pos.y + j * (this->size / 2),
					this->size / 2,
					255, 255, 255, 255,
					this->myPlanets,
					this

				);
			}
		}

		//Draw grids
		for (auto& subGrid : subGrids) {
			if (size < 10) {
				//DrawRectangleLines(subGrid.pos.x, subGrid.pos.y, subGrid.size, subGrid.size, RED);
			}
			else {
				//DrawRectangleLines(subGrid.pos.x, subGrid.pos.y, subGrid.size, subGrid.size, WHITE);
			}
		}
	}

	std::vector<Planet*> queryRange(const Rectangle& range) const {
		std::vector<Planet*> results;

		Rectangle nodeRect = { pos.x, pos.y, size, size };
		if (!rectanglesIntersect(nodeRect, range)) {
			return results; // No intersection, return empty.
		}

		if (subGrids.empty()) {
			for (const Planet& planet : myPlanets) {
				if (planet.pos.x >= range.x && planet.pos.x <= range.x + range.width &&
					planet.pos.y >= range.y && planet.pos.y <= range.y + range.height)
				{
					// Casting away const is not ideal; ideally store pointers in the tree.
					results.push_back((Planet*)&planet);
				}

			}
		}

		else {
			for (const Quadtree& child : subGrids) {
				std::vector<Planet*> childResults = child.queryRange(range);
				results.insert(results.end(), childResults.begin(), childResults.end());
			}
		}
		return results;
	}

	void calculateMasses() {

		for (auto& subGrid : subGrids) {
			subGrid.calculateMasses();
		}

		if (myPlanets.size() == 1 && subGrids.empty()) {
			gridMass = myPlanets[0].mass;
			centerOfMass = myPlanets[0].pos;
		}
		else if (!subGrids.empty()) {
			gridMass = 0;
			float totalMass = 0;
			centerOfMass = { 0,0 };

			for (auto& subGrid : subGrids) {
				if (subGrid.gridMass > 0) {
					gridMass += subGrid.gridMass;
					centerOfMass.x += subGrid.centerOfMass.x * subGrid.gridMass;
					centerOfMass.y += subGrid.centerOfMass.y * subGrid.gridMass;
					totalMass += subGrid.gridMass;
				}
			}

			if (totalMass > 0) {
				centerOfMass.x /= totalMass;
				centerOfMass.y /= totalMass;
			}
		}
		else {
			gridMass = 0;
			centerOfMass = { 0,0 };
		}
	}

	void printGridInfo() {
		std::cout << "Grid at (" << pos.x << ", " << pos.y << ") with size " << size << ":\n";
		std::cout << "  Mass: " << gridMass << "\n";
		std::cout << "  Center of Mass: (" << centerOfMass.x << ", " << centerOfMass.y << ")\n";
		std::cout << "  Number of planets: " << myPlanets.size() << "\n";
		std::cout << "  Number of subgrids: " << subGrids.size() << "\n";
	}

	void parentInfo() const {
		if (parent != nullptr) {
			std::cout << "Parent grid mass: " << parent->gridMass << '\n';
		}
		else {
			std::cout << "The grid has no parent (Root)" << '\n';
		}
	}

	void drawCenterOfMass() const {

		if (gridMass > 0 && (myPlanets.size() > 1 || subGrids.size() > 0)) {

			float circleSize = std::max(4.0f - (depth * 0.5f), 1.0f);


			Color depthColor = { 255,20 + (depth * 30),20,255 };

			DrawCircle(centerOfMass.x, centerOfMass.y, circleSize, depthColor);
		}


		for (const auto& subGrid : subGrids) {
			subGrid.drawCenterOfMass();
		}
	}

};

Quadtree gridFunction(std::vector<Planet> vectorPlanet) {
	Quadtree grid(
		0,             // posX
		0,             // posY
		screenWidth,   // size
		12,            // Red
		12,            // Green
		12,            // Blue
		12,            // Alpha
		vectorPlanet,  // planets
		nullptr        // parent (null for root)
	);

	// Calculate masses for the entire tree after construction
	grid.calculateMasses();
	//grid.printGridInfo();
	//grid.drawCenterOfMass();

	return grid;
}

bool isMouse0Pressed;
bool isMouse2SpacePressed;
bool isDragging = false;

Slingshot slingshotObject = Slingshot::planetSlingshot(isDragging, isMouse0Pressed, isMouse2SpacePressed);

Planet createPlanet() {


	Slingshot slingshot = slingshotObject.planetSlingshot(isDragging, isMouse0Pressed, isMouse2SpacePressed);


	Planet planet(GetMouseX(),
		GetMouseY(),
		4,
		slingshot.normalizedX * slingshot.length,
		slingshot.normalizedY * slingshot.length,
		50000000000000000,
		255,
		255,
		255,
		255,
		0,
		0,
		true
	);
	planet.pressureRadius *= 2;


	return planet;
}

int currentPlanet;
int previousPlanet;

bool enableBlur = false;
void drawScene(std::vector<Planet>& planets, std::vector<MouseTrailDot>& dots) {

	if (IsKeyPressed(KEY_B) && enableBlur) {
		enableBlur = false;
	}
	else if (IsKeyPressed(KEY_B) && !enableBlur) {
		enableBlur = true;
	}
	for (Planet& planet : planets) {


		if (enableBlur) {

			for (int i = 1; i <= 3; i++) {

				float t = static_cast<float>(i) / 90.0f;

				float falloff = 1.0f - (t * t);

				int blurSize = planet.size + static_cast<int>(35 * t);

				float newAlpha = planet.color.a * falloff / 5;
				if (newAlpha < 0.0f) newAlpha = 0.0f;
				if (newAlpha > 255.0f) newAlpha = 255.0f;

				Color blurColor = { planet.color.r, planet.color.g, planet.color.b, static_cast<unsigned char>(newAlpha) };

				DrawCircle(planet.pos.x, planet.pos.y, blurSize, blurColor);
			}
		}
		else {
			DrawCircle(planet.pos.x, planet.pos.y, planet.size, planet.color);
		}




	}

	for (const MouseTrailDot& dot : dots) {
		DrawCircle(dot.pos.x, dot.pos.y, dot.size, YELLOW);
	}
	DrawText(TextFormat("Particles: %i", planets.size()), 50, 50, 25, WHITE);

	if (GetFPS() >= 60) {
		DrawText(TextFormat("FPS: %i", GetFPS()), screenWidth - 150, 50, 18, GREEN);

	}
	else if (GetFPS() < 60 && GetFPS() > 30) {
		DrawText(TextFormat("FPS: %i", GetFPS()), screenWidth - 150, 50, 18, YELLOW);
	}
	else {
		DrawText(TextFormat("FPS: %i", GetFPS()), screenWidth - 150, 50, 18, RED);
	}



}

int planetIndex = 0;

//THIS FUNCTION BELOW CALCULATES BARNES HUT FORCES
bool isFourierSpaceEnabled = true;

Vector2 calculateForceFromGrid(Planet& planet, const Quadtree& grid) {

	Vector2 totalForce = { 0.0f, 0.0f };

	if (grid.gridMass <= 0) return totalForce;




	// Calculate gravitational force between A and B
	float dx = grid.centerOfMass.x - planet.pos.x;

	if (isFourierSpaceEnabled) {
	if (dx > screenWidth / 2)
		dx -= screenWidth;
	else if (dx < -screenWidth / 2)
		dx += screenWidth;
	}

	float dy = grid.centerOfMass.y - planet.pos.y;

	if (isFourierSpaceEnabled) {
	if (dy > screenHeight / 2)
		dy -= screenHeight;
	else if (dy < -screenHeight / 2)
		dy += screenHeight;
	}

	float distanceSq = dx * dx + dy * dy;

	// Avoid division by zero
	if (distanceSq < 5.0f) distanceSq = 90.0f;
	float distance = sqrt(distanceSq);

	float s_over_d = grid.size / distance;
	const float theta = 0.5f;

	if (s_over_d < theta || grid.subGrids.empty()) {

		if (grid.myPlanets.size() == 1 &&
			fabs(grid.myPlanets[0].pos.x - planet.pos.x) < 0.001f &&
			fabs(grid.myPlanets[0].pos.y - planet.pos.y) < 0.001f) {
			return totalForce;
		}
		double force = G * planet.mass * grid.gridMass / distanceSq;
		totalForce.x = (dx / distance) * force;
		totalForce.y = (dy / distance) * force;

		//std::cout << neighborCount << '\n';
		// TEST THIS IN A FUTURE FOR PRESSURE
		/*if (distanceSq < 300.0f) {
			totalForce.x = -totalForce.x * 0.6;
			totalForce.y = -totalForce.y * 0.6;
		}*/


	}
	else {
		for (const Quadtree& subGrid : grid.subGrids) {
			Vector2 childForce = calculateForceFromGrid(planet, subGrid);
			totalForce.x += childForce.x;
			totalForce.y += childForce.y;
		}
	}
	return totalForce;
}



// TRY TO OPTIMIZE THIS LATER. POSSIBLE SOLUTION POINTERS IN QUADTREE CLASS VECTOR PLANETS
float densityRadius = 7;
int maxNeighbors = 20;
void updateDensityColors(const Quadtree& qt, std::vector<Planet>& planets, float densityRadius, int maxNeighbors) {

	float densityRadiusSq = densityRadius * densityRadius;

	for (Planet& planet : planets) {

		Rectangle range;
		range.x = planet.pos.x - densityRadius;
		range.y = planet.pos.y - densityRadius;
		range.width = densityRadius * 2;
		range.height = densityRadius * 2;

		std::vector<Planet*> neighbors = qt.queryRange(range);
		int neighborCount = 0;
		for (Planet* other : neighbors) {
			if (other == &planet) continue;

			float dx = planet.pos.x - other->pos.x;
			float dy = planet.pos.y - other->pos.y;
			if (dx * dx + dy * dy < densityRadiusSq) {
				neighborCount++;
			}
		}

		float normalDensity = std::min((float)neighborCount / (float)maxNeighbors, 1.0f);

		planet.color.r = static_cast<unsigned char>(normalDensity * 255);
		planet.color.g = static_cast<unsigned char>(normalDensity * 140);
		planet.color.b = static_cast<unsigned char>(60);
		planet.color.a = static_cast<unsigned char>(90);


	}

}

//THIS FUNCTION BELOW IS USED TO CALCULATE DARK MATTER

struct DarkMatterHalo {
	Vector2 pos;      // Position of the halo center
	double mass;      // Mass of the halo (e.g. 10^19)
	double radius;    // Scale radius parameter of the halo

	// Constructor with default values
	DarkMatterHalo(Vector2 position = { 0.0f, 0.0f },
		double m = 1e19,
		double r = 200.0)
		: pos(position), mass(m), radius(r) {
	}
};

Vector2 darkMatterForce(const Planet& planet) {
	float centerX = screenWidth / 2.0f;
	float centerY = screenHeight / 2.0f;

	float dx = planet.pos.x - centerX;
	float dy = planet.pos.y - centerY;
	float radius = sqrt(dx * dx + dy * dy);
	if (radius < 1.0f) radius = 1.0f;

	// NFW parameters
	const double haloMass = 7e17;      // Total halo mass
	const double haloRadius = 650.0;    // Scale radius
	const double G = 6.674e-11;         // Gravitational constant

	// NFW enclosed mass formula
	double concentration = 10;        // Typical for galaxies
	double r_ratio = radius / haloRadius;
	double M_enclosed = haloMass * (log(1 + r_ratio) - r_ratio / (1 + r_ratio))
		/ (log(1 + concentration) - concentration / (1 + concentration));

	// Newtonian acceleration
	double acceleration = (G * M_enclosed) / (radius * radius);

	Vector2 force;
	force.x = -(dx / radius) * acceleration * planet.mass; // Force = mass * acceleration
	force.y = -(dy / radius) * acceleration * planet.mass;

	return force;
}

int threadsAmount = 14;
ThreadPool pool(threadsAmount);
bool isMultiThreadingEnabled = true;

bool barnesHutEnabled = true;
bool isDarkMatterEnabled = false;

void updateForcesMultithreading(std::vector<Planet>& planets, const Quadtree& grid,
	size_t start, size_t end, float fixedDeltaTime) {
	for (size_t i = start; i < end; ++i) {
		Planet& planet = planets[i];
		// Reset acceleration.
		planet.acceleration = { 0.0f, 0.0f };

		// Calculate net gravitational force.
		Vector2 netForce = calculateForceFromGrid(planet, grid);

		// Optionally add dark matter force.
		if (isDarkMatterEnabled) {
			Vector2 dmForce = darkMatterForce(planet);
			netForce.x += dmForce.x;
			netForce.y += dmForce.y;
		}

		// Update acceleration from net force.
		planet.acceleration.x = netForce.x / planet.mass;
		planet.acceleration.y = netForce.y / planet.mass;

		// Update velocity using your integration scheme.
		planet.velocity.x += fixedDeltaTime * ((3.0f / 2.0f)) * planet.acceleration.x - ((1.0f / 2.0f)) * planet.prevAcceleration.x;
		planet.velocity.y += fixedDeltaTime * ((3.0f / 2.0f)) * planet.acceleration.y - ((1.0f / 2.0f)) * planet.prevAcceleration.y;
	}
}



void updateScene(std::vector<Planet>& planets) {

	const float fixedDeltaTime = 0.02f;

	float timeStepMultiplier = 1;
	float deltaTime = GetFrameTime() * timeStepMultiplier;

	if (IsKeyPressed(KEY_F) && isFourierSpaceEnabled) {
		isFourierSpaceEnabled = false;
	}
	else if (IsKeyPressed(KEY_F) && !isFourierSpaceEnabled) {
		isFourierSpaceEnabled = true;
	}

	Quadtree grid = gridFunction(planets);


	Slingshot slingshot = slingshotObject.planetSlingshot(isDragging, isMouse0Pressed, isMouse2SpacePressed);

	

	if (IsMouseButtonReleased(0) && !IsKeyDown(KEY_SPACE)) {
		Planet planet = createPlanet();
		planets.push_back(planet);
		isDragging = false;
	}

	if (IsMouseButtonDown(2)) {
		Planet planet = createPlanet();
		for (int i = 0; i < 70; i++) {
			planets.emplace_back(
				GetMouseX() + rand() % 50 - 25,
				GetMouseY() + rand() % 50 - 25,
				1,
				0,
				0,
				2000000000000,
				128,
				128,
				128,
				100,
				0,
				0,
				false
			);
		}
	}

	if (IsMouseButtonPressed(1) && !isDragging) {
		Planet planet = createPlanet();
		for (int i = 0; i < 4000; i++) {
			float galaxyCenterX = 500;
			float galaxyCenterY = 500;

			float angle = static_cast<float>(rand()) / RAND_MAX * 2 * PI;
			float radius = static_cast<float>(rand()) / RAND_MAX * 200.0f + 8;

			float posX = galaxyCenterX + radius * cos(angle);
			float posY = galaxyCenterY + radius * sin(angle);

			float dx = posX - galaxyCenterX;
			float dy = posY - galaxyCenterY;

			float normalRadius = 10 / radius;

			float angularSpeed = 130 / (radius + 60);

			float velocityX = -dy * angularSpeed;
			float velocityY = dx * angularSpeed;

			planets.emplace_back(
				posX, posY,
				2, // Size
				velocityX, velocityY,
				50000000000, // Mass
				128, 128, 128, 100, // Color (RGBA)
				0, 0, // Acceleration
				false
			);
		}
	}

	if (IsMouseButtonReleased(0) && isDragging || IsKeyReleased(KEY_SPACE) && isDragging) {
		Planet planet = createPlanet();
		for (int i = 0; i < 3000; i++) {

			float galaxyCenterX = GetMouseX();
			float galaxyCenterY = GetMouseY();

			float angle = static_cast<float>(rand()) / RAND_MAX * 2 * PI;
			float radius = static_cast<float>(rand()) / RAND_MAX * 100.0f + 2;

			float posX = galaxyCenterX + radius * cos(angle);
			float posY = galaxyCenterY + radius * sin(angle);

			float dx = posX - galaxyCenterX;
			float dy = posY - galaxyCenterY;

			float normalRadius = 10 / radius;

			float angularSpeed = 60 / (radius + 60);

			float velocityX = -dy * angularSpeed;
			float velocityY = dx * angularSpeed;

			planets.emplace_back(
				posX,
				posY,
				1,
				velocityX + (slingshot.normalizedX * slingshot.length),
				velocityY + (slingshot.normalizedY * slingshot.length),
				340000000000,
				128,
				128,
				128,
				100,
				0,
				0,
				false
			);
			isDragging = false;
		}
	}

	if (IsKeyPressed(KEY_G)) {
		Planet planet = createPlanet();
		for (int i = 0; i < 10000; i++) {
			planets.emplace_back(rand() % screenWidth,
				rand() % screenHeight,
				1,
				0,
				0,
				5000000000000,
				128,
				128,
				128,
				100,
				0,
				0,
				false
			);
		}
	}

	if (isMultiThreadingEnabled) {
		size_t numPlanets = planets.size();
		size_t numThreads = threadsAmount; // same as our pool size.
		size_t chunkSize = (numPlanets + numThreads - 1) / numThreads;

		std::vector<std::future<void>> futures;

		// Enqueue tasks for each segment.
		for (size_t t = 0; t < numThreads; ++t) {
			size_t start = t * chunkSize;
			size_t end = std::min(start + chunkSize, numPlanets);
			futures.emplace_back(pool.enqueue(updateForcesMultithreading, std::ref(planets), std::cref(grid), start, end, fixedDeltaTime));
		}

		// Wait for all tasks to complete.
		for (auto& f : futures) {
			f.get();
		}
	}
	else {

	if (barnesHutEnabled) {
		for (Planet& planet : planets) {
			planet.acceleration = { 0.0f, 0.0f };

			//planet.prevAcceleration = planet.acceleration;

			Vector2 netForce = calculateForceFromGrid(planet, grid);
			if (isDarkMatterEnabled) {
			Vector2 dmForce = darkMatterForce(planet);
			netForce.x += dmForce.x;
			netForce.y += dmForce.y;

			}




			//float fx = (dx / distance) * force;
			//float fy = (dy / distance) * force;
			planet.acceleration.x = netForce.x / planet.mass;
			planet.acceleration.y = netForce.y / planet.mass;



			planet.velocity.x += fixedDeltaTime * ((3.0f / 2.0f)) * planet.acceleration.x - ((1.0f / 2.0f)) * planet.prevAcceleration.x;
			planet.velocity.y += fixedDeltaTime * ((3.0f / 2.0f)) * planet.acceleration.y - ((1.0f / 2.0f)) * planet.prevAcceleration.y;
			//planet.velocity.x += deltaTime * planet.acceleration.x;
			//.velocity.y += deltaTime * planet.acceleration.y;

		}
	}
	else {
		// Iterate over unique pairs of planets
		for (size_t i = 0; i < planets.size(); ++i) {
			for (size_t j = i + 1; j < planets.size(); ++j) {
				Planet& planetA = planets[i];
				Planet& planetB = planets[j];


				// Calculate gravitational force between A and B
				float dx = planetB.pos.x - planetA.pos.x;
				if (dx > screenWidth / 2)
					dx -= screenWidth;
				else if (dx < -screenWidth / 2)
					dx += screenWidth;

				float dy = planetB.pos.y - planetA.pos.y;
				if (dy > screenHeight / 2)
					dy -= screenHeight;
				else if (dy < -screenHeight / 2)
					dy += screenHeight;
				float distanceSq = dx * dx + dy * dy;

				// Avoid division by zero
				if (distanceSq < 200) continue;

				float distance = sqrt(distanceSq);
				float force = G * planetA.mass * planetB.mass / distanceSq; // Total force magnitude


				//planetB.proximity = force;


				// Normalize direction
				float fx = (dx / distance) * force;
				float fy = (dy / distance) * force;

				float accelPlanetAX = fx / planetA.mass;
				float accelPlanetAY = fy / planetA.mass;


				float accelPlanetBX = fx / planetB.mass;
				float accelPlanetBY = fy / planetB.mass;

				planetA.velocity.x += accelPlanetAX * deltaTime;
				planetA.velocity.y += accelPlanetAY * deltaTime;

				planetB.velocity.x -= accelPlanetBX * deltaTime;
				planetB.velocity.y -= accelPlanetBY * deltaTime;

			}
		}

	}
	}
	for (Planet& planet : planets) {
		planet.pos.x += planet.velocity.x * fixedDeltaTime;
		planet.pos.y += planet.velocity.y * fixedDeltaTime;
		//planet.pos.x += planet.velocity.x * deltaTime;
		//planet.pos.y += planet.velocity.y * deltaTime;

		//  THIS IS THE INFINITE SPACE CODE
		// 
		if (isFourierSpaceEnabled) {
		if (planet.pos.x < 0)
			planet.pos.x += screenWidth;
		else if (planet.pos.x >= screenWidth)
			planet.pos.x -= screenWidth;
		if (planet.pos.y < 0)
			planet.pos.y += screenHeight;
		else if (planet.pos.y >= screenHeight)
			planet.pos.y -= screenHeight;

		}

		// Wrap Y coordinate

	}

	//updateDensityColors(grid, planets, 7.0f, 20);







	 //PROXIMITY COLOR CODE

	

	slingshotObject.planetSlingshot(isDragging, isMouse0Pressed, isMouse2SpacePressed);
}

void particlesColorVisuals(std::vector<Planet>& planets) {

	const float densityRadius = 7.0f;
	const float densityRadiusSq = densityRadius * densityRadius;  // Precompute squared radius
	const int maxNeighbors = 30;


	std::vector<int> neighborCounts(planets.size(), 0);


	for (size_t i = 0; i < planets.size(); i++) {
		for (size_t j = i + 1; j < planets.size(); j++) {
			float dx = planets[i].pos.x - planets[j].pos.x;
			float dy = planets[i].pos.y - planets[j].pos.y;
			if (dx * dx + dy * dy < densityRadiusSq) {
				neighborCounts[i]++;
				neighborCounts[j]++;
			}
		}
	}


	for (size_t i = 0; i < planets.size(); i++) {
		float normalDensity = std::min(float(neighborCounts[i]) / maxNeighbors, 1.0f);
		if (!planets[i].customColor) {
			planets[i].color.r = static_cast<unsigned char>(normalDensity * 255);
			planets[i].color.g = static_cast<unsigned char>(normalDensity * 140);
			planets[i].color.b = static_cast<unsigned char>(60);
			planets[i].color.a = static_cast<unsigned char>(100);
		}
	}
}

void particlesColorVisualsParallel(std::vector<Planet>& planets) {
	// Set up constants.
	const float densityRadius = 7.0f;
	const float densityRadiusSq = densityRadius * densityRadius;
	const int maxNeighbors = 30;

	// Get the number of planets.
	size_t n = planets.size();

	// Choose how many threads to use (e.g. threadsAmount is a global or configured value).
	int numThreads = threadsAmount; // e.g., 8

	// Calculate the chunk size (each thread will process a contiguous block of indices for the outer loop).
	size_t chunkSize = (n + numThreads - 1) / numThreads;

	// Create a vector to hold local neighbor counts for each thread.
	// Each thread will maintain a vector of size 'n', all initialized to 0.
	std::vector<std::vector<int>> localCounts(numThreads, std::vector<int>(n, 0));

	// This vector will hold futures for each enqueued task.
	std::vector<std::future<void>> futures;

	// Enqueue tasks into the thread pool.
	for (int t = 0; t < numThreads; t++) {
		// Determine the range for the outer loop.
		size_t start = t * chunkSize;
		size_t end = std::min(start + chunkSize, n);

		// Capture 'start', 'end', and thread index 't' by value.
		// Also capture planets and localCounts by reference.
		futures.emplace_back(
			pool.enqueue([start, end, t, n, densityRadiusSq, &planets, &localCounts]() {
				// For each planet in our assigned range...
				for (size_t i = start; i < end; i++) {
					// Compare with every planet j > i (to avoid duplicate pairs).
					for (size_t j = i + 1; j < n; j++) {
						float dx = planets[i].pos.x - planets[j].pos.x;
						float dy = planets[i].pos.y - planets[j].pos.y;
						// If the squared distance is below our threshold...
						if (dx * dx + dy * dy < densityRadiusSq) {
							// Increment the count for both i and j in this thread’s local array.
							localCounts[t][i]++;
							localCounts[t][j]++;
						}
					}
				}
				})
		);
	}

	// Wait for all threads to finish processing.
	for (auto& f : futures) {
		f.get();
	}

	// Now reduce (sum) all the local neighbor counts into one global vector.
	std::vector<int> neighborCounts(n, 0);
	for (int t = 0; t < numThreads; t++) {
		for (size_t i = 0; i < n; i++) {
			neighborCounts[i] += localCounts[t][i];
		}
	}

	// Finally, update the color for each planet based on the neighbor count.
	for (size_t i = 0; i < n; i++) {
		float normalDensity = std::min(float(neighborCounts[i]) / maxNeighbors, 1.0f);
		// Only update the color if the planet doesn't have a custom color.
		if (!planets[i].customColor) {
			planets[i].color.r = static_cast<unsigned char>(normalDensity * 255);
			planets[i].color.g = static_cast<unsigned char>(normalDensity * 140);
			planets[i].color.b = static_cast<unsigned char>(60);
			planets[i].color.a = static_cast<unsigned char>(100);
		}
	}
}

bool trailsEnabled = false;
int trailDotFrameIndex = 0;
void mouseTrail(std::vector<MouseTrailDot>& dots, std::vector<Planet>& planets) {
	// Add new dots for all planets
	const int NUM_PLANETS = planets.size();

	if (IsKeyPressed(KEY_T) && trailsEnabled) {
		trailsEnabled = false;
		dots.clear();
	}
	else if (IsKeyPressed(KEY_T) && !trailsEnabled) {
		trailsEnabled = true;
	}



	if (trailsEnabled) {
		for (const Planet& planet : planets) {
			dots.push_back(MouseTrailDot(planet.pos.x, planet.pos.y, 1));

		}

	}
	trailDotFrameIndex = 0;

	// Calculate how many dots to remove to stay at MAX_DOTS
	const int MAX_DOTS = 14 * planets.size();
	if (dots.size() > MAX_DOTS) {
		int excess = dots.size() - MAX_DOTS;
		dots.erase(dots.begin(), dots.begin() + excess); // Remove oldest `excess` dots
	}
	if (IsKeyPressed(KEY_C)) {
		planets.clear();
		dots.clear();
	}



}





int main() {


	int planetPosX = 0;
	int planetPosY = 0;
	int planetSize = 0;

	std::vector<Planet> vectorPlanet;


	std::vector<MouseTrailDot> trailDots;








	InitWindow(screenWidth, screenHeight, "n-Body");



	SetTargetFPS(targetFPS);

	while (!WindowShouldClose()) {


		BeginDrawing();




		ClearBackground(BLACK);



		BeginBlendMode(1);

		mouseTrail(trailDots, vectorPlanet);


		drawScene(vectorPlanet, trailDots);

		EndBlendMode();

		//std::thread updateSceneThread(updateScene, std::ref(vectorPlanet));
		//std::thread particlesColorThread(particlesColorVisuals, std::ref(vectorPlanet));

		// Join threads to ensure they finish before the next frame or exit
		//updateSceneThread.join();
		//particlesColorThread.join();

		updateScene(vectorPlanet);

		if (isMultiThreadingEnabled) {
		particlesColorVisualsParallel(vectorPlanet);

		}
		else {
		particlesColorVisuals(vectorPlanet);

		}
		




		EndDrawing();


	}

	CloseWindow();



	return 0;
}