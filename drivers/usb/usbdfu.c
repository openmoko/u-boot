/*
 * (C) 2007 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 *
 * based on existing SAM7DFU code from OpenPCD:
 * (C) Copyright 2006 by Harald Welte <hwelte@hmw-consulting.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * TODO:
 * - make NAND support reasonably self-contained and put in apropriate
 *   ifdefs
 * - add some means of synchronization, i.e. block commandline access
 *   while DFU transfer is in progress, and return to commandline once
 *   we're finished
 * - add VERIFY support after writing to flash
 * - sanely free() resources allocated during first uppload/download
 *   request when aborting
 * - sanely free resources when another alternate interface is selected
 *
 * Maybe:
 * - add something like uImage or some other header that provides CRC
 *   checking?
 * - make 'dnstate' attached to 'struct usb_device_instance'
 */

#include <config.h>
#if defined(CONFIG_USBD_DFU)

#include <common.h>
DECLARE_GLOBAL_DATA_PTR;

#include <malloc.h>
#include <linux/types.h>
#include <linux/list.h>
#include <asm/errno.h>
#include <usbdcore.h>
#include <usb_dfu.h>
#include <usb_dfu_descriptors.h>
#include <usb_dfu_trailer.h>

#include <nand.h>
#include <jffs2/load_kernel.h>
int mtdparts_init(void);
extern struct list_head devices;

#include "usbdcore_s3c2410.h"
#include "../serial/usbtty.h"			/* for STR_* defs */

#define RET_NOTHING	0
#define RET_ZLP		1
#define RET_STALL	2

volatile enum dfu_state *system_dfu_state; /* for 3rd parties */


struct dnload_state {
	nand_info_t *nand;
	struct part_info *part;
	unsigned int part_net_size;	/* net sizee (excl. bad blocks) of part */

	nand_erase_options_t erase_opts;
	nand_write_options_t write_opts;
	nand_read_options_t read_opts;

	unsigned char *ptr;	/* pointer to next empty byte in buffer */
	unsigned int off;	/* offset of current erase page in flash chip */
	unsigned char *buf;	/* pointer to allocated erase page buffer */

	/* unless doing an atomic transfer, we use the static buffer below.
	 * This saves us from having to clean up dynamic allications in the
	 * various error paths of the code.  Also, it will always work, no
	 * matter what the memory situation is. */
	unsigned char _buf[0x20000];	/* FIXME: depends flash page size */
};

static struct dnload_state _dnstate;

static int dfu_trailer_matching(const struct uboot_dfu_trailer *trailer)
{
	if (trailer->magic != UBOOT_DFU_TRAILER_MAGIC ||
	    trailer->version != UBOOT_DFU_TRAILER_V1 ||
	    trailer->vendor != CONFIG_USBD_VENDORID ||
	    (trailer->product != CONFIG_USBD_PRODUCTID_CDCACM &&
	     trailer->product != CONFIG_USBD_PRODUCTID_GSERIAL))
		return 0;
#ifdef CONFIG_REVISION_TAG
	if (trailer->revision != CONFIG_USB_DFU_REVISION)
		return 0;
#endif

	return 1;
}

static struct part_info *get_partition_nand(int idx)
{
	struct mtd_device *dev;
	struct part_info *part;
	struct list_head *dentry;
	struct list_head *pentry;
	int i;

	if (mtdparts_init())
		return NULL;

	list_for_each(dentry, &devices) {
		dev = list_entry(dentry, struct mtd_device, link);
		if (dev->id->type == MTD_DEV_TYPE_NAND) {
			i = 0;
			list_for_each(pentry, &dev->parts) {
				if (i == idx)  {
					part = list_entry(pentry,
					    struct part_info, link);
					return part;
				}
				i++;
			}
			return NULL;
		}
	}
}

#define LOAD_ADDR ((unsigned char *)0x32000000)

static int initialize_ds_nand(struct usb_device_instance *dev, struct dnload_state *ds)
{
	ds->part = get_partition_nand(dev->alternate - 1);
	if (!ds->part) {
		printf("DFU: unable to find partition %u\b", dev->alternate-1);
   			dev->dfu_state = DFU_STATE_dfuERROR;
		dev->dfu_status = DFU_STATUS_errADDRESS;
		return RET_STALL;
	}
	ds->nand = &nand_info[ds->part->dev->id->num];
	ds->off = ds->part->offset;
	ds->part_net_size = nand_net_part_size(ds->part);

	if (ds->nand->erasesize > sizeof(ds->_buf)) {
		printf("*** Warning - NAND ERASESIZE bigger than static buffer\n");
		ds->buf = malloc(ds->nand->erasesize);
		if (!ds->buf) {
			printf("DFU: can't allocate %u bytes\n", ds->nand->erasesize);
   			dev->dfu_state = DFU_STATE_dfuERROR;
			dev->dfu_status = DFU_STATUS_errADDRESS;
			return RET_STALL;
		}
	} else
		ds->buf = ds->_buf;

	ds->ptr = ds->buf;

	memset(&ds->read_opts, 0, sizeof(ds->read_opts));

	memset(&ds->erase_opts, 0, sizeof(ds->erase_opts));
	ds->erase_opts.quiet = 1;
	/* FIXME: do this more dynamic */
	if (!strcmp(ds->part->name, "rootfs"))
		ds->erase_opts.jffs2 = 1;

	memset(&ds->write_opts, 0, sizeof(ds->write_opts));
	ds->write_opts.pad = 1;
	ds->write_opts.blockalign = 1;
	ds->write_opts.quiet = 1;

	debug("initialize_ds_nand(dev=%p, ds=%p): ", dev, ds);
	debug("nand=%p, ptr=%p, buf=%p, off=0x%x\n", ds->nand, ds->ptr, ds->buf, ds->off);

	return RET_NOTHING;
}

static int erase_flash_verify_nand(struct urb *urb, struct dnload_state *ds,
				   unsigned long erasesize, unsigned long size)
{
	struct usb_device_instance *dev = urb->device;
	int rc;

	debug("erase_flash_verify_nand(urb=%p, ds=%p, erase=0x%x size=0x%x)\n",
		urb, ds, erasesize, size);

	if (erasesize == ds->nand->erasesize) {
		/* we're only writing a single block and need to
		 * do bad block skipping / offset adjustments our own */
		while (ds->nand->block_isbad(ds->nand, ds->off)) {
			debug("SKIP_ONE_BLOCK(0x%08x)!!\n", ds->off);
			ds->off += ds->nand->erasesize;
		}
	}

	/* we have finished one eraseblock, flash it */
	ds->erase_opts.offset = ds->off;
	ds->erase_opts.length = erasesize;
	debug("Erasing 0x%x bytes @ offset 0x%x (jffs=%u)\n",
		ds->erase_opts.length, ds->erase_opts.offset,
		ds->erase_opts.jffs2);
	rc = nand_erase_opts(ds->nand, &ds->erase_opts);
	if (rc) {
		debug("Error erasing\n");
	    	dev->dfu_state = DFU_STATE_dfuERROR;
		dev->dfu_status = DFU_STATUS_errERASE;
		return RET_STALL;
	}

	ds->write_opts.buffer = ds->buf;
	ds->write_opts.length = size;
	ds->write_opts.offset = ds->off;
	debug("Writing 0x%x bytes @ offset 0x%x\n", size, ds->off);
	rc = nand_write_opts(ds->nand, &ds->write_opts);
	if (rc) {
		debug("Error writing\n");
  		dev->dfu_state = DFU_STATE_dfuERROR;
		dev->dfu_status = DFU_STATUS_errWRITE;
		return RET_STALL;
	}

	ds->off += size;
	ds->ptr = ds->buf;

	/* FIXME: implement verify! */
	return RET_NOTHING;
}

static int erase_tail_clean_nand(struct urb *urb, struct dnload_state *ds)
{
	struct usb_device_instance *dev = urb->device;
	int rc;

	ds->erase_opts.offset = ds->off;
	ds->erase_opts.length = ds->part->size - (ds->off - ds->part->offset);
	debug("Erasing tail of 0x%x bytes @ offset 0x%x (jffs=%u)\n",
		ds->erase_opts.length, ds->erase_opts.offset,
		ds->erase_opts.jffs2);
	rc = nand_erase_opts(ds->nand, &ds->erase_opts);
	if (rc) {
		printf("Error erasing tail\n");
	    	dev->dfu_state = DFU_STATE_dfuERROR;
		dev->dfu_status = DFU_STATUS_errERASE;
		return RET_STALL;
	}

	ds->off += ds->erase_opts.length; /* for consistency */

	return RET_NOTHING;
}

/* Read the next erase blcok from NAND into buffer */
static int read_next_nand(struct urb *urb, struct dnload_state *ds, int len)
{
	struct usb_device_instance *dev = urb->device;
	int rc;

	ds->read_opts.buffer = ds->buf;
	ds->read_opts.length = len;
	ds->read_opts.offset = ds->off;
	ds->read_opts.quiet = 1;

	debug("Reading 0x%x@0x%x to 0x%08p\n", len, ds->off, ds->buf);
	rc = nand_read_opts(ds->nand, &ds->read_opts);
	if (rc) {
		debug("Error reading\n");
  		dev->dfu_state = DFU_STATE_dfuERROR;
		dev->dfu_status = DFU_STATUS_errWRITE;
		return RET_STALL;
	}
	ds->off += len;
	ds->ptr = ds->buf;

	return RET_NOTHING;
}


static int handle_dnload(struct urb *urb, u_int16_t val, u_int16_t len, int first)
{
	struct usb_device_instance *dev = urb->device;
	struct dnload_state *ds = &_dnstate;
	unsigned int actual_len = len;
	unsigned int remain_len;
	unsigned long size;
	int rc;

	debug("download(len=%u, first=%u) ", len, first);

	if (len > CONFIG_USBD_DFU_XFER_SIZE) {
		/* Too big. Not that we'd really care, but it's a
		 * DFU protocol violation */
		debug("length exceeds flash page size ");
	    	dev->dfu_state = DFU_STATE_dfuERROR;
		dev->dfu_status = DFU_STATUS_errADDRESS;
		return RET_STALL;
	}

	if (first) {
		/* Make sure that we have a valid mtd partition table */
		char *mtdp = getenv("mtdparts");
		if (!mtdp)
			run_command("dynpart", 0);
	}

	if (len == 0) {
		debug("zero-size write -> MANIFEST_SYNC ");
		dev->dfu_state = DFU_STATE_dfuMANIFEST_SYNC;

		/* cleanup */
		switch (dev->alternate) {
			char buf[12];
		case 0:
			sprintf(buf, "%lx", ds->ptr - ds->buf);
			setenv("filesize", buf);
			ds->ptr = ds->buf;
			break;
		case 1:
			if (ds->ptr >
			    ds->buf + sizeof(struct uboot_dfu_trailer)) {
				struct uboot_dfu_trailer trailer;
				dfu_trailer_mirror(&trailer, ds->ptr);
				if (!dfu_trailer_matching(&trailer)) {
					printf("DFU TRAILER NOT OK\n");
					dev->dfu_state = DFU_STATE_dfuERROR;
					dev->dfu_status = DFU_STATUS_errTARGET;
					return RET_STALL;
				}

				rc = erase_flash_verify_nand(urb, ds,
							ds->part->size,
							ds->part_net_size);
				/* re-write dynenv marker in OOB */
				run_command("dynenv set u-boot_env", 0);
			}
			ds->nand = NULL;
			free(ds->buf);
			ds->ptr = ds->buf = ds->_buf;
			break;
		default:
			rc = 0;
			if (ds->ptr > ds->buf)
				rc = erase_flash_verify_nand(urb, ds,
							ds->nand->erasesize,
							ds->nand->erasesize);
			/* rootfs partition */
			if (!rc && !strcmp(ds->part->name, "rootfs"))
				rc = erase_tail_clean_nand(urb, ds);

			ds->nand = NULL;
			break;
		}

		return RET_ZLP;
	}

	if (urb->actual_length != len) {
		debug("urb->actual_length(%u) != len(%u) ?!? ",
			urb->actual_length, len);
	    	dev->dfu_state = DFU_STATE_dfuERROR;
		dev->dfu_status = DFU_STATUS_errADDRESS;
		return RET_STALL;
	}

	if (first && ds->buf && ds->buf != ds->_buf && ds->buf != LOAD_ADDR) {
		free(ds->buf);
		ds->buf = ds->_buf;
	}

	switch (dev->alternate) {
	case 0:
		if (first) {
			printf("Starting DFU DOWNLOAD to RAM (0x%08p)\n",
				LOAD_ADDR);
			ds->buf = LOAD_ADDR;
			ds->ptr = ds->buf;
		}

		memcpy(ds->ptr, urb->buffer, len);
		ds->ptr += len;
		break;
	case 1:
		if (first) {
			rc = initialize_ds_nand(dev, ds);
			if (rc)
				return rc;
			ds->buf = malloc(ds->part_net_size);
			if (!ds->buf) {
				printf("No memory for atomic buffer!!\n");
				dev->dfu_state = DFU_STATE_dfuERROR;
				dev->dfu_status = DFU_STATUS_errUNKNOWN;
				return RET_STALL;
			}
			ds->ptr = ds->buf;
			printf("Starting Atomic DFU DOWNLOAD to partition '%s'\n",
				ds->part->name);
		}

		remain_len = (ds->buf + ds->part_net_size) - ds->ptr;
		if (remain_len < len) {
			len = remain_len;
			printf("End of write exceeds partition end\n");
			dev->dfu_state = DFU_STATE_dfuERROR;
			dev->dfu_status = DFU_STATUS_errADDRESS;
			return RET_STALL;
		}
		memcpy(ds->ptr, urb->buffer, len);
		ds->ptr += len;
		break;
	default:
		if (first) {
			rc = initialize_ds_nand(dev, ds);
			if (rc)
				return rc;
			printf("Starting DFU DOWNLOAD to partition '%s'\n",
				ds->part->name);
		}

		size = ds->nand->erasesize;
		remain_len = ds->buf + size - ds->ptr;
		if (remain_len < len)
			actual_len = remain_len;

		memcpy(ds->ptr, urb->buffer, actual_len);
		ds->ptr += actual_len;

		/* check partition end */
		if (ds->off + (ds->ptr - ds->buf) > ds->part->offset + ds->part->size) {
			printf("End of write exceeds partition end\n");
			dev->dfu_state = DFU_STATE_dfuERROR;
			dev->dfu_status = DFU_STATUS_errADDRESS;
			return RET_STALL;
		}

		if (ds->ptr >= ds->buf + size) {
			rc = erase_flash_verify_nand(urb, ds,
						     ds->nand->erasesize,
						     ds->nand->erasesize);
			if (rc)
				return rc;
			/* copy remainder of data into buffer */
			memcpy(ds->ptr, urb->buffer + actual_len, len - actual_len);
			ds->ptr += (len - actual_len);
		}
		break;
	}

	return RET_ZLP;
}

static int handle_upload(struct urb *urb, u_int16_t val, u_int16_t len, int first)
{
	struct usb_device_instance *dev = urb->device;
	struct dnload_state *ds = &_dnstate;
	unsigned int remain;
	int rc;

	debug("upload(val=0x%02x, len=%u, first=%u) ", val, len, first);

	if (len > CONFIG_USBD_DFU_XFER_SIZE) {
		/* Too big */
		dev->dfu_state = DFU_STATE_dfuERROR;
		dev->dfu_status = DFU_STATUS_errADDRESS;
		//udc_ep0_send_stall();
		debug("Error: Transfer size > CONFIG_USBD_DFU_XFER_SIZE ");
		return -EINVAL;
	}

	switch (dev->alternate) {
	case 0:
		if (first) {
			printf("Starting DFU Upload of RAM (0x%08p)\n",
				LOAD_ADDR);
			ds->ptr = ds->buf;
		}

		/* FIXME: end at some more dynamic point */
		if (ds->ptr + len > LOAD_ADDR + 0x200000)
			len = (LOAD_ADDR + 0x200000) - ds->ptr;

		urb->buffer = ds->ptr;
		urb->actual_length = len;
		ds->ptr += len;
		break;
	default:
		if (first) {
			rc = initialize_ds_nand(dev, ds);
			if (rc)
				return -EINVAL;
			printf("Starting DFU Upload of partition '%s'\n",
				ds->part->name);
		}

		if (len > ds->nand->erasesize) {
			printf("We don't support transfers bigger than %u\n",
				ds->nand->erasesize);
			len = ds->nand->erasesize;
		}

		/* limit length to whatever number of bytes is left in
		 * this partition */
		remain = (ds->part->offset + ds->part->size) - ds->off;
		if (len > remain)
			len = remain;

		rc = read_next_nand(urb, ds, len);
		if (rc)
			return -EINVAL;

		debug("uploading %u bytes ", len);
		urb->buffer = ds->buf;
		urb->actual_length = len;
		break;
	}

	debug("returning len=%u\n", len);
	return len;
}

static void handle_getstatus(struct urb *urb, int max)
{
	struct usb_device_instance *dev = urb->device;
	struct dfu_status *dstat = (struct dfu_status *) urb->buffer;

	debug("getstatus ");

	if (!urb->buffer || urb->buffer_length < sizeof(*dstat)) {
		debug("invalid urb! ");
		return;
	}

	switch (dev->dfu_state) {
	case DFU_STATE_dfuDNLOAD_SYNC:
	case DFU_STATE_dfuDNBUSY:
#if 0
		if (fsr & AT91C_MC_PROGE) {
			debug("errPROG ");
			dev->dfu_status = DFU_STATUS_errPROG;
			dev->dfu_state = DFU_STATE_dfuERROR;
		} else if (fsr & AT91C_MC_LOCKE) {
			debug("errWRITE ");
			dev->dfu_status = DFU_STATUS_errWRITE;
			dev->dfu_state = DFU_STATE_dfuERROR;
		} else if (fsr & AT91C_MC_FRDY) {
#endif
			debug("DNLOAD_IDLE ");
			dev->dfu_state = DFU_STATE_dfuDNLOAD_IDLE;
#if 0
		} else {
			debug("DNBUSY ");
			dev->dfu_state = DFU_STATE_dfuDNBUSY;
		}
#endif
		break;
	case DFU_STATE_dfuMANIFEST_SYNC:
		break;
	default:
		//return;
		break;
	}

	/* send status response */
	dstat->bStatus = dev->dfu_status;
	dstat->bState = dev->dfu_state;
	dstat->iString = 0;
	/* FIXME: set dstat->bwPollTimeout */
	urb->actual_length = MIN(sizeof(*dstat), max);

	/* we don't need to explicitly send data here, will
	 * be done by the original caller! */
}

static void handle_getstate(struct urb *urb, int max)
{
	debug("getstate ");

	if (!urb->buffer || urb->buffer_length < sizeof(u_int8_t)) {
		debug("invalid urb! ");
		return;
	}

	urb->buffer[0] = urb->device->dfu_state & 0xff;
	urb->actual_length = sizeof(u_int8_t);
}

#ifndef CONFIG_USBD_PRODUCTID_DFU
#define CONFIG_USBD_PRODUCTID_DFU CONFIG_USBD_PRODUCTID_CDCACM
#endif

static const struct usb_device_descriptor dfu_dev_descriptor = {
	.bLength		= USB_DT_DEVICE_SIZE,
	.bDescriptorType	= USB_DT_DEVICE,
	.bcdUSB			= 0x0100,
	.bDeviceClass		= 0x00,
	.bDeviceSubClass	= 0x00,
	.bDeviceProtocol	= 0x00,
	.bMaxPacketSize0	= EP0_MAX_PACKET_SIZE,
	.idVendor		= CONFIG_USBD_VENDORID,
	.idProduct		= CONFIG_USBD_PRODUCTID_DFU,
	.bcdDevice		= 0x0000,
	.iManufacturer		= DFU_STR_MANUFACTURER,
	.iProduct		= DFU_STR_PRODUCT,
	.iSerialNumber		= DFU_STR_SERIAL,
	.bNumConfigurations	= 0x01,
};

static struct _dfu_desc dfu_cfg_descriptor = {
	.ucfg = {
		.bLength		= USB_DT_CONFIG_SIZE,
		.bDescriptorType	= USB_DT_CONFIG,
		.wTotalLength		= USB_DT_CONFIG_SIZE +
					  DFU_NUM_ALTERNATES * USB_DT_INTERFACE_SIZE +
					  USB_DT_DFU_SIZE,
		.bNumInterfaces		= 1,
		.bConfigurationValue	= 1,
		.iConfiguration		= DFU_STR_CONFIG,
		.bmAttributes		= BMATTRIBUTE_RESERVED,
		.bMaxPower		= 50,
	},
	.func_dfu = DFU_FUNC_DESC,
};

int dfu_ep0_handler(struct urb *urb)
{
	int rc, ret = RET_NOTHING;
	u_int8_t req = urb->device_request.bRequest;
	u_int16_t val = urb->device_request.wValue;
	u_int16_t len = urb->device_request.wLength;
	struct usb_device_instance *dev = urb->device;

	debug("dfu_ep0(req=0x%x, val=0x%x, len=%u) old_state = %u ",
		req, val, len, dev->dfu_state);

	switch (dev->dfu_state) {
	case DFU_STATE_appIDLE:
		switch (req) {
		case USB_REQ_DFU_GETSTATUS:
			handle_getstatus(urb, len);
			break;
		case USB_REQ_DFU_GETSTATE:
			handle_getstate(urb, len);
			break;
		case USB_REQ_DFU_DETACH:
			dev->dfu_state = DFU_STATE_appDETACH;
			ret = RET_ZLP;
			goto out;
			break;
		default:
			ret = RET_STALL;
		}
		break;
	case DFU_STATE_appDETACH:
		switch (req) {
		case USB_REQ_DFU_GETSTATUS:
			handle_getstatus(urb, len);
			break;
		case USB_REQ_DFU_GETSTATE:
			handle_getstate(urb, len);
			break;
		default:
			dev->dfu_state = DFU_STATE_appIDLE;
			ret = RET_STALL;
			goto out;
			break;
		}
		/* FIXME: implement timer to return to appIDLE */
		break;
	case DFU_STATE_dfuIDLE:
		switch (req) {
		case USB_REQ_DFU_DNLOAD:
			if (len == 0) {
				dev->dfu_state = DFU_STATE_dfuERROR;
				ret = RET_STALL;
				goto out;
			}
			dev->dfu_state = DFU_STATE_dfuDNLOAD_SYNC;
			ret = handle_dnload(urb, val, len, 1);
			break;
		case USB_REQ_DFU_UPLOAD:
			dev->dfu_state = DFU_STATE_dfuUPLOAD_IDLE;
			handle_upload(urb, val, len, 1);
			break;
		case USB_REQ_DFU_ABORT:
			/* no zlp? */
			ret = RET_ZLP;
			break;
		case USB_REQ_DFU_GETSTATUS:
			handle_getstatus(urb, len);
			break;
		case USB_REQ_DFU_GETSTATE:
			handle_getstate(urb, len);
			break;
		case USB_REQ_DFU_DETACH:
			/* Proprietary extension: 'detach' from idle mode and
			 * get back to runtime mode in case of USB Reset.  As
			 * much as I dislike this, we just can't use every USB
			 * bus reset to switch back to runtime mode, since at
			 * least the Linux USB stack likes to send a number of resets
			 * in a row :( */
			dev->dfu_state = DFU_STATE_dfuMANIFEST_WAIT_RST;
			break;
		default:
			dev->dfu_state = DFU_STATE_dfuERROR;
			ret = RET_STALL;
			goto out;
			break;
		}
		break;
	case DFU_STATE_dfuDNLOAD_SYNC:
		switch (req) {
		case USB_REQ_DFU_GETSTATUS:
			handle_getstatus(urb, len);
			/* FIXME: state transition depending on block completeness */
			break;
		case USB_REQ_DFU_GETSTATE:
			handle_getstate(urb, len);
			break;
		default:
			dev->dfu_state = DFU_STATE_dfuERROR;
			ret = RET_STALL;
			goto out;
		}
		break;
	case DFU_STATE_dfuDNBUSY:
		switch (req) {
		case USB_REQ_DFU_GETSTATUS:
			/* FIXME: only accept getstatus if bwPollTimeout
			 * has elapsed */
			handle_getstatus(urb, len);
			break;
		default:
			dev->dfu_state = DFU_STATE_dfuERROR;
			ret = RET_STALL;
			goto out;
		}
		break;
	case DFU_STATE_dfuDNLOAD_IDLE:
		switch (req) {
		case USB_REQ_DFU_DNLOAD:
			dev->dfu_state = DFU_STATE_dfuDNLOAD_SYNC;
			ret = handle_dnload(urb, val, len, 0);
			break;
		case USB_REQ_DFU_ABORT:
			dev->dfu_state = DFU_STATE_dfuIDLE;
			ret = RET_ZLP;
			break;
		case USB_REQ_DFU_GETSTATUS:
			handle_getstatus(urb, len);
			break;
		case USB_REQ_DFU_GETSTATE:
			handle_getstate(urb, len);
			break;
		default:
			dev->dfu_state = DFU_STATE_dfuERROR;
			ret = RET_STALL;
			break;
		}
		break;
	case DFU_STATE_dfuMANIFEST_SYNC:
		switch (req) {
		case USB_REQ_DFU_GETSTATUS:
			/* We're MainfestationTolerant */
			dev->dfu_state = DFU_STATE_dfuIDLE;
			handle_getstatus(urb, len);
			break;
		case USB_REQ_DFU_GETSTATE:
			handle_getstate(urb, len);
			break;
		default:
			dev->dfu_state = DFU_STATE_dfuERROR;
			ret = RET_STALL;
			break;
		}
		break;
	case DFU_STATE_dfuMANIFEST:
		/* we should never go here */
		dev->dfu_state = DFU_STATE_dfuERROR;
		ret = RET_STALL;
		break;
	case DFU_STATE_dfuMANIFEST_WAIT_RST:
		/* we should never go here */
		break;
	case DFU_STATE_dfuUPLOAD_IDLE:
		switch (req) {
		case USB_REQ_DFU_UPLOAD:
			/* state transition if less data then requested */
			rc = handle_upload(urb, val, len, 0);
			if (rc >= 0 && rc < len)
				dev->dfu_state = DFU_STATE_dfuIDLE;
			break;
		case USB_REQ_DFU_ABORT:
			dev->dfu_state = DFU_STATE_dfuIDLE;
			/* no zlp? */
			ret = RET_ZLP;
			break;
		case USB_REQ_DFU_GETSTATUS:
			handle_getstatus(urb, len);
			break;
		case USB_REQ_DFU_GETSTATE:
			handle_getstate(urb, len);
			break;
		default:
			dev->dfu_state = DFU_STATE_dfuERROR;
			ret = RET_STALL;
			break;
		}
		break;
	case DFU_STATE_dfuERROR:
		switch (req) {
		case USB_REQ_DFU_GETSTATUS:
			handle_getstatus(urb, len);
			break;
		case USB_REQ_DFU_GETSTATE:
			handle_getstate(urb, len);
			break;
		case USB_REQ_DFU_CLRSTATUS:
			dev->dfu_state = DFU_STATE_dfuIDLE;
			dev->dfu_status = DFU_STATUS_OK;
			/* no zlp? */
			ret = RET_ZLP;
			break;
		default:
			dev->dfu_state = DFU_STATE_dfuERROR;
			ret = RET_STALL;
			break;
		}
		break;
	default:
		return DFU_EP0_UNHANDLED;
		break;
	}

out:
	debug("new_state = %u, ret = %u\n", dev->dfu_state, ret);

	switch (ret) {
	case RET_ZLP:
		//udc_ep0_send_zlp();
		urb->actual_length = 0;
		return DFU_EP0_ZLP;
		break;
	case RET_STALL:
		//udc_ep0_send_stall();
		return DFU_EP0_STALL;
		break;
	case RET_NOTHING:
		break;
	}

	return DFU_EP0_DATA;
}

void str2wide (char *str, u16 * wide);
static struct usb_string_descriptor *create_usbstring(char *string)
{
	struct usb_string_descriptor *strdesc;
	int size = sizeof(*strdesc) + strlen(string)*2;

	if (size > 255)
		return NULL;

	strdesc = malloc(size);
	if (!strdesc)
		return NULL;

	strdesc->bLength = size;
	strdesc->bDescriptorType = USB_DT_STRING;
	str2wide(string, strdesc->wData);

	return strdesc;
}


static void dfu_init_strings(struct usb_device_instance *dev)
{
	int i;
	struct usb_string_descriptor *strdesc;

	strdesc = create_usbstring(CONFIG_DFU_CFG_STR);
	usb_strings[DFU_STR_CONFIG] = strdesc;

	for (i = 0; i < DFU_NUM_ALTERNATES; i++) {
		if (i == 0) {
			strdesc = create_usbstring(CONFIG_DFU_ALT0_STR);
		} else {
			struct part_info *part = get_partition_nand(i-1);

			if (part)
				strdesc = create_usbstring(part->name);
			else
				strdesc =
				    create_usbstring("undefined partition");
		}
		if (!strdesc)
			continue;
		usb_strings[STR_COUNT+i+1] = strdesc;
	}
}


#ifdef CONFIG_NAND_DYNPART

void dfu_update_strings(void)
{
	int i;

	if (!system_dfu_state) {
		printf("NASTY SURPRISE: system_dfu_state not set\n");
		return;
	}

	for (i = 1; i != DFU_NUM_ALTERNATES; i++) {
		struct part_info *part = get_partition_nand(i-1);
		struct usb_string_descriptor *strdesc, **slot;

		if (part)
			strdesc = create_usbstring(part->name);
		else
			strdesc = create_usbstring("undefined partition");
		if (!strdesc)
			continue;
		slot = usb_strings+STR_COUNT+i+1;
		if (*slot)
			free(*slot);
		*slot = strdesc;
	}
}

#endif /* CONFIG_NAND_DYNPART */


int dfu_init_instance(struct usb_device_instance *dev)
{
	int i;

	for (i = 0; i != DFU_NUM_ALTERNATES; i++) {
		struct usb_interface_descriptor *uif =
		    dfu_cfg_descriptor.uif+i;

		uif->bLength		= USB_DT_INTERFACE_SIZE;
		uif->bDescriptorType	= USB_DT_INTERFACE;
		uif->bAlternateSetting	= i;
		uif->bInterfaceClass	= 0xfe;
		uif->bInterfaceSubClass	= 1;
		uif->bInterfaceProtocol	= 2;
		uif->iInterface		= DFU_STR_ALT(i);
	}

	dev->dfu_dev_desc = &dfu_dev_descriptor;
	dev->dfu_cfg_desc = &dfu_cfg_descriptor;
	dev->dfu_state = DFU_STATE_appIDLE;
	dev->dfu_status = DFU_STATUS_OK;

	if (system_dfu_state)
		printf("SURPRISE: system_dfu_state is already set\n");
	system_dfu_state = &dev->dfu_state;

	dfu_init_strings(dev);

	return 0;
}

static int stdout_switched;

/* event handler for usb device state events */
void dfu_event(struct usb_device_instance *device,
	       usb_device_event_t event, int data)
{
	char *out;

	switch (event) {
	case DEVICE_RESET:
		switch (device->dfu_state) {
		case DFU_STATE_appDETACH:
			device->dfu_state = DFU_STATE_dfuIDLE;
			out = getenv("stdout");
			if (out && !strcmp(out, "usbtty")) {
				setenv("stdout", "vga");
				setenv("stderr", "vga");
				stdout_switched = 1;
			}
			printf("DFU: Switching to DFU Mode\n");
			break;
		case DFU_STATE_dfuMANIFEST_WAIT_RST:
			device->dfu_state = DFU_STATE_appIDLE;
			printf("DFU: Switching back to Runtime mode\n");
			if (stdout_switched) {
				setenv("stdout", "usbtty");
				setenv("stderr", "usbtty");
				stdout_switched = 0;
			}
			break;
		default:
			break;
		}
		break;
	case DEVICE_CONFIGURED:
	case DEVICE_DE_CONFIGURED:
		debug("SET_CONFIGURATION(%u) ", device->configuration);
		/* fallthrough */
	case DEVICE_SET_INTERFACE:
		debug("SET_INTERFACE(%u,%u) old_state = %u ",
			device->interface, device->alternate,
			device->dfu_state);
		switch (device->dfu_state) {
		case DFU_STATE_appIDLE:
		case DFU_STATE_appDETACH:
		case DFU_STATE_dfuIDLE:
		case DFU_STATE_dfuMANIFEST_WAIT_RST:
			/* do nothing, we're fine */
			break;
		case DFU_STATE_dfuDNLOAD_SYNC:
		case DFU_STATE_dfuDNBUSY:
		case DFU_STATE_dfuDNLOAD_IDLE:
		case DFU_STATE_dfuMANIFEST:
			device->dfu_state = DFU_STATE_dfuERROR;
			device->dfu_status = DFU_STATUS_errNOTDONE;
			/* FIXME: free malloc()ed buffer! */
			break;
		case DFU_STATE_dfuMANIFEST_SYNC:
		case DFU_STATE_dfuUPLOAD_IDLE:
		case DFU_STATE_dfuERROR:
			device->dfu_state = DFU_STATE_dfuERROR;
			device->dfu_status = DFU_STATUS_errUNKNOWN;
			break;
		}
		debug("new_state = %u\n", device->dfu_state);
		break;
	default:
		break;
	}
}
#endif /* CONFIG_USBD_DFU */
