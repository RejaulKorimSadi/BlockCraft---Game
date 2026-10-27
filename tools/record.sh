#!/bin/bash
# BlockCraft by Md Rejaul Korim Sadi (231-115-349)
# Records a short clip of the game on a virtual screen, the way the GIFs in docs/media were made.
# Needs Linux with Xvfb, xdotool and ffmpeg.  Build first with: make
#
#   tools/record.sh <name> <seconds> "<xdotool actions>" [seed]
#
# example, walk forward for two seconds after starting the game:
#   tools/record.sh walk 5 "xdotool mousemove 640 360 click 1; sleep 0.5; xdotool keydown w; sleep 2; xdotool keyup w"

set -e
NAME=${1:?name}; SECS=${2:?seconds}; ACTIONS=${3:-}; SEED=${4:-20261028}
ROOT=$(cd "$(dirname "$0")/.." && pwd)
OUT="$ROOT/docs/media"
export DISPLAY=:97

Xvfb :97 -screen 0 1280x720x24 >/dev/null 2>&1 &
XV=$!
sleep 1.5
cd "$ROOT/bin"
./blockcraft "$SEED" >/dev/null 2>&1 &
GAME=$!
for i in $(seq 1 40); do
    WID=$(xdotool search --name BlockCraft 2>/dev/null | head -1)
    [ -n "$WID" ] && break
    sleep 0.25
done
sleep 0.3
xdotool windowactivate --sync "$WID" 2>/dev/null || true

ffmpeg -loglevel error -y -f x11grab -draw_mouse 0 -framerate 15 -video_size 1280x720 -i :97 -t "$SECS" "/tmp/$NAME.mp4" &
REC=$!
eval "$ACTIONS"
wait $REC
kill $GAME $XV 2>/dev/null || true

ffmpeg -loglevel error -y -i "/tmp/$NAME.mp4" \
    -vf "fps=10,scale=600:-1:flags=lanczos,split[a][b];[a]palettegen=max_colors=128:stats_mode=diff[p];[b][p]paletteuse=dither=bayer:bayer_scale=4:diff_mode=rectangle" \
    "$OUT/$NAME.gif"
echo "saved $OUT/$NAME.gif"
