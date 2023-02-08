# TwitchMicroscope

This repository uses LSED and CartesianRobot submodules to, among others, control a microscope via Twitch.

## Build and run
### Cartesian manipulator
```bash
cd CartesianRobot
make && make flash
```

### Microscope
Open Microscope/Microscope.ino with Arduino IDE and load.

### LSED
```bash
cd LSED
mvn javafx:run -Djavafx.args="../config/LSEDConfig.yaml"
```

You will need to modify the contents of config files and provide your streaming service user name and tokens as well as change serial port names.

