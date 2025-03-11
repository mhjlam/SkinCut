# Real-time Simulation and Visualization of Cutting Wounds

C++ source code for master thesis **Real-time Simulation and Visualization of Cutting Wounds**.

**Author:** Maurits Lam </br>
**Paper:** https://studenttheses.uu.nl/handle/20.500.12932/22174 </br>
**Abstract:**
> Many modern computer games and medical computer simulations feature skin injuries such as cuts and incisions. These two fields often approach this topic in different ways, where medical simulations commonly have a minimal visualization, and visualization in games is usually heavily based on artistic influence. As far as we know, no methods currently exist that combine mesh cutting simulation with skin visualization techniques in order to synthesize cutting wounds in real time. Previous works neglect to describe a complete remeshing scheme that is able to maintain the topology and parameterization of an input surface mesh during interactive creation of arbitrary cuts. Additionally, the appearance and synthesis of cutting wounds has not been sufficiently addressed. In this thesis, we explore the feasibility of constructing a damage model that simulates and visualizes natural-looking cutting wounds by generating new geometry and texture maps in real time. We present a cutting simulation approach to geometrically merge a cutting line into a mesh surface which is then subsequently opened, revealing interior wound geometry that is generated in situ. Likewise, a wound texture for surface injury visualization is generated during runtime. Finally, we propose an extension to subsurface scattering to locally discolor skin surface around a cutting area. Our approach is lightweight: using a mid-range desktop computer, cuts can be created in about 50 milliseconds on average, and a typical frame is rendered in about 2.5 milliseconds. We think that our approach can be attractive for increasing the realism of cutting wounds in real-time applications without heavy reliance on manual artistic input.

## Minimum system requirements

* Windows 8 or later (x86/x64)
* 1 GHz processor (minimum for Windows 8)
* 1 GB memory (minimum for Windows 8)
* DirectX 11.0 compatible video card

Probably also works on Windows 7 with some minor modifications.

## Build requirements

* Visual Studio 2015 or later
* Windows SDK 8.0 or later

Open the provided Visual Studio solution to build the solution. Required libraries are included.

## Libraries used

* [DirectX Tool Kit](https://github.com/Microsoft/DirectXTK) (January 5, 2016)
* [DirectXTex texture processing library](https://github.com/Microsoft/DirectXTex) (November 30, 2015)
* [ImGui](https://github.com/ocornut/imgui) (March 26, 2015; v1.37)
* [SimpleJSON](https://github.com/MJPA/SimpleJSON) (May 26, 2012)

## Resource files

The application requires several resources to be present. By default it looks for a directory relative to the executable at `../../resources`. This directory and all subdirectories must remain intact. An alternate (relative or absolute) path to the `resources` directory can also be supplied as an argument to the program executable. Files `resources/config.json` and `resources/scene.json` can be modified to make changes to the configuration and scene definition, respectively.

## Quick user guide

Hold **_Shift_** and click **_Left Mouse_** to select the start of a new cut. Repeat to select the end of the cut. Note that a cut must span at least two mesh faces. Pressing **_P_** cycles through the available pick modes (draw wound texture only, draw wound texture and merge cutting line, or draw wound texture and carve incision).

The camera can be operated with the mouse. Hold **_Left Mouse_** to rotate around the model. **_Right Mouse_** zooms in and out. **_Middle Mouse_** pans the camera.

Rendering options can be modified by using the panels on the left side of the screen. Change the renderer with the drop down panel. For the primary shader, *Kelemen/Szirmay-Kalos*, several rendering features can be toggled on or off, its shading can be fine-tuned, and the intensity of the scene lights can be modified. The user interface itself can be toggled on and off by pressing **_F1_**.

Hold **_Ctrl_** and click **_Left Mouse_** to subdivide the face under the mouse cursor with the selected subdivision mode (3-split, 4-split, or 6-split). The current mode is shown in the bottom-right and can be cycled through with the **_S_** key.

Press **_R_** to reload the scene at any time. In case of a critical runtime error, the application will ask to reload the scene as well. Note that this process may take a few seconds during which the application becomes unresponsive.

The performance test described in the thesis can be executed by pressing **_T_**. Running times (in milliseconds) are written to the console window. This process can take several _minutes_ during which the application becomes unresponsive.

## License
This source code is released under the [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0) license.
