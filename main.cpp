#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <cassert>
#include <mutex>
#include <queue>
#include <vector>
#include <condition_variable>
#include <iostream>
#include <string>

struct Samples {
    std::vector<sf::Int16> samples;
    Samples() = default;
    
    Samples(sf::Int16 const* ss, std::size_t count) {
        samples.reserve(count);
        std::copy_n(ss, count, std::back_inserter(samples));
    }
};


class PlaybackRecorder : public sf::SoundRecorder, public sf::SoundStream {
    std::mutex mutex; 
    std::condition_variable cv; 
    std::queue<Samples> data; 
    Samples playingSamples; 

public: 

    void start() {
        sf::SoundRecorder::start();
        sf::SoundStream::initialize(sf::SoundRecorder::getChannelCount(), sf::SoundRecorder::getSampleRate());
        sf::SoundStream::play();
    }

    void stop() {
        sf::SoundRecorder::stop();
        sf::SoundStream::stop();
    }


    ~PlaybackRecorder() {
        stop();
    }


protected: 

    bool onProcessSamples(sf::Int16 const* samples, std::size_t sampleCount) override {
        {
            std::lock_guard<std::mutex> lock(mutex);
            data.emplace(samples, sampleCount);
        }
        cv.notify_one();
        return true; 
    }

    bool onStart() override {
        return true;
    }

    void onStop() override {
        cv.notify_one();
    }


    bool onGetData(Chunk& chunk) override {

        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this]{ return  !data.empty(); });

        playingSamples.samples = std::move(data.front().samples);
        data.pop();
        chunk.sampleCount = playingSamples.samples.size();
        chunk.samples = playingSamples.samples.data();
        return true;
        
    }

    void onSeek(sf::Time) override { 
        //left empty
     }


   

};

class Button : public sf::RectangleShape{
    std::string name = "";
    public:
    Button(sf::Vector2f XY){
        this->setSize(XY);
    }

    bool isPressed(int x, int y){

        auto bounds = this->getGlobalBounds();

        return bounds.contains(x,y);

    }

    void setName(std::string newName){
        this->name = newName;
    }

    auto getName() const{
        return this->name;
    }
};


int main(int, char const**)
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Add effect");
    Button HighPitch(sf::Vector2f(60,60));
    HighPitch.setName("HighPitch");
    HighPitch.setPosition(0, 80);
    Button LowPitch(sf::Vector2f(60,60));
    LowPitch.setPosition(80, 80);
    LowPitch.setName("LowPitch");

    std::vector<Button> buttons = { HighPitch, LowPitch};

    PlaybackRecorder input;
    input.start();
    

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if(event.type == sf::Event::MouseButtonPressed){
                if(event.mouseButton.button == sf::Mouse::Left){
                    // std::cout << "the left button was pressed" << std::endl;
                    // std::cout << "mouse x: " << event.mouseButton.x << std::endl;
                    // std::cout << "mouse y: " << event.mouseButton.y << std::endl;
                    for(auto but: buttons){
                        if(but.isPressed(event.mouseButton.x, event.mouseButton.y)){
                            std::cout <<"Clicked " << but.getName() <<std::endl;
                        }
                    }
                
                }


            }


        }

        
        window.clear(sf::Color::Green);

        for(auto button: buttons){
            window.draw(button);
        }

        window.display();
    }


}