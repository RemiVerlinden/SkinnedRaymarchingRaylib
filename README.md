# Raymaching skinned meshes Demo

## Note
I will update the readme with a consice explanation about the demo when it is finished.  

## prerequisites
- CMAKE
- Visual Studio 22 c++
- Windows 10/11? 
## How to build and run
1. clone repo
2. right click inside root repo folder, select "Open with Visual Studio".
![image](https://github.com/user-attachments/assets/72d820ad-789a-4ecd-b02e-deaea4d311d5)

3. Click the "Select Startup Item..." dropdown.
![image](https://github.com/user-attachments/assets/680e35b7-d006-4f0a-9f63-ab56d1573b53)

4. look for "RaymarchingSkinnedMesh.exe" in between the long list of junk. (if you cant find it -> wait until you see "1> CMake generation finished." in the vs22 output console)
![image](https://github.com/user-attachments/assets/bd940f27-58f2-4850-b83f-008dd490a478)

5. select it and run the project.

## Info
These libraries are used:
- Raylib
- Raylib-gizmo
- tinyEXR (Raylib makes use of stb_image which does not provide support for OpenEXR files. This library is necessary as my volumetric SDF texture is encoded in EXR)
- KLEIN (math library for easy acces to required Dual Quaternion operations)


This demo has partially repurposed the shader file provided in the GPU skinning example  from raylib.

# This project is not finished yet
