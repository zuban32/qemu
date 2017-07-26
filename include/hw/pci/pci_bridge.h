/*
 * QEMU PCI bridge
 *
 * Copyright (c) 2004 Fabrice Bellard
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * split out pci bus specific stuff from pci.[hc] to pci_bridge.[hc]
 * Copyright (c) 2009 Isaku Yamahata <yamahata at valinux co jp>
 *                    VA Linux Systems Japan K.K.
 *
 */

#ifndef QEMU_PCI_BRIDGE_H
#define QEMU_PCI_BRIDGE_H

#include "hw/pci/pci.h"

#define PCI_BRIDGE_DEV_PROP_CHASSIS_NR "chassis_nr"
#define PCI_BRIDGE_DEV_PROP_MSI        "msi"
#define PCI_BRIDGE_DEV_PROP_SHPC       "shpc"

int pci_bridge_ssvid_init(PCIDevice *dev, uint8_t offset,
                          uint16_t svid, uint16_t ssid,
                          Error **errp);

PCIDevice *pci_bridge_get_device(PCIBus *bus);
PCIBus *pci_bridge_get_sec_bus(PCIBridge *br);

pcibus_t pci_bridge_get_base(const PCIDevice *bridge, uint8_t type);
pcibus_t pci_bridge_get_limit(const PCIDevice *bridge, uint8_t type);

void pci_bridge_update_mappings(PCIBridge *br);
void pci_bridge_write_config(PCIDevice *d,
                             uint32_t address, uint32_t val, int len);
void pci_bridge_disable_base_limit(PCIDevice *dev);
void pci_bridge_reset(DeviceState *qdev);

void pci_bridge_initfn(PCIDevice *pci_dev, const char *typename);
void pci_bridge_exitfn(PCIDevice *pci_dev);


/*
 * before qdev initialization(qdev_init()), this function sets bus_name and
 * map_irq callback which are necessry for pci_bridge_initfn() to
 * initialize bus.
 */
void pci_bridge_map_irq(PCIBridge *br, const char* bus_name,
                        pci_map_irq_fn map_irq);

/* TODO: add this define to pci_regs.h in linux and then in qemu. */
#define  PCI_BRIDGE_CTL_VGA_16BIT	0x10	/* VGA 16-bit decode */
#define  PCI_BRIDGE_CTL_DISCARD		0x100	/* Primary discard timer */
#define  PCI_BRIDGE_CTL_SEC_DISCARD	0x200	/* Secondary discard timer */
#define  PCI_BRIDGE_CTL_DISCARD_STATUS	0x400	/* Discard timer status */
#define  PCI_BRIDGE_CTL_DISCARD_SERR	0x800	/* Discard timer SERR# enable */

typedef struct PCIBridgeQemuCap {
    uint8_t id;     /* Standard PCI capability header field */
    uint8_t next;   /* Standard PCI capability header field */
    uint8_t len;    /* Standard PCI vendor-specific capability header field */
    uint8_t type;   /* Red Hat vendor-specific capability type.
                       Types are defined with REDHAT_PCI_CAP_ prefix */

    uint32_t bus_res;   /* Minimum number of buses to reserve */
    uint64_t io;        /* IO space to reserve */
    uint64_t mem;       /* Non-prefetchable memory to reserve */
    uint64_t mem_pref;  /* Prefetchable memory to reserve */
} PCIBridgeQemuCap;

#define REDHAT_PCI_CAP_QEMU_RESERVE     1

int pci_bridge_qemu_reserve_cap_init(PCIDevice *dev, int cap_offset,
                              uint32_t bus_reserve, uint64_t io_reserve,
                              uint64_t non_pref_mem_reserve,
                              uint64_t pref_mem_reserve,
                              Error **errp);

#endif /* QEMU_PCI_BRIDGE_H */
