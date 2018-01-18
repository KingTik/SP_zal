#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <cassert>
#include <mutex>
#include <queue>
#include <vector>
#include <condition_variable>

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


            assert(!data.empty());

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

int main(int, char const**)
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Add effect");
    sf::RectangleShape HighPitch(sf::Vector2f(60,60));
    HighPitch.setPosition(0, 62);
    sf::RectangleShape LowPitch(sf::Vector2f(60,60));

    std::vector<sf::RectangleShape> buttons = { HighPitch, LowPitch};

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



        }

        
        window.clear(sf::Color::Green);

        for(auto button: buttons){
            window.draw(button);
        }

        window.display();
    }


}