#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  /*int i;
  int w = (inl(VGACTL_ADDR) >> 16);  // TODO: get the correct width
  int h = (uint16_t)inl(VGACTL_ADDR); // TODO: get the correct height
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (i = 0; i < w * h; i ++) fb[i] = i;
  outl(SYNC_ADDR, 1);*/
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = (inl(VGACTL_ADDR) >> 16), .height = (uint16_t)inl(VGACTL_ADDR),
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int W = (inl(VGACTL_ADDR) >> 16);
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (int i = 0; i < ctl->h; i++) 
    for (int j = 0; j < ctl->w; j++){
      fb[W * (ctl->y + i) + ctl->x + j] = *((uint32_t *)(ctl->pixels) + i * (ctl->w) + j);
    }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
