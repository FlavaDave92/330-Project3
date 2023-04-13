#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/mm.h>
#include <linux/sched/mm.h>
#include <linux/module.h>

unsigned long timer_interval_ns = 10e9; // 10-second timer
static struct hrtimer hr_timer;
int RSS, WSS, SWAP; //counters

enum hrtimer_restart timer_callback( struct hrtimer *timer_for_restart )
{
	// Resetting the timer, which also means… ?
  	ktime_t currtime , interval;
  	currtime  = ktime_get();
  	interval = ktime_set(0,timer_interval_ns); 
  	hrtimer_forward(timer_for_restart, currtime , interval);

	// Do the measurement, like looking into VMA and walking through memory pages
	// pte_t *ppte, pte; ppte means 'pointer of PTE'
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
