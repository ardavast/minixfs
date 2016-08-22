#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/signalvar.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/fcntl.h>
#include <sys/namei.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/priv.h>
#include <sys/buf.h>

#include <geom/geom.h>
#include <geom/geom_vfs.h>

#define MINIXFS_ROOTINO 1

struct minixfs_mnt {
    struct mount *mm_mountp;
    struct vnode *pm_devvp;
    struct cdev *pm_dev;
};

struct minixfs_sb {
    uint16_t s_ninodes;
    uint16_t s_nzones;
    uint16_t s_imap_blocks;
    uint16_t s_zmap_blocks;
    uint16_t s_firstdatazone;
    uint16_t s_log_zone_size;
    uint32_t s_max_size;
    uint16_t s_magic;
    uint16_t s_state;
    uint32_t s_zones;
};

#define VFSTOMINIXFS(mp) ((struct minixfs_mnt *)mp->mnt_data)

static const char *minixfs_opts[] = {"fstype", "fspath", "from", "ro", NULL };

static int minixfs_mountfs(struct vnode *devvp, struct mount *mp);

static int
minixfs_mount(struct mount *mp)
{
    struct vfsoptlist *opts;
    struct vnode *devvp;
    struct thread *td;
    struct nameidata ndp;
    char *from;
    int len;
    accmode_t accmode;
    int error;

    td = curthread;
    opts = mp->mnt_optnew;
    if (vfs_filteropt(opts, minixfs_opts))
        return (EINVAL);

    /* force ro */
    MNT_ILOCK(mp);
    mp->mnt_flag |= MNT_RDONLY;
    MNT_IUNLOCK(mp);

    from = NULL;
    error = vfs_getopt(opts, "from", (void **)&from, &len);
    if (error)
        return (EINVAL);

    /* check if the name is a disk device */
    NDINIT(&ndp, LOOKUP, FOLLOW | LOCKLEAF, UIO_SYSSPACE, from, td);
    error = namei(&ndp);
    if (error)
        return (error);
    NDFREE(&ndp, NDF_ONLY_PNBUF);
    devvp = ndp.ni_vp;

    if (!vn_isdisk(devvp, &error)) {
        vput(devvp);
        return (error);
    }

    /* if mount by non-root, check permissions */
    accmode = VREAD;
    if ((mp->mnt_flag & MNT_RDONLY) == 0)
        accmode |= VWRITE;
    error = VOP_ACCESS(devvp, accmode, td->td_ucred, td);
    if (error)
        error = priv_check(td, PRIV_VFS_MOUNT_PERM);
    if (error) {
        vput(devvp);
        return (error);
    }

    error = minixfs_mountfs(devvp, mp);
    if (error) {
        vrele(devvp);
        return (error);
    }

    vfs_mountedfrom(mp, from);

    return (0);
}

static int
minixfs_mountfs(struct vnode *devvp, struct mount *mp)
{
    //struct cdev *dev = devvp->v_rdev;
    struct g_consumer *cp;
    struct bufobj *bo;
    struct buf *bp;
    struct minixfs_sb *ms;
    int error;

    g_topology_lock();
    error = g_vfs_open(devvp, &cp, "minixfs", 0);
    g_topology_unlock();
    VOP_UNLOCK(devvp, 0);
    if (error)
        return (error);

    bo = &devvp->v_bufobj;


    bp = NULL;
    error = bread(devvp, 2, 1024, NOCRED, &bp);
    if (error)
        goto out;
    ms = (struct minixfs_sb *)bp->b_data;

    printf("s_ninodes: %i ", ms->s_ninodes);
    printf("s_nzones: %i ", ms->s_nzones);
    printf("s_imap_blocks: %i ", ms->s_imap_blocks);
    printf("s_zmap_blocks: %i ", ms->s_zmap_blocks);
    printf("s_firstdatazone: %i ", ms->s_firstdatazone);
    printf("s_log_zone_size: %i ", ms->s_log_zone_size);
    printf("s_max_size: %i ", ms->s_max_size);
    printf("s_magic: %i ", ms->s_magic);
    printf("s_state: %i ", ms->s_state);
    printf("s_zones: %i", ms->s_zones);
    printf("\n");

out:
    if (bp)
        brelse(bp);
    if (cp != NULL) {
        g_topology_lock();
        g_vfs_close(cp);
        g_topology_unlock();
    }

    return (1);
}

static int
minixfs_root(struct mount *mp, int flags, struct vnode **vpp)
{
    struct vnode *nvp;
    int error;

    error = VFS_VGET(mp, MINIXFS_ROOTINO, LK_EXCLUSIVE, &nvp);
    if (error)
        return (error);
    *vpp = nvp;
    return (0);
}

static struct vfsops minixfs_vfsops = {
//    .vfs_fhtovp = minixfs_fhtovp,
    .vfs_mount = minixfs_mount,
    .vfs_root = minixfs_root,
//    .vfs_statfs = minixfs_statfs,
//    .vfs_unmount = minixfs_unmount,
//    .vfs_vget = minixfs_vget,
};

VFS_SET(minixfs_vfsops, minixfs, 0);
MODULE_VERSION(minixfs, 1);
