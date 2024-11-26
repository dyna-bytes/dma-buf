#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/dma-buf.h>
#include <linux/module.h>
#include <linux/slab.h>
#include "exporter.h"

static struct platform_device *pdev;
static struct cdev cdev;
static dev_t dev_number;
static struct class *dev_class;
struct dma_buf *dmabuf_exported;
EXPORT_SYMBOL(dmabuf_exported);

#define virt_to_pfn(kaddr)    (virt_to_phys(kaddr) >> PAGE_SHIFT)

static int exporter_attach(
    struct dma_buf *dmabuf,
    struct dma_buf_attachment *attachment
)
{
    print_info("dmabuf attach device: %s\n", dev_name(attachment->dev));
    return 0;
}

static void exporter_detach(
    struct dma_buf *dmabuf,
    struct dma_buf_attachment *attachment
)
{
    print_info("dmabuf detach device: %s\n", dev_name(attachment->dev));
}

static struct sg_table *exporter_map_dma_buf(
    struct dma_buf_attachment *attachment,
    enum dma_data_direction dir
)
{
    struct sg_table *table;
    void *vaddr = attachment->dmabuf->priv;
    int err;

    print_info("start\n");
    table = kmalloc(sizeof(*table), GFP_KERNEL);
    if (!table)
        return ERR_PTR(-ENOMEM);

    err = sg_alloc_table(table, 1, GFP_KERNEL);
    if (err) {
        kfree(table);
        return ERR_PTR(err);
    }

    sg_dma_len(table->sgl) = PAGE_SIZE;
    sg_dma_address(table->sgl) = dma_map_single(
        &pdev->dev, vaddr, PAGE_SIZE, dir);
    print_info("end\n");

    return table;
}

static void exporter_unmap_dma_buf(
    struct dma_buf_attachment *attachment,
    struct sg_table *table,
    enum dma_data_direction dir
)
{
    dma_unmap_single(&pdev->dev,
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
    print_info("start\n");
    iosys_map_set_vaddr(map, dmabuf->priv);
    print_info("end\n");
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
    void *vaddr = dmabuf->priv;
    return remap_pfn_range(vma, vma->vm_start,
        virt_to_pfn(vaddr), PAGE_SIZE, vma->vm_page_prot);
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

static long exporter_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int fd = dma_buf_fd(dmabuf_exported, O_CLOEXEC);
    if (copy_to_user((int __user *)arg, &fd, sizeof(fd))) {
        print_err("faild to send fd to user\n");
        return -EINVAL;
    }

    return 0;
}

static struct file_operations exporter_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = exporter_ioctl,
};

static int __init exporter_init(void)
{
    int ret;
    print_info("start\n");

    pdev = platform_device_alloc(DEVICE_NAME, -1);
    if (!pdev) {
        ret = -ENOMEM;
        print_err("failed to allocate platform device(%d)\n", ret);
        goto err;
    }

    if (ret = platform_device_add(pdev)) {
        print_err("failed to add platform device(%d)\n", ret);
        goto err2;
    }
    print_info("Platform device created successfully\n");

    if (ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME)) {
        print_err("failed to allocate character device region(%d)\n", ret);
        goto err2;
    }

    cdev_init(&cdev, &exporter_fops);
    cdev.owner = THIS_MODULE;

    if (ret = cdev_add(&cdev, dev_number, 1)) {
        print_err("filed to add character device\n");
        goto err3;
    }
    print_info("Kernel module inserted successfully. Major = %d Minor = %d\n",
        MAJOR(dev_number), MINOR(dev_number));

    dev_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(dev_class)) {
        ret = PTR_ERR(dev_class);
        print_err("Failed to create device class(%d)\n", ret);
        goto err4;
    }

    if (device_create(dev_class, NULL, dev_number, NULL, DEVICE_NAME) == NULL) {
        ret = -ENODEV;
        print_err("Failed to create device(%d)\n", ret);
        goto err5;
    }

    dmabuf_exported = exporter_alloc_page();
	if (!dmabuf_exported) {
        ret = -ENOMEM;
		print_err("exporter alloc page failed(%d)\n", ret);
        goto err6;
	}

    print_info("end\n");
    return 0;
err6:
    device_destroy(dev_class, dev_number);
err5:
    class_destroy(dev_class);
err4:
    cdev_del(&cdev);
err3:
    unregister_chrdev_region(dev_number, 1);
err2:
    platform_device_put(pdev);
err:
    return ret;
}

static void __exit exporter_exit(void)
{
    print_info("start\n");
    device_destroy(dev_class, dev_number);
    class_destroy(dev_class);
    cdev_del(&cdev);
    unregister_chrdev_region(dev_number, 1);
    platform_device_unregister(pdev);
    print_info("end\n");
}

module_init(exporter_init);
module_exit(exporter_exit);

MODULE_AUTHOR("Jihyuk Park");
MODULE_IMPORT_NS(DMA_BUF);
MODULE_DESCRIPTION("DMA-BUF example for EXPORTER");
MODULE_LICENSE("GPL v2");
