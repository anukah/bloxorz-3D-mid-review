# Bloxorz 3D - Mid Review

A 3D implementation of the classic puzzle game **Bloxorz**.

---

## Platform Block States

| Code | Description        |
|------|--------------------|
| 0    | Empty space        |
| 1    | Normal block       |
| 2    | Target block       |
| 3    | Fragile tile (TODO)|
| 4    | Toggle tile (TODO) |
| 5    | Toggle action tile (TODO) |

---

## Camera Controls

- **W** → Zoom in  
- **S** → Zoom out  
- **A** → Rotate right  
- **D** → Rotate left  
- **1** → Preset camera angle 1  
- **2** → Preset camera angle 2  

---

## Notes
Compile using
```bash
clang++ main.cpp levels.cpp -o Bloxorz-3D -framework GLUT -framework OpenGL
```

After adding textures, compile using
```bash
clang++ main.cpp levels.cpp dependencies/include/SOIL2/SOIL2.c dependencies/include/SOIL2/image_DXT.c dependencies/include/SOIL2/image_helper.c dependencies/include/SOIL2/wfETC.c -o Bloxorz-3D -std=c++11 -I dependencies/include -framework CoreFoundation -framework GLUT -framework OpenGL
```
---
