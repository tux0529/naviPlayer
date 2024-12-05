#!/bin/zsh

sips -z 16 16 ./Musicpl_1024.png  --out ./AppIcon.iconset/icon_16x16.png
sips -z 32 32 ./Musicpl_1024.png  --out ./AppIcon.iconset/icon_32x32.png
sips -z 128 128 ./Musicpl_1024.png  --out ./AppIcon.iconset/icon_128x128.png
sips -z 256 256 ./Musicpl_1024.png  --out ./AppIcon.iconset/icon_256x256.png

iconutil -c icns AppIcon.iconset -o ../AppIcon.icns