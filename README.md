# Seam Carving 
Content-aware resizing using [seam carving](https://en.wikipedia.org/wiki/Seam_carving)

https://github.com/reddykartik18/radish/assets/155046193/fa4b451a-720f-42be-b26f-1c45d7ac0867
*Original*        |   *Seam insertion*
:---------------:|:--------------------:
![original](images/radish.png) |  ![output](https://github.com/reddykartik18/radish/assets/155046193/5dc01e6b-d58d-45a4-9f36-ce1d3fbdfcf4)

## Quick start
```console
$ cc -o nob nob.c
$ nob images/radish.png output.png
$ feh output.png
```

## TODO
  - *only \*.png files for output is working.*
  - *seam insertion is not working as expected.*
  - *implementation for horizontal seams.*
  - *make it faster.*
