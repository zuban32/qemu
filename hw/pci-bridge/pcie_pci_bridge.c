/*
 * QEMU Generic PCIE-PCI Bridge
 *
 * Copyright (c) 2017 Aleksandr Bezzubikov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/pci/pci.h"
#include "hw/pci/pci_bus.h"
#include "hw/pci/pci_bridge.h"
#include "hw/pci/msi.h"
#include "hw/pci/shpc.h"
#include "hw/pci/slotid_cap.h"

typedef struct PCIEPCIBridge {
    /*< private >*/
    PCIBridge parent_obj;
    uint32_t flags;

    MemoryRegion bar;
    /*< public >*/
} PCIEPCIBridge;

#define TYPE_PCIE_PCI_BRIDGE_DEV      "pcie-pci-bridge"
#define PCIE_PCI_BRIDGE_DEV(obj) \
        OBJECT_CHECK(PCIEPCIBridge, (obj), TYPE_PCIE_PCI_BRIDGE_DEV)

static void pciepci_bridge_realize(PCIDevice *d, Error **errp)
{
    PCIBridge *br = PCI_BRIDGE(d);
    PCIEPCIBridge *bridge_dev = PCIE_PCI_BRIDGE_DEV(d);
    int rc, pos;
    Error *local_err = NULL;

    pci_bridge_initfn(d, TYPE_PCI_BUS);

    d->config[PCI_INTERRUPT_PIN] = 0x1;
    memory_region_init(&bridge_dev->bar, OBJECT(d), "shpc-bar",
                       shpc_bar_size(d));
    rc = shpc_init(d, &br->sec_bus, &bridge_dev->bar, 0, &local_err);
    if (rc) {
        error_propagate(errp, local_err);
        goto error;
    }

    rc = pcie_cap_init(d, 0, PCI_EXP_TYPE_PCI_BRIDGE, 0, &local_err);
    if (rc < 0) {
        error_propagate(errp, local_err);
        goto error;
    }

    pos = pci_add_capability(d, PCI_CAP_ID_PM, 0, PCI_PM_SIZEOF, &local_err);
    if (pos < 0) {
        error_propagate(errp, local_err);
        goto error;
    }
    d->exp.pm_cap = pos;
    pci_set_word(d->config + pos + PCI_PM_PMC, 0x3);

    pcie_cap_arifwd_init(d);
    pcie_cap_deverr_init(d);

    rc = pcie_aer_init(d, PCI_ERR_VER, 0x100, PCI_ERR_SIZEOF, &local_err);
    if (rc < 0) {
        error_propagate(errp, local_err);
        goto error;
    }

    rc = msi_init(d, 0, 1, true, true, &local_err);
    if (rc < 0) {
        error_propagate(errp, local_err);
        goto error;
    }

    pci_register_bar(d, 0, PCI_BASE_ADDRESS_SPACE_MEMORY |
                     PCI_BASE_ADDRESS_MEM_TYPE_64, &bridge_dev->bar);

    return;

    error:
    pci_bridge_exitfn(d);
}

static void pciepci_bridge_exit(PCIDevice *d)
{
    PCIEPCIBridge *bridge_dev = PCIE_PCI_BRIDGE_DEV(d);
    pcie_cap_exit(d);
    shpc_cleanup(d, &bridge_dev->bar);
    pci_bridge_exitfn(d);
}

static void pciepci_bridge_reset(DeviceState *qdev)
{
    PCIDevice *d = PCI_DEVICE(qdev);
    pci_bridge_reset(qdev);
    msi_reset(d);
    shpc_reset(d);
}

static void pcie_pci_bridge_write_config(PCIDevice *d,
        uint32_t address, uint32_t val, int len)
{
    pci_bridge_write_config(d, address, val, len);
    msi_write_config(d, address, val, len);
    shpc_cap_write_config(d, address, val, len);
}

static bool pci_device_shpc_present(void *opaque, int version_id)
{
    PCIDevice *dev = opaque;

    return shpc_present(dev);
}

static Property pcie_pci_bridge_dev_properties[] = {
        DEFINE_PROP_END_OF_LIST(),
};

static const VMStateDescription pciepci_bridge_dev_vmstate = {
        .name = TYPE_PCIE_PCI_BRIDGE_DEV,
        .fields = (VMStateField[]) {
            VMSTATE_PCI_DEVICE(parent_obj, PCIBridge),
            SHPC_VMSTATE(shpc, PCIDevice, pci_device_shpc_present),
            VMSTATE_END_OF_LIST()
        }
};

static void pcie_pci_bridge_hotplug_cb(HotplugHandler *hotplug_dev,
                                      DeviceState *dev, Error **errp)
{
    PCIDevice *pci_hotplug_dev = PCI_DEVICE(hotplug_dev);

    if (!shpc_present(pci_hotplug_dev)) {
        error_setg(errp, "standard hotplug controller has been disabled for "
                   "this %s", TYPE_PCIE_PCI_BRIDGE_DEV);
        return;
    }
    shpc_device_hotplug_cb(hotplug_dev, dev, errp);
}

static void pcie_pci_bridge_hot_unplug_request_cb(HotplugHandler *hotplug_dev,
                                                 DeviceState *dev,
                                                 Error **errp)
{
    PCIDevice *pci_hotplug_dev = PCI_DEVICE(hotplug_dev);

    if (!shpc_present(pci_hotplug_dev)) {
        error_setg(errp, "standard hotplug controller has been disabled for "
                   "this %s", TYPE_PCIE_PCI_BRIDGE_DEV);
        return;
    }
    shpc_device_hot_unplug_request_cb(hotplug_dev, dev, errp);
}

static void pciepci_bridge_class_init(ObjectClass *klass, void *data)
{
    PCIDeviceClass *k = PCI_DEVICE_CLASS(klass);
    DeviceClass *dc = DEVICE_CLASS(klass);
    HotplugHandlerClass *hc = HOTPLUG_HANDLER_CLASS(klass);

    k->is_express = 1;
    k->is_bridge = 1;
    k->vendor_id = PCI_VENDOR_ID_REDHAT;
    k->device_id = PCI_DEVICE_ID_REDHAT_PCIE_BRIDGE;
    k->realize = pciepci_bridge_realize;
    k->exit = pciepci_bridge_exit;
    k->config_write = pcie_pci_bridge_write_config;
    dc->vmsd = &pciepci_bridge_dev_vmstate;
    dc->props = pcie_pci_bridge_dev_properties;
    dc->vmsd = &pciepci_bridge_dev_vmstate;
    dc->reset = &pciepci_bridge_reset;
    set_bit(DEVICE_CATEGORY_BRIDGE, dc->categories);
    hc->plug = pcie_pci_bridge_hotplug_cb;
    hc->unplug_request = pcie_pci_bridge_hot_unplug_request_cb;
}

static const TypeInfo pciepci_bridge_info = {
        .name = TYPE_PCIE_PCI_BRIDGE_DEV,
        .parent = TYPE_PCI_BRIDGE,
        .instance_size = sizeof(PCIEPCIBridge),
        .class_init = pciepci_bridge_class_init,
        .interfaces = (InterfaceInfo[]) {
            { TYPE_HOTPLUG_HANDLER },
            { },
        }
};

static void pciepci_register(void)
{
    type_register_static(&pciepci_bridge_info);
}

type_init(pciepci_register);
