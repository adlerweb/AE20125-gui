AE20125-gui
===========

QT-GUI for Ascel AE20125 Function Generator

Authors
=======

Florian Knodt <adlerweb@adlerweb.info>

License
=======

This software is licensed under the GPL V3. The full text of the license is available here: http://www.gnu.org/licenses/gpl-3.0

Compiling
=========

There are 2 folders in this project:

/build
------

This folder includes the Makefile needed to build the software. The included binary was build on a recent Arch Linux x86_64. You are advised to rebuild it yourself to ensure compatibility with your machine. To compile use the following steps:

- Make sure you have installed QT4 including its developement packages
  - Ubuntu/Debian: libqt4-dev
  - Arch: qt4
  - Gentoo: dev-qt/qtcore (i guess)

- Make sure you have installed QtSerialPort as shown here: http://qt-project.org/wiki/QtSerialPort
  - Arch has an AUR: https://aur.archlinux.org/packages/qtserialport-qt4-git/

- cd into the build directory

- run make to (re)build the binary

The software was only tested on Linux, however according to Qt and QtSerialPort it should also work on Windows, Mac and other platforms.

/src
----

This folder includes all original sources and layout files needed to edit and build the application in QTCreator.

Known issues
============

- Preset Load/Save is not implemented yet
- Using Sweep or Modulation locks up my device (however this also applies to the original Windows(r) software)

Screenshot
==========
![screenshot](http://www.adlerweb.info/blog/wp-content/uploads/2013/07/Bildschirmfoto-AE20125gui-1.png "Screenshot")
