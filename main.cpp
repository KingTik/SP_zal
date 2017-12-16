#include <iostream>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <vector>


class McirophoneStream: public sf::SoundStream{

    std::vector<sf::Int16> m_samples;
    std::size_t m_currentSample;

    public:
    void load(const sf::SoundBuffer& buffer){
        m_samples.assign(buffer.getSamples(), buffer.getSamples()+buffer.getSampleCount ());
        m_currentSample = 0;
        initialize(buffer.getChannelCount(), buffer.getSampleRate());

    }

    private:
    virtual bool onGetData(Chunk& data){

        const int samplesToStream = 50000;
        data.samples = &m_samples[m_currentSample];
        if (m_currentSample + samplesToStream <= m_samples.size())
        {
            // end not reached: stream the samples and continue
            data.sampleCount = samplesToStream;
            m_currentSample += samplesToStream;
            return true;
        }
        else
        {
            // end of stream reached: stream the remaining samples and stop playback
            data.sampleCount = m_samples.size() - m_currentSample;
            m_currentSample = m_samples.size();
            return false;
        }

    }

    virtual void onSeek(sf::Time timeOffset){
        // //left empty
    }
};



int main()
{



    std::vector<std::string> availableDevices = sf::SoundRecorder::getAvailableDevices();
    for(auto device: availableDevices ){
        std::cout << device << std::endl;
    }


    sf::SoundBuffer buffer;
    auto retVal = buffer.loadFromFile("groove.wav");
    std::cout << retVal <<std::endl;
    McirophoneStream stream;
    stream.load(buffer);
    stream.play();


    while(stream.getStatus() == McirophoneStream::Playing)
        sf::sleep(sf::seconds(0.1f));
   
}