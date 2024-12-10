#include <linux/fb.h>
#include <fbink.h>

void fbpad_fbink_start(int fd, struct fb_var_screeninfo *vinfo, struct fb_fix_screeninfo *finfo);
void fbpad_fbink_stop(void);
void *fbpad_fbink_worker(void *p);
void fbpad_fbink_refresh(void);
