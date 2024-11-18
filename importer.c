#include <linux/dma-buf.h>
#include <linux/module.h>
#include <linux/slab.h>
#include "importer.h"

static int importer_test(struct dma_buf *dmabuf)
{
	struct iosys_map map;

	if (!dmabuf) {
		pr_err("dmabuf_exported is null\n");
		return -EINVAL;
	}

	dma_buf_vmap(dmabuf, &map);
	pr_info("read from dmabuf vmap: %s\n", (char *)map.vaddr);
	dma_buf_vunmap(dmabuf, &map);

    return 0;
}

static int __init importer_init(void)
{
	pr_info("start %s\n", __func__);
	importer_test(dmabuf_exported);
    pr_info("end %s\n", __func__);
	return 0;
}

static void __exit importer_exit(void)
{
	pr_info("start %s\n", __func__);
    pr_info("end %s\n", __func__);
}

module_init(importer_init);
module_exit(importer_exit);


MODULE_AUTHOR("Jihyuk Park");
MODULE_IMPORT_NS(DMA_BUF);
MODULE_DESCRIPTION("DMA-BUF example for IMPORTER");
MODULE_LICENSE("GPL v2");