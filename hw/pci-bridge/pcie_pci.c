#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/pci/pci.h"
#include "hw/pci/pci_bus.h"
#include "hw/pci/pci_bridge.h"

typedef struct PCIEPCIBridge {
    /*< private >*/
    PCIBridge parent_obj;
    /*< public >*/
} PCIEPCIBridge;


static void pciepci_bridge_realize(PCIDevice *d, Error **errp)
{
	int rc;
    pci_bridge_initfn(d, TYPE_PCI_BUS);

    rc = pci_bridge_ssvid_init(d, 0x40, 0, 0x32);
    if(rc < 0) {
    	error_setg(errp, "Can't add SSVID, error %d", rc);
    	goto error;
    }

    rc = pcie_cap_init(d, 0, PCI_EXP_TYPE_PCI_BRIDGE, 0);
    if(rc < 0) {
    	error_setg(errp, "Can't add PCIE-PCI bridge capability, error %d", rc);
    	goto error;
    }

    return;

error:
	pci_bridge_exitfn(d);
}

static void pciepci_bridge_exit(PCIDevice *d)
{
	pcie_cap_exit(d);
	pci_bridge_exitfn(d);
}

static const VMStateDescription pciepci_bridge_dev_vmstate = {
    .name = "pciepci_bridge",
    .fields = (VMStateField[]) {
        VMSTATE_PCI_DEVICE(parent_obj, PCIBridge),
        VMSTATE_END_OF_LIST()
    }
};

static void pciepci_bridge_class_init(ObjectClass *klass, void *data)
{
    PCIDeviceClass *k = PCI_DEVICE_CLASS(klass);
    DeviceClass *dc = DEVICE_CLASS(klass);

    k->is_express = 1;
    k->is_bridge = 1;
    k->vendor_id = PCI_VENDOR_ID_REDHAT;
    k->device_id = PCI_DEVICE_ID_REDHAT_PCIE_BRIDGE;
    k->realize = pciepci_bridge_realize;
    k->exit = pciepci_bridge_exit;
    k->config_write = pci_bridge_write_config;
    dc->vmsd = &pciepci_bridge_dev_vmstate;
    set_bit(DEVICE_CATEGORY_BRIDGE, dc->categories);
}

static const TypeInfo pciepci_bridge_info = {
    .name          = "pciepci-bridge",
    .parent        = TYPE_PCI_BRIDGE,
    .instance_size = sizeof(PCIEPCIBridge),
    .class_init    = pciepci_bridge_class_init,
};

static void pciepci_register(void)
{
    type_register_static(&pciepci_bridge_info);
}

type_init(pciepci_register);
