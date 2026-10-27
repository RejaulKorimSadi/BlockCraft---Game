# BlockCraft: the complete guide

*Md Rejaul Korim Sadi · ID 231-115-349 · CSE 58th Batch, Section I · CSE 402 Computer Graphics & Image Processing Lab*

This guide starts from zero. If you know some C++ but have never used OpenGL, read it from top to bottom and you will be able to explain every line of BlockCraft, change it, and answer viva questions about it. Each part builds on the one before.

Contents

1. The big picture
2. OpenGL in ten minutes
3. Building, running and testing
4. How the world is stored
5. Making terrain out of noise
6. Caves, ore and plants
7. Painting textures in code
8. Turning blocks into triangles
9. Light: face shading, ambient occlusion and cave darkness
10. The camera
11. The player: movement, gravity, collision
12. Pointing at a block: the ray cast
13. Particles and the block in your hand
14. The 2D layer: HUD, title screen and developer credit
15. Sky, fog, sun, stars, clouds and water
16. Saving worlds and screenshots
17. One frame, start to finish
18. Reproducibility and the tests
19. Things you can try changing
20. Viva questions and answers
21. Plan for the progress update and the final submission

---

## 1. The big picture

Minecraft's world is a 3D grid. Each cell holds one block, and "air" is just a block type that is never drawn. So the whole world is one big 3D array of small numbers, and everything else in the game is a way of reading or changing that array:

- **generation** fills the array once at the start
- **rendering** looks at the array and draws the faces you can see
- **lighting** looks at the neighbours of each face to decide how bright it is
- **physics** checks the array to find floors and walls
- **the ray cast** walks through the array to find the block under the crosshair
- **breaking and placing** change one cell
- **saving** writes the array to a file

Keep that picture in mind and every file makes sense.

The code is split in two on purpose.

**`src/main.cpp` is the program you present.** It is exactly 100 lines and holds the parts a graphics course asks about: opening the window, the OpenGL settings, the camera transform, the order things are drawn in each frame, the game loop, and the keyboard and mouse callbacks.

**`lib/` is the engine underneath.** It does the heavy lifting, and `main.cpp` only calls into it. When someone asks "where is X?", the answer is usually one of these files:

| File | Job |
|---|---|
| `lib/blocks.h` | block types, which texture each face uses, which blocks are solid, see-through or plants |
| `lib/noise.h` | 2D and 3D value noise for hills and caves |
| `lib/textures.cpp` | paints all textures into one image at startup, and animates the water |
| `lib/world.cpp` | the block array, terrain, caves, ore, meshes, lighting, ray cast, save and load |
| `lib/player.cpp` | walking, jumping, gravity, swimming, flying and collision |
| `lib/game.cpp` | the shared game state, starting a world, the name sign, the per-frame update, key actions, breaking and placing |
| `lib/sky.cpp` | sky colour, fog, sun, moon, stars and clouds |
| `lib/effects.cpp` | the block outline, the break particles and the block in your hand |
| `lib/hud.cpp` | everything flat on screen: HUD, title screen, developer credit, screenshots |
| `tests/world_test.cpp` | checks that run without a window |

For a quick lookup of which file and line draws each shape on screen, see [CODE_MAP.md](CODE_MAP.md).

### main.cpp, the 100 lines to know by heart

| Function | What it does | Graphics idea |
|---|---|---|
| `initGL()` | turns on the depth test, back-face culling, alpha test, blending and linear fog, then builds the texture atlas | OpenGL state machine, hidden-surface removal, transparency |
| `setCamera()` | `gluPerspective` for the lens, then `glRotatef` for pitch and yaw and `glTranslatef` by minus the eye position | projection and viewing transforms |
| `display()` | clear to the sky colour, set the camera, draw sun and stars, solid chunks, the outline and particles, clouds, then water with blending, then the 2D layer, then swap buffers | the rendering pipeline and draw order |
| `timer()` | every 16 ms: work out the time step, update the game, ask for a redraw | the game loop and animation |
| `mouseMove()` | turns mouse movement into yaw and pitch and warps the pointer back to the centre (`Player::update` keeps pitch within ±89°) | first-person camera control |
| `mouseButton()` | casts a ray from the eye; left click breaks, right click places, middle click picks | picking with ray casting |
| `reshape()` | keeps the viewport the size of the window | viewport transform |
| `main()` | creates a double-buffered RGBA window with a depth buffer, builds the world, registers every callback (the keyboard ones live in `lib/game.cpp`) and starts `glutMainLoop()` | event-driven programming with GLUT |

## 2. OpenGL in ten minutes

OpenGL is a library for telling the graphics card what to draw. BlockCraft uses the classic "fixed-function" style, the one taught in most graphics labs, because you can draw with very little setup.

**freeglut** opens the window and reads the keyboard and mouse, which OpenGL itself can't do. You hand freeglut your own functions (callbacks) such as `display`, `keyDown` and `mouseButton`, and it calls them when something happens. It also has two fonts we use: bitmap fonts for small HUD text and a stroke (line-drawn) font for the big name on the title screen.

**GLU** gives us `gluPerspective` (a 3D lens) and `gluOrtho2D` (a flat 2D view for the HUD).

**Drawing a shape.** Start a shape, list its corners, end it:

```cpp
glBegin(GL_TRIANGLES);
glVertex3f(0, 0, 0);
glVertex3f(1, 0, 0);
glVertex3f(1, 1, 0);
glEnd();
```

Each block face in BlockCraft is two triangles like this.

**State machine.** OpenGL remembers settings until you change them. `glColor3f` sets the colour for every vertex after it, `glEnable(GL_FOG)` keeps fog on until `glDisable(GL_FOG)`. A lot of the drawing code is switching these on and off at the right moment.

**Matrices.** Two of them matter:

- the **projection matrix** is the lens, deciding how 3D points squash onto the flat screen so far things look smaller
- the **modelview matrix** is the camera position, moving the whole world so the camera sits at the origin looking down −Z

`glMatrixMode` picks which one the next calls change, `glLoadIdentity` resets it, and `glRotatef` and `glTranslatef` multiply into it.

**Depth buffer.** For every pixel the card remembers how far away the thing drawn there is, and a new pixel only wins if it is closer. That is how a hill hides the lake behind it whatever order they were drawn in.

**Double buffering.** We draw into a hidden back buffer, then `glutSwapBuffers()` shows it all at once, so you never see a half-drawn frame.

**Textures.** A texture is an image glued onto a shape. Each vertex gets a texture coordinate (u, v) from 0 to 1 that says which point of the image lands on that corner, set with `glTexCoord2f` just before `glVertex3f`.

**Display lists.** Normally every vertex is sent to the card every frame, which is slow with hundreds of thousands of faces. A display list records a batch of commands once (`glNewList` … `glEndList`) and replays it with one call (`glCallList`). This is the main reason BlockCraft runs smoothly.

## 3. Building, running and testing

On Windows, double-click `open_in_codeblocks.bat`, then press F9 in Code::Blocks. The freeglut headers and import library ship with the project in `deps\`, and the project file tells the compiler to look there, so it always compiles. The first time, the script installs Code::Blocks and the freeglut package with `winget`. After each build, Code::Blocks copies `libfreeglut.dll` into `bin\`, the folder the game runs from. On Linux: `make run`.

The project compiles `src/main.cpp` together with every `.cpp` in `lib/`. `main.cpp` reaches the engine through one include, `#include "../lib/game.h"`, which pulls in the other headers.

Libraries linked: `freeglut` (window and input), `opengl32` (OpenGL), `glu32` (GLU), and `winmm` and `gdi32`, which freeglut needs on Windows.

The Code::Blocks project has two targets. **Release** builds the game. **Tests** builds `world_test`, which checks the world without opening a window (section 18). On Linux the same thing is `make test`.

Starting the program with a number, like `BlockCraft 12345`, chooses the world seed.

## 4. How the world is stored

`lib/world.h` sets the size:

```cpp
const int WORLD_X = 192;
const int WORLD_Z = 192;
const int WORLD_Y = 64;
```

Y is up. That is 192 × 192 × 64 = 2,359,296 blocks, one byte each, so about 2.3 MB in one `std::vector<Block>`.

A 3D position becomes one index like pages in a book. Each Y level is a page of 192 × 192 cells, and inside a page each Z is a row of 192:

```cpp
int index(int x, int y, int z) const { return (y * WORLD_Z + z) * WORLD_X + x; }
```

`World::get(x, y, z)` adds two safety rules: anything outside the world or above the top is AIR, and anything below y = 0 is BEDROCK. The rest of the code never has to worry about reading outside the array.

`World::set` writes a block, updates the column's ground height (section 9), marks the nearby chunks for rebuilding, and if the block below a flower is removed, removes the flower too so it can't float.

**Block types** in `blocks.h`:

```cpp
AIR, GRASS, DIRT, STONE, SAND, LOG, LEAVES, PLANKS, COBBLE, GLASS, WATER, BEDROCK, BRICK,
TALL_GRASS, ROSE, DANDELION, COAL_ORE, IRON_ORE, GOLD_ORE, DIAMOND_ORE
```

Small helpers decide behaviour:

- `isPlant(b)`: tall grass and the two flowers, drawn as crossed pictures
- `isSolid(b)`: you collide with it (not air, water or plants)
- `isTransparent(b)`: you can see through it (air, leaves, glass, water, plants)
- `castsShadow(b)`: it darkens the corners next to it (solid and not glass)

`tileFor(block, face)` picks the texture. Grass has a green top, dirt bottom and a dirt-with-grass-edge side. Logs show bark on the sides and rings on top.

## 5. Making terrain out of noise

If you set each column's height with `rand()`, you get spikes. Real hills are smooth, with neighbouring columns at similar heights. **Value noise** (`noise.h`) gives that.

1. **A random number per grid point.** `hash3(x, y, z)` mixes the integers with the seed using multiplications and bit shifts and returns a number in [0, 1). The same input always gives the same output, which is what makes worlds repeatable.
2. **Blend between grid points.** For a point between grid corners, `value()` blends the four corner numbers (eight in 3D for `value3()`). The blend factor goes through `smooth(t) = t²(3 − 2t)`, which eases in and out so there are no creases at grid lines.
3. **Add layers (octaves).** `fractal()` adds several copies, each with double the detail and half the strength. Big hills plus medium bumps plus small roughness looks natural. This is called fractal or fBm noise.

In `World::generate()`:

```cpp
float h = hills.fractal(x / 48.0f, z / 48.0f, 4);
float d = detail.fractal(x / 12.0f, z / 12.0f, 2);
int height = 12 + (int)(h * 26.0f + d * 4.0f);
```

Dividing by 48 stretches the hills so one is roughly 48 blocks across, and the detail layer adds small bumps. Heights land between about 12 and 42.

Each column is then filled from the bottom: bedrock at y = 0, stone deep down, three layers of dirt with grass on top, or sand if the column is near sea level (which makes beaches). Air below `SEA_LEVEL` (22) becomes water, which makes lakes.

## 6. Caves, ore and plants

**Tunnels.** Take two separate 3D noise fields, `caveA` and `caveB`. Each is close to 0.5 on a thin, wiggly surface running through the ground. Where *both* are close to 0.5 at once, the two surfaces cross, and the crossing is a long winding line, a tunnel:

```cpp
bool tunnel = std::fabs(a - 0.5f) < 0.035f && std::fabs(c - 0.5f) < 0.035f && ...;
```

People call these "spaghetti caves". On dry land tunnels may break through to the surface, giving cave entrances. Under lakes they stay at least four blocks down so the water doesn't hang over empty space.

**Rooms.** A third 3D noise field makes bigger open caves wherever it rises above 0.74, kept at least five blocks below the surface.

With seed 20261028 about 8% of the underground ends up as cave.

**Ore.** Every stone block draws one hash number `r`, and its depth decides what it can become:

| Ore | Where | Rough chance per stone block |
|---|---|---|
| Diamond | below y = 12 | 0.25% |
| Gold | below y = 20 | 0.35 to 0.6% |
| Iron | below y = 34 | 0.8 to 1.4% |
| Coal | anywhere | 2% |

The ranges come from the checks running in order (diamond first, coal last), so a rarer ore takes its share first where it can appear.

**Trees** grow on about 1.5% of grass columns: a trunk of 4 to 6 logs with a rounded ball of leaves.

**Plants.** After the trees, each grass block with air above it draws one more number: above 0.86 it gets tall grass, a little lower a rose, a little lower a dandelion.

## 7. Painting textures in code

There are no PNG files. `lib/textures.cpp` paints every texture into one image, called an **atlas**, when the game starts.

- Each tile is 16 × 16 pixels.
- The atlas is 8 tiles wide and 4 tall (128 × 64 pixels, a power of two that every graphics card accepts). 21 of the 32 slots are used.
- `paint(tile)` loops over the 256 pixels of a tile and picks a colour.

The trick that makes it look like pixel art is `grain()`: take a base colour and nudge each pixel's brightness up or down by a small random amount. Each tile then adds its own pattern:

- stone: darker specks
- grass side: green on the top 3 to 5 rows with a jagged edge, dirt below
- log top: rings, from the distance to the centre
- planks: seam lines every 4 rows
- bricks: mortar lines in an offset pattern
- leaves: some pixels get alpha 0, which makes holes
- glass: see-through except the frame and a little shine line
- tall grass and flowers: drawn on a fully transparent background
- ores: stone with five small lumps of colour (black coal, tan iron, yellow gold, cyan diamond)

`buildAtlas()` uploads the pixels with `glTexImage2D`. The key setting is `GL_NEAREST`. With the default `GL_LINEAR`, the card blurs between pixels and the blocks look smeared. Nearest keeps every texel a sharp square.

`tileUV(tile, …)` turns a tile number into its (u, v) rectangle in the atlas. Tile 19 (gold) is column 19 % 8 = 3, row 19 / 8 = 2. A tiny inward margin stops the neighbouring tile from bleeding in at the edges.

**Animated water.** `animateWater()` repaints only the 16 × 16 water tile with moving ripples (two sine waves that shift with time) and sends just those pixels to the card with `glTexSubImage2D`. It runs about eight times a second. The chunks don't need rebuilding because they only store texture coordinates, not the pixels.

## 8. Turning blocks into triangles

This is the heart of the graphics work.

**Only draw what can be seen.** A buried cube touches other blocks on all six sides, so drawing it is waste. For each block and each of its six faces we look at the neighbour on that side and ask `faceVisible(self, next)`:

- neighbour is air: draw the face
- I am water: only draw where I touch air (the surface)
- neighbour is see-through: draw the face unless the neighbour is the same type (glass next to glass would show an inner wall). Leaves are the exception, since their holes should show the leaves behind.
- otherwise the neighbour is solid and covers the face, so skip it

On a normal world this removes most faces.

**Faces as tables.** `CORNER[6][4][3]` lists the four corners of each face (as 0 or 1 offsets inside the cube), counter-clockwise when you look at the face from outside. The order matters because back-face culling (`glCullFace(GL_BACK)`) throws away any triangle whose corners appear clockwise on screen, meaning we're looking at its back. `CORNER_UV` says which corner of the texture tile each one uses.

**Two triangles per face.** A square can be split along either diagonal. Normally it doesn't matter, but with corner shadows it does (section 9), so `emitFace` chooses the diagonal itself and emits six vertices with `GL_TRIANGLES`.

**Plants.** `emitPlant` draws two pictures crossing in an X through the block. Each is drawn twice with opposite corner order, so it shows from both sides even with culling on. The alpha test cuts away the transparent background.

**Chunks.** The world is cut into columns of 16 × 16 blocks, 12 × 12 = 144 chunks. Each chunk gets two display lists:

- pass 0: solid, cut-out and plant blocks
- pass 1: water only

When you change a block, `markDirty` flags its chunk and the chunks around it, because a neighbour's faces and corner shadows may have changed. `rebuildDirty()` rebuilds only flagged chunks on the next tick, which takes a few milliseconds.

**Why water is drawn last.** See-through things must be drawn after what's behind them, or the depth buffer hides what should show through. So `display()` draws all solid chunks first, then turns on blending, turns off depth writing (`glDepthMask(GL_FALSE)`) and draws the water on top.

**Alpha test vs blending.** Leaves, glass and plants are fully see-through or fully solid at each pixel, so `glAlphaFunc(GL_GREATER, 0.5)` just skips pixels below 0.5. That works in any order. Water is half see-through, which needs real blending with `GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA`.

**Distance culling.** `chunkInRange` skips chunks more than 115 blocks away. Fog hides them anyway.

## 9. Light: face shading, ambient occlusion and cave darkness

There is no real lighting maths. Three cheap tricks multiply together into one grey value per vertex, which `GL_MODULATE` multiplies into the texture colour.

**1. Face direction.** `SHADE` gives each direction a fixed brightness: top 1.0, the X sides 0.8, the Z sides 0.68, bottom 0.55. It reads as sunlight from above and to one side.

**2. Ambient occlusion (corner shadows).** In real life, inside corners are darker because less light reaches them. For each corner of a face, `cornerAO` looks at the three blocks around that corner on the open side of the face: two "side" blocks and one "corner" block.

```cpp
if (side1 && side2) return 0;
return 3 - (side1 + side2 + corner);
```

That gives a level from 0 (both sides blocked, darkest) to 3 (open, full light), and `AO_LIGHT` turns it into 0.45, 0.64, 0.82 or 1.0. OpenGL then blends the colour smoothly across the face. This method and the formula come from Mikola Lysenko's article *Ambient occlusion for Minecraft-like worlds* (0fps.net, 2013).

**The diagonal flip.** If one corner is dark and we split the square along the diagonal through it, the shadow smears across to the opposite corner. The fix from the same article is to split along whichever diagonal has the brighter pair of corners:

```cpp
const int* order = (ao[0] + ao[2] < ao[1] + ao[3]) ? B : A;
```

**3. Cave darkness.** `tops` stores the highest ground block in every column. `skyLight(x, y, z)` checks how far below that a face is: at or above the ground it's 1.0, and every block deeper takes away 7%, down to a floor of 0.42. Caves and mine shafts get gloomy, and the block in your hand dims with them.

## 10. The camera

OpenGL has no camera object. You move the whole world the opposite way instead. If the eye is at (10, 30, 50), shift everything by (−10, −30, −50) so the eye lands on the origin.

```cpp
gluPerspective(fov, aspect, 0.05, 260.0);
glRotatef(-v.pitch, 1, 0, 0);
glRotatef(v.yaw, 0, 1, 0);
glTranslatef(-v.x, -v.y, -v.z);
```

- `gluPerspective`: vertical field of view (70°, opening to 78° when you sprint), window width/height ratio, and near and far clipping planes
- the rotations turn the world around the eye: pitch (up/down) around X, yaw (left/right) around Y
- the translation moves the world so the eye is at the origin

OpenGL applies these to each vertex in reverse order: translate first, then yaw, then pitch. That is "move the world to the player, then turn it".

`updateView()` in `lib/game.cpp` fills in `view` each frame. In the game it's the player's eye, 1.62 blocks above the feet, plus a small up-and-down **walking bob** that follows `walkPhase`. On the title screen it swings slowly from side to side 103 blocks in front of the name sign, with the yaw worked out by `atan2` so it always looks at the sign. The title view also draws chunks up to 200 blocks away (`drawRange`) with lighter fog, so the whole sign and the land behind it show.

**Which way am I looking?** `Player::lookDir` turns yaw and pitch into a direction:

```
dx =  sin(yaw) · cos(pitch)
dy =  sin(pitch)
dz = −cos(yaw) · cos(pitch)
```

At yaw 0 and pitch 0 that's (0, 0, −1), straight down −Z, OpenGL's default forward.

**Mouse look.** While the mouse is captured, each move is measured from the window centre, turned into yaw and pitch (0.15° per pixel), and the pointer is warped back to the centre. Pitch is clamped to ±89° so you can't flip over. The warp itself causes a zero move, which is ignored.

## 11. The player

The player is an invisible box 0.6 wide and 1.8 tall, standing on its feet position.

Each tick `Player::update`:

1. Builds a direction from W, A, S, D using the yaw: forward is (sin yaw, −cos yaw), right is (cos yaw, sin yaw). The result is normalised so diagonals aren't faster.
2. Picks a speed: 4.3 blocks/s walking, 5.6 sprinting, 11 flying, half in water.
3. Sets the vertical speed:
   - walking: gravity of 28 blocks/s² (capped at 40), and Space gives 8.6 upwards if you're on the ground, which clears a bit over one block
   - in water: weaker gravity, Space pushes up at 3, sinking is capped
   - flying: no gravity, Space up, Shift down
4. Moves.

**Collision.** `collides()` takes a box position, finds every block cell it touches by flooring its corners, and returns true if any is solid. The world edge counts as a wall. Plants and water aren't solid, so you walk through them.

`moveAxis` moves along one axis at a time, X then Y then Z, in 4 small steps. If a step makes the box overlap something, it goes back to the last good spot and stops on that axis. Hitting something below means you're on the ground; either way vertical speed becomes 0. One axis at a time is what lets you slide along a wall: walk into it at an angle and only the blocked direction stops.

`dt` is capped at 0.05 s so a slow frame can't carry you through a floor. Fall below y = −10 and you're put back on the surface.

## 12. Pointing at a block: the ray cast

To break or place, we need the block under the crosshair. We send a ray from the eye along `lookDir` and walk through the grid cell by cell until we hit something that isn't air or water, up to 6 blocks.

Stepping a tiny distance at a time is slow and can skip corners. `World::raycast` uses the **Amanatides & Woo** method (1987), which visits exactly the cells the ray passes through, in order:

- `tdx` is how far along the ray you go to cross one whole cell in X (same for `tdy`, `tdz`)
- `tx` is how far along the ray the *next* X boundary is (same for `ty`, `tz`)
- at each step, the smallest of `tx`, `ty`, `tz` says which boundary comes first, so we step into the next cell on that axis and push that boundary one cell further

It also remembers the previous cell, the empty space right in front of the face you're looking at, which is where a new block goes.

- **Left click** removes the hit block (not bedrock), spawns particles and swings the hand.
- **Right click** places the selected block in the previous cell if it's empty, water or a plant, and isn't where you're standing. Clicking a flower replaces it. Flowers can only go on grass or dirt.
- **Middle click** copies the block you're looking at into the current hotbar slot, so you can build with ore, sand or anything else you find.

The black outline is 12 lines drawn with `GL_LINES`, a hair larger than the block so it doesn't flicker against the faces.

## 13. Particles and the block in your hand

**Particles.** Breaking a block throws out 28 little squares. Each one shows a random 4 × 4 corner of the broken block's texture, flies off with a random speed, falls under gravity and stops when it lands, then disappears after half a second to a second.

They are **billboards**: flat squares that always face the camera. The trick is in the modelview matrix. Its first row is the camera's "right" direction and its second row is "up", so `drawParticles()` in `lib/effects.cpp` reads the matrix with `glGetFloatv` and builds each square from those two directions.

**The hand.** After the world is drawn, `drawHand()` clears the depth buffer (so the hand never sinks into a wall), sets its own projection with a very near clipping plane, resets the modelview matrix and draws a small textured cube at the lower right of the view, turned so three faces show. It moves:

- a gentle sway while walking, following the same `walkPhase` as the camera bob
- a quarter-second swing when you break or place a block, `sin(progress × π)` so it goes out and comes back
- a flat picture instead of a cube when you hold a flower

## 14. The 2D layer: HUD, title screen and developer credit

After the 3D scene, `ortho()` in `lib/hud.cpp` switches to a flat view:

```cpp
gluOrtho2D(0, winW, winH, 0);
```

One unit is now one pixel, (0, 0) is the top-left corner and y grows downwards. Depth test and fog go off, so 2D shapes simply draw on top.

**HUD pieces**

- crosshair: two thin rectangles
- hotbar: nine slots, the selected one outlined in white
- block icons: three textured quads arranged as an isometric cube, side faces darker
- text: `glutBitmapString`, drawn twice (dark and shifted by one pixel, then the colour on top) so it reads on bright sky
- debug info (F3), controls panel (H)
- overlays: dark blue at night, blue when your eye is under water

**Developer credit.** Three places, all in `lib/hud.cpp`:

- `drawCreditCard()` draws the "DEVELOPED BY" card: letter-spaced heading, the name *Md Rejaul Korim Sadi* in freeglut's stroke font with a shadow, a gold underline that grows outwards, and the ID, batch and university underneath. The title screen shows it, and `I` brings it back during play.
- `drawIntroToast()` slides a "Welcome to BlockCraft, developed by Md Rejaul Korim Sadi" banner down from the top when a game starts, holds it, then slides it away.
- `drawDevBadge()` keeps a small developer badge in the bottom-left corner for the whole game.
- `buildNameSign()` in `lib/game.cpp` builds **MD REJAUL KORIM SADI** out of brick blocks in every new world, 36 blocks in front of the start point. Each letter is a 5 × 7 grid; every filled cell becomes a block, two deep, on a cobblestone and log frame. The trees between the start point and the sign are cleared so it can be seen. Because it's made of real blocks, it's drawn by the same chunk meshes as everything else, and it can be broken like anything else. The title screen camera swings slowly in front of it.

The stroke font is made of lines, so it scales to any size. `strokeText()` scales it with `glScalef(k, -k, 1)` (the minus flips it, because our 2D view has y pointing down) and turns on `GL_LINE_SMOOTH` for clean edges.

**The title screen** (`drawTitle()`):

- the world behind everything, with the camera swinging slowly in front of the name sign, and dark gradients top and bottom so the text stands out
- the **BLOCKCRAFT logo**, built from 5 × 7 pixel letters in `GLYPHS`. Each "on" pixel is a little square textured with stone, or with grass side if nothing is above it, with a drop shadow. It drops in from above as it fades in.
- a **yellow splash line** that pulses and leans at an angle, a nod to Minecraft's menu
- the developer card fading in and sliding up
- "Click or press Enter to play", gently blinking

All the movement uses `ease(t) = 1 − (1 − t)³`, which starts fast and settles softly. Esc in the game frees the mouse; Esc again goes back to this screen, and Esc there quits.

## 15. Sky, fog, sun, stars, clouds and water

- **Sky colour** is the clear colour: light blue by day, orange at sunset, deep blue at night, and murky blue when your eye is under water.
- **Fog** is linear and set to the sky colour every frame, so the land fades into the sky and hides where drawing stops. Normal range 40 to 95 blocks, 70 to 150 on the title screen, 0 to 16 under water.
- **Sun and moon:** one quad 120 units away that moves with the camera, so it never gets closer and looks infinitely far. It's drawn first with depth writing off, so terrain covers it.
- **Stars:** at night, 600 points on a big dome around the camera, placed by the hash function so they're the same every night, each twinkling on its own sine wave.
- **Clouds:** flat white squares on an 8 × 8 grid above the world. A noise function decides which squares have cloud, and the layer slides slowly with the clock.
- **T** cycles day, sunset and night.

## 16. Saving worlds and screenshots

**F5** writes `world.dat` next to the program:

| Bytes | Content |
|---|---|
| 4 | the text `BCW1`, to recognise our own file |
| 4 | the seed |
| 12 | the three world sizes |
| 20 | player x, y, z, yaw, pitch |
| 2,359,296 | every block, one byte each |

**F9** reads it back. Every read is checked; if the header or sizes don't match, or the file is short, nothing changes. After loading, ground heights are recomputed and every chunk is rebuilt.

**F2** saves a screenshot. It sets a flag, and the next frame reads the finished picture from the back buffer with `glReadPixels` just before swapping. The pixels go into a BMP file: a 54-byte header, then rows from bottom to top in blue-green-red order, each padded to a multiple of 4 bytes. That happens to be the order `glReadPixels` gives with `GL_BGR_EXT`, so no flipping is needed. Files are named by date and time, for example `screenshot_20261108_101500.bmp`.

## 17. One frame, start to finish

`main()` in `src/main.cpp`:

1. `glutInit`, ask for a double-buffered RGBA window with a depth buffer, create it.
2. `initGL()`: depth test, back-face culling, alpha threshold, blend function, fog, texture atlas.
3. `newWorld(seed)`: generate terrain, build all 144 chunks, find grass near the middle and put the player there.
4. Register the callbacks, start a 16 ms timer and hand over to `glutMainLoop()`.

Every 16 ms `timer()` calls `updateGame(frameTime())` from `lib/game.cpp`, which:

1. Works out `dt` from the elapsed time.
2. Counts down timers (messages, intro popup, hand swing) and updates the water texture when due.
3. On the title screen, advances the title clock. In the game, updates the player, the walking bob, the field of view and the particles.
4. Rebuilds edited chunks and works out where the camera is.

Then `display()` in `src/main.cpp`:

1. `clearSky()`: clear to the sky colour and set the fog to match.
2. `setCamera()`; `drawSky()` for the stars and sun.
3. Solid chunks with the atlas and alpha test.
4. In the game: ray cast, selection outline, particles.
5. Clouds.
6. Water with blending.
7. `drawOverlay()`: in the game, the hand and then the HUD; on the title screen, the title layer. A screenshot is saved here if one was asked for.
8. Swap buffers.

## 18. Reproducibility and the tests

Nothing in world generation uses `rand()` or the clock. Every random number comes from `hash3()` with the seed, so the same seed gives the same world on every run. The only exception is **N**, which deliberately seeds from the clock; the seed it picks shows on screen and in F3, so you can always get back to it.

`tests/world_test.cpp` checks this without a window:

- builds the seed-20261028 world twice and compares an FNV-1a fingerprint of all 2.3 million blocks, and checks another seed gives a different world
- terrain has water, trees and grass; ore exists and diamonds are rarer than coal; caves exist but take up less than a quarter of the underground
- the sky is bright and deep underground is darker
- the bottom layer is all bedrock
- a ray straight down hits the surface, and the cell in front is the one above
- a world with a change in it saves and loads back block for block, with the player position

It also prints the fingerprint next to a reference value (`E8616371`, from GCC 13.3 on x86-64). A different compiler may round a float slightly differently and build a slightly different world, so a mismatch is reported but doesn't fail the test.

The pictures are reproducible too. Every GIF and screenshot in `docs/media` came from seed `20261028`, and `tools/record.sh` can record them again on Linux: it opens the game on an invisible screen (Xvfb), plays back key presses and mouse moves with xdotool, and turns the recording into a GIF with ffmpeg.

## 19. Things you can try changing

Good for showing you understand the code, and for the progress update.

- World size: `WORLD_X`, `WORLD_Z` in `lib/world.h` (keep them multiples of 16)
- Flatter or wilder land: the `26.0f` or the `/ 48.0f` in `generate()`
- More caves: raise `0.035f`; bigger rooms: lower `0.74f`
- More diamonds: raise `0.0025f`
- More trees: lower `0.985f`
- Higher sea: `SEA_LEVEL`
- Jump: `8.6f` in `lib/player.cpp`; gravity: `28.0f`
- Field of view: `70.0f` and `78.0f` in `updateGame()` in `lib/game.cpp`
- Corner shadow strength: `AO_LIGHT`; cave darkness: the `0.07f` and `0.42f` in `skyLight()`
- Turn off ambient occlusion to compare: make `cornerAO` return 3
- A new block: add it to `BlockType`, add a tile to `Tile`, paint it in `paint()`, map it in `tileFor()`, name it in `blockName()`. Eleven atlas slots are still free.

## 20. Viva questions and answers

**Why is the code split into `main.cpp` and `lib/`?**
`main.cpp` holds the graphics program itself: window, OpenGL state, camera, draw order, loop and input, in 100 lines. `lib/` holds the engine those steps call: the world, the player, the sky, effects and the 2D layer. Each part can be read and changed on its own.

**Walk me through one frame.**
`timer()` runs every 16 ms and calls `updateGame()`, which moves the player and works out the camera. Then `display()` clears to the sky colour, sets the projection and view in `setCamera()`, draws the sun and stars, the solid blocks, the outline and particles, the clouds, then the water with blending, then the hand and HUD, and swaps the buffers.

**Why fixed-function OpenGL and not shaders?**
It's what the lab uses, and every step (projection, transforms, texturing, fog, blending) shows as a plain function call. The same ideas carry over to shader OpenGL.

**How big is the world in memory?**
192 × 192 × 64 = 2,359,296 blocks at one byte each, about 2.3 MB.

**Why not draw every face?**
Most faces touch a solid block and can never be seen. `faceVisible` skips them. Together with display lists that's the biggest speed-up.

**What is a display list?**
A batch of OpenGL commands recorded once and stored by the driver. We record each chunk once and replay it every frame with one call.

**Why chunks?**
Changing a block only rebuilds one 16 × 16 × 64 piece and its neighbours, and far chunks can be skipped.

**What is back-face culling?**
OpenGL drops triangles whose corners appear clockwise on screen, which means they face away. We list corners counter-clockwise from outside, so only faces pointing at the camera are drawn.

**How does the depth buffer work?**
It stores a depth per pixel and only accepts a new fragment if it's closer, so solid objects hide each other correctly in any order.

**Then why draw water last?**
Water is see-through. Blending mixes it with what's already drawn, so the land under it must be there first. Water also doesn't write depth, so it doesn't hide other water faces behind it.

**Alpha test or blending?**
Alpha test is all-or-nothing per pixel and needs no sorting, right for leaves, glass and flowers. Blending gives partial transparency, right for water, but order matters.

**What is ambient occlusion here?**
Darkening each face corner by how many of the three blocks around it are solid, then letting OpenGL blend across the face. It makes edges and corners read clearly without any real lighting.

**Why choose the diagonal when splitting a face?**
Colour is blended across each triangle. Splitting through a dark corner smears the shadow to the opposite corner; splitting along the brighter diagonal keeps it where it belongs.

**How are caves made?**
Two 3D noise fields are each near 0.5 on a thin surface. Where both are near 0.5 at once the surfaces cross in a winding line, which becomes a tunnel. A third field makes larger rooms.

**How does the water move if the chunks are display lists?**
The lists only store texture coordinates. We repaint the water tile's pixels and upload them with `glTexSubImage2D`, so every water face changes at once without rebuilding anything.

**How do particles always face you?**
The first two rows of the modelview matrix are the camera's right and up directions. Each particle square is built from those, so it always lies flat to the screen.

**How is the hand drawn without going into walls?**
After the world is drawn, the depth buffer is cleared and the hand gets its own projection and view, so it's always in front.

**How is the camera implemented?**
By transforming the world the opposite way: rotate by −pitch around X, by yaw around Y, then translate by minus the eye position, with `gluPerspective` as the lens.

**How does it know which block you're pointing at?**
A voxel ray cast (Amanatides & Woo) from the eye along the look direction, stepping to whichever cell boundary is nearest, up to 6 blocks.

**Where does a new block go?**
In the last empty cell the ray passed before the hit, the one touching the face you're looking at.

**How does collision work?**
The player is a box. We move one axis at a time in small steps and undo a step that would overlap a solid cell, which also lets you slide along walls.

**Why is the world the same for one seed?**
All randomness comes from a hash of position and seed, never from `rand()` or the time. The test proves it with a fingerprint of every block.

**How is your name drawn on the title screen?**
With freeglut's stroke font, which is made of lines, scaled with `glScalef` and smoothed with `GL_LINE_SMOOTH`. The smaller text uses bitmap fonts.

**How is the screenshot saved?**
`glReadPixels` reads the back buffer just before the swap, in blue-green-red order and bottom-to-top, which is exactly how a BMP stores pixels. We write a 54-byte header and the rows.

**What did you take from outside?**
The idea comes from Minecraft. Block picking follows Amanatides & Woo (1987). Corner shadows follow Mikola Lysenko's 0fps article (2013). freeglut provides the window, input and fonts. Everything else, including all textures, is written in this project.

## 21. Plan for the progress update and the final submission

**Progress update, 8 November 2026 (in person).** Show:

- the title screen with the developer card, then the game on the lab PC: walking, flying, breaking and placing
- two different seeds, and the same seed twice to show it's repeatable (`world_test` makes this quick)
- the texture atlas, and `GL_NEAREST` vs `GL_LINEAR` (switch it for a moment to show the blur)
- ambient occlusion on and off (make `cornerAO` return 3) to show what it adds
- what's left: polish, demo video, slides

**Final submission, 25 November 2026.** Name everything `231-115-349_Final_Project`.

1. **Source code**: this repository zipped, with the README.

   To put it on GitHub, make an empty repository called `BlockCraft` on your account (no README, no licence, since both are already here), then in the project folder:

   ```bash
   git init
   git add .
   git commit -m "BlockCraft v1.1"
   git branch -M main
   git remote add origin https://github.com/RejaulKorimSadi/BlockCraft.git
   git push -u origin main
   ```

   `.gitignore` keeps build output, saved worlds and screenshots out of the repository. `CITATION.cff` makes GitHub show a "Cite this repository" button with your name.
2. **Demo video**, 2 to 4 minutes:
   - the title screen with the name card
   - walk, then press F and fly up for a view over the land
   - break blocks (particles), build a small house with planks, glass and bricks
   - middle-click a block to pick it
   - dig down into a cave and find ore
   - swim in a lake for the underwater look
   - T for sunset and night with stars
   - F5 save, N new world, F9 load the old one back
   - F2 for a screenshot, I for the developer card

   Windows has a free recorder built in (Win + G, Xbox Game Bar), or use OBS Studio.
3. **Slides**, about 10 to 12: title with name and ID; objective; tools; how the world is stored; terrain and caves from noise; the texture atlas; meshing and hidden faces; lighting (shading, ambient occlusion, cave darkness); camera and projection; collision and ray casting; screenshots; reproducibility; limitations and future work; conclusion. The images in `docs/media` can go straight onto the slides.
