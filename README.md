#1 Setup
Install sfml:
    sudo apt-get install libsfml-dev

Install jack controll server:
    sudo apt install jackd2
After that you might want to restart PC

Side note:
If there is still problem try to type in terminal:
    sudo usermod -a -G audio user_name

#2 How to run
1. Start jack controll server by typing in terminal:
    jack_control start
2. In projects cataogue:
    make
3. Run executable file:
    ./sfml-app