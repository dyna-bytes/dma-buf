#include <linux/dma-buf.h>
#include <linux/module.h>
#include <linux/slab.h>
#include "importer.h"

static int importer_test_sg(struct dma_buf *dmabuf)
{
	struct dma_buf_attachment *attachment;
	struct sg_table *table;
	struct device *dev;
	unsigned int reg_addr, reg_size;

	if (!dmabuf)
		return -EINVAL;

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -EINVAL;

	dev_set_name(dev, "importer");

	attachment = dma_buf_attach(dmabuf, dev);
	if (IS_ERR(attachment)) {
		pr_err("dma_buf_attach() failed\n");
		return PTR_ERR(attachment);
	}

	table = dma_buf_map_attachment(attachment, DMA_BIDIRECTIONAL);
	if (IS_ERR(table)) {
		pr_err("dma_buf_map_attachment() failed\n");
		dma_buf_detach(dmabuf, attachment);
	}

	reg_addr = sg_dma_address(table->sgl);
	reg_size = sg_dma_len(table->sgl);
	pr_info("reg_addr = 0x%08x, reg_size = 0x%08x\n", reg_addr, reg_size);

	dma_buf_unmap_attachment(attachment, table, DMA_BIDIRECTIONAL);
	dma_buf_detach(dmabuf, attachment);
	kfree(dev);

	return 0;
}

static int importer_test_vmap(struct dma_buf *dmabuf)
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
	importer_test_sg(dmabuf_exported);
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