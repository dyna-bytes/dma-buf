#include <linux/dma-buf.h>
#include <linux/module.h>
#include <linux/slab.h>
#include "exporter.h"

struct dma_buf *dmabuf_exported;
EXPORT_SYMBOL(dmabuf_exported);

static struct sg_table *exporter_map_dma_buf(
    struct dma_buf_attachment *attachment,
    enum dma_data_direction dir
)
{
    return NULL;
}

static void exporter_unmap_dma_buf(
    struct dma_buf_attachment *attachment,
    struct sg_table *table,
    enum dma_data_direction dir
)
{
}

static void exporter_release(
    struct dma_buf *dmabuf
)
{
}

static int exporter_vmap(
    struct dma_buf *dmabuf,
    struct iosys_map *map)
{
    pr_info("start %s\n", __func__);
    iosys_map_set_vaddr(map, dmabuf->priv);
    pr_info("end %s\n", __func__);
	return 0;
}

static void exporter_vunmap(
    struct dma_buf *dmabuf,
    struct iosys_map *map)
{
    iosys_map_clear(map);
}

static int exporter_mmap(
    struct dma_buf *dmabuf,
    struct vm_area_struct *vma
)
{
    return -ENODEV;
}

static const struct dma_buf_ops exp_dmabuf_ops = {
    .map_dma_buf = exporter_map_dma_buf,
    .unmap_dma_buf = exporter_unmap_dma_buf,
    .release = exporter_release,
    .vmap = exporter_vmap,
    .vunmap = exporter_vunmap,
    .mmap = exporter_mmap,
};

static struct dma_buf *exporter_alloc_page(void)
{
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	struct dma_buf *dmabuf;
	void *vaddr;

	vaddr = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!vaddr)
		return NULL;

	exp_info.ops = &exp_dmabuf_ops;
	exp_info.size = PAGE_SIZE;
	exp_info.flags = O_CLOEXEC;
	exp_info.priv = vaddr;

	dmabuf = dma_buf_export(&exp_info);
	if (IS_ERR(dmabuf)) {
		kfree(vaddr);
		return NULL;
	}

	sprintf(vaddr, "hello world!");

	return dmabuf;
}


static int __init exporter_init(void)
{
    pr_info("start %s\n", __func__);

    dmabuf_exported = exporter_alloc_page();
	if (!dmabuf_exported) {
		pr_err("error: exporter alloc page failed\n");
		return -ENOMEM;
	}

    pr_info("end %s\n", __func__);

    return 0;
}

static void __exit exporter_exit(void)
{
    pr_info("start %s\n", __func__);
    pr_info("end %s\n", __func__);
}

module_init(exporter_init);
module_exit(exporter_exit);

MODULE_AUTHOR("Jihyuk Park");
MODULE_IMPORT_NS(DMA_BUF);
MODULE_DESCRIPTION("DMA-BUF example for EXPORTER");
MODULE_LICENSE("GPL v2");
