#!/bin/bash

# sleep for 30 seconds to let any readings happening on-the-minute happen
sleep 30

# Soft reset - I2C 0xD304
pigs i2cwd 0 0xd3 0x04 > /dev/null
sleep 1

# Start continuous reading again
pigs i2cwd 0 0x00 0x10 > /dev/null
sleep 1;

# Check if there's anything to read
pigs i2cwd 0 0x02 0x02 > /dev/null
sleep 1;

# Read the result, and just ignore it lol
pigs i2crd 0 3 > /dev/null
sleep 1;

# Ask for the latest result
pigs i2cwd 0 0x03 0x00 > /dev/null
sleep 1;

# Read the latest result, and parse it
DATE=$(date +%s)
echo -n "${DATE}, "; /home/pi/parse_manual_argv $(pigs i2crd 0 18)

# now read a second time?
sleep 5

# Start continuous reading again
pigs i2cwd 0 0x00 0x10 > /dev/null
sleep 1;

# Check if there's anything to read
pigs i2cwd 0 0x02 0x02 > /dev/null
sleep 1;

# Read the result, and just ignore it lol
pigs i2crd 0 3 > /dev/null
sleep 1;

# Ask for the latest result
pigs i2cwd 0 0x03 0x00 > /dev/null
sleep 1;

# Read the latest result, and parse it
DATE=$(date +%s)
echo -n "${DATE}, "; /home/pi/parse_manual_argv $(pigs i2crd 0 18)

# now read a third time???
sleep 5

# Start continuous reading again
pigs i2cwd 0 0x00 0x10 > /dev/null
sleep 1;

# Check if there's anything to read
pigs i2cwd 0 0x02 0x02 > /dev/null
sleep 1;

# Read the result, and just ignore it lol
pigs i2crd 0 3 > /dev/null
sleep 1;

# Ask for the latest result
pigs i2cwd 0 0x03 0x00 > /dev/null
sleep 1;

# Read the latest result, and parse it
DATE=$(date +%s)
echo -n "${DATE}, "; /home/pi/parse_manual_argv $(pigs i2crd 0 18)


