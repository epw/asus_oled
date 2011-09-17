#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x69cd1ba3, "module_layout" },
	{ 0x13095525, "param_ops_uint" },
	{ 0x942037f2, "usb_deregister" },
	{ 0xe43bdd78, "class_remove_file" },
	{ 0xc84e8d97, "class_destroy" },
	{ 0x5d5fda7a, "usb_register_driver" },
	{ 0x2baf0401, "class_create_file" },
	{ 0xd6c132c7, "__class_create" },
	{ 0xc0b6d671, "device_create" },
	{ 0xa4b0818, "device_create_file" },
	{ 0xbda33868, "usb_get_dev" },
	{ 0x20000329, "simple_strtoul" },
	{ 0xde0bdcff, "memset" },
	{ 0x5a34a45c, "__kmalloc" },
	{ 0x11089ac7, "_ctype" },
	{ 0x4bc0435f, "dev_err" },
	{ 0x40a8d09e, "kmem_cache_alloc_trace" },
	{ 0xc2859108, "kmalloc_caches" },
	{ 0x236c8c64, "memcpy" },
	{ 0x5b126365, "_dev_info" },
	{ 0x37a0cba, "kfree" },
	{ 0x4cabd293, "usb_put_dev" },
	{ 0x6f789b7, "device_unregister" },
	{ 0x7b205ecd, "device_remove_file" },
	{ 0x8091a7da, "dev_set_drvdata" },
	{ 0xdacf38c9, "dev_get_drvdata" },
	{ 0xff1cd7f7, "usb_bulk_msg" },
	{ 0x27e1a049, "printk" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("usb:v0B05p1726d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v0B05p175Bd*dc*dsc*dp*ic*isc*ip*");

MODULE_INFO(srcversion, "A3BEFEA058843E5E3042039");
