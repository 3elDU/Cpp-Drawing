// Including all libraries
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>


// No need to explain
int main() {

	// Mode prompt
	std::cout << "s: Server, c: Client" << std::endl;
	char mode;
	std::cin >> mode;


	// This side coordinates
	int thisSideX = 50;
	int thisSideY = 50;

	
	// Other's side coordinates
	int thatSideX = 50;
	int thatSideY = 50;


	// For server
	sf::TcpListener server;

	// For client / server connection
	sf::TcpSocket socket;


	// Starting server/client depending on mode you choose
	if (mode == 's') {
		server.listen(25565);

		server.accept(socket);

		socket.setBlocking(false);
	}
	else if (mode == 'c') {

		// Server ip prompt
		std::string serverIP;
		std::cout << "Server ip: "; std::cin >> serverIP;

		sf::Socket::Status status = socket.connect(serverIP, 25565);
		if (status != sf::Socket::Done) {
			return 2;
		}
		sf::Packet packet;
		packet << thisSideX << thisSideY;
		socket.send(packet);
		packet.clear();

		socket.setBlocking(false);
	}
	else {
		std::cout << "Invalid mode!" << std::endl;
		return 1;
	}


	// Creating window
	sf::RenderWindow window(sf::VideoMode(500, 500), "");

	
	// Coordinate from previous frame ( for optimization purposes )
	int prevx = 50, prevy = 50;
	int thatprevx = 50, thatprevy = 50;

	// Moving speed
	int speed = 3;

	// Creating timer to measure time
	// Because player on other side will move in 20 fps, while we will be in 60 fps.
	sf::Clock timer;

	// Yeah, multiplayer drawing.
	sf::RenderTexture texture;
	texture.create(500, 500);
	texture.clear();

	bool draw = false;
	bool erasing = false;
	bool thatSideDraw = false;
	bool thatSideErasing = false;

	std::tuple<int, int, bool, bool> prevFrame = { thisSideX, thisSideY, draw, erasing };
	
	// Main loop
	while (window.isOpen()) {

		// Evevnt loop. Checking, is the "close window" button pressed
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::F) {
					draw = !draw;
					erasing = false;
				}
				else if (event.key.code == sf::Keyboard::E) {
					draw = false;
					erasing = !erasing;
				}
			}
		}

		// Checking keys ( obly if this window is active )
		if (window.hasFocus()) {
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
				thisSideY -= speed;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
				thisSideX -= speed;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
				thisSideY += speed;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
				thisSideX += speed;
			}
		}

		// Sending our coordinates to other side
		if (timer.getElapsedTime().asMilliseconds() >= 33) {
			std::tuple<int, int, bool, bool> curFrame = {thisSideX, thisSideY, draw, erasing};
			if (curFrame != prevFrame) {
				sf::Packet packet;
				packet << thisSideX << thisSideY << draw << erasing;
				socket.send(packet);
			}
			timer.restart();
		}

		// Receiving other's side coordinates
		sf::Packet packet;
		socket.receive(packet);
		packet >> thatSideX >> thatSideY >> thatSideDraw >> thatSideErasing;

		// Clearing window
		window.clear();

		// Displaying lines
		if (draw) {
			sf::VertexArray lines(sf::Lines, 2);
			lines[0].position = sf::Vector2f(prevx+25, prevy+25);
			lines[0].color = sf::Color::White;
			lines[1].position = sf::Vector2f(thisSideX+25, thisSideY+25);
			lines[1].color = sf::Color::White;
			texture.draw(lines);
		}
		else if (erasing) {
			sf::CircleShape circle(20);
			circle.setFillColor(sf::Color::Black);
			circle.setPosition(thisSideX, thisSideY);
			texture.draw(circle);
		}
		if (thatSideDraw) {
			sf::VertexArray lines(sf::Lines, 2);
			lines[0].position = sf::Vector2f(thatprevx+25, thatprevy+25);
			lines[0].color = sf::Color::Green;
			lines[1].position = sf::Vector2f(thatSideX+25, thatSideY+25);
			lines[1].color = sf::Color::Green;
			texture.draw(lines);
		}
		else if (thatSideErasing) {
			sf::CircleShape circle(20);
			circle.setFillColor(sf::Color::Black);
			circle.setPosition(thatSideX, thatSideY);
			texture.draw(circle);
		}
		texture.display();
		sf::Sprite spr(texture.getTexture());
		window.draw(spr);

		// Displaying our "players"
		sf::CircleShape thisSide(20);
		thisSide.setPosition(thisSideX, thisSideY);
		if (erasing) {
			thisSide.setFillColor(sf::Color(255, 0, 0, 128));
		}
		else {
			thisSide.setFillColor(sf::Color(255, 255, 255, 128));
		}
		window.draw(thisSide);
		
		sf::CircleShape otherSide(20);
		thisSide.setPosition(thatSideX, thatSideY);
		if (thatSideErasing) {
			thisSide.setFillColor(sf::Color(255, 0, 0, 128));
		}
		else {
			thisSide.setFillColor(sf::Color(0, 255, 0, 128));
		}
		window.draw(thisSide);

		// Updating screen
		window.display();

		// Saving this frame coordinates for the next frame ( for optimization purposes )
		prevx = thisSideX;
		prevy = thisSideY;
		
		thatprevx = thatSideX;
		thatprevy = thatSideY;

		prevFrame = {thisSideX, thisSideY, draw, erasing};

		// Game will be in 60 FPS.
		sf::sleep(sf::milliseconds(16));
	}

	system("pause");
	
	return 0;
}