#!/bin/bash

echo $(date)
echo "$(date +%s): Good morning, time for a nap"
sleep 10
echo "$(date +%s): Hello again, is it time to wake up now?"
sleep 10
/home/pi/setup_pigs.sh &
