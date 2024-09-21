// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) Jim Ma - Compatibility header for backport functions */

#ifndef COMPAT_H
#define COMPAT_H

#include <linux/module.h>
#include <linux/types.h>
#include <linux/if_vlan.h>
#include <linux/aer.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/ip.h>
#include <linux/pm_runtime.h>
#include <linux/prefetch.h>
#include <net/pkt_sched.h>
#include <net/ipv6.h>

#ifndef HAVE_SKB_CHECKSUM_START
static inline unsigned char *skb_checksum_start(const struct sk_buff *skb)
{
    return skb->head + skb->csum_start;
}
#endif

#ifndef HAVE_CSUM_REPLACE_BY_DIFF
static inline void csum_replace_by_diff(__sum16 *sum, __wsum diff)
{
    *sum = csum_fold(csum_add(diff, ~csum_unfold(*sum)));
}
#endif

#ifndef HAVE_NET_PREFETCH
static inline void net_prefetch(void *p)
{
    prefetch(p);
    #if L1_CACHE_BYTES < 128
    prefetch((u8 *)p + L1_CACHE_BYTES);
    #endif
}
#endif

#ifndef HAVE_DEV_PAGE_IS_REUSABLE
static inline bool dev_page_is_reusable(struct page *page)
{
    return likely(page_to_nid(page) == numa_mem_id() &&
                  !page_is_pfmemalloc(page));
}
#endif

// Add more functions or checks as needed, similar to what was in backport.h

#endif /* COMPAT_H */
