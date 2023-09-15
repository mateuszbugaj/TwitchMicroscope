# TwitchMicroscope

This repository uses [LSED](https://github.com/mateuszbugaj/LSED) and [CartesianRobot](https://github.com/mateuszbugaj/CartesianManipulator) submodules to control a microscope via Twitch chat. </br>

This is an example usecase or practical demonstration for the LSED system.
Configuration files at `config/devices/` specify how devices (Manipulator and Microscope) should be percived by the system and what capabilities viewers can use. 

Demo of the TwitchMicroscope system: [LSEDDemo.gif](LSEDDemo.gif)

![df](LSEDDemo.gif)

## Build and run
To build it, load the CartesianManipulator firmware as specified in `CartesianManipulator/README.md`. </br>
Next, load the Microscope code onto the Arduino. It is also responsible for the sample selector and lighting.

```bash
arduino Microscope/Microscope.ino
```

Finally, change to the `LSED/` directory and use Maven to run the LSED system using provided configuration files.

```bash
cd LSED
mvn javafx:run -Djavafx.args="../config/LSEDConfig.yaml"
```

You will need to modify the contents of config files and provide your streaming service user name and tokens as well as change serial port names.

