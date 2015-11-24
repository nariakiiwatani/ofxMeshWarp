# ofxMeshWarp
Warping mesh points with mouse and keyboard.  
You can do it by code, of course.

## How to use
### code
See example

### selecting points
You can select/deselect point(s) by clicking on the point or making rectangle including the points.  
With shift key pressed, additive.  
With command key pressed, alternative.

### moving points
You can move points' position by dragging on a selected point.  
With option key pressed, texcoord will be moved instead.  
With shift key pressed, points move according axis.  
You can also move by arrow keys.

### switching nodal or not
Each point has a property of nodal or not.  
By clicking on a point with option key pressed, you can switch it.  
If a point is nodal, you can control its position or texcoord directly.  
If a point is not nodal, you cannot but it will be controled by mood.

## Other Features
### Save/Load
See example

### dividing mesh
Mesh dividing function is already implemented but interface is not yet.  
I haven't come up with any good idea of it. XD  

### Creating you own controller
Since some of the functions of ofxMeshWarpController are marked virtual, you can create your own controller.  
It means you can use it with your confortable key bindings.  
If you created better bindings especially on other platforms, please send PR!

## todo
- dividing interface  
- erasing implementation and interface  
- exporting as ofMesh(if needed)  

## Tested on
- Mac OSX 10.10.5  
- oF 0.9.0  
- Xcode 7.0.1

## License
MIT license.