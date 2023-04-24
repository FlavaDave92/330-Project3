#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/mm.h>
#include <linux/sched/mm.h>
#include <linux/module.h>
#include <linux/pid.h> //i added this for get_pid_task pid maybe not right?




unsigned long timer_interval_ns = 10e9; // 10-second timer
static struct hrtimer hr_timer;
int pid;
unsigned long WSS = 0;
unsigned long RSS = 0;
unsigned long SWAP = 0;
module_param(pid, int, 0);
unsigned long page;

struct vm_area_struct *virtual_memory;


void walk_pte_in_memory(const struct mm_struct *const mm, const unsigned long address)
{
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *ppte, pte;
	
	pgd = pgd_offset(mm, address);
	if (pgd_none(*pgd) || pgd_bad(*pgd))
		return;
	
	p4d = p4d_offset(pgd, address);
	if (p4d_none(*p4d) || p4d_bad(*p4d))
		return;
	
	pud = pud_offset(p4d, address);
	if (pud_none(*pud) || pud_bad(*pud))
		return;
	
	pmd = pmd_offset(pud, address);
	if (pmd_none(*pmd) || pmd_bad(*pmd))
		return;
	
	ppte = pte_offset_map(pmd, address);
	if (!ppte)
		return;

    pte = *ppte;

        if(!pte_none(pte)){
		if(pte_present(pte)) {
			RSS++;
			if(pte_young(pte)){
				WSS++;
				test_and_clear_bit(_PAGE_BIT_ACCESSED, (unsigned long *)ppte);
			}
		} 	
		else {
			SWAP++;
		}
	}    
	
	
}

enum hrtimer_restart timer_callback( struct hrtimer *timer_for_restart )
{
	// Resetting the timer, which also means… ?
  	ktime_t currtime , interval;
  	currtime  = ktime_get();
  	interval = ktime_set(0,timer_interval_ns); 
  	hrtimer_forward(timer_for_restart, currtime , interval);

	// Do the measurement, like looking into VMA and walking through memory pages

	//task = pid_task(find_vpid(pid), PIDTYPE_PID);
	struct task_struct *task;
	for_each_process(task)
		if (task->pid == pid)
		{
			virtual_memory = task->mm->mmap;
			while(virtual_memory){

    				for(page = (unsigned long) virtual_memory->vm_start; page < (unsigned long) virtual_memory->vm_end; page+=PAGE_SIZE)
    				{
        				walk_pte_in_memory(task->mm, page);
    				}
        			virtual_memory = virtual_memory->vm_next;
    			}
	
		}
	// And also do the Kernel log printing aka printk per requirements
	printk(KERN_INFO "PID [%d]: RSS=[%ld] KB, SWAP=[%ld] KB, WSS=[%ld] KB", pid,RSS,SWAP,WSS);
	return HRTIMER_RESTART;
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
MODULE_LICENSE("GPL");
