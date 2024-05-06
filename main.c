#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include "stb_image.h"
#include "stb_image_write.h"
//#include "raylib.h"

#define MAT_AT(mat, x, y) (mat).data[(y)*(mat.stride)+(x)]

typedef struct Img {
  int width, height, stride;
  uint32_t *data;
} Img;

typedef struct Mat {
  int width, height, stride;
  float *data;
} Mat;

static bool Malloc(int width, int height, Mat *mat) {
  float *data = malloc(width*height*sizeof(float));
  if (!data) return false;
  if (mat->data) free(mat->data);
  *mat = (Mat) { width, height, width, data };
  return true;
}

static bool load_image(const char *input, Img *img) {
  int width, height;
  uint32_t *data = (uint32_t *) stbi_load(input, &width, &height, NULL, 4);
  if (!data) return false;
  *img = (Img) { width, height, width, data };
  return true;
}

static void grayscale(Img img, Mat mat) {
  size_t size = img.width * img.height;
  for (size_t index = 0; index < size; ++index) {
    uint32_t value = img.data[index];
    float r = (0xFF & (value >> (8*0))) / 255.0f;
    float g = (0xFF & (value >> (8*1))) / 255.0f;
    float b = (0xFF & (value >> (8*2))) / 255.0f;
    mat.data[index] = (0.2126*r + 0.7152*g + 0.0722*b);
  }
}

static float sobel_at(int cx, int cy, Mat mat) {
  float filter[][3] = {
    {1.0f, 0.0f, -1.0f},
    {2.0f, 0.0f, -2.0f},
    {1.0f, 0.0f, -1.0f}
  };
  float sx = 0.0f;
  float sy = 0.0f;
  for (int dy = 0; dy <= 2; ++dy) {
    for (int dx = 0; dx <= 2; ++dx) {
      int y = cy + dy - 1;
      int x = cx + dx - 1;
      if (0 <= x && x < mat.width && 0 <= y && y < mat.height) {
        sx += filter[dy][dx] * MAT_AT(mat, x, y);
        sy += filter[dx][dy] * MAT_AT(mat, x, y);
      }
    }
  }
  return sqrtf(sx*sx + sy*sy);
}

static void sobel(Mat mat, Mat grad) {
  for (int y = 0; y < mat.height; ++y) {
    for (int x = 0; x < mat.width; ++x) {
      MAT_AT(grad, x, y) = sobel_at(x, y, mat);
    }
  }
}


//static void print_mat(Mat grad) {
//  for (int y = 0; y < grad.height; ++y) {
//    for (int x = 0; x < grad.width; ++x) {
//      printf("%.2f ", MAT_AT(grad, x, y));
//    }
//    printf("\n");
//  }
//}

//static void forward(Mat grad, Mat map) {
//  for (int x = 0; x < grad.width; ++x) {
//    MAT_AT(map, x, 0) = MAT_AT(grad, x, 0);
//  }
//
//  for (int y = 1; y < grad.height; ++y) {
//    for (int cx = 0; cx < grad.width; ++cx) {
//      float min = MAT_AT(grad, cx, y-1);
//      int index = 0;
//      for (int dx=-1; dx <= 1; ++dx) {
//        int x = cx+dx;
//        if (0<=x && x < grad.width && MAT_AT(grad, x, y-1) < min) {
//          min = MAT_AT(grad, x, y-1);
//          index = dx;
//        }
//      }
//      float value = 0.0f;
//      if (0<=cx-1 && cx+1 < grad.width) {
//        value = fabsf(MAT_AT(grad, cx+1, y) - MAT_AT(grad, cx-1, y));
//        switch (index) {
//          case -1: 
//            value += fabsf(MAT_AT(grad, cx-1, y) - MAT_AT(grad, cx, y-1));
//            break;
//          case 1:
//            value += fabsf(MAT_AT(grad, cx+1, y) - MAT_AT(grad, cx, y-1));
//            break;
//        }
//      }
//      MAT_AT(map, cx, y) = (MAT_AT(grad, cx, y)) + min + value;
//    }
//  }
//}

static void energy_y(Mat grad, Mat map) {
  for (int x = 0; x < grad.width; ++x) {
    MAT_AT(map, x, 0) = MAT_AT(grad, x, 0);
  }
  
  for (int y = 1; y < grad.height; ++y) {
    for (int cx = 0; cx < grad.width; ++cx) {
      float min = FLT_MAX;
      for (int dx = 0; dx <= 2; ++dx) {
        int x = cx + dx - 1;
        if (0 <= x && x < grad.width) {
          min = fminf(min, MAT_AT(map, x, y-1));
        }
      }
      MAT_AT(map, cx, y) = (MAT_AT(grad, cx, y) + min);
    }
  }
}

//static void energy_x(Mat grad, Mat map) {
//  for (int y = 0; y < grad.height; ++y) {
//    MAT_AT(map, 0, y) = MAT_AT(grad, 0, y);
//  }
//  
//  for (int x = 1; x < grad.width; ++x) {
//    for (int cy = 0; cy < grad.height; ++cy) {
//      float min = FLT_MAX;
//      for (int dy = 0; dy <= 2; ++dy) {
//        int y = cy + dy - 1;
//        if (0 <= y && y < grad.height) {
//          min = fminf(min, MAT_AT(map, x-1, y));
//        }
//      }
//      MAT_AT(map, x, cy) = (MAT_AT(grad, x, cy) + min);
//    }
//  }
//}

static void compute_seam(Mat map, int *seam) {
  int y = map.height-1;
  seam[y] = 0;
  for (int x = 1; x < map.width; ++x) {
    if (MAT_AT(map, x, y) < MAT_AT(map, seam[y], y)) {
      seam[y] = x;
    }
  }

  for (int y = map.height-2; y >= 0; --y) {
    seam[y] = seam[y+1];
    for (int dx = -1; dx <= 1; ++dx) {
      int x = seam[y+1]+dx;
      if (0 <= x && x < map.width && MAT_AT(map, x, y) < MAT_AT(map, seam[y], y)) {
        seam[y] = x;
      }
    }
  }
}

static void rm_img(Img img, int x, int y) {
  uint32_t *row = &MAT_AT(img, 0, y);
  memmove(row+x, row+x+1, (img.width-x-1)*sizeof(uint32_t));
}

static void rm_mat(Mat mat, int x, int y) {
  float *row = &MAT_AT(mat, 0, y);
  memmove(row+x, row+x+1, (mat.width-x-1)*sizeof(float));
}

static void rm(Img *img, Mat *mat, Mat *grad, Mat *map, int *seam) {
  for (int y = 0; y < img->height; ++y) {
    int x = seam[y];
    rm_img(*img, x ,y);
    rm_mat(*mat, x, y);
  }
  --(img->width);
  --(mat->width);
  --(grad->width);
  --(map->width);
}

//static void add_img(Img img, int x, int y) {
//  uint32_t *row = &MAT_AT(img, 0, y);
//  memmove(row+x+1, row+x, (img.width-x)*sizeof(uint32_t));
//  row[x]=row[x+1];
//  //row[x]=0xFF0000FF;
//}
//
//static void add_mat(Mat mat, int x, int y) {
//  float *row = &MAT_AT(mat, 0, y);
//  memmove(row+x+1, row+x, (mat.width-x)*sizeof(float));
//  row[x]=row[x+1];
//  //row[x]=0xFF0000FF;
//}
//
//static void add(Img *img, Mat *mat, Mat *grad, Mat *map, int *seam) {
//  img->data = (uint32_t *) realloc(img->data, (img->width+1)*img->height*sizeof(uint32_t));
//  mat->data = (float *) realloc(mat->data, (mat->width+1)*mat->height*sizeof(float));
//  grad->data = (float *) realloc(grad->data, (grad->width+1)*grad->height*sizeof(float));
//  map->data = (float *) realloc(map->data, (map->width+1)*map->height*sizeof(float));
//  for (int y = 0; y < img->height; ++y) {
//    int x = seam[y];
//    add_img(*img, x, y);    
//    add_mat(*mat, x, y);
//  }
//
//  ++(img->width);
//  ++(mat->width);
//  ++(grad->width);
//  ++(map->width);
//
//  //++(img->stride);
//  //++(mat->stride);
//  //++(grad->stride);
//  //++(map->stride);
//}

//static void normalize(Mat mat) {
//  size_t size = mat.width * mat.height;
//  float max = FLT_MIN;
//  float min = FLT_MAX;
//  for (size_t index = 0; index < size; ++index) {
//    min = fminf(min, mat.data[index]);
//    max = fmaxf(max, mat.data[index]);
//  }
//
//  for (size_t index = 0; index < size; ++index) {
//    mat.data[index] = ((mat.data[index] - min) / (max - min))*255.0f;
//  }
//}
//
//static void colored(Mat mat, Img img) {
//  size_t size = mat.width * mat.height;
//  for (size_t index = 0; index < size; ++index) {
//    uint32_t value = mat.data[index];
//    img.data[index] = (0xFF000000|(value << (8*2))|(value << (8*1))|(value << (8*0)));
//  }
//}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Expected 2 arguments but found %d\n", argc-1);
    return 1;
  }
  const char *input = argv[1];
  const char *output = argv[2];
  int errno = 0;

  Img img = {0};
  if (!load_image(input, &img)) {
    fprintf(stderr, "Could not load image\n");
    return 1;
  }

  Mat mat = {0};
  Mat grad = {0};
  Mat map = {0};
  int *seam = (int *) malloc(img.height*sizeof(int));
  if(!seam) {
    fprintf(stderr, "Could not alloc memory for seam\n");
    errno=1;
    goto terminate;
  }

  if (!Malloc(img.width, img.height, &mat)) {
    fprintf(stderr, "Could not alloc memory for matrix\n");
    errno = 1;
    goto terminate;
  }
  
  if (!Malloc(img.width, img.height, &grad)) {
    fprintf(stderr, "Could not alloc memory for matrix\n");
    errno = 1;
    goto terminate;
  }

  if (!Malloc(img.width, img.height, &map)) {
    fprintf(stderr, "Could not alloc memory for matrix\n");
    errno = 1;
    goto terminate;
  }
  
//  InitWindow(img.width, img.height, "radish");
//  SetTargetFPS(60);
//  while (!WindowShouldClose()) {
    grayscale(img, mat);
    int update = img.width*0.5f;
    for (int i = 0; i < update; ++i) {
      sobel(mat, grad);
      energy_y(grad, map);
      compute_seam(map, seam);
//      BeginDrawing();
//      ClearBackground(BLACK);
//      for (int y = 0; y < img.height; ++y) {
//        for (int x = 0; x < img.width; ++x) {
//          uint32_t value = MAT_AT(img, x, y);
//          uint8_t r = (0xFF & (value >> (8*0)));
//          uint8_t g = (0xFF & (value >> (8*1)));
//          uint8_t b = (0xFF & (value >> (8*2)));
//          Color color = { r, g, b, 255 };
//          DrawPixel(x, y, color);
//        }
//      }
//      EndDrawing();
      rm(&img, &mat, &grad, &map, seam);
    //normalize(map);
    //colored(map, img);
    }
//    break;
//  } 
//  CloseWindow();
  if (!stbi_write_png(output, img.width, img.height, 4, img.data, img.stride*4)) {
    fprintf(stderr, "Could not save image\n");
    errno = 1;
    goto terminate;
  }
  
terminate:
  if (img.data) free(img.data);
  if (mat.data) free(mat.data);
  if (grad.data) free(grad.data);
  if (map.data) free(map.data);
  if (seam) free(seam);
  return errno;
}

