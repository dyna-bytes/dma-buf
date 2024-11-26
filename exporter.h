#ifndef __EXPORTER_H__
#define __EXPORTER_H__

#define DEVICE_NAME "exporter"
#define CLASS_NAME "exporter_class"

#define print_info(fmt, ...) \
    pr_info("[INFO][%s](%d)\t" fmt, __func__, __LINE__, ##__VA_ARGS__)
#define print_err(fmt, ...) \
    pr_err("[ERROR][%s](%d)\t" fmt, __func__, __LINE__, ##__VA_ARGS__)

extern struct dma_buf *dmabuf_exported;

#endif /* __EXPORTER_H__ */