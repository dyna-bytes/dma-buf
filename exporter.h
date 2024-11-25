#ifndef __EXPORTER_H__
#define __EXPORTER_H__

extern struct dma_buf *dmabuf_exported;

#define PR_INFO(fmt, ...) \
    pr_info("[INFO][%s](%d)\t" fmt, __func__, __LINE__, ##__VA_ARGS__)
#define PR_ERR(fmt, ...) \
    pr_err("[ERROR][%s](%d)\t" fmt, __func__, __LINE__, ##__VA_ARGS__)

#endif /* __EXPORTER_H__ */