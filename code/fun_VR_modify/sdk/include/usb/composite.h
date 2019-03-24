 /**
 *
 * this is usb device driver file
 * composite.h
 * author IP2/Luka
 */
#ifndef __SONIX_USBD_COMPOSITE_H
#define __SONIX_USBD_COMPOSITE_H

#include "ch9.h"
#include "usb_device.h"

#if defined(CONFIG_SN_GCC_SDK)
    #include <nonstdlib.h>
#endif

#define MAX_CONFIG_INTERFACES                   16      /* arbitrary; max 255 */

#if defined(CONFIG_SN_GCC_SDK)
#define list_entry(ptr, type, member) container_of(ptr, type, member)
#define list_first_entry(ptr, type, member) list_entry((ptr)->next, type, member)
#define list_next_entry(pos, member) list_entry((pos)->member.next, typeof(*(pos)), member)
#define list_for_each_entry(pos, head, member)                      \
    for (pos = list_first_entry(head, typeof(*pos), member);        \
        &pos->member != (head);                                     \
        pos = list_next_entry(pos, member))
#endif


struct usb_function;
struct usb_gadget_strings;
struct usb_configuration ;
struct usb_composite_dev;
struct usb_gadget;
struct usb_composite_driver;
struct usb_ep;

struct usb_function {
    const char                              *name;
    struct usb_gadget_strings               *strings;
    struct usb_configuration                *config;

    int                                     (*bind)(struct usb_configuration *, struct usb_function *);
    void                                    (*unbind)(struct usb_configuration *, struct usb_function *);
    int                                     (*get_alt)(struct usb_function *, unsigned interface, unsigned alt);
    int                                     (*set_alt)(struct usb_function *, unsigned interface, unsigned alt);
    int                                     (*setup)(struct usb_function *, const struct usb_ctrlrequest *);
    void                                    (*disable)(struct usb_function *);
    void                                    (*ep_callback[15])(struct usb_function *, struct usb_ep *);
#if defined(CONFIG_SN_GCC_SDK)
    struct list_head                        list;
#endif
    struct usb_descriptor_header            **descriptors;
    struct usb_descriptor_header            **hs_descriptors;
}__attribute__ ((packed));

struct usb_configuration {
    const char                              *label;
    struct usb_function                     *function;

    int                                     (*bind)(struct usb_configuration *);
    /* fields in the config descriptor */
    uint8_t                                 bConfigurationValue;
    uint8_t                                 iConfiguration;
    uint8_t                                 bmAttributes;
    uint8_t                                 bMaxPower;

    uint8_t                                 next_interface_id;

    struct usb_composite_dev                *cdev;
    struct usb_function                     *interface[MAX_CONFIG_INTERFACES];
    struct usb_gadget_strings               *strings;
};

struct usb_string {
    unsigned char                          id;
    char                                   s[50];
};

struct usb_gadget_strings {
    unsigned short                          language;
    struct usb_string                       *strings;
};

struct usb_request {
    uint8_t                                 *buf;
    uint32_t                                length;
#if defined(CONFIG_SN_KEIL_SDK)
    void                                    *parent_req;
#endif
};

struct usb_composite_dev {
    uint32_t                                bufsiz;
    struct usb_gadget                       *gadget;
    struct usb_request                      *req;
    struct usb_composite_driver             *driver;

    struct usb_device_descriptor            desc;
    struct usb_qualifier_descriptor         *qualifier;
    struct usb_configuration                *config;
#if defined(CONFIG_SN_GCC_SDK)
    struct list_head                        configs;
#else
    struct usb_configuration*                configs[5];
#endif
    unsigned char                           next_string_id;
};

struct usb_composite_driver {
    char                            name[50];
    const struct usb_device_descriptor      *dev;
    struct usb_qualifier_descriptor     *qualifier;

    struct usb_gadget_strings               *strings;

    int                                     (*bind)(struct usb_composite_dev *);
    int                                     (*unbind)(struct usb_composite_dev *);

    void                                    (*suspend)(struct usb_composite_dev *);
    void                                    (*resume)(struct usb_composite_dev *);
};

struct usb_gadget {
    struct usb_ep                           *ep0;
#if defined(CONFIG_SN_GCC_SDK)
    struct list_head                        ep_list;
#endif
#if defined(CONFIG_SN_KEIL_SDK)
    struct usb_ep                           *ep_list[15];
#endif
    /*
    const struct usb_gadget_ops             *ops;


    enum usb_device_speed                   speed;
    unsigned                                is_dualspeed:1;
    unsigned                                is_otg:1;
    unsigned                                is_a_peripheral:1;
    unsigned                                b_hnp_enable:1;
    unsigned                                a_hnp_support:1;
    unsigned                                a_alt_hnp_support:1;
    const char                              *name;
    */
#if ERK_MOD == 1
    enum usb_device_speed                   speed;
#endif
    struct usb_composite_dev                *cdev;
};


struct usb_gadget_driver {
    char                                    *function;
    enum usb_device_speed                   speed;
    int                                     (*bind)(struct usb_gadget *);
    void                                    (*unbind)(struct usb_gadget *);
    int                                     (*setup)(struct usb_gadget *,const struct usb_ctrlrequest *);
    void                                    (*disconnect)(struct usb_gadget *);
    void                                    (*suspend)(struct usb_gadget *);
    void                                    (*resume)(struct usb_gadget *);
    void                                    (*ep_callback[15])(struct usb_gadget *,struct usb_ep *);
};


struct usb_ep_ops {
    int                                     (*queue) (struct usb_ep *ep, struct usb_request *req);
    int                                     (*reset) (struct usb_ep *ep);
    int                                     (*enable) (struct usb_ep *ep,const struct usb_endpoint_descriptor *desc);
    int                                     (*disable) (struct usb_ep *ep);
    struct usb_request*                     (*alloc_request)(struct usb_ep *ep);
    void                                    (*free_request) (struct usb_ep *ep, struct usb_request *req);
    int32_t                                     (*set_halt) (uint32_t epunm, uint32_t value);
};

struct usb_ep{
#if defined(CONFIG_SN_GCC_SDK)
    void                                    *driver_data;
#endif
#if (CONFIG_SN_KEIL_SDK)
    void                                    *parent_ep;
#endif
    char                                    name[10];
    const struct usb_ep_ops                 *ops;
#if defined(CONFIG_SN_GCC_SDK)
    struct list_head                        ep_list;
#endif
    uint16_t                                maxpacket;
    uint32_t                                state;
#if ERK_MOD == 1
    uint32_t                                payload_xfr_size; //for iso
#endif
};

/*Add By Howard*/
struct g_usb_copy_descriptors_struct{
 struct usb_interface_descriptor  g_fsg_intf_desc;
 struct usb_endpoint_descriptor   g_fsg_bulk_in_desc;
 struct usb_endpoint_descriptor   g_fsg_bulk_out_desc;
};

struct g_usb_copy_descriptors{
    uint8_t cnt;
    struct g_usb_copy_descriptors_struct buf[2];
};


int config_desc(struct usb_composite_dev *cdev, unsigned w_value);
int bos_desc(struct usb_composite_dev *cdev);
int get_string(struct usb_composite_dev *cdev,uint8_t *buf, uint16_t language, int id);
int count_configs(struct usb_composite_dev *cdev, unsigned type);
int usb_add_config(struct usb_composite_dev *cdev,struct usb_configuration *config);
int usb_add_function(struct usb_configuration *config,struct usb_function *function);
int usb_interface_id(struct usb_configuration *config,  struct usb_function *function);
void usb_free_descriptors(struct usb_descriptor_header **v);
struct usb_ep *usb_ep_autoconfig (struct usb_gadget *gadget, struct usb_endpoint_descriptor  *desc);
struct usb_descriptor_header **usb_copy_descriptors(struct usb_descriptor_header **src);
int usb_ep_queue(struct usb_ep *ep,  struct usb_request *req);
int usb_ep_set_halt(struct usb_ep *ep,int epnum, int value);
int usb_ep_reset(struct usb_ep *ep);
int usb_ep_enable(struct usb_ep *ep,const struct usb_endpoint_descriptor *desc);
int usb_ep_disable(struct usb_ep *ep);
int usb_composite_register(struct usb_composite_driver *driver);
int usb_composite_unregister(void);
struct usb_request *usb_ep_alloc_request(struct usb_ep *ep);
int usb_string_id(struct usb_composite_dev *cdev);
void usb_ep_free_request(struct usb_ep *ep,struct usb_request *req);
#endif
