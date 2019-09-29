Audio synthesizer turning RTSP video into a sound
=================================================

Whiteroom is a synthesizer that generates sound using video stream from RTSP camera.
It is assumed that video has white background.
Objects at the left side generate lower frequencies and ones at the right side generate higher frequencies.
Larger and darker objects generate louder sound.
Objects at the top generate louder sound than the objects at the bottom.

Building on macOS
-----------------

* Install __XCode__ app from the App Store
* Install __homebrew__

  Open the Terminal and type

      /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

* Clone the project

  Go to your projects directory in the Terminal e.g. ~/projects

  Clone the project from Github

      git clone --recurse-submodules -j8 https://github.com/igrmk/whiteroom.git
      cd whiteroom

* Setup required packages

      ./macenv

* Build libraries

      ./build-ffmpeg
      ./build-portaudio

* Build whiteroom

      ./build-whiteroom

  Now you have __whiteroom.app__ in whiteroom/bundle folder. You can install and run it
