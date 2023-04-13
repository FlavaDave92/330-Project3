#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/mm.h>
#include <linux/sched/mm.h>
#include <linux/module.h>


unsigned long timer_interval_ns = 10e9; // 10-second timer
static struct hrtimer hr_timer;
int RSS, WSS, SWAP, pid;
module_param(pid, int, 0);
//int ptep_test_and_clear_young (struct vm_area_struct *vma, unsigned long addr, pte_t *ptep);

enum hrtimer_restart timer_callback( struct hrtimer *timer_for_restart )
{
	// Resetting the timer, which also means… ?
  	ktime_t currtime , interval;
  	currtime  = ktime_get();
  	interval = ktime_set(0,timer_interval_ns); 
  	hrtimer_forward(timer_for_restart, currtime , interval);

	// Do the measurement, like looking into VMA and walking through memory pages
	// pte_t *ppte, pte; ppte means 'pointer of PTE'
	task_struct *task;
	pte_t *ptep = search_pte_in_memory(task->mm, /*sth here*/); // missing mm_struct and address, don't know how to find them
	pte = *ppte;
	If pte exists {
		If pte is present {
			RSS++
			If pte is young {
				WSS++
				test_and_clear_bit(_PAGE_BIT_ACCESSED,(unsigned long *)ppte);
			}
		} 	
		Else {
			SWAP++
		}
	}

	// And also do the Kernel log printing aka printk per requirements
	printk(KERN_INFO "PID [%s]: RSS=[%d] KB, SWAP=[%d] KB, WSS=[%d] KB", PID,RSS,SWAP,WSS);

	return HRTIMER_RESTART;
}

/*
int ptep_test_and_clear_young (struct vm_area_struct *vma, unsigned long addr, pte_t *ptep)
{
	int ret = 0;
	if (pte_young(*ptep))
		ret = test_and_clear_bit(_PAGE_BIT_ACCESSED, (unsigned long *) &ptep->pte);
	return ret;
}
*/

struct pte_t *search_pte_in_memory(const struct mm_struct *const mm, const unsigned long address)
{
	pgd_t *pdg;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *ptep;
	
	pgd = pgd_offset(mm, address);
	if (pgd_none(*pgd) || pgd_bad(*pgd))
		return NULL;
	
	p4d = p4d_offset(pgd, address);
	if (p4d_none(*p4d) || p4d_bad(*p4d))
		return NULL;
	
	pud = pud_offset(p4d, address);
	if (pud_none(*pud) || pud_bad(*pud))
		return NULL;
	
	pmd = pmd_offset(pud, address);
	if (pmd_none(*pmd) || pmd_bad(*pmd))
		return NULL;
	
	ptep = pte_offset_map(pmd, address);
	if (!ptep)
		return NULL;
	
	return ptep;
}

static int __init timer_init(void) {
	ktime_t ktime ;
	ktime = ktime_set( 0, timer_interval_ns );
	hrtimer_init( &hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
	hr_timer.function = &timer_callback;
 	hrtimer_start( &hr_timer, ktime, HRTIMER_MODE_REL );
	return 0;
}

static void __exit timer_exit(void) {
	int ret;
  	ret = hrtimer_cancel( &hr_timer );
  	if (ret) printk("The timer was still in use...\n");
  	printk("HR Timer module uninstalling\n");
	
}

module_init(timer_init);
module_exit(timer_exit);