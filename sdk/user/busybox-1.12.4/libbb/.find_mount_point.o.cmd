cmd_libbb/find_mount_point.o := /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/rsdk-linux-gcc -Wp,-MD,libbb/.find_mount_point.o.d   -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D"BB_VER=KBUILD_STR(1.12.4)" -DBB_BT=AUTOCONF_TIMESTAMP -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DUSE_LIBMD5 -I/home/gangadhar/2PORTONT/sdk/include -I/home/gangadhar/2PORTONT/sdk/lib/libmd5 -Dlinux -D__linux__ -Dunix -DEMBED -D_USE_RSDK_WRAPPER_ -D_LINUX_2_6_ -D_LINUX_2_6_30_ -include linux/config.h -I/home/gangadhar/2PORTONT/sdk -include /home/gangadhar/2PORTONT/sdk/autoconf.h -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/include -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/system/include -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/romeDriver -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/tr142/include -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/src/app/omci_v1/API -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement -Wold-style-definition -fno-builtin-strlen -finline-limit=0 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -Os    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(find_mount_point)"  -D"KBUILD_MODNAME=KBUILD_STR(find_mount_point)" -c -o libbb/find_mount_point.o libbb/find_mount_point.c

deps_libbb/find_mount_point.o := \
  libbb/find_mount_point.c \
  /home/gangadhar/2PORTONT/sdk/include/linux/config.h \
    $(wildcard include/config/h.h) \
  /home/gangadhar/2PORTONT/sdk/include/linux/autoconf.h \
    $(wildcard include/config/mtd/spi/nand/rtk.h) \
    $(wildcard include/config/rtk/voip/g7231.h) \
    $(wildcard include/config/rtk/voip/2/physical/offset.h) \
    $(wildcard include/config/squashfs/lzma.h) \
    $(wildcard include/config/arch/cpu/sync.h) \
    $(wildcard include/config/frame/warn.h) \
    $(wildcard include/config/luna/soc/gpio.h) \
    $(wildcard include/config/posix/mqueue/sysctl.h) \
    $(wildcard include/config/rtk/voip/con/ch/num.h) \
    $(wildcard include/config/crypto/blkcipher2.h) \
    $(wildcard include/config/nf/nat/proto/gre.h) \
    $(wildcard include/config/crypto/md5.h) \
    $(wildcard include/config/flatmem.h) \
    $(wildcard include/config/gpio/led/tr068.h) \
    $(wildcard include/config/rtk/voip/ilbc.h) \
    $(wildcard include/config/crypto/workqueue.h) \
    $(wildcard include/config/crypto/cbc.h) \
    $(wildcard include/config/flash/size.h) \
    $(wildcard include/config/luna/pbo/dl/dram/offset.h) \
    $(wildcard include/config/rtk/voip/defer/snddev/init.h) \
    $(wildcard include/config/rtk/l34/enable.h) \
    $(wildcard include/config/rg/callback/environment/variable/path.h) \
    $(wildcard include/config/ipv6/multiple/tables.h) \
    $(wildcard include/config/timerfd.h) \
    $(wildcard include/config/kernel/stack/size/order.h) \
    $(wildcard include/config/rtk/voip/drivers.h) \
    $(wildcard include/config/rtl865x/eth/priv/skb.h) \
    $(wildcard include/config/mtd/map/bank/width/1.h) \
    $(wildcard include/config/broken/on/smp.h) \
    $(wildcard include/config/arch/cache/wbc.h) \
    $(wildcard include/config/blk/dev/sd.h) \
    $(wildcard include/config/has/dma.h) \
    $(wildcard include/config/mtd/map/bank/width/2.h) \
    $(wildcard include/config/netfilter/xt/match/dscp.h) \
    $(wildcard include/config/cpu/rlx5281.h) \
    $(wildcard include/config/netfilter/xt/match/mark.h) \
    $(wildcard include/config/pci/hci.h) \
    $(wildcard include/config/mtd/map/bank/width/4.h) \
    $(wildcard include/config/rtk/voip/drivers/slic/zarlink.h) \
    $(wildcard include/config/rg/napt/dmz/support.h) \
    $(wildcard include/config/rtk/oam/v1.h) \
    $(wildcard include/config/module/force/load.h) \
    $(wildcard include/config/bridge/ebt/ip6.h) \
    $(wildcard include/config/nf/nat/h323.h) \
    $(wildcard include/config/hw/has/pci.h) \
    $(wildcard include/config/rtl867x/netlog.h) \
    $(wildcard include/config/bridge/ebt/broute.h) \
    $(wildcard include/config/netfilter/xt/target/tcpmss.h) \
    $(wildcard include/config/has/ioport.h) \
    $(wildcard include/config/nf/conntrack/ipv4.h) \
    $(wildcard include/config/rtl/wps2/support.h) \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/luna/pbo/ul/dram/offset.h) \
    $(wildcard include/config/nf/conntrack/ipv6.h) \
    $(wildcard include/config/yaffs/auto/yaffs2.h) \
    $(wildcard include/config/rtk/voip/ivr/dev.h) \
    $(wildcard include/config/rtl/ipv6readylogo.h) \
    $(wildcard include/config/crypto/des.h) \
    $(wildcard include/config/rg/accesswan/category/portmask.h) \
    $(wildcard include/config/nf/defrag/ipv4.h) \
    $(wildcard include/config/nf/nat/ftp.h) \
    $(wildcard include/config/netfilter/xtables.h) \
    $(wildcard include/config/generic/irq/probe.h) \
    $(wildcard include/config/crypto/algapi2.h) \
    $(wildcard include/config/nf/conntrack/mark.h) \
    $(wildcard include/config/defconfig/list.h) \
    $(wildcard include/config/list.h) \
    $(wildcard include/config/rtl/multi/wan.h) \
    $(wildcard include/config/nf/nat/sip.h) \
    $(wildcard include/config/voip/cop3/profile.h) \
    $(wildcard include/config/slab.h) \
    $(wildcard include/config/inotify.h) \
    $(wildcard include/config/wlan/hal/8192ee.h) \
    $(wildcard include/config/tx/power/gpo/pin.h) \
    $(wildcard include/config/hotplug.h) \
    $(wildcard include/config/flatmem/manual.h) \
    $(wildcard include/config/rtl/tdls/support.h) \
    $(wildcard include/config/net/cls/fw.h) \
    $(wildcard include/config/bridge/ebt/mark/t.h) \
    $(wildcard include/config/rg/callback.h) \
    $(wildcard include/config/apollo/romedriver.h) \
    $(wildcard include/config/ipv6.h) \
    $(wildcard include/config/rtl867x/ipv4/acc/none.h) \
    $(wildcard include/config/wan/router.h) \
    $(wildcard include/config/flat/node/mem/map.h) \
    $(wildcard include/config/cpu/big/endian.h) \
    $(wildcard include/config/rtl8686/ipc/processor.h) \
    $(wildcard include/config/net/cls.h) \
    $(wildcard include/config/bridge/ebt/log.h) \
    $(wildcard include/config/vfat/fs.h) \
    $(wildcard include/config/mtd/cmdline/parts.h) \
    $(wildcard include/config/bridge/ebt/arp.h) \
    $(wildcard include/config/irq/vec.h) \
    $(wildcard include/config/slhc.h) \
    $(wildcard include/config/switch/init/linkdown.h) \
    $(wildcard include/config/embedded.h) \
    $(wildcard include/config/fuse/fs.h) \
    $(wildcard include/config/1.h) \
    $(wildcard include/config/proc/fs.h) \
    $(wildcard include/config/fiber/module/loss.h) \
    $(wildcard include/config/rtk/voip/ioc/slic/relay.h) \
    $(wildcard include/config/wps/led.h) \
    $(wildcard include/config/rtk/voip/rtcp/xr.h) \
    $(wildcard include/config/rtl9602c/series.h) \
    $(wildcard include/config/inet.h) \
    $(wildcard include/config/sysvipc.h) \
    $(wildcard include/config/scsi.h) \
    $(wildcard include/config/tcp/cong/cubic.h) \
    $(wildcard include/config/rtk/voip/rx/netfilter/hook.h) \
    $(wildcard include/config/ntfs/fs.h) \
    $(wildcard include/config/rg/pppoe/passthrough.h) \
    $(wildcard include/config/rg/ipv6/neighbor/auto/learn.h) \
    $(wildcard include/config/irq/cpu.h) \
    $(wildcard include/config/rtk/voip/package/867x.h) \
    $(wildcard include/config/crypto/deflate.h) \
    $(wildcard include/config/cpu/supports/highmem.h) \
    $(wildcard include/config/cc/optimize/for/size.h) \
    $(wildcard include/config/iosched/cfq.h) \
    $(wildcard include/config/nf/conntrack/pptp.h) \
    $(wildcard include/config/mips.h) \
    $(wildcard include/config/imem/16k.h) \
    $(wildcard include/config/pageflags/extended.h) \
    $(wildcard include/config/crypto/manager.h) \
    $(wildcard include/config/have/mlocked/page/bit.h) \
    $(wildcard include/config/bridge/ebt/stp.h) \
    $(wildcard include/config/crc32.h) \
    $(wildcard include/config/rg/accesswan/timer/delay.h) \
    $(wildcard include/config/default/cfq.h) \
    $(wildcard include/config/ip/nf/iptables.h) \
    $(wildcard include/config/fat/default/iocharset.h) \
    $(wildcard include/config/apollofe/driver/testchip.h) \
    $(wildcard include/config/extra/firmware.h) \
    $(wildcard include/config/textsearch/fsm.h) \
    $(wildcard include/config/net/cls/u32.h) \
    $(wildcard include/config/bridge/nf/ebtables.h) \
    $(wildcard include/config/rg/layer2/software/learn.h) \
    $(wildcard include/config/textsearch/kmp.h) \
    $(wildcard include/config/netfilter/advanced.h) \
    $(wildcard include/config/rtk/voip/log/ioctl/size.h) \
    $(wildcard include/config/ntfs/rw.h) \
    $(wildcard include/config/rg/napt/port/collision/prevention.h) \
    $(wildcard include/config/slot/0/ant/switch.h) \
    $(wildcard include/config/unix.h) \
    $(wildcard include/config/32bit.h) \
    $(wildcard include/config/rtk/voip/drivers/pcm/linear/8k.h) \
    $(wildcard include/config/rtl8686/ipc/dev.h) \
    $(wildcard include/config/serial/core.h) \
    $(wildcard include/config/crypto/authenc.h) \
    $(wildcard include/config/sched/omit/frame/pointer.h) \
    $(wildcard include/config/rtk/voip/con/busfifo/lecbuf.h) \
    $(wildcard include/config/rtl/igmp/snooping.h) \
    $(wildcard include/config/scsi/wait/scan/module.h) \
    $(wildcard include/config/crypto/arc4.h) \
    $(wildcard include/config/rtk/voip/drivers/pcm.h) \
    $(wildcard include/config/crypto/aead.h) \
    $(wildcard include/config/rg/napt/inbound/tracking.h) \
    $(wildcard include/config/classic/rcu.h) \
    $(wildcard include/config/arch/cpu/eb.h) \
    $(wildcard include/config/nf/conntrack/proc/compat.h) \
    $(wildcard include/config/ip/fib/hash.h) \
    $(wildcard include/config/default/mmap/min/addr.h) \
    $(wildcard include/config/sys/supports/highmem.h) \
    $(wildcard include/config/rtl8686/asic.h) \
    $(wildcard include/config/rtl8192cd.h) \
    $(wildcard include/config/crypto/sha1.h) \
    $(wildcard include/config/anon/inodes.h) \
    $(wildcard include/config/generic/find/last/bit.h) \
    $(wildcard include/config/slabinfo.h) \
    $(wildcard include/config/rtk/voip/bus/pcm/ch/num.h) \
    $(wildcard include/config/mtd/blkdevs.h) \
    $(wildcard include/config/nls/default.h) \
    $(wildcard include/config/mtd/partitions.h) \
    $(wildcard include/config/split/ptlock/cpus.h) \
    $(wildcard include/config/rtk/voip/standalone/arch.h) \
    $(wildcard include/config/rtl867x/preallocate/skb/size.h) \
    $(wildcard include/config/rtl/multi/lan/dev.h) \
    $(wildcard include/config/rtk/voip/board.h) \
    $(wildcard include/config/rtk/voip/g729ab.h) \
    $(wildcard include/config/rtk/voip/drivers/slic/le9641/nr.h) \
    $(wildcard include/config/crypto/rng2.h) \
    $(wildcard include/config/crypto/hmac.h) \
    $(wildcard include/config/generic/find/next/bit.h) \
    $(wildcard include/config/rtk/voip/drivers/slic/le9641.h) \
    $(wildcard include/config/sdk/modules.h) \
    $(wildcard include/config/ipv6/tunnel.h) \
    $(wildcard include/config/rtk/voip/slic/zarlink/886/series/wideband.h) \
    $(wildcard include/config/rg/ipv6/software/shortcut/support.h) \
    $(wildcard include/config/rtk/omci/v1.h) \
    $(wildcard include/config/cpu/has/wbc.h) \
    $(wildcard include/config/serial/8250/runtime/uarts.h) \
    $(wildcard include/config/rg/debug.h) \
    $(wildcard include/config/arch/suspend/possible.h) \
    $(wildcard include/config/serial/8250/console.h) \
    $(wildcard include/config/ipv6/mroute.h) \
    $(wildcard include/config/rtl/multi/eth/wan.h) \
    $(wildcard include/config/net/sch/htb.h) \
    $(wildcard include/config/slot/0/tx/beamforming.h) \
    $(wildcard include/config/tr142/module.h) \
    $(wildcard include/config/mtd/char.h) \
    $(wildcard include/config/netfilter/xt/match/limit.h) \
    $(wildcard include/config/scsi/dma.h) \
    $(wildcard include/config/ip/nf/target/redirect.h) \
    $(wildcard include/config/default/iosched.h) \
    $(wildcard include/config/crypto/hash.h) \
    $(wildcard include/config/legacy/ptys.h) \
    $(wildcard include/config/netfilter/xt/match/state.h) \
    $(wildcard include/config/mtd/nand.h) \
    $(wildcard include/config/inet/tunnel.h) \
    $(wildcard include/config/8185/wscd.h) \
    $(wildcard include/config/wlan/hal.h) \
    $(wildcard include/config/netfilter/xt/target/dscp.h) \
    $(wildcard include/config/netfilter/xt/target/mark.h) \
    $(wildcard include/config/rg/napt/upnp/support.h) \
    $(wildcard include/config/rtk/voip/gpio/defined.h) \
    $(wildcard include/config/imem1/16k.h) \
    $(wildcard include/config/yaffs/xattr.h) \
    $(wildcard include/config/block.h) \
    $(wildcard include/config/rtk/voip/g722/itu/use.h) \
    $(wildcard include/config/rtk/voip/gsmfr.h) \
    $(wildcard include/config/generic/clockevents/build.h) \
    $(wildcard include/config/generic/hweight.h) \
    $(wildcard include/config/crypto/manager2.h) \
    $(wildcard include/config/bridge/ebt/dnat.h) \
    $(wildcard include/config/wifi/simple/config.h) \
    $(wildcard include/config/squashfs/fragment/cache/size.h) \
    $(wildcard include/config/squashfs.h) \
    $(wildcard include/config/wireless.h) \
    $(wildcard include/config/ip/advanced/router.h) \
    $(wildcard include/config/one/wps/led.h) \
    $(wildcard include/config/pon/detect/feature.h) \
    $(wildcard include/config/kallsyms.h) \
    $(wildcard include/config/yaffs/fs.h) \
    $(wildcard include/config/rtl8686/ipc/single/cpu.h) \
    $(wildcard include/config/bridge/ebt/mark.h) \
    $(wildcard include/config/mac/phy/rf/parameter/v702b/skyworth.h) \
    $(wildcard include/config/bridge/ebt/802/3.h) \
    $(wildcard include/config/sysvipc/sysctl.h) \
    $(wildcard include/config/firmware/in/kernel.h) \
    $(wildcard include/config/ipv6/sit.h) \
    $(wildcard include/config/dma/noncoherent.h) \
    $(wildcard include/config/proc/sysctl.h) \
    $(wildcard include/config/ip6/nf/iptables.h) \
    $(wildcard include/config/cpu/has/tls.h) \
    $(wildcard include/config/fiber/detect/sd.h) \
    $(wildcard include/config/cpu/has/uls.h) \
    $(wildcard include/config/bridge/ebt/vlan.h) \
    $(wildcard include/config/bridge/ebt/pkttype.h) \
    $(wildcard include/config/imq/behavior/ab.h) \
    $(wildcard include/config/rtl/nf/conntrack/garbage/new.h) \
    $(wildcard include/config/rg/sw/napt/size/l.h) \
    $(wildcard include/config/nf/nat/pptp.h) \
    $(wildcard include/config/pon/led/profile/demo/rtl9602c.h) \
    $(wildcard include/config/bridge/ebt/snat.h) \
    $(wildcard include/config/epoll.h) \
    $(wildcard include/config/luna/pbo/dl/dram/size.h) \
    $(wildcard include/config/crypto/aead2.h) \
    $(wildcard include/config/strip/asm/syms.h) \
    $(wildcard include/config/ip/multiple/tables.h) \
    $(wildcard include/config/luna/wdt.h) \
    $(wildcard include/config/voip/sdk.h) \
    $(wildcard include/config/rtl/igmp/proxy.h) \
    $(wildcard include/config/nls/codepage/437.h) \
    $(wildcard include/config/16m/flash.h) \
    $(wildcard include/config/rtk/voip/ivr/text.h) \
    $(wildcard include/config/yaffs/yaffs1.h) \
    $(wildcard include/config/uevent/helper/path.h) \
    $(wildcard include/config/bridge/ebt/ftos.h) \
    $(wildcard include/config/yaffs/yaffs2.h) \
    $(wildcard include/config/rg/napt/virtual/server/support.h) \
    $(wildcard include/config/rtk/voip/soc/8686/cpu0.h) \
    $(wildcard include/config/pppoe.h) \
    $(wildcard include/config/single/image.h) \
    $(wildcard include/config/zlib/deflate.h) \
    $(wildcard include/config/rtk/voip/hardware/spi.h) \
    $(wildcard include/config/file/locking.h) \
    $(wildcard include/config/decompress/lzma.h) \
    $(wildcard include/config/arch/flatmem/enable.h) \
    $(wildcard include/config/ask/ip/fib/hash.h) \
    $(wildcard include/config/arch/cpu/rlx5281.h) \
    $(wildcard include/config/module/build/in.h) \
    $(wildcard include/config/generic/time.h) \
    $(wildcard include/config/decompress/lzma/needed.h) \
    $(wildcard include/config/rtk/voip/platform/8686.h) \
    $(wildcard include/config/luna/pbo/ul/dram/size.h) \
    $(wildcard include/config/rtk/voip/drivers/pcm/law.h) \
    $(wildcard include/config/rtk/voip.h) \
    $(wildcard include/config/slot/0/8192ee.h) \
    $(wildcard include/config/experimental.h) \
    $(wildcard include/config/rtl8672/bridge/fastpath.h) \
    $(wildcard include/config/virt/to/bus.h) \
    $(wildcard include/config/localversion.h) \
    $(wildcard include/config/hz/100.h) \
    $(wildcard include/config/master/wlan0/enable.h) \
    $(wildcard include/config/zlib/inflate.h) \
    $(wildcard include/config/init/env/arg/limit.h) \
    $(wildcard include/config/iosched/deadline.h) \
    $(wildcard include/config/rtk/voip/drivers/slic/le9641/pin/cs.h) \
    $(wildcard include/config/crypto/hash2.h) \
    $(wildcard include/config/netfilter/xt/target/connmark.h) \
    $(wildcard include/config/nf/conntrack.h) \
    $(wildcard include/config/rtk/voip/rx.h) \
    $(wildcard include/config/rg/arp/auto/ageout.h) \
    $(wildcard include/config/bridge/ebt/redirect.h) \
    $(wildcard include/config/sysfs.h) \
    $(wildcard include/config/rtk/voip/tx.h) \
    $(wildcard include/config/compat/net/dev/ops.h) \
    $(wildcard include/config/new/portmapping.h) \
    $(wildcard include/config/iosched/noop.h) \
    $(wildcard include/config/cpu/supports/32bit/kernel.h) \
    $(wildcard include/config/netfilter/xt/target/classify.h) \
    $(wildcard include/config/inet6/tunnel.h) \
    $(wildcard include/config/trad/signals.h) \
    $(wildcard include/config/use/pcie/slot/0.h) \
    $(wildcard include/config/epon/feature.h) \
    $(wildcard include/config/generic/clockevents.h) \
    $(wildcard include/config/defaults/kernel/2/6.h) \
    $(wildcard include/config/ipv6/route/info.h) \
    $(wildcard include/config/lockdep/support.h) \
    $(wildcard include/config/msdos/partition.h) \
    $(wildcard include/config/gpon/feature.h) \
    $(wildcard include/config/ip/nf/mangle.h) \
    $(wildcard include/config/ipv6/sit/6rd.h) \
    $(wildcard include/config/rg/mld/snooping.h) \
    $(wildcard include/config/ipv6/privacy.h) \
    $(wildcard include/config/hz.h) \
    $(wildcard include/config/arch/bus/pci.h) \
    $(wildcard include/config/wlan/on/off/button.h) \
    $(wildcard include/config/rtl8190/priv/skb.h) \
    $(wildcard include/config/mtd/nand/ids.h) \
    $(wildcard include/config/realtek/voip.h) \
    $(wildcard include/config/nls/iso8859/1.h) \
    $(wildcard include/config/net/ethernet.h) \
    $(wildcard include/config/dma/need/pci/map/state.h) \
    $(wildcard include/config/rtl/11w/support.h) \
    $(wildcard include/config/arch/irq/vec.h) \
    $(wildcard include/config/tx/power/turn/on/feature.h) \
    $(wildcard include/config/net/sch/fifo.h) \
    $(wildcard include/config/rtl8686nic.h) \
    $(wildcard include/config/ssb/possible.h) \
    $(wildcard include/config/cpu1/uart/base/addr.h) \
    $(wildcard include/config/rtk/voip/log/ioctl.h) \
    $(wildcard include/config/rtk/voip/t38.h) \
    $(wildcard include/config/vlan/8021q.h) \
    $(wildcard include/config/preempt/none.h) \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
    $(wildcard include/config/band/5g/on/wlan0.h) \
    $(wildcard include/config/net/sched.h) \
    $(wildcard include/config/bridge.h) \
    $(wildcard include/config/netfilter.h) \
    $(wildcard include/config/crc/ccitt.h) \
    $(wildcard include/config/ip/multicast.h) \
    $(wildcard include/config/mtd/block.h) \
    $(wildcard include/config/prevent/firmware/build.h) \
    $(wildcard include/config/cpu/has/llsc.h) \
    $(wildcard include/config/arch/irq/cpu.h) \
    $(wildcard include/config/ip/nf/filter.h) \
    $(wildcard include/config/ipv6/optimistic/dad.h) \
    $(wildcard include/config/generic/calibrate/delay.h) \
    $(wildcard include/config/sys/supports/32bit/kernel.h) \
    $(wildcard include/config/has/iomem.h) \
    $(wildcard include/config/fw/loader.h) \
    $(wildcard include/config/netfilter/xt/match/mac.h) \
    $(wildcard include/config/luna/reserve/dram/for/pbo.h) \
    $(wildcard include/config/bridge/ebt/arpreply.h) \
    $(wildcard include/config/module/force/unload.h) \
    $(wildcard include/config/arch/cpu/tls.h) \
    $(wildcard include/config/packet.h) \
    $(wildcard include/config/use/uart0.h) \
    $(wildcard include/config/arch/cpu/uls.h) \
    $(wildcard include/config/bridge/netfilter.h) \
    $(wildcard include/config/serial/core/console.h) \
    $(wildcard include/config/netfilter/xt/match/string.h) \
    $(wildcard include/config/nf/ct/proto/gre.h) \
    $(wildcard include/config/generic/hardirqs.h) \
    $(wildcard include/config/fib/rules.h) \
    $(wildcard include/config/ipv6/ndisc/nodetype.h) \
    $(wildcard include/config/net/sch/prio.h) \
    $(wildcard include/config/netfilter/xt/match/tcpmss.h) \
    $(wildcard include/config/wireless/ext.h) \
    $(wildcard include/config/rtk/voip/port/link.h) \
    $(wildcard include/config/rtk/voip/con/polling/process.h) \
    $(wildcard include/config/crypto/pcomp.h) \
    $(wildcard include/config/bridge/ebt/t/filter.h) \
    $(wildcard include/config/cpu/has/sync.h) \
    $(wildcard include/config/inotify/user.h) \
    $(wildcard include/config/sys/supports/arbit/hz.h) \
    $(wildcard include/config/rtk/voip/circular/log/dev.h) \
    $(wildcard include/config/rtk/voip/slic/zarlink/886/series.h) \
    $(wildcard include/config/arch/32bit.h) \
    $(wildcard include/config/imem/size.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
    $(wildcard include/config/posix/mqueue.h) \
    $(wildcard include/config/bridge/ebt/limit.h) \
    $(wildcard include/config/rtk/voip/wideband/support.h) \
    $(wildcard include/config/apollo.h) \
    $(wildcard include/config/ethwan.h) \
    $(wildcard include/config/imq/num/devs.h) \
    $(wildcard include/config/select/memory/model.h) \
    $(wildcard include/config/rtl8672.h) \
    $(wildcard include/config/nf/conntrack/events.h) \
    $(wildcard include/config/rtk/voip/drivers/ata/slic.h) \
    $(wildcard include/config/arch/supports/oprofile.h) \
    $(wildcard include/config/ppp/async.h) \
    $(wildcard include/config/boot/size.h) \
    $(wildcard include/config/ip/nf/target/reject.h) \
    $(wildcard include/config/rtl8686.h) \
    $(wildcard include/config/rtk/voip/g722.h) \
    $(wildcard include/config/fiber/speed/auto.h) \
    $(wildcard include/config/default/tcp/cong.h) \
    $(wildcard include/config/rtk/voip/drivers/slic/zarlink/on/new/arch.h) \
    $(wildcard include/config/rtk/voip/g726.h) \
    $(wildcard include/config/fat/fs.h) \
    $(wildcard include/config/tx/disable/gpio/pin.h) \
    $(wildcard include/config/fiber/feature.h) \
    $(wildcard include/config/rtk/voip/dsp.h) \
    $(wildcard include/config/rg/wlan/hwnat/acceleration.h) \
    $(wildcard include/config/rtl/vap/support.h) \
    $(wildcard include/config/crypto/blkcipher.h) \
    $(wildcard include/config/ip/mroute.h) \
    $(wildcard include/config/ip6/nf/mangle.h) \
    $(wildcard include/config/nf/conntrack/h323.h) \
    $(wildcard include/config/nlattr.h) \
    $(wildcard include/config/fat/default/codepage.h) \
    $(wildcard include/config/rg/napt/arp/auto/learn.h) \
    $(wildcard include/config/nf/nat.h) \
    $(wildcard include/config/page/size/4kb.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/rtk/voip/ivr/linear/8k.h) \
    $(wildcard include/config/rg/napt/tcp/auto/learn.h) \
    $(wildcard include/config/nf/nat/needed.h) \
    $(wildcard include/config/serial/8250/nr/uarts.h) \
    $(wildcard include/config/rg/api/rle0601.h) \
    $(wildcard include/config/llc.h) \
    $(wildcard include/config/bridge/ebt/t/nat.h) \
    $(wildcard include/config/rtk/voip/ivr.h) \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/crypto/hw.h) \
    $(wildcard include/config/mii.h) \
    $(wildcard include/config/bridge/ebt/ip.h) \
    $(wildcard include/config/nf/conntrack/ftp.h) \
    $(wildcard include/config/2g/cg/trx/diversity.h) \
    $(wildcard include/config/nf/conntrack/sip.h) \
    $(wildcard include/config/crypto.h) \
    $(wildcard include/config/rtl9602c/board/v00.h) \
    $(wildcard include/config/sysctl.h) \
    $(wildcard include/config/misc/filesystems.h) \
    $(wildcard include/config/rg/callback/multi/wan/device/name.h) \
    $(wildcard include/config/rg/wmux/support.h) \
    $(wildcard include/config/wlan/hal/88xx.h) \
    $(wildcard include/config/mtd.h) \
    $(wildcard include/config/zone/dma/flag.h) \
    $(wildcard include/config/rtk/voip/rtp/redundant.h) \
    $(wildcard include/config/rg/napt/auto/ageout.h) \
    $(wildcard include/config/ip6/nf/filter.h) \
    $(wildcard include/config/imq.h) \
    $(wildcard include/config/net.h) \
    $(wildcard include/config/rtk/voip/manager.h) \
    $(wildcard include/config/module/unload.h) \
    $(wildcard include/config/standalone.h) \
    $(wildcard include/config/textsearch/bm.h) \
    $(wildcard include/config/netfilter/xt/match/iprange.h) \
    $(wildcard include/config/netdevices.h) \
    $(wildcard include/config/rtk/voip/drivers/slic/zsi.h) \
    $(wildcard include/config/nls.h) \
    $(wildcard include/config/rtl/odm/wlan/driver.h) \
    $(wildcard include/config/rtk/voip/log/ioctl/type/text.h) \
    $(wildcard include/config/crypto/algapi.h) \
    $(wildcard include/config/serial/8250.h) \
    $(wildcard include/config/mtd/cfi/i1.h) \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/rtk/voip/silence.h) \
    $(wildcard include/config/mtd/cfi/i2.h) \
    $(wildcard include/config/ppp.h) \
    $(wildcard include/config/rg/igmp/snooping.h) \
    $(wildcard include/config/txpwr/lmt.h) \
    $(wildcard include/config/ipv6/subtrees.h) \
    $(wildcard include/config/ipv6/router/pref.h) \
    $(wildcard include/config/arch/cpu/llsc.h) \
    $(wildcard include/config/base/small.h) \
    $(wildcard include/config/cpu1/dis/uart.h) \
    $(wildcard include/config/imem1/size.h) \
    $(wildcard include/config/binfmt/elf.h) \
    $(wildcard include/config/rtk/voip/drivers/slic/le9641/wideband.h) \
    $(wildcard include/config/squashfs/embedded.h) \
    $(wildcard include/config/untag/cvid/when/receive.h) \
    $(wildcard include/config/stp.h) \
    $(wildcard include/config/legacy/pty/count.h) \
    $(wildcard include/config/ip/nf/target/masquerade.h) \
    $(wildcard include/config/have/mlock.h) \
    $(wildcard include/config/rtl/mld/snooping.h) \
    $(wildcard include/config/bitreverse.h) \
    $(wildcard include/config/textsearch.h) \
    $(wildcard include/config/log/buf/shift.h) \
    $(wildcard include/config/rtk/voip/ioc/led.h) \
    $(wildcard include/config/cmdline.h) \
    $(wildcard include/config/arch/populates/node/map.h) \
    $(wildcard include/config/igmp/proxy/multiwan.h) \
    $(wildcard include/config/rtl8686/switch.h) \
  /home/gangadhar/2PORTONT/sdk/autoconf.h \
    $(wildcard include/config/defaults/realtek/luna.h) \
    $(wildcard include/config/luna/dual/linux.h) \
    $(wildcard include/config/kernel/2/6/19.h) \
    $(wildcard include/config/kernel/2/6/30.h) \
    $(wildcard include/config/use/rsdk/wrapper.h) \
    $(wildcard include/config/rsdk/dir.h) \
    $(wildcard include/config/defaults/kernel.h) \
    $(wildcard include/config/defaults/vendor.h) \
    $(wildcard include/config/mconf/bzbox.h) \
    $(wildcard include/config/image/tiny.h) \
    $(wildcard include/config/voip/ipc/dsp/architecture.h) \
    $(wildcard include/config/plr/none.h) \
    $(wildcard include/config/use/preloader/parameters.h) \
    $(wildcard include/config/plr/3x.h) \
    $(wildcard include/config/luna.h) \
    $(wildcard include/config/luna/memory/auto/detection.h) \
    $(wildcard include/config/prd/signature.h) \
    $(wildcard include/config/prj/signature.h) \
  include/libbb.h \
    $(wildcard include/config/selinux.h) \
    $(wildcard include/config/locale/support.h) \
    $(wildcard include/config/use/bb/pwd/grp.h) \
    $(wildcard include/config/feature/shadowpasswds.h) \
    $(wildcard include/config/use/bb/shadow.h) \
    $(wildcard include/config/lfs.h) \
    $(wildcard include/config/feature/buffers/go/on/stack.h) \
    $(wildcard include/config/buffer.h) \
    $(wildcard include/config/ubuffer.h) \
    $(wildcard include/config/feature/buffers/go/in/bss.h) \
    $(wildcard include/config/inux.h) \
    $(wildcard include/config/feature/ipv6.h) \
    $(wildcard include/config/feature/check/names.h) \
    $(wildcard include/config/feature/prefer/applets.h) \
    $(wildcard include/config/busybox/exec/path.h) \
    $(wildcard include/config/getopt/long.h) \
    $(wildcard include/config/feature/pidfile.h) \
    $(wildcard include/config/feature/syslog.h) \
    $(wildcard include/config/feature/individual.h) \
    $(wildcard include/config/o.h) \
    $(wildcard include/config/ntf.h) \
    $(wildcard include/config/t.h) \
    $(wildcard include/config/l.h) \
    $(wildcard include/config/wn.h) \
    $(wildcard include/config/.h) \
    $(wildcard include/config/ktop.h) \
    $(wildcard include/config/route.h) \
    $(wildcard include/config/feature/hwib.h) \
    $(wildcard include/config/debug/crond/option.h) \
    $(wildcard include/config/use/bb/crypt.h) \
    $(wildcard include/config/ioctl/hex2str/error.h) \
    $(wildcard include/config/feature/editing.h) \
    $(wildcard include/config/feature/editing/history.h) \
    $(wildcard include/config/ture/editing/savehistory.h) \
    $(wildcard include/config/feature/editing/savehistory.h) \
    $(wildcard include/config/feature/tab/completion.h) \
    $(wildcard include/config/feature/username/completion.h) \
    $(wildcard include/config/feature/editing/vi.h) \
    $(wildcard include/config/feature/topmem.h) \
    $(wildcard include/config/pgrep.h) \
    $(wildcard include/config/pkill.h) \
    $(wildcard include/config/pidof.h) \
    $(wildcard include/config/feature/devfs.h) \
  include/platform.h \
    $(wildcard include/config/werror.h) \
    $(wildcard include/config///.h) \
    $(wildcard include/config/nommu.h) \
    $(wildcard include/config//nommu.h) \
    $(wildcard include/config//mmu.h) \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/byteswap.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/byteswap.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/endian.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/features.h \
    $(wildcard include/config/c99.h) \
    $(wildcard include/config/c95.h) \
    $(wildcard include/config/ix.h) \
    $(wildcard include/config/ix2.h) \
    $(wildcard include/config/ix199309.h) \
    $(wildcard include/config/ix199506.h) \
    $(wildcard include/config/en.h) \
    $(wildcard include/config/en/extended.h) \
    $(wildcard include/config/x98.h) \
    $(wildcard include/config/en2k.h) \
    $(wildcard include/config/en2k8.h) \
    $(wildcard include/config/gefile.h) \
    $(wildcard include/config/gefile64.h) \
    $(wildcard include/config/e/offset64.h) \
    $(wildcard include/config/d.h) \
    $(wildcard include/config/c.h) \
    $(wildcard include/config/ile.h) \
    $(wildcard include/config/ntrant.h) \
    $(wildcard include/config/tify/level.h) \
    $(wildcard include/config/i.h) \
    $(wildcard include/config/ix/implicitly.h) \
    $(wildcard include/config/ern/inlines.h) \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/uClibc_config.h \
    $(wildcard include/config/mips/o32/abi//.h) \
    $(wildcard include/config/cpu/has/uls//.h) \
    $(wildcard include/config/mips/isa/1//.h) \
    $(wildcard include/config//.h) \
    $(wildcard include/config/link//.h) \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/uClibc_arch_features.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sys/cdefs.h \
    $(wildcard include/config/espaces.h) \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/endian.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/arpa/inet.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/netinet/in.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/stdint.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/wchar.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/wordsize.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sys/socket.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sys/uio.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sys/types.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/types.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/gcc/stddef.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/kernel_types.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/typesizes.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/pthreadtypes.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/sched.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/time.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sys/select.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/select.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/sigset.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/time.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sys/sysmacros.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/uio.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/socket.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/limits.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/gcc/limits.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/posix1_lim.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/local_lim.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/linux/limits.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/uClibc_local_lim.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/posix2_lim.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/xopen_lim.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/stdio_lim.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/sockaddr.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/asm/socket.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/asm/sockios.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/asm/ioctl.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/asm-generic/ioctl.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/in.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/gcc/stdbool.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sys/mount.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sys/ioctl.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/ioctls.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/asm/ioctls.h \
    $(wildcard include/config/pppoe/proxy.h) \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/ioctl-types.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sys/ttydefaults.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/ctype.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/uClibc_touplow.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/dirent.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/dirent.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/errno.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/errno.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/linux/errno.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/asm/errno.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/asm-generic/errno-base.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sys/syscall.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/sysnum.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/fcntl.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/fcntl.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sgidefs.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sys/stat.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/stat.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/inttypes.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/netdb.h \
    $(wildcard include/config/3/ascii/rules.h) \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/rpc/netdb.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/siginfo.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/netdb.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/setjmp.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/setjmp.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/signal.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/signum.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/sigaction.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/sigcontext.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/sigstack.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/ucontext.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sys/ucontext.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/sigthread.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/stdio.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/uClibc_stdio.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/wchar.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/uClibc_mutex.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/pthread.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sched.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/uClibc_clk_tck.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/initspin.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/uClibc_pthread.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/gcc/stdarg.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/stdlib.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/waitflags.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/waitstatus.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/alloca.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/string.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sys/poll.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/poll.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sys/mman.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/mman.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sys/time.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sys/wait.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sys/resource.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/resource.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/termios.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/termios.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/unistd.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/posix_opt.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/environments.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/confname.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/getopt.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/utime.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sys/param.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/linux/param.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/asm/param.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/mntent.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/paths.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/sys/statfs.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/bits/statfs.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/pwd.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/include/grp.h \
  include/xatonum.h \

libbb/find_mount_point.o: $(deps_libbb/find_mount_point.o)

$(deps_libbb/find_mount_point.o):
