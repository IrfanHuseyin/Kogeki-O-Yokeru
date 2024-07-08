#include <SFML/Audio.hpp> 
#include <SFML/Graphics.hpp>
#include <vector>
#include <chrono>
#include <random>
#include <iostream>
#include <sstream>

const int width = 960;
const int height = 540;

void moveParticle(sf::CircleShape& particle, float speed, float delta)
{
    particle.setPosition(particle.getPosition().x - speed * delta, particle.getPosition().y);
    if (particle.getPosition().x < -particle.getRadius())
    {
        particle.setPosition(width + particle.getRadius(), static_cast<float>(rand() % height));
    }
}

void drawParticle(sf::RenderWindow& window, const sf::CircleShape& particle)
{
    window.draw(particle);
}

int main()
{
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode(width, height), "Avoid Attack By IF", sf::Style::Default, settings);
    window.setFramerateLimit(60);
    window.setMouseCursorVisible(false);

    sf::Font font;
    if (!font.loadFromFile("res/space.ttf"))
    {
        std::cout << "Error loading font file!" << std::endl;
        return -1;
    }

    const int textSize = 30;
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(textSize);
    text.setFillColor(sf::Color::Red);
    text.setPosition(width / 10, 10);

    sf::Text text2;
    text2.setFont(font);
    text2.setCharacterSize(textSize);
    text2.setFillColor(sf::Color::Green);
    text2.setPosition(10.f, 10.f);

    int playerScore = 0;

    sf::ConvexShape player(3); // Create a triangle shape with 3 vertices
    player.setPoint(0, sf::Vector2f(0, -20)); // Top vertex
    player.setPoint(1, sf::Vector2f(80, 20)); // Bottom right vertex
    player.setPoint(2, sf::Vector2f(0, 50)); // Bottom left vertex
    player.setFillColor(sf::Color::Green);
    player.setOutlineColor(sf::Color::White);
    player.setOutlineThickness(2.f);
    player.setPosition(width / 2, height / 2);

    std::vector<sf::CircleShape> balls;
    std::vector<sf::CircleShape> bullets;

    bool isMovingLeft = false;
    bool isMovingRight = false;
    bool isMovingUp = false;
    bool isMovingDown = false;
    bool isFiring = false;
    bool isGameRunning = true; // Flag to control game running state
    bool isCollision = false; // Flag to indicate collision
    bool isMovingBalls = true; // Flag to control ball movement

    auto lastBallTime = std::chrono::steady_clock::now();
    const std::chrono::milliseconds ballInterval(100);

    auto lastBulletTime = std::chrono::steady_clock::now();
    const std::chrono::milliseconds bulletInterval(100);

    // Sound buffer
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile("res/sounds/sound_shoot.wav"))
    {
        std::cout << "Error loading the sound file! " << std::endl;
        return -1;
    }
    sf::Sound shootSound;
    shootSound.setBuffer(buffer);

    // Sound buffer for shooting balls
    sf::SoundBuffer bulletHitBuffer;
    if (!bulletHitBuffer.loadFromFile("res/sounds/sound_pop.wav"))
    {
        std::cout << "Error loading the ball shoot sound file!" << std::endl;
        return -1;
    }
    sf::Sound bulletHitSound;
    bulletHitSound.setBuffer(bulletHitBuffer);

    std::vector<sf::CircleShape> particles;
    srand(static_cast<unsigned>(time(0)));

    for (int i = 0; i < 200; ++i) {
        float speed = static_cast<float>(rand() % 50) / 10.0f + 1.0f;
        sf::Vector2f position(static_cast<float>(rand() % 800), static_cast<float>(rand() % 600));
        float size = static_cast<float>(rand() % 2) + 1.0f;
        sf::Uint8 shade = rand() % 128 + 128; // Random value between 128 and 255
        sf::Color color(shade, shade, shade); // Gray color
        sf::CircleShape particle(size);
        particle.setPosition(position);
        particle.setFillColor(color);
        particles.push_back(particle);
    }

    sf::Text gameOverText("Game Over", font, 70);
    gameOverText.setFillColor(sf::Color::Yellow);
    gameOverText.setPosition(width / 2 - gameOverText.getLocalBounds().width / 2, height / 2 - gameOverText.getLocalBounds().height / 2);

    sf::Text retryText("Retry (R)", font, 30);
    retryText.setFillColor(sf::Color::Magenta);
    retryText.setPosition(width / 2 - retryText.getLocalBounds().width / 2, height / 2 + 50);

    sf::Text quitText("Quit (esc)", font, 30);
    quitText.setFillColor(sf::Color::Magenta);
    quitText.setPosition(width / 2 - quitText.getLocalBounds().width / 2, height / 2 + 100);

    while (window.isOpen() && !sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                {
                    window.close();
                }
                else if (event.key.code == sf::Keyboard::R && !isGameRunning)
                {
                    // Retry the game
                    isGameRunning = true;
                    isCollision = false;
                    isMovingBalls = true;
                    player.setPosition(width / 2, height / 2);
                    balls.clear();
                    bullets.clear();
                    lastBallTime = std::chrono::steady_clock::now();
                    lastBulletTime = std::chrono::steady_clock::now();
                    playerScore = 0; // Reset Score
                }
                else if (event.key.code == sf::Keyboard::Q && !isGameRunning)
                {
                    // Quit the game
                    window.close();
                }
                else if (event.key.code == sf::Keyboard::Left)
                {
                    isMovingLeft = true;
                }
                else if (event.key.code == sf::Keyboard::Right)
                {
                    isMovingRight = true;
                }
                else if (event.key.code == sf::Keyboard::Up)
                {
                    isMovingUp = true;
                }
                else if (event.key.code == sf::Keyboard::Down)
                {
                    isMovingDown = true;
                }
                else if (event.key.code == sf::Keyboard::Space)
                {
                    isFiring = true;
                }
            }
            else if (event.type == sf::Event::KeyReleased)
            {
                if (event.key.code == sf::Keyboard::Left)
                {
                    isMovingLeft = false;
                }
                else if (event.key.code == sf::Keyboard::Right)
                {
                    isMovingRight = false;
                }
                else if (event.key.code == sf::Keyboard::Up)
                {
                    isMovingUp = false;
                }
                else if (event.key.code == sf::Keyboard::Down)
                {
                    isMovingDown = false;
                }
                else if (event.key.code == sf::Keyboard::Space)
                {
                    isFiring = false;
                }
            }
        }

        if (isGameRunning)
        {
            // Left and Right Borders
            if (isMovingLeft && player.getPosition().x > 0)
            {
                player.move(-5.f, 0.f);
            }
            if (isMovingRight && player.getPosition().x < window.getSize().x - 85)
            {
                player.move(5.f, 0.f);
            }

            // Up and Down Movements
            if (isMovingUp && player.getPosition().y <= -50)
            {
                player.setPosition(player.getPosition().x, window.getSize().y - -20);
            }
            else if (isMovingUp)
            {
                player.move(0.f, -5.f);
            }

            if (isMovingDown && player.getPosition().y >= window.getSize().y - -20)
            {
                player.setPosition(player.getPosition().x, -50);
            }
            else if (isMovingDown)
            {
                player.move(0.f, 5.f);
            }

            // Update game state
            float delta = 0.5f; // Delta time for particle movement
            for (auto& particle : particles)
            {
                moveParticle(particle, particle.getRadius(), delta); // Move particle with its own speed
            }

            auto currentTime = std::chrono::steady_clock::now();
            if (isFiring && currentTime - lastBulletTime > bulletInterval)
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
                {
                    shootSound.play();

                    sf::CircleShape bullet(3.f);
                    bullet.setFillColor(sf::Color::Red);
                    bullet.setOutlineColor(sf::Color::White);
                    bullet.setOutlineThickness(1.f);
                    bullet.setPosition(player.getPosition().x + player.getPoint(1).x, player.getPosition().y + player.getPoint(1).y);
                    bullets.push_back(bullet);
                }

                lastBulletTime = currentTime;
            }

            if (currentTime - lastBallTime > ballInterval)
            {
                sf::CircleShape ball(20.f);
                ball.setFillColor(sf::Color::Red);
                ball.setOutlineColor(sf::Color::White);
                ball.setOutlineThickness(1.f);
                ball.setPosition(width, static_cast<float>(rand() % height));
                balls.push_back(ball);
                lastBallTime = currentTime;
            }

            // Check for collisions between player and balls
            if (!isCollision)
            {
                for (auto& ball : balls)
                {
                    sf::Vector2f ballCenter = ball.getPosition() + sf::Vector2f(ball.getRadius(), ball.getRadius());

                    // Check collision with each vertex of the player triangle
                    for (size_t i = 0; i < player.getPointCount(); ++i)
                    {
                        sf::Vector2f playerVertex = player.getTransform().transformPoint(player.getPoint(i));

                        // Calculate distance between ball center and player vertex
                        float dx = ballCenter.x - playerVertex.x;
                        float dy = ballCenter.y - playerVertex.y;
                        float distance = std::sqrt(dx * dx + dy * dy);

                        // If distance is less than ball radius, it's a collision
                        if (distance < ball.getRadius())
                        {
                            // Handle collision
                            isGameRunning = false;
                            isCollision = true;
                            isMovingBalls = false;
                            break;
                        }
                    }
                }
            }

            // Check for collision between bullets and balls
            for (auto bullet = bullets.begin(); bullet != bullets.end();)
            {
                bool bulletRemoved = false;

                for (auto ball = balls.begin(); ball != balls.end();)
                {
                    sf::Vector2f bulletCenter = (*bullet).getPosition() + sf::Vector2f((*bullet).getRadius(), (*bullet).getRadius());
                    sf::Vector2f ballCenter = (*ball).getPosition() + sf::Vector2f((*ball).getRadius(), (*ball).getRadius());
                    float dx = bulletCenter.x - ballCenter.x;
                    float dy = bulletCenter.y - ballCenter.y;
                    float distance = std::sqrt(dx * dx + dy * dy);

                    if (distance < (*bullet).getRadius() + (*ball).getRadius())
                    {
                        bulletHitSound.play();
                        bullet = bullets.erase(bullet);
                        ball = balls.erase(ball);
                        bulletRemoved = true;

                        // Increase Score
                        playerScore += 10;
                        break;
                    }
                    else
                    {
                        ++ball;
                    }
                }

                if (!bulletRemoved)
                {
                    ++bullet;
                }
            }

            if (!isGameRunning)
            {
                // Game over logic
                window.clear();
                window.draw(gameOverText);
                window.draw(retryText);
                window.draw(quitText);
                window.display();
            }
        }

        window.clear();

        // Draw game objects
        for (auto& particle : particles)
        {
            drawParticle(window, particle); // Draw particle
        }

        // Draw player
        window.draw(player);

        // Draw balls
        for (auto& ball : balls)
        {
            if (isMovingBalls)
            {
                ball.move(-10.f, 0.f); // Move the ball if the game is running
            }

            window.draw(ball);
        }

        // Draw bullets
        for (auto& bullet : bullets)
        {
            bullet.move(10.f, 0.f);
            window.draw(bullet);
        }

        // Erase balls that go off-screen
        balls.erase(std::remove_if(balls.begin(), balls.end(), [&](const sf::CircleShape& ball) {
            return ball.getPosition().x < -ball.getRadius();
            }), balls.end());

        // Erase bullets that go off-screen
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [&](const sf::CircleShape& bullet) {
            return bullet.getPosition().y < -bullet.getRadius();
            }), bullets.end());

        if (!isGameRunning) // If game is not running (i.e., frozen), continue drawing the current state
        {
            window.draw(gameOverText);
            window.draw(retryText);
            window.draw(quitText);
        }

        std::stringstream ss;
        ss << playerScore;
        text2.setString(ss.str());
        window.draw(text2);

        window.display();
    }

    return 0;

}
