#include <linux/fb.h>
#include <time.h>
#include <pthread.h>
#include <fbink.h>

#include "eink.h"
#include "draw.h"

// Right now we just refresh the whole framebuffer at every update
// We can do way better: we could communicate small rectangles to
// fbink_worker, but the math for this involves math I'm too lazy
// for right now.
static const FBInkConfig fbpad_fbink_einkConfig = { 0U }; 
static const struct timespec fbpad_fbink_cooldown_ts = { 0, 30*1000*1000 };

#define REFRESH_STACK_LEN 1

static pthread_t fbpad_fbink_refresh_thread;
static pthread_mutex_t fbpad_fbink_stop_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t fbpad_fbink_refresh_mutex = PTHREAD_MUTEX_INITIALIZER;
static int fbpad_fbink_refresh_stack = 0;
static bool fbpad_fbink_stop_flag = false;
static int fbpad_fbink_fb_fd;

void fbpad_fbink_start(int fd, struct fb_var_screeninfo *pvinfo, struct fb_fix_screeninfo *pfinfo) {
  printf("Using eink.\n");
  fbpad_fbink_fb_fd = fd;
  fbink_init(fbpad_fbink_fb_fd, &fbpad_fbink_einkConfig);
  fbink_get_fb_info(pvinfo, pfinfo); // fbink performs platform-specific corrections to screen info structs
  pthread_create(&fbpad_fbink_refresh_thread, NULL, &fbpad_fbink_worker, NULL);
}

void fbpad_fbink_stop(void) {
  pthread_mutex_lock(&fbpad_fbink_stop_mutex);
  fbpad_fbink_stop_flag = true;
  pthread_mutex_unlock(&fbpad_fbink_stop_mutex);
  pthread_join(fbpad_fbink_refresh_thread, NULL);
}

void fbpad_fbink_refresh(void) {
  pthread_mutex_lock(&fbpad_fbink_refresh_mutex);
  if(fbpad_fbink_refresh_stack < REFRESH_STACK_LEN) {
    fbpad_fbink_refresh_stack++;
  }
  pthread_mutex_unlock(&fbpad_fbink_refresh_mutex);
}

void *fbpad_fbink_worker(void *p) {
  bool b_stop = false;
  int n_refresh = 0;
  while(true) {
    pthread_mutex_lock(&fbpad_fbink_stop_mutex);
    b_stop = fbpad_fbink_stop_flag;
    pthread_mutex_unlock(&fbpad_fbink_stop_mutex);
    if(b_stop) break;
    
    nanosleep(&fbpad_fbink_cooldown_ts, NULL);

    pthread_mutex_lock(&fbpad_fbink_refresh_mutex);
    n_refresh = fbpad_fbink_refresh_stack;
    pthread_mutex_unlock(&fbpad_fbink_refresh_mutex);
    if(n_refresh > 0) { 
      pthread_mutex_lock(&fbpad_fbink_refresh_mutex);
      fbpad_fbink_refresh_stack--;
      pthread_mutex_unlock(&fbpad_fbink_refresh_mutex);
      
      // WIP: improve the following call
      fbink_refresh(fbpad_fbink_fb_fd, fb_yoffset(), fb_xoffset(), fb_cols(), fb_rows(), &fbpad_fbink_einkConfig); 
    }
  }
  return p;
}
