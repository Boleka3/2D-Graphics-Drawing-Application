# 2D Graphics Drawing Application

This is a comprehensive 2D graphics application built using C++ and the native Windows GDI library. It provides a user interface to draw various geometric shapes, apply filling techniques, and perform clipping operations using a suite of classic graphics algorithms. The application serves as a practical demonstration of fundamental concepts in computer graphics.

## Features

The application implements a wide range of 2D graphics algorithms, accessible through a simple menu-driven interface.

### Drawing Algorithms
- **Line Drawing:**
  - Digital Differential Analyzer (DDA)
  - Midpoint Line Algorithm
  - Parametric Line Drawing
- **Circle Drawing:**
  - Direct (Polynomial) Method
  - Polar Coordinates Method
  - Iterative Polar Coordinates Method
  - Midpoint Circle Algorithm
  - Modified Midpoint Circle Algorithm
- **Ellipse Drawing:**
  - Direct (Polynomial) Method
  - Polar Coordinates Method
  - Midpoint Ellipse Algorithm
- **Curve Drawing:**
  - Cardinal Spline

### Filling Algorithms
- **Circle Filling:**
  - Filling a quarter of a circle with lines.
  - Filling a quarter of a circle with smaller circles.
- **Shape Filling with Curves:**
  - Filling a square with Hermite curves.
  - Filling a rectangle with Bezier curves.
- **Polygon Filling:**
  - Convex Polygon Filling
  - Non-Convex Polygon Filling (using Winding Number rule)
- **Seed Fill Algorithms:**
  - Recursive Flood Fill
  - Non-Recursive (Stack-based) Flood Fill

### Clipping Algorithms
- **Point Clipping:**
  - Clipping points against a rectangular or square window.
- **Line Clipping:**
  - Cohen-Sutherland Line Clipping algorithm.
- **Polygon Clipping:**
  - Sutherland-Hodgman Polygon Clipping algorithm.

### Other Features
- **Color Selection:** Choose from Red, Green, Blue, or Black.
- **Save/Load:** Save the current canvas state to a file (`shapes.txt`) and load it back.
- **Clear Screen:** Reset the canvas.
- **Persistent Graphics:** The drawn shapes are stored and repainted whenever the window is refreshed.

## How to Compile and Run

### Prerequisites
- A Windows operating system.
- A C++ compiler that supports the Windows API, such as:
  - MinGW-w64 (via g++)
  - Microsoft Visual C++ (MSVC)

### Compilation (using g++)
To compile the project, open a terminal or command prompt in the project directory and run the following command. You must link against the `gdi32` library, which provides the graphics functions.

```bash
g++ graphics.cpp -o DrawingApp.exe -lgdi32
```

### Running the Application
After successful compilation, an executable file `DrawingApp.exe` will be created. Run it from the command line or by double-clicking it.

```bash
./DrawingApp.exe
```

## How to Use the Application

The application is controlled via the menu bar and mouse inputs.

### Menu
- **Algorithms:** Select the desired drawing, filling, or clipping algorithm.
- **Colors:** Select the color for the next shape you draw.
- **Clear Screen:** Clears all shapes from the canvas.
- **Save:** Saves the current drawing to `shapes.txt`.
- **Load:** Loads a drawing from `shapes.txt`.

### Mouse Controls
The drawing behavior depends on the selected algorithm.

- **For Lines, Circles, Ellipses, and basic fills:**
  1. Select the algorithm from the menu.
  2. **Click and hold** the left mouse button to define the starting point (or center).
  3. **Drag** the mouse to the desired endpoint (or radius/boundary).
  4. **Release** the left mouse button to draw the shape.

- **For Polygons, Cardinal Splines, and Polygon Clipping:**
  1. Select the algorithm from the menu.
  2. **Left-click** repeatedly on the canvas to add vertices to the polygon/spline. You need at least 3 points for a polygon or 4 for a spline.
  3. **Right-click** to finalize the shape. The polygon will be drawn/filled, or the spline will be rendered.

- **For Flood Fill:**
  1. Select either Recursive or Non-Recursive Flood Fill.
  2. **Left-click** inside an enclosed area to fill it with the selected color.

- **For Clipping:**
  1. The application pre-draws two clipping windows: a rectangular one and a red, dashed square one.
  2. Select a clipping algorithm (e.g., "Clip Rect: Line").
  3. Draw the shape (point, line, or polygon) as described above. The shape will be clipped against the corresponding window upon being drawn.

## Code Structure

The entire application is contained within `graphics.cpp`.

- `WinMain`: The entry point for the Win32 application. Initializes the window class and creates the window.
- `WindowProc`: The core message loop handler. It processes user input (`WM_COMMAND`, `WM_LBUTTONDOWN`, etc.) and system messages (`WM_PAINT`, `WM_DESTROY`).
- `AddMenus`: Dynamically creates and attaches the menu bar to the window.
- `Shape` (struct): A data structure that stores the properties of a single drawn object, including its type (`DrawMode`), key points, color, and any extra data (like the quarter for circle fills).
- `shapes` (`std::vector<Shape>`): A global vector that stores all shapes drawn on the canvas. This vector is iterated during the `WM_PAINT` message to redraw the scene.
- **Algorithm Functions:** Each graphics algorithm is implemented in a dedicated function (e.g., `DrawLineDDA`, `ClipPolygon`, `FillConvexPolygon`).
- **Save/Load Functions:** `SaveShapes` and `LoadShapes` handle the serialization and deserialization of the `shapes` vector to and from a text file.
