#include <linux/platform_device.h>
#include <linux/dma-buf.h>
#include <linux/module.h>
#include <linux/slab.h>
#include "exporter.h"

static struct platform_device *exporter_device;
struct dma_buf *dmabuf_exported;
EXPORT_SYMBOL(dmabuf_exported);

static int exporter_attach(
    struct dma_buf *dmabuf,
    struct dma_buf_attachment *attachment
)
{
    PR_INFO("dmabuf attach device: %s\n", dev_name(attachment->dev));
    return 0;
}

static void exporter_detach(
    struct dma_buf *dmabuf,
    struct dma_buf_attachment *attachment
)
{
    PR_INFO("dmabuf detach device: %s\n", dev_name(attachment->dev));
}

static struct sg_table *exporter_map_dma_buf(
    struct dma_buf_attachment *attachment,
    enum dma_data_direction dir
)
{
    struct sg_table *table;
    void *vaddr = attachment->dmabuf->priv;
    int err;

    PR_INFO("start\n");
    table = kmalloc(sizeof(*table), GFP_KERNEL);
    if (!table)
        return ERR_PTR(-ENOMEM);

    err = sg_alloc_table(table, 1, GFP_KERNEL);
    if (err) {
        kfree(table);
        return ERR_PTR(err);
    }

    sg_dma_len(table->sgl) = PAGE_SIZE;
    sg_dma_address(table->sgl) = dma_map_single(&exporter_device->dev,
            vaddr, PAGE_SIZE, dir);
    PR_INFO("end\n");

    return table;
}

static void exporter_unmap_dma_buf(
    struct dma_buf_attachment *attachment,
    struct sg_table *table,
    enum dma_data_direction dir
)
{
    dma_unmap_single(&exporter_device->dev,
            sg_dma_address(table->sgl), PAGE_SIZE, dir);
    sg_free_table(table);
    kfree(table);
}

static void exporter_release(
    struct dma_buf *dmabuf
)
{
    kfree(dmabuf->priv);
}

static int exporter_vmap(
    struct dma_buf *dmabuf,
    struct iosys_map *map)
{
    PR_INFO("start\n");
    iosys_map_set_vaddr(map, dmabuf->priv);
    PR_INFO("end\n");
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
    .attach = exporter_attach,
    .detach = exporter_detach,
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
    PR_INFO("start\n");

    exporter_device = platform_device_alloc("exporter_device", -1);
    if (!exporter_device) {
        PR_ERR("failed to allocate platform device\n");
        return -ENOMEM;
    }

    if (platform_device_add(exporter_device)) {
        PR_ERR("failed to add platform device\n");
        platform_device_put(exporter_device);
        return -EINVAL;
    }

    PR_INFO("Platform device created successfully\n");

    dmabuf_exported = exporter_alloc_page();
	if (!dmabuf_exported) {
		PR_ERR("exporter alloc page failed\n");
		return -ENOMEM;
	}

    PR_INFO("end\n");

    return 0;
}

static void __exit exporter_exit(void)
{
    PR_INFO("start\n");
    platform_device_unregister(exporter_device);
    PR_INFO("end\n");
}

module_init(exporter_init);
module_exit(exporter_exit);

MODULE_AUTHOR("Jihyuk Park");
MODULE_IMPORT_NS(DMA_BUF);
MODULE_DESCRIPTION("DMA-BUF example for EXPORTER");
MODULE_LICENSE("GPL v2");
