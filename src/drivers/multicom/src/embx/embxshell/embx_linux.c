/*******************************************************************/
/* Copyright 2008 STMicroelectronics R&D Ltd. All rights reserved. */
/*                                                                 */
/* File: embx_linux.c                                              */
/*                                                                 */
/* Description:                                                    */
/*         Operating system abstraction implementation             */
/*                                                                 */
/*******************************************************************/

#include "embx_osinterface.h"
#include "debug_ctrl.h"

#if defined(__KERNEL__)
#ifndef ioremap_cache
/* XXXX What can we do in this case ? */
#define ioremap_cache ioremap
#endif
#endif

/*
 * This header is needed to collect the prototype for memalign.
 */
#include <malloc.h>

EMBX_VOID *EMBX_OS_ContigMemAlloc(EMBX_UINT size, EMBX_UINT align)
{
    void *alignedAddr = NULL;

    EMBX_Info(EMBX_INFO_OS, (">>>>ContigMemAlloc(size=%u, align=%u)\n",size,align));

#if defined(__KERNEL__) && defined(__sh__)

    /* For Linux bigphysarea must be available and set to a sensible size */
    {
        int pages      = (0 == size  ? 0 : ((size  - 1) / PAGE_SIZE) + 1);
        int page_align = (0 == align ? 0 : ((align - 1) / PAGE_SIZE) + 1);

	/* allocate an carefully aligned block of memory */
	EMBX_Info(EMBX_INFO_OS, ("    trying to allocate %d pages (aligned to %d pages)\n", pages, page_align));
        alignedAddr = (void *) bigphysarea_alloc_pages(pages, page_align, GFP_KERNEL);

	if (alignedAddr) {
	    /* ensure there are no cache entries covering this address */
	    dma_cache_wback_inv(alignedAddr, size);
	}
    }

#elif defined(__KERNEL__)

    printk("EMBX_OS_ContigMemAlloc Not implemented yet!!!\n");

#else
    /* allocate a carefully aligned block of memory */
    alignedAddr = memalign(align, size);
#endif

    EMBX_Info(EMBX_INFO_OS, ("<<<<ContigMemAlloc = 0x%08x\n", (unsigned) alignedAddr));
    return (EMBX_VOID *)alignedAddr;
}


void EMBX_OS_ContigMemFree(EMBX_VOID *addr, EMBX_UINT size)
{
    EMBX_Info(EMBX_INFO_OS, (">>>>ContigMemFree\n"));

#if defined(__KERNEL__) && defined(__sh__)

    bigphysarea_free_pages(addr);

#elif defined(__KERNEL__)

    printk("EMBX_OS_ContigMemFree not implemented yet\n");

#else
    
    free(addr);

#endif

    EMBX_Info(EMBX_INFO_OS, ("<<<<ContigMemFree\n"));
}



EMBX_VOID *EMBX_OS_MemAlloc(EMBX_UINT size)
{
    void *pAddr;

#if defined(__KERNEL__)

#if defined(__sh__)
    if( size >= (PAGE_SIZE * 4) )
        pAddr = (EMBX_VOID *)bigphysarea_alloc( size );
    else
#endif
        pAddr = (EMBX_VOID *)kmalloc( size, GFP_KERNEL );

#else

    pAddr = (EMBX_VOID *)malloc(size);

#endif

    return pAddr;
}



/* This is an ANSI C like memory deallocate. 
 *
 * Because it is ANSI C like it is defined that this function will ignore
 * a NULL argument and return immediately.
 */
void EMBX_OS_MemFree(EMBX_VOID *addr)
{

#if defined(__KERNEL__)

#if defined __sh__
    unsigned long  Base, Size;
    unsigned long  Address = (unsigned long)addr;

    bigphysarea_memory( &Base, &Size );
    if( (Address >= Base) && (Address < (Base + Size)) )
        bigphysarea_free_pages((void*)addr);
    else
#endif
        /*
         * kfree will (correctly) ignore NULL arguments
         */
        kfree((void*)addr);

#else

    free((void *)addr);

#endif

}

/*------------------------ MEMORY ADDRESS TRANSLATION -----------------------*/

EMBX_VOID *EMBX_OS_PhysMemMap(EMBX_UINT pMem, int size, int cached)
{
    EMBX_VOID *vaddr = NULL;

    EMBX_Info(EMBX_INFO_OS, (">>>>PhysMemMap(0x%08x, %d)\n", (unsigned int) pMem, size));

#if defined(__KERNEL__)
    if (cached)
	vaddr = ioremap_cache((unsigned long) pMem, size);
    else
	vaddr = ioremap_nocache((unsigned long) pMem, size);
#else

    EMBX_Info(EMBX_INFO_OS, ("<<<<PhysMemMap\n"));

    return (EMBX_VOID *)pMem;

#endif

    EMBX_Info(EMBX_INFO_OS, ("PhysMemMap: *vMem = %p\n", vaddr));
    
    EMBX_Info(EMBX_INFO_OS, ("<<<<PhysMemMap\n"));
    
    return vaddr;
}



void EMBX_OS_PhysMemUnMap(EMBX_VOID *vMem)
{
    EMBX_Info(EMBX_INFO_OS, (">>>>PhysMemUnMap\n"));

#if defined(__KERNEL__)
    iounmap(vMem);
#endif

    EMBX_Info(EMBX_INFO_OS, ("<<<<PhysMemUnMap\n"));
}

/* 
 * Translate a kernel virtual address to a phyiscal one 
 *
 * Returns either EMBX_SUCESS or EMBX_INCOHERENT_MEMORY when translation succeeds
 * Returns EMBX_INVALID_ARGUMENT otherwise
 */
EMBX_ERROR EMBX_OS_VirtToPhys(EMBX_VOID *vaddr, EMBX_UINT *paddrp)
{
#if defined(__KERNEL__)

    unsigned long addr = (unsigned long) vaddr;

    if (addr < P1SEG || ((addr >= VMALLOC_START) && (addr < VMALLOC_END)))
    {
	/*
	 * Find the virtual address of either a user page (<P1SEG) or VMALLOC (P3SEG)
	 *
	 * This code is based on vmalloc_to_page() in mm/memory.c
	 */
	struct mm_struct *mm;

	pgd_t *pgd;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10)
	pud_t *pud;
#endif
	pmd_t *pmd;
	pte_t *ptep, pte;
	
	/* Must use the correct mm based on whether this is a kernel or a userspace address */
	if (addr >= VMALLOC_START)
	    mm = &init_mm;
	else
	    mm = current->mm;

	/* Safety first! */
	if (mm == NULL)
	    return EMBX_INVALID_ARGUMENT;

	spin_lock(&mm->page_table_lock);
	
	pgd = pgd_offset(mm, addr);
	if (pgd_none(*pgd) || pgd_bad(*pgd))
	    goto out;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10)
	pud = pud_offset(pgd, addr);
	if (pud_none(*pud) || pud_bad(*pud))
	    goto out;
	
	pmd = pmd_offset(pud, addr);
#else
	pmd = pmd_offset(pgd, addr);
#endif
	if (pmd_none(*pmd) || pmd_bad(*pmd))
	    goto out;
	
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
	ptep = pte_offset(pmd, addr);
#else
	ptep = pte_offset_map(pmd, addr);
#endif
	if (!ptep) 
	    goto out;

	pte = *ptep;
	if (pte_present(pte)) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,9)
	    pte_unmap(ptep);
#endif
	    spin_unlock(&mm->page_table_lock);
	    
	    /* pte_page() macro is broken for SH in linux 2.6.20 and later */
	    *paddrp = page_to_phys(pfn_to_page(pte_pfn(pte))) | (addr & (PAGE_SIZE-1));
	    
	    /* INSbl28636: P3 segment pages cannot be looked up with pmb_virt_to_phys()
	     * instead we need to examine the _PAGE_CACHABLE bit in the pte
	     */
	    return ((pte_val(pte) & _PAGE_CACHABLE) ? EMBX_INCOHERENT_MEMORY : EMBX_SUCCESS);
	}
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,9)
	pte_unmap(ptep);
#endif
	
    out:
	spin_unlock(&mm->page_table_lock);

	/* Failed to find a pte */
	return EMBX_INVALID_ARGUMENT;
    }
    else
#if defined(CONFIG_32BIT)
    {
	unsigned long flags;
	
	/* Try looking for an ioremap() via the PMB */
	if (pmb_virt_to_phys(vaddr, (unsigned long *)paddrp, &flags) == 0)
	{
	    /* Success: Test the returned PMB flags */
	    return ((flags & PMB_C) ? EMBX_INCOHERENT_MEMORY : EMBX_SUCCESS);
	}

	/* Failed to find a mapping */
	return EMBX_INVALID_ARGUMENT;
    }
#else
    {
	unsigned long addr = (unsigned long) vaddr;
	
	/* Assume 29-bit SH4 Linux */
	*(paddrp)= PHYSADDR(addr);
	
	/* only the P2SEG is uncached (doubt we will see P4SEG addresses) */
	return ((PXSEG(addr) == P2SEG) ? EMBX_SUCCESS: EMBX_INCOHERENT_MEMORY);
    }
#endif /* CONFIG_32BIT */

#endif /* __KERNEL__ */

    /* Not implemented */
    return EMBX_INVALID_ARGUMENT;
}


/*--------------------------- THREAD CREATION -------------------------------*/

#if defined(__KERNEL__)

#include <linux/sched.h>

static char *_default_name = "embx";

static char *get_default_name(void)
{
    /* Trivial for now, think about adding an incrementing
     * number to the name, but how do we deal with mutual
     * exclusion, or don't we care for this?
     */
    return _default_name;
}

struct ThreadInfo
{
    struct task_struct *kthread;
    void (*entry)(void *);
    void *param;
    EMBX_INT priority;
};

#ifdef __sh__

static void SetPriority(struct ThreadInfo *t, int policy, struct sched_param *sp)
{
    int res;

#ifdef MULTICOM_GPL
    res = sched_setscheduler(t->kthread, policy, &sp); 	/* GPL only function */
#else
    /* Call sched_setscheduler() via an in kernel syscall */
    register long __sc0 __asm__ ("r3") = __NR_sched_setscheduler;
    register long __sc4 __asm__ ("r4") = (long) t->kthread->pid;
    register long __sc5 __asm__ ("r5") = (long) policy;
    register long __sc6 __asm__ ("r6") = (long) sp;

    __asm__ __volatile__ ("trapa #0x13" : "=z" (__sc0)
			  : "0" (__sc0), "r" (__sc4), "r" (__sc5), "r" (__sc6)
			  : "memory");

    res = __sc0;
#endif /* MULTICOM_GPL */

    if (res)
	EMBX_Info(EMBX_INFO_OS, ("sched_setscheduler(%p, %d) failed : %d\n", t->kthread, sp->sched_priority, res));
}
#endif /* __sh__ */

static int ThreadHelper(void *param)
{
    struct ThreadInfo *t = param;

    /* priorities beyond -20 provoke a different scheduling policy */
    if (t->priority < -20)
    {
	struct sched_param sp;

	/* Set the RT thread priority (1 .. 99) and RT scheduler class */
	sp.sched_priority = -(t->priority + 20);

	SetPriority(t, SCHED_RR, &sp);

	/* Run the actual task code */
	t->entry(t->param);
    }
    else
    {
	/* Priorities >= -20 we assume its a nice level */
	set_user_nice(t->kthread, t->priority);
	
	t->entry(t->param);

	set_user_nice(t->kthread, 19);
    }

    /* wait for the user to call kthread_stop() */
    while (!kthread_should_stop()) {
	set_current_state(TASK_INTERRUPTIBLE);
	schedule();
    }

    return 0;
}

EMBX_THREAD EMBX_OS_ThreadCreate(void (*entry)(void *), void *param, EMBX_INT priority, const EMBX_CHAR *name)
{
    struct ThreadInfo *t;
    
    EMBX_Info(EMBX_INFO_OS, (">>>>EMBX_OS_ThreadCreate priority %d\n", priority));

    t = (struct ThreadInfo *) EMBX_OS_MemAlloc(sizeof(struct ThreadInfo));
    if (!t) {
	EMBX_Info(EMBX_INFO_OS, ("<<<<EMBX_OS_ThreadCreate = EMBX_INVALID_THREAD (#1)\n"));
	return EMBX_INVALID_THREAD;
    }

    t->entry = entry;
    t->param = param;
    t->priority = priority;

    if (name == EMBX_DEFAULT_THREAD_NAME) {
	name = get_default_name();
    }

    t->kthread = kthread_create(ThreadHelper, t, "%s", name);

    if (IS_ERR(t->kthread)) {
	EMBX_OS_MemFree(t);
	EMBX_Info(EMBX_INFO_OS, ("<<<<EMBX_OS_ThreadCreate = EMBX_INVALID_THREAD (#2)\n"));
	return EMBX_INVALID_THREAD;
    }

    wake_up_process(t->kthread);

    EMBX_Info(EMBX_INFO_OS, ("<<<<EMBX_OS_ThreadCreate = 0x%p\n", t));
    return t;
}

EMBX_ERROR EMBX_OS_ThreadDelete(EMBX_THREAD thread)
{
    struct ThreadInfo *t = (struct ThreadInfo *) thread;
    int res;

    EMBX_Info(EMBX_INFO_OS, (">>>>EMBX_OS_ThreadDelete\n"));

    if(thread == EMBX_INVALID_THREAD) {
	EMBX_Info(EMBX_INFO_OS, ("<<<<EMBX_OS_ThreadDelete = EMBX_SUCCESS (invalid task)\n"));
        return EMBX_SUCCESS;
    }

    res = kthread_stop(t->kthread);
    if (0 != res) {
	EMBX_Info(EMBX_INFO_OS, ("<<<<EMBX_OS_ThreadDelete = EMBX_SYSTEM_INTERRUPT\n"));
	return EMBX_SYSTEM_INTERRUPT;
    }
    
    EMBX_OS_MemFree(t);

    EMBX_Info(EMBX_INFO_OS, ("<<<<EMBX_OS_ThreadDelete = EMBX_SUCCESS\n"));
    return EMBX_SUCCESS;
}

#else

/*
 * User space libraries, use pthreads
 */
EMBX_THREAD EMBX_OS_ThreadCreate(void (*thread)(void *), void *param, EMBX_INT priority, const EMBX_CHAR *name)
{
    pthread_t *tid;

    EMBX_Info(EMBX_INFO_OS, (">>>>ThreadCreate\n"));

    tid = (pthread_t *)EMBX_OS_MemAlloc(sizeof(pthread_t));

    if(tid != EMBX_INVALID_THREAD)
    {
        if(pthread_create(tid, NULL, (void*(*)(void*))thread, param))
        {
            EMBX_DebugMessage(("ThreadCreate: task_create failed.\n"));

            EMBX_OS_MemFree(tid);
            tid = EMBX_INVALID_THREAD;
        }
    }

    EMBX_Info(EMBX_INFO_OS, ("<<<<ThreadCreate\n"));

    return tid;
}

EMBX_ERROR EMBX_OS_ThreadDelete(EMBX_THREAD thread)
{
    EMBX_Info(EMBX_INFO_OS, (">>>>EMBX_OS_ThreadDelete\n"));

    if(thread == EMBX_INVALID_THREAD) {
	EMBX_Info(EMBX_INFO_OS, ("<<<<EMBX_OS_ThreadDelete = EMBX_SUCCESS (invalid task)\n"));
        return EMBX_SUCCESS;
    }

    pthread_join((pthread_t)thread, NULL);
    EMBX_OS_MemFree(thread);

    EMBX_Info(EMBX_INFO_OS, ("<<<<EMBX_OS_ThreadDelete = EMBX_SUCCESS\n"));
    return EMBX_SUCCESS;
}

#endif /* __KERNEL__ */


/*
 * Local Variables:
 *  tab-width: 8
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
