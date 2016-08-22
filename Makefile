KMOD=minixfs
SRCS=minixfs.c vnode_if.h

DEBUG_FLAGS=-g

.include <bsd.kmod.mk>
