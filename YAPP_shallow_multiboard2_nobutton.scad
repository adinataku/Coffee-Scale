// this version is customized for just the Feather ESP32S3RevTFT, small battery, and push buttons

//---------------------------------------------------------
// Yet Another Parameterized Projectbox generator
//
//  This will generate a projectbox for a "Adafruit Feather"
//
//  Version 3.0 (02-12-2023)
//
// This design is parameterized based on the size of a PCB.
//---------------------------------------------------------
include <../YAPPgenerator_v3.scad>
include </Users/adimulawarman/Documents/OpenSCAD/libraries/BOSL2-master/std.scad>
include </Users/adimulawarman/Documents/OpenSCAD/libraries/BOSL2-master/screws.scad>

// Note: length/lengte refers to X axis, 
//       width/breedte to Y, 
//       height/hoogte to Z

/*
      padding-back|<------pcb length --->|<padding-front
                            RIGHT
        0    X-as ---> 
        +----------------------------------------+   ---
        |                                        |    ^
        |                                        |   padding-right 
        |                                        |    v
        |    -5,y +----------------------+       |   ---              
 B    Y |         | 0,y              x,y |       |     ^              F
 A    - |         |                      |       |     |              R
 C    a |         |                      |       |     | pcb width    O
 K    s |         |                      |       |     |              N
        |         | 0,0              x,0 |       |     v              T
      ^ |   -5,0  +----------------------+       |   ---
      | |                                        |    padding-left
      0 +----------------------------------------+   ---
        0    X-as --->
                          LEFT
*/

//-- which half do you want to print? set this one at a time so the STL file is only the base or the lid. If the printer is large you maybe able to print both at the same time.
printBaseShell    = 1;
printLidShell     = 1;


myPCB1 = "STL/MODELS/5691Feather.stl";
myPCB2 = "STL/MODELS/4538NAU7802RevB.stl";
myPCB3 = "STL/MODELS/2922Feather.stl";



if (true)
{
  %translate([3.7, 30.5, 17.]) // [3.7, 30.5, 13.]
  {
    rotate([180,0,0]) color("lightgreen") import(myPCB1);
  }
  %translate([9., 36.5, 11.5]) 
  {
    rotate([0,0,0]) color("lightgreen") import(myPCB2);
  }
  %translate([54.5, 81.8, 11.5]) 
  {
    rotate([0,0,180]) color("lightgreen") import(myPCB3);
  }
}


//-- Edit these parameters for your own board dimensions
wallThickness       = 1.8;
basePlaneThickness  = 1.2;
lidPlaneThickness   = 1.7;

//-- Total height of box = basePlaneThickness + lidPlaneThickness 
//--                     + baseWallHeight + lidWallHeight
//-- space between pcb and lidPlane :=
//--      (baseWallHeight+lidWallheight) - (standoff_height+pcbThickness)
//--      (6.2 + 4.5) - (3.5 + 1.5) ==> 5.7
baseWallHeight    = 7.;//7.3
lidWallHeight     = 10.;//12.5

//-- Feather pcb dimensions - include the wing and everything if needed
pcbLength         = 82;
pcbWidth          = 75;
pcbThickness      = 2;
                            
//-- padding between pcb and inside wall
paddingFront      = 2;
paddingBack       = 2;
paddingRight      = 2;
paddingLeft       = 6;//6

//-- ridge where base and lid off box can overlap
//-- Make sure this isn't less than lidWallHeight
ridgeHeight       = 3.5;
roundRadius       = 2.0;

//-- How much the PCB needs to be raised from the base
//-- to leave room for solderings and whatnot
standoffHeight    = 10; //10
pinDiameter       = 1.; //
standoffDiameter  = 5;
//standoffPinDiameter = 2.4; not used
//standoffHoleSlack   = 0.4; no used

//-- C O N T R O L -------------//-> Default ---------
showSideBySide      = 1 ;     //-> true
previewQuality      = 11;        //-> from 1 to 32, Default = 5
renderQuality       = 16;        //-> from 1 to 32, Default = 8
onLidGap            = 0;
shiftLid            = 2; // enter the distance between lid and top when it is shows side by side
hideLidWalls        = 0;    //-> false
colorLid            = "gray";   
hideBaseWalls       = false;    //-> false
colorBase           = "gray";
showPCB             = 0;
showSwitches        = 1;
showPCBmarkers      = true;
showShellZero       = false;
showCenterMarkers   = false;
inspectX            = 0;        //-> 0=none (>0 from Back)
inspectY            = 0;        //-> 0=none (>0 from Right)
inspectZ            = 0;        //-> 0=none (>0 from Bottom)
inspectXfromBack    = 1;     //-> View from the inspection cut foreward
inspectYfromLeft    = true;     //-> View from the inspection cut to the right
inspectZfromTop     = true;    //-> View from the inspection cut down

  



//-- C O N T R O L ---------------------------------------


//===================================================================
// *** PCB Supports ***
// Pin and Socket standoffs 
//-------------------------------------------------------------------
//  Default origin =  yappCoordPCB : pcb[0,0,0]
//
//  Parameters:
//   Required:
//    (0) = posx
//    (1) = posy
//   Optional:
//    (2) = Height to bottom of PCB : Default = standoffHeight
//    (3) = PCB Gap : Default = -1 : Default for yappCoordPCB=pcbThickness, yappCoordBox=0
//    (4) = standoffDiameter    Default = standoffDiameter;
//    (5) = standoffPinDiameter Default = standoffPinDiameter;
//    (6) = standoffHoleSlack   Default = standoffHoleSlack;
//    (7) = filletRadius (0 = auto size)
//    (n) = { <yappBoth> | yappLidOnly | yappBaseOnly }
//    (n) = { yappHole, <yappPin> } // Baseplate support treatment
//    (n) = { <yappAllCorners> | yappFrontLeft | yappFrontRight | yappBackLeft | yappBackRight }
//    (n) = { yappCoordBox, <yappCoordPCB> }  
//    (n) = { yappNoFillet }
//-------------------------------------------------------------------



pcbStands = 
[// measurement from Adafruit was in inches so need to convert to mm for OpenScad
  [2.54,2.54,14,yappDefault,4,1.8, yappBoth,yappPin,yappNoFillet]       
 ,[2.54,20.32,14,yappDefault,4,1.8,  yappBoth, yappPin,yappNoFillet]    
 ,[48.1,1.9,14,yappDefault,4,1.8,    yappBoth, yappPin,yappNoFillet]  
 ,[48.26,20.8,14,yappDefault,4,1.8,  yappBoth, yappPin,yappNoFillet] 
 ,[2.54,27.,yappDefault,yappDefault,4,1.8, yappBoth,yappPin,yappNoFillet]
 ,[2.54,44.78,yappDefault,yappDefault,4,1.8, yappBoth,yappPin,yappNoFillet]  
 ,[22.86,27.,yappDefault,yappDefault,4,1.8, yappBoth,yappPin,yappNoFillet]
 ,[22.86,44.78,yappDefault,yappDefault,4,1.8, yappBoth,yappPin,yappNoFillet] 
 ,[2.54,53.54,yappDefault,yappDefault,4,1.8, yappBoth,yappPin,yappNoFillet]       
 ,[2.54,(53.54+17.78),yappDefault,yappDefault,4,1.8,  yappBoth, yappPin,yappNoFillet]    
 ,[2.54+45.72,53.54,yappDefault,yappDefault,4,1.8,    yappBoth, yappPin,yappNoFillet]  
 ,[2.54+45.72,53.54+17.78,yappDefault,yappDefault,4,1.8,  yappBoth, yappPin,yappNoFillet] 

];

//===================================================================
//  *** Cutouts ***
//    There are 6 cutouts one for each surface:
//      cutoutsBase (Bottom), cutoutsLid (Top), cutoutsFront, cutoutsBack, cutoutsLeft, cutoutsRight
//-------------------------------------------------------------------
//  Default origin = yappCoordBox: box[0,0,0]
//
//                        Required                Not Used        Note
//                      +-----------------------+---------------+------------------------------------
//  yappRectangle       | width, length         | radius        |
//  yappCircle          | radius                | width, length |
//  yappRoundedRect     | width, length, radius |               |     
//  yappCircleWithFlats | width, radius         | length        | length=distance between flats
//  yappCircleWithKey   | width, length, radius |               | width = key width length=key depth
//  yappPolygon         | width, length         | radius        | yappPolygonDef object must be provided
//
//  Parameters:
//   Required:
//    (0) = from Back
//    (1) = from Left
//    (2) = width
//    (3) = length
//    (4) = radius
//    (5) = shape : {yappRectangle | yappCircle | yappPolygon | yappRoundedRect | yappCircleWithFlats | yappCircleWithKey}
//  Optional:
//    (6) = depth : Default = 0/Auto : 0 = Auto (plane thickness)
//    (7) = angle : Default = 0
//    (n) = { yappPolygonDef } : Required if shape = yappPolygon specified -
//    (n) = { yappMaskDef } : If a yappMaskDef object is added it will be used as a mask for the cutout.
//    (n) = { [yappMaskDef, hOffset, vOffst, rotation] } : If a list for a mask is added it will be used as a mask for the cutout. With the Rotation and offsets applied. This can be used to fine tune the mask placement within the opening.
//    (n) = { <yappCoordBox> | yappCoordPCB }
//    (n) = { <yappOrigin>, yappCenter }
//  (n) = { yappLeftOrigin, <yappGlobalOrigin> } // Only affects Top(lid), Back and Right Faces
//-------------------------------------------------------------------
cutoutsLid =  
[
    [ 4, 2, 42, 19,   1,   yappRoundedRect]  // TFT display and buttons
    ,[ 55, 25, 27, 16,   1,   yappRoundedRect] //opening below the load cell
];
              
//-- base plane    -- origin is pcb[0,0,0]
// (0) = posx
// (1) = posy
cutoutsBase =   
[
    
];

//-- back plane  -- origin is pcb[0,0,0]
// (0) = posy
// (1) = posz
cutoutsBack = 
[
     [11.4,  0.5, 12, 5.5, 2, yappRoundedRect, 4, yappCenter, yappCoordPCB] // USB-c


];
//cutoutsFront = 
//[
//     [11,  -18, 16, 8, 1, yappRoundedRect, 0, yappCenter, yappCoordPCB] // SD card slot
//     ,[29,  -18, 0, 0, 4, yappCircle, 0, yappCenter, yappCoordPCB] // cable out
//    
//];

//-- right plane  -- origin is pcb[0,0,0]
// (0) = posX
// (1) = posZ
cutoutsRight = 
[
     // right headers
];
//-- right plane  -- origin is pcb[0,0,0]
// (0) = posX
// (1) = posZ

//maybe built all the button on this side
//on/off, TAR, etc.
cutoutsLeft = 
[
     [18,-6.0, 7, 7, 1, yappRoundedRect, 0, yappCenter, yappCoordPCB]
     ,[27,-6.0, 7, 7, 1, yappRoundedRect, 0, yappCenter, yappCoordPCB]
     ,[35,-6.0, 7, 7, 1, yappRoundedRect, 0, yappCenter, yappCoordPCB]
     ,[43,-6.0, 7, 7, 1, yappRoundedRect, 0, yappCenter, yappCoordPCB]
     ,[51,-6.0, 7, 7, 1, yappRoundedRect, 0, yappCenter, yappCoordPCB]
    ,[62,-4, 13, 13, 1, yappRoundedRect, 0, yappCenter, yappCoordPCB]   // on off buttons 8.65, 4.75
];


//===================================================================
//  *** Snap Joins ***
//-------------------------------------------------------------------
//  Default origin = yappCoordBox: box[0,0,0]
//
//  Parameters:
//   Required:
//    (0) = posx | posy
//    (1) = width
//    (n) = yappLeft / yappRight / yappFront / yappBack (one or more)
//   Optional:
//    (n) = { <yappOrigin> | yappCenter }
//    (n) = { yappSymmetric }
//    (n) = { yappRectangle } == Make a diamond shape snap
//-------------------------------------------------------------------
snapJoins   =   
[
    [5, 5,  yappRight, yappLeft,yappFront, yappSymmetric]
];

//===================================================================
//  *** Labels ***
//-------------------------------------------------------------------
//  Default origin = yappCoordBox: box[0,0,0]
//
//  Parameters:
//   (0) = posx
//   (1) = posy/z
//   (2) = rotation degrees CCW
//   (3) = depth : positive values go into case (Remove) negative values are raised (Add)
//   (4) = plane {yappLeft | yappRight | yappFront | yappBack | yappLid | yappBottom}
//   (5) = font
//   (6) = size
//   (7) = "label text"
//-------------------------------------------------------------------
//labelsPlane = [
//               [4, 5,  0, 1, yappLid, "Arial:style=bold", 4, "X" ]
//
//            ];

// this part is to include a slit on the side to insert magnets, 2.45 is the thickness and you can change it.
//translate([basePlaneThickness,pcbWidth+paddingLeft+paddingRight-2.45,0]){
//cube([pcbLength+paddingBack+paddingFront+basePlaneThickness,basePlaneThickness,baseWallHeight]);}


// this part is to include a base for the load sensor to be screwed on
translate([60.,34.2,0]){
color("gray")
cube([25,12.7,21]); }
translate([80.,40.55,+12]){
color("red")
screw_hole("M5",l=20.7);}
translate([65.,40.55,+12]){
color("red")
screw_hole("M5",l=20.7);}

// this part is to model the load sensor to be screwed on
/*
%translate([85,47,21]){
rotate([0,0,180])
cube([80,12.7,12.7])
;}
*/

//===================================================================
//  *** Push Buttons ***
//-------------------------------------------------------------------
//  Default origin = yappCoordPCB: PCB[0,0,0]
//
//  Parameters:
//   Required:
//    p(0) = posx
//    p(1) = posy
//    p(2) = capLength 
//    p(3) = capWidth 
//    p(4) = capRadius 
//    p(5) = capAboveLid
//    p(6) = switchHeight
//    p(7) = switchTravel
//    p(8) = poleDiameter
//   Optional:
//    p(9) = Height to top of PCB : Default = standoffHeight + pcbThickness
//    p(10) = { yappRectangle | yappCircle | yappPolygon | yappRoundedRect 
//                    | yappCircleWithFlats | yappCircleWithKey } : Shape, Default = yappRectangle
//    p(11) = angle : Default = 0
//    p(12) = filletRadius          : Default = 0/Auto 
//    p(13) = buttonWall            : Default = 2.0;
//    p(14) = buttonPlateThickness  : Default= 2.5;
//    p(15) = buttonSlack           : Default= 0.25;
//    p(16) = snapSlack             : Default= 0.10;
//    n(a) = { <yappCoordPCB> | yappCoordBox | yappCoordBoxInside } 
//    n(b) = { <yappGlobalOrigin>,  yappLeftOrigin }
//    n(c) = { yappNoFillet }
//    n(d) = [yappPCBName, "XXX"] : Specify a PCB. Defaults to [yappPCBName, "Main"]
//-------------------------------------------------------------------


/*pushButtons = 
[
    [6.8, 18.2,0, 0, 2, 2.5,2.55, 1, 1.5, yappDefault,yappCircle,yappDefault,yappDefault,yappDefault,1.2,yappDefault,yappDefault,yappNoFillet]
   ,[6.8, 11.2,0, 0, 2, 2.5,2.55, 1, 1.5, yappDefault,yappCircle,yappDefault,yappDefault,yappDefault,1.2,yappDefault,yappDefault,yappNoFillet]
   ,[6.8, 4.2,0, 0, 2, 2.5,2.55, 1, 1.5, yappDefault,yappCircle,yappDefault,yappDefault,yappDefault,1.2,yappDefault,yappDefault,yappNoFillet]   
   ,[44.8, 11.2,0, 0, 2, 2.5,2.55, 1, 1.5, yappDefault,yappCircle,yappDefault,yappDefault,yappDefault,1.22,yappDefault,yappDefault,yappNoFillet]     
];       
*/

YAPPgenerate();