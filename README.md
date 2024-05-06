# Seam Carving 
---
Content aware resizing using ![seam carving](https://en.wikipedia.org/wiki/Seam_carving)

---
*Seam removal*


---
*Original        |   Seam insertion*
:---------------:|:--------------------:
![original](images/radish.png) |  


## Quick start
---

```console
$ cc -o nob nob.c
$ nob images/radish.png output.png
$ feh output.png
```

> [!TODO]
> - *only \*.png files for output is working.
> - seam insertion is not working as expected.
> - implementation for horizontal seams.
> - make it faster.*

