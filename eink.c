#include <linux/fb.h>
#include <time.h>
#include <pthread.h>
#include <fbink.h>

#include "eink.h"
#include "draw.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

static const FBInkConfig fbpad_fbink_einkConfig = { 0U }; 
static const struct timespec fbpad_fbink_cooldown_ts = { 0, 5*1000*1000 };
static int fbpad_fbink_fb_fd;

static bool fbpad_fbink_refresh_flag = 0;
static bool fbpad_fbink_stop_flag = false;
static pthread_t fbpad_fbink_refresh_thread;
static pthread_mutex_t fbpad_fbink_stop_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t fbpad_fbink_refresh_mutex = PTHREAD_MUTEX_INITIALIZER;

static FBInkRect fbpad_fbink_rect = { 0U };
static FBInkRect fbpad_fbink_rect_buf = { 0U };
static const FBInkRect* fbpad_fbink_rect_arg = &fbpad_fbink_rect_buf;

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

void fbpad_fbink_refresh(unsigned short int left, unsigned short int top, unsigned short int width, unsigned short int height) {
  unsigned short int ec_old = fbpad_fbink_rect.left + fbpad_fbink_rect.width;
  unsigned short int er_old = fbpad_fbink_rect.top  + fbpad_fbink_rect.height;
  unsigned short int ec = left + width;
  unsigned short int er = top  + height;
  pthread_mutex_lock(&fbpad_fbink_refresh_mutex);
  fbpad_fbink_rect.left =   MAX(fb_xoffset(), MIN(fbpad_fbink_rect.left,   left));
  fbpad_fbink_rect.top =    MAX(fb_yoffset(), MIN(fbpad_fbink_rect.top,    top));
  fbpad_fbink_rect.width =  MIN(fb_cols(),    MAX(ec_old,ec)-fbpad_fbink_rect.left);
  fbpad_fbink_rect.height = MIN(fb_rows(),    MAX(er_old,er)-fbpad_fbink_rect.top );
  fbpad_fbink_refresh_flag = 1;
  pthread_mutex_unlock(&fbpad_fbink_refresh_mutex);
}

void *fbpad_fbink_worker(void *p) {
  bool b_stop = false;
  bool b_refresh = 0;
  while(true) {
    pthread_mutex_lock(&fbpad_fbink_stop_mutex);
    b_stop = fbpad_fbink_stop_flag;
    pthread_mutex_unlock(&fbpad_fbink_stop_mutex);
    if(b_stop) break;
    
    nanosleep(&fbpad_fbink_cooldown_ts, NULL);

    pthread_mutex_lock(&fbpad_fbink_refresh_mutex);
    b_refresh = fbpad_fbink_refresh_flag;
    pthread_mutex_unlock(&fbpad_fbink_refresh_mutex);

    if(b_refresh) { 
      pthread_mutex_lock(&fbpad_fbink_refresh_mutex);
      fbpad_fbink_rect_buf = fbpad_fbink_rect;
      fbpad_fbink_refresh_flag = 0;
      fbpad_fbink_rect = (FBInkRect) { 0U };
      pthread_mutex_unlock(&fbpad_fbink_refresh_mutex);
      
      fbink_refresh_rect(fbpad_fbink_fb_fd, fbpad_fbink_rect_arg, &fbpad_fbink_einkConfig);
    }
  }
  return p;
}
