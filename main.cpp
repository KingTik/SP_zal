#include <iostream>
// #include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <vector>

int main()
{



    std::vector<std::string> availableDevices = sf::SoundRecorder::getAvailableDevices();


     	
    for(auto device: availableDevices ){
        std::cout << device << std::endl;
    }

   
}