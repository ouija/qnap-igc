// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) Jim Ma */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/if_vlan.h>
#include <linux/aer.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/ip.h>
#include <linux/pm_runtime.h>
#include <linux/prefetch.h>
#include <linux/slab.h>  // Required for container_of
#include <linux/timer.h> // Required for timer_list
#include <linux/string.h> // Required for strlcpy
#include <net/pkt_sched.h>
#include <net/ipv6.h>

/* Include the compatibility header */
#include "compat.h"

/**
 * Compatibility definition for strscpy
 * @dest: destination buffer
 * @src: source string
 * @size: size of the destination buffer
 *
 * This function is a compatibility wrapper for strscpy that uses
 * strlcpy for older kernel versions.
 */
static inline size_t strscpy(char *dest, const char *src, size_t size)
{
    return strlcpy(dest, src, size);
}

/**
 * from_timer - get the structure from a timer
 * @ptr: pointer to the structure
 * @timer: pointer to the timer
 * @member: name of the member within the structure
 *
 * This macro is used to retrieve the pointer to the containing
 * structure from a timer_list pointer. It uses the container_of
 * macro to achieve this.
 */
#define from_timer(ptr, timer, member) \
    container_of(timer, typeof(*(ptr)), member)

/**
 * timer_setup - initialize a timer
 * @timer: pointer to the timer_list structure
 * @func: callback function to call when the timer expires
 * @flags: timer flags (not used in this implementation)
 *
 * This function sets up a timer by initializing its function and
 * setting a default expiration time. The timer must be added to
 * the timer queue using add_timer() after this function is called.
 */

// Define a function pointer type compatible with older kernels
typedef void (*timer_callback_t)(struct timer_list *);

// Updated timer_setup function
static inline void timer_setup(struct timer_list *timer, timer_callback_t func, unsigned int flags)
{
    // Assign the function to the timer
    timer->function = (void (*)(unsigned long))func;  // Cast to expected type
    timer->expires = jiffies; // Default expiration time
    add_timer(timer); // Add the timer to the system
}

/**
 * refcount_read - get a refcount's value
 * @r: the refcount
 *
 * Return: the refcount's value
 *
 * This function retrieves the current value of a reference count.
 */
static inline unsigned int refcount_read(atomic_t *r)
{
    return atomic_read(r);
}

/**
 * refcount_dec_and_test - decrement a refcount and test if it is 0
 * @r: the refcount
 *
 * This function decrements the reference count and checks if it
 * has reached zero. It will WARN on underflow.
 *
 * Return: true if the resulting refcount is 0, false otherwise
 */
static inline bool refcount_dec_and_test(atomic_t *r)
{
    return atomic_sub_and_test(1, r);
}

/**
 * skb_unref - decrement the skb's reference count
 * @skb: buffer
 *
 * This function decrements the reference count for a socket buffer
 * (skb). If the reference count reaches zero, the skb can be freed.
 *
 * Return: true if we can free the skb, false otherwise.
 */
static inline bool skb_unref(struct sk_buff *skb)
{
    if (unlikely(!skb))
        return false;
    if (likely(atomic_read(&skb->users) == 1))
        smp_rmb();
    else if (likely(!refcount_dec_and_test(&skb->users)))
        return false;

    return true;
}

/**
 * pci_request_mem_regions - request memory regions for a PCI device
 * @pdev: the PCI device
 * @name: name for the memory region
 *
 * This function requests memory regions for a PCI device and 
 * returns 0 on success or a negative error code on failure.
 */
static inline int pci_request_mem_regions(struct pci_dev *pdev, const char *name)
{
    return pci_request_selected_regions(pdev, pci_select_bars(pdev, IORESOURCE_MEM), name);
}

/**
 * pci_release_mem_regions - release memory regions for a PCI device
 * @pdev: the PCI device
 *
 * This function releases the memory regions previously allocated
 * to a PCI device.
 */
static inline void pci_release_mem_regions(struct pci_dev *pdev)
{
    pci_release_selected_regions(pdev, pci_select_bars(pdev, IORESOURCE_MEM));
}

/**
 * page_ref_sub_and_test - decrement a page's reference count and test if it is 0
 * @page: pointer to the page
 * @nr: number to decrement by
 *
 * This function decrements the reference count of a page and checks
 * if it has reached zero.
 *
 * Return: true if the resulting refcount is 0, false otherwise.
 */
static inline int page_ref_sub_and_test(struct page *page, int nr)
{
    return atomic_sub_and_test(nr, &page->_count);
}

/**
 * __page_frag_cache_drain - drain a page fragment cache
 * @page: pointer to the page
 * @count: number of fragments to drain
 *
 * This function decrements the reference count of a page and frees
 * it if the count reaches zero.
 */
static inline void __page_frag_cache_drain(struct page *page, unsigned int count)
{
    if (page_ref_sub_and_test(page, count)) {
        unsigned int order = compound_order(page);
        __free_pages(page, order);
    }
}
