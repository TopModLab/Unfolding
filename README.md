# Unfolding
Geometry Unfolding
## Qt Version
Qt 5.6.0

## Supported Compiler
MinGW-x86-v4.9.2, MSVC++ 14.0(Visual Studio 2015)

## IDE & Project Files
### Qt Creator 3.6.1
Open Unfolding.pro in root directory to edit and compile with Qt Creator IDE, all compiler supported.
### Visual Studio 2015
Open Unfolding.sln under directory _visual_studio_  to compile and edit with Visual Studio 2015, only MSVC++ 14.0 supported with with file.

## OpenGL Version
Minimum requirement: OpenGL 3.3 Core Profile

## Branches
- master: contains Weaving operations. Uses vector storage as data structure @ShenyaoKe.
- conical_mesh: 
  - Oct Weaving, Weaving, Cross Weaving, Classical Weaving, Conical Weaving, BiTri Weaving operations @ShenyaoKe
  - Revise on Conical Weaving, added refID for printing @urianawu
  - Triangle Weaving operation @urianawu
- NewOrigami: attempt on single-panel origami unfolding scheme, only face bands are created @urianawu
- Origami: attempt on single-panel origami unfolding scheme @liz425
- master3.0: 
  - contains GRS, GES, Half-Edge, Quad-Edge, Winged-Edge operations @urianawu, 
  - obj reader, hds data structure and unfolder originally implemented by @phg1024, 
  - obj reader and unfolder refactored and revised by @ShenyaoKe, 
  - printing and connectors generation by @ShenyaoKe
- dforms: an obsolete branch only @ShenyaoKe knows what it is

## Unfolding workflow written by @ShenyaoKe

1. Download the software in releases tab.
Latest Unfolding(contains Half-Edge, Quad-Edge, Winged-Edge) release:
Latest Woven release:

2. Import your model from File->Import, or Click Import Button on Toolbar, or Press Ctrl+N on Keyboard;
3. Generate Developable Surface by Clicking the corresponding operation buttons;
4. For Unfolding: Adjust bridger size and flap size (from 0 to 0.98), adjust bridger sample in Set Connector option; For Woven: Adjust patch scale, strip width, layer offset(thickness of material).
5. Unfold current mesh by Clicking Unfold Mesh Button, or Press ALT+U on Keyboard;
6. Export as SVG file from File->Export, or Clicking Export Button on Toolbar, or Press Ctrl+E;
7. Adjust export settings, select path to save exported file, set scale to proper value to make enough space for holes, change pinhole size to exact size of your fasteners, change pinhole count and etch segments depending on your flap size and material rigidity.

Note:
- You might come back to re-export SVG file if the result is not satisfying, especially in changing the scaling. 
- Attention, our software will generate a file with "SVG" in filename. It's used for other purpose and is not the exported SVG file. 

