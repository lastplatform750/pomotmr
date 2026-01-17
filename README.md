pomotmr
=======

A crappy ncurses pomodoro timer

## dependencies
  * ncurses: for the ui
  * alsa: to play alarm
  * meson: to build it

## building
  Run the make_tar.sh script. The thing you want is the "dist/pomotmr" folder. Put that folder wherever you want, and just run the binary in there to start the program.

## socket
  If you run the executable with **--server** enabled, the program will make a unix domain socket that you can use to get the remaining time in the current section.
  
  The default path for the socket is /tmp/pomotmr.sock

  Right now all the socket does is send out the remaining time (in seconds)

## controls
  * **space**: toggles play/pause, and stops the alarm sound
  * **c**: resets the time of the current pomo section and pauses
  * **enter**: skips to the next pomo section and pauses

## command line options
  * **--no-alarm**: don't play a sound at the end of pomo sections
  * **--server**: enable the unix socket
  * **-a <path>**: set the path for the alarm sound
  * **-s <path>**: set the path for the socket

  * **--test**: make pomo sections really short (for testing)
  