# BlockCraft code map

*Md Rejaul Korim Sadi · ID 231-115-349 · CSE 402 Computer Graphics & Image Processing Lab*

Use this page to jump straight to the code behind anything shown in the game. Line numbers are for version 1.1.

**Finding a line in Code::Blocks:** open the file from the **Projects** panel on the left (BlockCraft, then Sources, then `lib` or `src`), press **Ctrl+G** and type the line number. **Ctrl+Shift+G** jumps to a function by name.

## The folders

```
BlockCraft/
├── src/
│   └── main.cpp        the program: OpenGL setup, camera, draw order, loop, mouse     (show this first)
├── lib/
│   ├── world.cpp       every block in the world: the cube faces, plants, chunks, ray cast
│   ├── effects.cpp     the outline box, flying particles, the block in your hand
│   ├── sky.cpp         sun, moon, stars, clouds, sky colour and fog
│   ├── hud.cpp         flat 2D shapes: crosshair, hotbar, block icons, logo, text, title screen
│   ├── textures.cpp    the pictures on the blocks, painted pixel by pixel
│   ├── player.cpp      walking, gravity and collision (no drawing)
│   ├── game.cpp        game state, keys, breaking and placing (no drawing)
│   ├── blocks.h        the list of block types and which picture each face uses
│   └── noise.h         the random numbers behind hills and caves
├── tests/              checks that run without a window
└── deps/               freeglut headers and import library for the Windows build
```

## Where each shape is drawn

| Shape on screen | OpenGL primitive | File and line | Function | How to explain it |
|---|---|---|---|---|
| **A block (cube)** | `GL_TRIANGLES`, 2 per face | `lib/world.cpp` 190 | `emitFace()` | A cube has 6 faces. For each face, the 4 corners come from the `CORNER` table (line 150) and are listed counter-clockwise from outside. Each face is split into 2 triangles. |
| The cube's corner table | data | `lib/world.cpp` 150 | `CORNER[6][4][3]` | For each of the 6 directions (+Y, −Y, +X, −X, +Z, −Z), the 4 corner positions of a unit cube |
| Which faces get drawn | logic | `lib/world.cpp` 167 | `faceVisible()` | A face is drawn only if the block next to it is air or see-through. Buried faces are skipped. |
| A whole chunk of blocks | `glNewList` + `GL_TRIANGLES` | `lib/world.cpp` 237 | `buildChunk()` | Loops over every block in a 16 × 16 column, emits its visible faces and records them in a display list |
| Flowers and tall grass | `GL_TRIANGLES` | `lib/world.cpp` 217 | `emitPlant()` | Two flat pictures crossing in an X, each drawn from both sides |
| Water surface | `GL_TRIANGLES` | `lib/world.cpp` 190 | `emitFace()` | The same cube faces, with the top lowered to 0.88 and drawn in the blended pass |
| Trees | blocks | `lib/world.cpp` 114 | `placeTree()` | Not a model. A column of log blocks with a rounded blob of leaf blocks on top. |
| **The giant MD REJAUL KORIM SADI sign** | blocks | `lib/game.cpp` 65 | `buildNameSign()` | Each letter is a 5 × 7 grid (`letterRows()`, line 43). Every filled cell becomes a brick block, 2 deep, on a stone and log frame in front of the start point. The world mesh then draws it like any other block. |
| **Outline around the target block** | `GL_LINES` | `lib/effects.cpp` 8 | `drawSelection()` | The 12 edges of a box drawn as lines, slightly bigger than the block |
| **Broken-block particles** | `GL_QUADS` | `lib/effects.cpp` 74 | `drawParticles()` | Small squares that always face the camera (billboards). The camera's right and up directions are read from the modelview matrix. |
| **Block in your hand** | `GL_QUADS` | `lib/effects.cpp` 108 | `drawHand()` | 3 faces of a cube (top, front, side) with their own projection, moved with `glTranslatef`, `glRotatef` and `glScalef` |
| **Sun and moon** | `GL_QUADS` | `lib/sky.cpp` 51 | `drawSky()` | One square, 120 units away, moved with the player so it looks infinitely far |
| **Stars** | `GL_POINTS` | `lib/sky.cpp` 33 | `drawStars()` | 600 points spread over a half-sphere, brightness changing with `sin` |
| **Clouds** | `GL_QUADS` | `lib/sky.cpp` 73 | `drawClouds()` | Flat 8 × 8 squares high in the sky, picked by noise and sliding slowly |
| **Rectangles: crosshair, panels, hotbar slots** | `GL_QUADS` | `lib/hud.cpp` 76 | `rect()` | A square from 4 corners in 2D pixel coordinates |
| Fade at top and bottom of the title screen | `GL_QUADS` | `lib/hud.cpp` 82 | `gradientRect()` | A rectangle with different colours at the top and bottom corners, which OpenGL blends smoothly |
| **Isometric block icons in the hotbar** | `GL_QUADS` | `lib/hud.cpp` 90 | `blockIcon()` | 3 textured diamonds (top, left, right) placed to look like a 3D cube in 2D |
| **The BLOCKCRAFT logo** | `GL_QUADS` | `lib/hud.cpp` 157 | `drawLogo()` | Each letter is a 5 × 7 grid (`GLYPHS`, line 137). Every filled cell is a small textured square with a shadow. |
| Crosshair | `GL_QUADS` | `lib/hud.cpp` 300 | `drawHUD()` | Two thin rectangles crossing in the middle of the screen |
| Big text (your name, splash line) | stroke font | `lib/hud.cpp` 58 | `strokeText()` | freeglut's line-drawn letters, scaled with `glScalef` and rotated with `glRotatef` |
| Small text (HUD) | bitmap font | `lib/hud.cpp` 23 | `text()` | `glutBitmapString` at a raster position, drawn twice for a shadow |

## Where each graphics idea lives

| Idea | File and line | What to point at |
|---|---|---|
| OpenGL settings | `src/main.cpp` 4 | `initGL()`: depth test, culling, alpha test, blending, fog |
| Perspective projection | `src/main.cpp` 15 | `gluPerspective` in `setCamera()` |
| Viewing transform (camera) | `src/main.cpp` 15 | `glRotatef` for pitch and yaw, `glTranslatef` by minus the eye |
| Draw order of a frame | `src/main.cpp` 28 | `display()`: solid first, water last, 2D on top, then `glutSwapBuffers` |
| Animation loop | `src/main.cpp` 51 | `timer()` every 16 ms |
| Viewport | `src/main.cpp` 57 | `reshape()` |
| Mouse look | `src/main.cpp` 64 | `mouseMove()` |
| Picking a block | `src/main.cpp` 72, `lib/world.cpp` 295 | `mouseButton()` calls `World::raycast()` |
| Translate, rotate, scale on one object | `lib/effects.cpp` 108 | `drawHand()` |
| 2D overlay (orthographic) | `lib/hud.cpp` 10 | `ortho()` with `gluOrtho2D` |
| Texture creation | `lib/textures.cpp` 158 | `buildAtlas()`: `glGenTextures`, `GL_NEAREST`, `glTexImage2D` |
| Texture coordinates | `lib/textures.cpp` 189 | `tileUV()` finds one picture inside the atlas |
| Texture animation | `lib/textures.cpp` 173 | `animateWater()` with `glTexSubImage2D` |
| Shading by face direction | `lib/world.cpp` 142 | `SHADE[6]`: top bright, sides darker, bottom darkest |
| Corner shadows (ambient occlusion) | `lib/world.cpp` 175 | `cornerAO()` |
| Display lists | `lib/world.cpp` 237, 284 | `glNewList` in `buildChunk()`, `glCallList` in `drawOpaque()` |
| Sky colour and fog | `lib/sky.cpp` 21 | `clearSky()` |
| Collision | `lib/player.cpp` 15, 35 | `collides()` and `moveAxis()` |

## If she asks...

**"Show me how you draw a cube."**
Open `lib/world.cpp` at line 150 for the `CORNER` table, then line 190 for `emitFace()`. Say: "A cube is 6 faces. Each face has 4 corners from this table, sent as 2 triangles with `glVertex3f`. `glTexCoord2f` puts the texture on it, and `glColor3f` sets how bright it is."

**"Why don't you draw every face?"**
`lib/world.cpp` line 167, `faceVisible()`. Faces touching another solid block can never be seen, so they're skipped. That's most of the faces in the world.

**"Where is a 2D shape?"**
`lib/hud.cpp` line 76, `rect()`, and line 90, `blockIcon()`. After `ortho()` switches to `gluOrtho2D`, one unit is one pixel, so a rectangle is just 4 `glVertex2f` corners.

**"Where do you use lines and points?"**
Lines: `lib/effects.cpp` line 8, the outline box with `GL_LINES`. Points: `lib/sky.cpp` line 33, the stars with `GL_POINTS`.

**"Where are transformations on an object?"**
`lib/effects.cpp` line 108, `drawHand()`: `glTranslatef` moves the cube to the lower right, `glRotatef` turns it and swings it, `glScalef` makes it small. `glPushMatrix` / `glPopMatrix` around the sun in `lib/sky.cpp` line 51 is another example.

**"Where is the camera?"**
`src/main.cpp` line 15, `setCamera()`.

**"How is the texture made?"**
`lib/textures.cpp` line 51, `paint()`, sets each 16 × 16 picture pixel by pixel. `buildAtlas()` at line 158 uploads them as one texture.
