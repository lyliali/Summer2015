# Summer 2015 Work
This is a repository of the work I did over the summer setting up the raspberry pi.

Notes Directory:
Contains textfiles which will help you get started on developing the animation from here.
  - notes/setup.txt: Explains the process to install and configure the raspberry pi from a fresh install. Also contains debug information.
  - notes/summer_notes.txt: Concerns and thoughts about the animation. Contains information on what we still have to tackle and what might be a problem in the future.
  - notes/wemo_commands.txt: All of the possible commands that could be sent to the wemo, found on the wemo itself. 

Dev Directory:
Contains files that I worked on, but later decided not to actively use in the animation.
  - dev/egauge.py: A very simple interface for connecting to an egauge device.
  - dev/hive_slow.py: An attempt to translate the animation into python for ease of development. Unfortunately this is far too slow to work and is extremely buggy. Works really well on Ubuntu!
  - dev/kellogg_mail.py: A script to send emails through gmail! Could be used to notify users in the future.
  - dev/test_readers.py: An implementation of a reader for testing purposes.
  - dev/wemo.py: A comprehensive interface for connecting to a wemo insight switch.

Graphics Directory:
Contains all of the files necessary for running the animation designed by Sarah Abramson.
  - graphics/Makefile: A convenient makefile for compiling the animation. Simply call "make".
  - graphics/example.conf: An example configuration file for the animation.
  - graphics/hive.conf: The configuration file used by the animation.
  - graphics/hive_animation.cpp: The code for the animation itself.
  - graphics/readers: A directory containing all of the drivers and interfaces for the various monitoring devices.
