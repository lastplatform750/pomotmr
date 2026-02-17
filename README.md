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
  If you run the executable with **--enable-server**, the program will make a unix domain socket that you can use to get the remaining time in the current section.
  
  The default path for the socket is /tmp/pomotmr.sock

  The socket sends out a character for what the current pomo section is, then sends out the remaining section time (in seconds).

  The three possible characters (all capitals) for the pomo sections are:
  * **F**: Focus section
  * **S**: Short break section
  * **L**: Long break section

  For example, if the timer is in the focus section with 132 seconds left, the socket sends out "F132"

## logging
  If you run the executable with **--enable-timer-log**, the program will keep a log of how much time you spent on focusing and on break time. The log also lets you make a list of "tasks" to log how much time you spent on each task

  You can specify the task names with **--task-names** followed by a comma separated list of the task names you want to use. If you don't specify any task names then the only task will be "Generic Task".

  The default location for the log is "timer.log", in the same directory as the executable. You can specify where to put the log with **--timer-log-path**

  You can select which task you want to log using **tab**. The log gets updated on pressing **tab** only if some amount of focus or break time elapsed for the previous task.

  Each line in the log is a comma separated list for a single task. The entries of a line, in order, are:
  * A unix timestamp for when the logging of the task began
  * A unix timestamp for when the logging of the task ended
  * An integer for how much break time elapsed for the task
  * An integer for how much focus time elapsed for the task

## controls
  * **space**: toggles play/pause, and stops the alarm sound
  * **c**: resets the time of the current pomo section and pauses
  * **enter**: skips to the next pomo section and pauses
  * **tab**: (if logging is enabled) writes a log entry for the current task (only if there was a nonzero amount of focus or break time) and then advances to the next task

## command line options
  * **--disable-alarm**: don't play a sound at the end of pomo sections
  * **--enable-server**: enable the unix socket
  *
  * **--alarm-path \<path\>**: set the path for the alarm sound
  * **--socket-path \<path\>**: set the path for the socket
  * **--error-path \<path\>**: set the path for error logging
  * **--timer-log-path \<path\>**: set the path for timer logging

  * **--test**: make pomo sections really short (for testing)
  