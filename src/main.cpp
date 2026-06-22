#include <SFML/Graphics.hpp>
#include <optional>
#include <iostream>

int main()
{
    // Create the main window with modern SFML 3 sf::VideoMode constructor
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML 3 Test Window");
    window.setFramerateLimit(60);

    // Create a green circle shape to test graphics rendering
    sf::CircleShape circle(100.f);
    circle.setFillColor(sf::Color::Green);
    
    // Set position to center the circle
    circle.setPosition({300.f, 200.f});

    std::cout << "Starting SFML 3 game loop..." << std::endl;

    // Main window loop
    while (window.isOpen())
    {
        // Modern SFML 3 event loop using std::optional
        while (const std::optional event = window.pollEvent())
        {
            // Check for window close event
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            // Check for escape key pressed
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                {
                    window.close();
                }
            }
        }

        // Render frame
        window.clear(sf::Color::Black);
        window.draw(circle);
        window.display();
    }

    std::cout << "SFML 3 window closed. Exit successful." << std::endl;
    return 0;
}
