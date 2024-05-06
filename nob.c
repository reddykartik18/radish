#define NOB_IMPLEMENTATION
#include "nob.h"
#include <time.h>

static double get_time(void) {
  struct timespec ts = {0};
  int ret = clock_gettime(CLOCK_MONOTONIC, &ts);
  NOB_ASSERT(ret == 0);
  return ts.tv_sec + ts.tv_nsec*0.000000001;
}

static void cc(Nob_Cmd *cmd) {
  nob_cmd_append(cmd, "cc");
  nob_cmd_append(cmd, "-Wall", "-Wextra", "-ggdb", "-pedantic");
  nob_cmd_append(cmd, "-O3", "-msse2", "-mfpmath=sse", "-ftree-vectorizer-verbose=5");
}

static bool rebuild_stb_if_needed(Nob_Cmd *cmd, const char *impl, const char *output, const char *input) {
  if (nob_needs_rebuild1(output, input)) {
    cmd->count = 0;
    cc(cmd);
    nob_cmd_append(cmd, impl);
    nob_cmd_append(cmd, "-x", "c");
    nob_cmd_append(cmd, "-c");
    nob_cmd_append(cmd, "-o", output);
    nob_cmd_append(cmd, input);
    return nob_cmd_run_sync(*cmd);
  } else {
    nob_log(NOB_INFO, "%s is up to date", output);
    return true;
  }
}

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);
  
  const char *program = nob_shift_args(&argc, &argv);
  (void) program;
  
  Nob_Cmd cmd = {0};

  if (!nob_mkdir_if_not_exists("builds/")) return 1;
  if (!rebuild_stb_if_needed(&cmd, "-DSTB_IMAGE_IMPLEMENTATION", "builds/stb_image.o", "include/stb_image.h")) return 1;
  if (!rebuild_stb_if_needed(&cmd, "-DSTB_IMAGE_WRITE_IMPLEMENTATION", "builds/stb_image_write.o", "include/stb_image_write.h")) return 1;
  const char *input = "main.c"; 
  const char *output = "builds/main";
  
  cmd.count = 0;
  cc(&cmd);
  nob_cmd_append(&cmd, "-o", output);
  nob_cmd_append(&cmd, input);
  nob_cmd_append(&cmd, "-Iinclude/");
  nob_cmd_append(&cmd, "builds/stb_image.o");
  nob_cmd_append(&cmd, "builds/stb_image_write.o");
  nob_cmd_append(&cmd, "-lm", "-lraylib");
  if (!nob_cmd_run_sync(cmd)) return 1;

  cmd.count = 0;
  nob_cmd_append(&cmd, output);
  nob_da_append_many(&cmd, argv, argc);
  double start = get_time();
  if (!nob_cmd_run_sync(cmd)) return 1;
  nob_log(NOB_INFO, "time taken %lfsec", get_time() - start);
  return 0;
}

