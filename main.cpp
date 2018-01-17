#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <atomic>
#include <cassert>
#include <mutex>
#include <queue>
#include <condition_variable>


// Useful to hold onto the memory when converting it into a Chunk.
struct Samples {
    Samples(sf::Int16 const* ss, std::size_t count) {
        samples.reserve(count);
        std::copy_n(ss, count, std::back_inserter(samples));
    }

    Samples() {}

    std::vector<sf::Int16> samples;
};


class PlaybackRecorder : private sf::SoundRecorder, private sf::SoundStream {
public: /** API **/

    // Initialise capturing input & setup output
    void start() {
        sf::SoundRecorder::start();

        sf::SoundStream::initialize(sf::SoundRecorder::getChannelCount(), sf::SoundRecorder::getSampleRate());
        sf::SoundStream::play();
    }

    // Stop both recording & playback
    void stop() {
        sf::SoundRecorder::stop();
        sf::SoundStream::stop();
    }

    bool isRunning() { return isRecording; }


    ~PlaybackRecorder() {
        stop();
    }


protected: /** OVERRIDING SoundRecorder **/

    bool onProcessSamples(sf::Int16 const* samples, std::size_t sampleCount) override {
        {
            std::lock_guard<std::mutex> lock(mutex);
            data.emplace(samples, sampleCount);
        }
        cv.notify_one();
        return true; // continue capture
    }

    bool onStart() override {
        isRecording = true;
        return true;
    }

    void onStop() override {
        isRecording = false;
        cv.notify_one();
    }


protected: /** OVERRIDING SoundStream **/

    bool onGetData(Chunk& chunk) override {
        // Wait until either:
        //  a) the recording was stopped
        //  b) new data is available
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this]{ return !isRecording || !data.empty(); });

        // Lock was acquired, examine which case we're into:
        if (!isRecording) return false; // stop playing.
        else {
            assert(!data.empty());

            playingSamples.samples = std::move(data.front().samples);
            data.pop();
            chunk.sampleCount = playingSamples.samples.size();
            chunk.samples = playingSamples.samples.data();
            return true;
        }
    }

    void onSeek(sf::Time) override { /* Not supported, silently does nothing. */ }

private:
    std::atomic<bool> isRecording{false};
    std::mutex mutex; // protects `data`
    std::condition_variable cv; // notify consumer thread of new samples
    std::queue<Samples> data; // samples come in from the recorder, and popped by the output stream
    Samples playingSamples; // used by the output stream.
};

int main(int, char const**)
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML PlayBack");

    if (!sf::SoundRecorder::isAvailable()) {
        return EXIT_FAILURE;
    }

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

            if (event.type == sf::Event::KeyPressed) {
                if (input.isRunning()) input.stop();
                else input.start();
            }
        }

        window.clear(input.isRunning() ? sf::Color::White : sf::Color::Black);
        window.display();
    }

    return EXIT_SUCCESS;
}