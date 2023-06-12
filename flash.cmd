echo off

echo Erasing flash...
esptool.py -p COM4 erase_flash

echo Build app, flash and monitor...
idf.py -p COM4 app flash monitor
