
//*********************************for windows
//all the dll and library automaticly copy to the output build folder. 

//if build in linux and configure : please remove config.h and rename the configMS.h to config.h
//the config.h that is built with the ./configure has decoder preprocessor and the code will not compiled. 

mkdir build
cd build
cmake .. -G "Visual Studio 15 2017"

//run and compile and build with visual sutdio 


//*********************************for linux
//for linux the GCC version should be 8+ to support filesystem library in 
//std++ 17

mkdir build
cd build
cmake ..
make


build lame :

//*********************************for windows
mkdir build 
cd build
cmake -- -G "Visual Studio 15 2017"


//*********************************for linux
./configure
make
sudo make install


