//
//  spid.c: Simple Parallel (Port) interrupt driver
//
//  Copyright (C) 2003
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

// 
// Based on code from O'Reilly's Linux device drivers (second edition)
// 
// Implemented and commented by T. Muller
// ASTRON (Netherlands Foundation for Research in Atronomy)
//

#ifndef __KERNEL__
#  define __KERNEL__  /* Preprocessor : Code for Kernel */
#endif
#ifndef MODULE
#  define MODULE      /* Preprocessor : Code for Module */
#endif

#ifdef __sparc__
#  error "This module can't compile on the sparc platform"
#else

#include <linux/module.h> /* for module in linux kernel */

#include <linux/sched.h>
#include <linux/kernel.h> /* for printk() etc. */
#include <linux/fs.h>     /* everything... read, write etc. */
#include <linux/errno.h>  /* error codes */
#include <linux/delay.h>  /* udelay */
#include <linux/slab.h>   /* malloc is deprecated, use slab.h instead */
#include <linux/mm.h>
#include <linux/ioport.h> /* check and reserv ports or memory */
#include <linux/interrupt.h>
#include <linux/tqueue.h>
#include <linux/poll.h>

#include <asm/io.h>       /* send/receive data of port or memory */
#include <asm/uaccess.h>  /* copy between userspace and kernelspace V.V. */

/* debugging defines */
#ifdef CONFIG_NTP_PPS_SPID
#undef DEBUG_NTP_PPS
/*#define DEBUG_NTP_PPS*/
#endif /* CONFIG_NTP_PPS_SPID */

#ifdef	CONFIG_NTP_PPS_SPID
#include <linux/time.h> 	/* struct timeval, do_gettimeofday(),
				   hardpps() */
#include <linux/timepps.h>	/* PPS API */
#include <linux/slab.h>		/* kmalloc(), kfree() */
#endif

#ifdef MODULE
#ifndef CONFIG_NTP_PPS_SPID
MODULE_DESCRIPTION("Simple Parallel (Port) Interrupt Driver for MAC");
#else
MODULE_DESCRIPTION("Simple Parallel (Port) Interrupt Driver for MAC +PPS_API (RFC-2783)");
#endif /* CONFIG_NTP_PPS_SPID */
MODULE_AUTHOR("Tilman Muller");
MODULE_LICENSE("GPL");

MODULE_PARM (spid_base, "i");
MODULE_PARM (spid_irq, "h");
#endif

int spid_major = 0;

#ifdef  __alpha__
int spid_base = 0x3bc;   /* Port address Parallel Port */ 
#else
int spid_base = 0x378;   /* Port address Parallel Port */
#endif

unsigned char spid_irq_requested = 0;
unsigned int spid_interrupt = 0;
int spid_irq = 7;


int spid_open (struct inode *inode, struct file *filp);
void spid_isr(int irq, void* dev_id, struct pt_regs* regs);
int spid_release (struct inode* inode, struct file* filp);
ssize_t spid_read (struct file* filp, char* buf, size_t count, loff_t* f_pos);
ssize_t spid_write (struct file* filp, const char* buf, size_t count, loff_t* f_pos);
unsigned int spid_poll(struct file* filp, poll_table* wait);
int spid_ioctl(struct inode* inode, struct file* filp, unsigned int cmd, unsigned long arg);

// first, the port-oriented device
// Up to 8 different Devices possible from 0x378 to 0x380
// spid0 - spid7;
// spid0p - spid7p;
// spid0s - spid7s

// Each Device is accessing one Memoryaddress (Read/Write)
// Actually the driver is more a Memoryaddress interface than a Port interface

// There are 3 modes available : 
// spid0 for normal 8 bits operations; Minor_Type = 0
// spid0p for 8 bits operations with delays between operations; Minor_Type=1
// spid0s for string operations; Minor_Type=2  
// Enum Modes == Minor_Type
 
enum spid_modes {SPID_DEFAULT=0, SPID_PAUSE, SPID_STRING};

#define SPID_IOC_MAGIC 'E'

#define SPID_IOCHARDREST _IO(SPID_IOC_MAGIC, 0)
#define SPID_IOC_MAXNR 0

// In the file operations table the pointers to functions are given which
// the driver can use. This are the operations the driver can handle.
// The file operations table is a standard table and is in every driver
// available. In that table the functions are given that are necessary 
// for that type of driver. So the list could be longer. 

struct file_operations spid_fops = {
    read: spid_read,
    write: spid_write,
    open: spid_open,
    release: spid_release,
    poll: spid_poll,
    ioctl: spid_ioctl,
};

#ifdef	CONFIG_NTP_PPS_SPID
#define NANOSECOND	1000000000

/* update PPS info from the event time stamp stored in etime and ecount. */
static inline void pps_update_event(struct file * file)
{
        struct pps * pp = (struct pps*)file->private_data;
	int mode;

	if (NULL == pp) {
#ifdef DEBUG_NTP_PPS
		printk(KERN_ERR
		       "pps_update_event(): no pps struct");
#endif /* DEBUG_NTP_PPS */
		return;
	}
	mode = pp->state.parm.mode;
	if ((mode & (PPS_ECHOASSERT|PPS_ECHOCLEAR)) != 0) {
		/* echo event, how? */
#if 0
		info->MCR |= UART_MCR_RTS;		/* signal no event */
		if (((modem_status & UART_MSR_DCD) != 0) ==
		    ((mode & PPS_ECHOASSERT) != 0))
			info->MCR &= ~UART_MCR_RTS;	/* signal event */
		serial_out(info, UART_MCR, info->MCR);
#endif
	}
	
	/* get timestamp */
	/*if ((modem_status & UART_MSR_DCD) != 0)*/
	{
		struct timespec ts;
		ts = pp->state.etime;
		pp->state.info.current_mode = mode;
		if ((mode & PPS_OFFSETASSERT) != 0) {
			ts.tv_nsec += pp->state.parm.assert_offset.tv_nsec;
			if (ts.tv_nsec >= NANOSECOND) {
				ts.tv_nsec -= NANOSECOND;
				++ts.tv_sec;
			} else if (ts.tv_nsec < 0) {
				ts.tv_nsec += NANOSECOND;
				--ts.tv_sec;
			}
		}
		if ((pps_kc_hardpps_mode & PPS_CAPTUREASSERT) != 0 &&
		    pps_kc_hardpps_dev == (void *) file) {
#ifdef DEBUG_NTP_PPS
		    printk(KERN_INFO "spid: calling hardpps\n");
#endif
                    hardpps(&ts, pp->state.ecount);
		}
		if ((mode & PPS_CAPTUREASSERT) != 0) {
			pp->state.info.assert_timestamp = ts;
			++pp->state.info.assert_sequence;
			if (waitqueue_active(&pp->state.ewait))
				wake_up_interruptible(&pp->state.ewait);
		}
#ifdef DEBUG_NTP_PPS
		printk(KERN_INFO
		       "ASSERT event #%lu for %p at %lu.%09ld (%9ld)\n",
		       pp->state.info.assert_sequence, pp, ts.tv_sec,
		       ts.tv_nsec, pp->state.ecount);
#endif /* DEBUG_NTP_PPS */
	} 
#if 0
/*else*/ {
		struct timespec ts;
		ts = pp->state.etime;
		pp->state.info.current_mode = mode;
		if ((mode & PPS_OFFSETCLEAR) != 0) {
			ts.tv_nsec += pp->state.parm.clear_offset.tv_nsec;
			if (ts.tv_nsec >= NANOSECOND) {
				ts.tv_nsec -= NANOSECOND;
				++ts.tv_sec;
			} else if (ts.tv_nsec < 0) {
				ts.tv_nsec += NANOSECOND;
				--ts.tv_sec;
			}
		}
		if ((pps_kc_hardpps_mode & PPS_CAPTURECLEAR) != 0 &&
		    pps_kc_hardpps_dev == (void *) file)
			hardpps(&ts, pp->state.ecount);
		if ((mode & PPS_CAPTURECLEAR) != 0) {
			pp->state.info.clear_timestamp = ts;
			++pp->state.info.clear_sequence;
			if (waitqueue_active(&pp->state.ewait))
				wake_up_interruptible(&pp->state.ewait);
		}
#ifdef DEBUG_NTP_PPS
		printk(KERN_INFO
		       "CLEAR event #%lu for %p at %lu.%09ld (%9ld)\n",
		       pp->state.info.clear_sequence, pp, ts.tv_sec,
		       ts.tv_nsec, pp->state.ecount);
#endif /* DEBUG_NTP_PPS */
	}
#endif /* 0 */
#undef NANOSECOND
}
#endif

/* init and cleanup */

// Insert a Module in the Linux Kernel
// check the memory area is not in use and register that memory area 
int init_module(void)
{
  // check if Port is in use
  int result = check_region(spid_base,4);

  // if result == 0 then Port is available and Port is reserved for 
  // this Device 
  if (result) 
  {
    printk(KERN_INFO "spid: can't get I/O port address 0x%x\n",
            spid_base);
    return result;
  }

  // Now the Port is reserved the Device Driver can be registed in the
  // kernel. A new character device is generated with the name spid
  // and gets it's Majornumber.
  // When spid_major is set to "0" before calling this routine the
  // kernel is generating a Majornumber (255 - xx). This is called
  // dynamic allocation.   
  result = register_chrdev(spid_major, "spid", &spid_fops);
  if (result < 0) 
  {
    printk(KERN_INFO "spid: can't get major number\n");
    // free the reservation of the Port for this device
    release_region(spid_base,4);
    return result;
  }
  // dynamic allocation; kernel is generating a new Majornumber (255 - xx)
  if (spid_major == 0) spid_major = result;

  return 0;
}

// remove the Module out of the Linux Kernel
void cleanup_module(void)
{
  // Majornumber is released; Available again for antoher Device Driver.
  unregister_chrdev(spid_major, "spid");

  // free the reservation of the Port for this device
  release_region(spid_base,4);
}

/*
 * The devices with low minor numbers write/read burst of data to/from
 * specific I/O ports (by default the parallel ones).
 * 
 * The device with 128 as minor number returns ascii strings telling
 * when interrupts have been received. Writing to the device toggles
 * 00/FF on the parallel data lines. If there is a loopback wire, this
 * generates interrupts.  
 */

// Opening a Device because device is treated like a normal file.
// Call to MOD_INC_USE_COUNT, because kernel administrates the number of
// use of the device. Therefor count has to be 0 to release a Module.

int spid_open (struct inode *inodeisr , struct file *filp)
{
  if (!MOD_IN_USE)  outb_p(0x00, spid_base);     // clear the bit
  
  MOD_INC_USE_COUNT;
  
  if (spid_irq < 0)
  {
    int count = 0;
    unsigned long mask;
    do 
    {
      mask = probe_irq_on();
      outb_p(0x10, spid_base + 2); // enable reporting
      outb_p(0x00, spid_base);     // clear the bit
      outb_p(0x10, spid_base);     // set the bit: interrupt!!!      
      outb_p(0x00, spid_base + 2); // disable reporting
      udelay(5);
      spid_irq = probe_irq_off(mask);

      if (spid_irq == 0) 
      { // none of them
        printk(KERN_DEBUG "spid: no irq reported by probe %d\n", count);
        spid_irq = -1;
      }        
    } while (spid_irq < 0 && count++ < 5);
  }
  
  if (spid_irq_requested == 0)
  {
    if (spid_irq < 0)
    {
      printk(KERN_INFO "spid: probe failed 5 times, giving up\n");
    }
    else
    {
      spid_irq_requested = 1;
      printk(KERN_INFO "spid: Found and use irq %d\n", spid_irq);
      inb(spid_base + 1);
      outb(0x10, spid_base + 2); // enables reporting the interrupt
#ifndef CONFIG_NTP_PPS_SPID
      request_irq(spid_irq, spid_isr, SA_INTERRUPT, "spid interrupt service routine", NULL);
#else
      request_irq(spid_irq, spid_isr, SA_INTERRUPT, "spid +PPS (RFC-2783)", filp);
#endif
    }
  }

#ifdef CONFIG_NTP_PPS_SPID
  filp->private_data = NULL; /* holds pointer to the struct pps */
#endif /* CONFIG_NTP_PPS_SPID */

  return 0;
}

// This is the interrupt service routine, which is passed in the request_irq method 
// in spid_open, it will invoked if the value of pin 10 in the parallel port changed 
// from 0 to 1 (interrupt)

void spid_isr(int irq, void* dev_id, struct pt_regs* regs)
{
  /*printk(KERN_INFO "spid: Interrupt\n");*/
  
  outb_p(0x00, spid_base);
  // in some cases it seams this is needed to force a down edge of the pin 10
  inb(spid_base + 1); 
  if (MOD_IN_USE) spid_interrupt++;

#ifdef CONFIG_NTP_PPS_SPID
  struct file * file = (struct file*) dev_id;
  struct pps * pps = (file ? file->private_data : NULL);
				
  if (NULL != pps)
  {
    pps_update_event(file);
    
    /* store timestamp in case it is wanted later */
    pps->state.ecount = do_clock_gettime(CLOCK_REALTIME,
					    &pps->state.etime);
    
/*#ifdef DEBUG_NTP_PPS*/
    printk(KERN_INFO
	   "spid_isr: time %ld:%ld(%ld)\n",
	   pps->state.etime.tv_sec,
	   pps->state.etime.tv_nsec,
	   pps->state.ecount);
/*#endif*/
  }
#endif /* CONFIG_NTP_PPS_SPID */
}

// Closing a Device because device is treated like a normal file.
// Call to MOD_DEC_USE_COUNT, because kernel administrates the number of
// use of the device. Therefor count has to be 0 to release a Module.

int spid_release (struct inode* inode, struct file* filp)
{
  MOD_DEC_USE_COUNT;
  if (!MOD_IN_USE)
  {    
#ifdef CONFIG_NTP_PPS_SPID
    struct pps* pps = (struct pps*)filp->private_data;
    
    if (NULL != pps)
    {
      struct pps *tmp = pps;
      if (waitqueue_active(&tmp->state.ewait))
	wake_up_interruptible(&tmp->state.ewait);
      /*info->tty->termios->c_line = N_TTY;*/
      filp->private_data = NULL; /*info->tty->disc_data = NULL;*/
      /*tmp->magic = 0;*/
      kfree(tmp);
      if (pps_kc_hardpps_dev == (void *) filp) {
	pps_kc_hardpps_mode = 0;
	pps_kc_hardpps_dev = NULL;
#ifdef DEBUG_NTP_PPS
	printk(KERN_INFO
	       "spid_release(): unbound kernel consumer dev %p\n",
	       filp);
#endif /* DEBUG_NTP_PPS */
      }
#ifdef DEBUG_NTP_PPS
      printk(KERN_INFO "spid_release(): removed pps %p from file %p\n",
	     tmp, filp);
#endif /* DEBUG_NTP_PPS */
    }
#endif /* CONFIG_NTP_PPS_SPID */

    if (spid_irq > 0)
    {
      printk(KERN_INFO "spid: Release irq %d\n", spid_irq);
#ifndef CONFIG_NTP_PPS_SPID
      free_irq(spid_irq, NULL);
#else
      free_irq(spid_irq, filp);
#endif
    }
    outb_p(0x00, spid_base + 2); // disable reporting the interrupt
    spid_interrupt = 0;
    spid_irq_requested = 0;
  }
  return 0;
}

// spid_read( filepointer to struct with fileparameters b.v. f_count & f_pos,
//             pointer to buffer in userspace, 
//             number of Bytes,
//             pointer for position in the file == f_pos)

ssize_t spid_read (struct file* filp, char* buf, size_t count, loff_t* f_pos)
{
  if (spid_interrupt == 0) return 0; // no interrupt 
    
  if (count > 0)
  {
    char interruptCount[] = { '0' + spid_interrupt, 0};
    copy_to_user(buf, interruptCount, 1);
  }
  spid_interrupt = 0; // reset interrupt indication
  
  return 1;
}

// spid_write( filepointer to struct with fileparameters b.v. f_count & f_pos,
//              pointer to buffer in userspace, 
//              number of Bytes,
//              pointer to position in the file == f_pos)

ssize_t spid_write (struct file* filp, const char* buf, size_t count, loff_t* f_pos)
{
  int retval = count;

  printk(KERN_INFO "spid: write %d bytes\n", count);

  if (buf[0] == 's' || buf[0] == 'S')
  {
    printk(KERN_INFO "spid: starts with 'S|s'; it simulates an interrupt\n");
    spid_interrupt = 1;
    return count;
  }
  
  // get Minor_Type(high nibble) = 0 for spid0; 1 for spid0p and
  // 2 for spid0s.
  int mode = (MINOR(filp->f_dentry->d_inode->i_rdev)&0x70) >> 4;
  
  unsigned char toggelVal = 0xFF;
  
  retval = count;
  // switch mode depend on what type I/O is selected
  // afterthat call the right routine for that I/O type
  switch(mode) 
  {
    case SPID_PAUSE:
      outb_p(toggelVal, spid_base);
      printk(KERN_INFO "spid: toggle pin 9 (PAUSE)\n");
      break;

    case SPID_STRING:
#ifndef __alpha__  /* Alpha doesn't export insb: fall through  */
      outsb(spid_base, &toggelVal, 1);
      printk(KERN_INFO "spid: toggle pin 9 (STRING)\n");
      break;
#endif

    case SPID_DEFAULT:
      outb(toggelVal, spid_base);
      printk(KERN_INFO "spid: toggle pin 9 (DEFAULT)\n");        
      break;

    default: /* no more modes defined by now */
      retval = -EINVAL;
      printk(KERN_INFO "spid: write returns: -EINVAL\n");
      break;
  }
  return retval;
}

// This is the method, which will be called on select (in MAC/GCF) or poll in userspace
unsigned int spid_poll(struct file* filp, poll_table* wait)
{    
  unsigned int mask = 0;
  
  printk(KERN_DEBUG "spid: poll\n");
  if (spid_interrupt > 0) mask |= POLLIN | POLLRDNORM; // readable
  return mask;
}

#ifdef	CONFIG_NTP_PPS_SPID
/* Return allowed mode bits for given pps struct, file's mode, and user.
 * Bits set in `*obligatory' must be set.  Returned bits may be set.
 */
static int pps_allowed_mode(const struct file * file, mode_t fmode,
			    int *obligatory)
{
        const struct pps * pps = (struct pps*)file->private_data;
	int 			cap = pps->state.cap;

	cap &= ~PPS_CANWAIT;				/* always RO */
	*obligatory = PPS_TSFMT_TSPEC;		/* only supported format */
	if ((fmode & FMODE_WRITE) == 0) {	/* read-only mode */
		cap = *obligatory = pps->state.parm.mode;
	} else if (!capable(CAP_SYS_TIME)) {	/* may not manipulate time */
		int	fixed_bits;
		int	active_flags = pps->state.parm.mode;

		if (pps_kc_hardpps_dev == file) {
			fixed_bits = PPS_OFFSETASSERT|PPS_OFFSETCLEAR;
			fixed_bits &= active_flags;
			*obligatory |= fixed_bits;
		}
	}
	return cap;
}
#endif

// This method will be called on ioctl in userspace
// NOTE: Only FIONREAD will be serviced to find out there is an interrupt or not
int spid_ioctl(struct inode* inode, struct file* filp, unsigned int cmd, unsigned long arg)
{    
#ifdef	CONFIG_NTP_PPS_SPID
  int error = 0;
#endif

  switch (cmd)  
  {
    case FIONREAD:
    {
      printk(KERN_DEBUG "spid: ioctl\n");
      unsigned int* bytesToRead = (unsigned int* ) arg;
      *bytesToRead = (spid_interrupt > 0 ? 1 : 0);
      return spid_interrupt;
    }
    case SPID_IOCHARDREST:
    {
      while(MOD_IN_USE)
        MOD_DEC_USE_COUNT;
      MOD_INC_USE_COUNT;
      return 0;
    }

#ifdef	CONFIG_NTP_PPS_SPID
#ifdef DEBUG_NTP_PPS
#if 0
#define RESTORE_FLAGS_AND_RETURN	\
	do { \
		printk(KERN_INFO "ioctl() returns %d\n", error);\
		restore_flags(flags); return error;\
	} while (0);
#else
#define RESTORE_FLAGS_AND_RETURN \
        do { return error; } while (0)
#endif
#else
#define RESTORE_FLAGS_AND_RETURN	\
	do { /*restore_flags(flags);*/ return error; } while (0);
#endif

    case PPS_IOC_CREATE:
    {
      /* initialize the tty data struct */
      struct pps *pps = filp->private_data;

      /*save_flags(flags); cli();*/
#ifdef DEBUG_NTP_PPS
      printk(KERN_INFO
	     "PPS_IOC_CREATE: file/data = %p/%p\n",
	     filp, pps);
#endif /* DEBUG_NTP_PPS */
      if (pps != NULL) {
#ifdef DEBUG_NTP_PPS
	printk(KERN_INFO
	       "PPS_IOC_CREATE: magic = 0x%x\n",
	       pps->magic);
#endif /* DEBUG_NTP_PPS */
	/* share the handle if valid, otherwise fail */
	if (pps->magic != PPSCLOCK_MAGIC)
	  error = -EBADF;
	RESTORE_FLAGS_AND_RETURN;
      }
      if ((filp->f_mode & FMODE_WRITE) == 0) {
	error = -EBADF;
	RESTORE_FLAGS_AND_RETURN;
      }
      if ((pps = (struct pps *) kmalloc(sizeof(struct pps),
					GFP_KERNEL)) == NULL)
      {
	printk(KERN_ERR
	       "PPS_IOC_CREATE: kmalloc failed\n");
	error = -ENOMEM;
	RESTORE_FLAGS_AND_RETURN;
      }
      /* clear all parameters */
      memset(pps, 0, sizeof(struct pps));
      pps->magic = PPSCLOCK_MAGIC;
      pps->state.parm.api_version = PPS_API_VERS_1;
      pps->state.parm.mode = PPS_TSFMT_TSPEC;
      pps->state.cap = (PPS_CAPTUREASSERT|
			PPS_CAPTURECLEAR|
			PPS_OFFSETASSERT|
			PPS_OFFSETCLEAR|
			PPS_ECHOASSERT|
			PPS_ECHOCLEAR|
			PPS_CANWAIT|
			PPS_TSFMT_TSPEC);
      init_waitqueue_head(&pps->state.ewait);
      filp->private_data = (void*)pps;
      /*tty->termios->c_line = N_PPSCLOCK;*/
#ifdef DEBUG_NTP_PPS
      printk(KERN_INFO
	     "PPS_IOC_CREATE: new pps at %p, magic 0x%0x\n",
	     pps, pps->magic);
#endif /* DEBUG_NTP_PPS */
      error = 0;
      RESTORE_FLAGS_AND_RETURN;
    }

    case PPS_IOC_DESTROY:
    {
      /* draft 03 says the settings are unaffected. */
      struct pps *pps;

      /*save_flags(flags); cli();*/
      if ((filp->f_mode & FMODE_WRITE) == 0) {
	error = -EBADF;
	RESTORE_FLAGS_AND_RETURN;
      }
      pps = filp->private_data;
#ifdef DEBUG_NTP_PPS
      printk(KERN_INFO
	     "PPS_IOC_DESTROY: file/data = %p/%p\n",
	     filp, pps);
#endif /* DEBUG_NTP_PPS */
      if (pps == NULL || pps->magic != PPSCLOCK_MAGIC) {
	error = -EOPNOTSUPP;
	RESTORE_FLAGS_AND_RETURN;
      }
      if (waitqueue_active(&pps->state.ewait)) {
#ifdef DEBUG_NTP_PPS
	printk(KERN_INFO
	       "PPS_IOC_DESTROY: wait queue busy\n");
#endif /* DEBUG_NTP_PPS */
	error = -EBUSY;
	RESTORE_FLAGS_AND_RETURN;
      }
      error = 0;
      RESTORE_FLAGS_AND_RETURN;
    }

    case PPS_IOC_FETCH:
    {
      struct pps *pps = (struct pps*)filp->private_data;
      struct pps_fetch_args parms;
      long timeout;

      /*save_flags(flags); cli();*/
#ifdef DEBUG_NTP_PPS
      printk(KERN_INFO
	     "PPS_IOC_FETCH: file/pps = %p/%p\n",
	     filp, pps);
#endif /* DEBUG_NTP_PPS */
      if (pps == NULL || pps->magic != PPSCLOCK_MAGIC) {
	error = -EOPNOTSUPP;
	RESTORE_FLAGS_AND_RETURN;
      }
      if (copy_from_user(&parms,
			 (struct pps_fetch_args *) arg,
			 sizeof(struct pps_fetch_args))
	  != 0) {
	error = -EFAULT;
	RESTORE_FLAGS_AND_RETURN;
      }
      if (parms.tsformat != PPS_TSFMT_TSPEC) {
	error = -EINVAL;
	RESTORE_FLAGS_AND_RETURN;
      }
      timeout = HZ * parms.timeout.tv_sec;
      timeout += parms.timeout.tv_nsec / (1000000000 / HZ);
      if (timeout != 0) {
	/*restore_flags(flags);*/
	if (parms.timeout.tv_sec == -1)
	  interruptible_sleep_on(&pps->state.ewait);
	else {
	  timeout = interruptible_sleep_on_timeout(
	    &pps->state.ewait,
	    timeout);
	  if (timeout <= 0) {
	    error = -ETIMEDOUT;
	    return error;
	    /* flags already restored */
	  }
	}
	/*save_flags(flags); cli();*/
	pps = (struct pps*)filp->private_data;
	if (pps == NULL || pps->magic != PPSCLOCK_MAGIC)
	{
#ifdef DEBUG_NTP_PPS
	  printk(KERN_INFO "PPS_IOC_FETCH: "
		 "file %p lacks pps\n",
		 filp);
#endif /* DEBUG_NTP_PPS */
	  error = -EOPNOTSUPP;
	  RESTORE_FLAGS_AND_RETURN;
	}
	if (signal_pending(current)) {
	  error = -EINTR;
	  RESTORE_FLAGS_AND_RETURN;
	}
      }
      parms.pps_info_buf = pps->state.info;
      if (copy_to_user((struct pps_fetch_args *) arg,
		       &parms,
		       sizeof(struct pps_fetch_args)) != 0)
	error = -EFAULT;
      RESTORE_FLAGS_AND_RETURN;
    }

    case PPS_IOC_SETPARMS:
    {
      struct pps *pps = (struct pps*)filp->private_data;
      struct pps_params parms;
      int may_bits, must_bits;

      /*save_flags(flags); cli();*/
      if ((filp->f_mode & FMODE_WRITE) == 0) {
	error = -EBADF;
	RESTORE_FLAGS_AND_RETURN;
      }
#ifdef DEBUG_NTP_PPS
      printk(KERN_INFO
	     "PPS_IOC_SETPARAMS: file/pps = %p/%p\n",
	     filp, pps);
#endif /* DEBUG_NTP_PPS */
      if (pps == NULL || pps->magic != PPSCLOCK_MAGIC) {
	error = -EOPNOTSUPP;
	RESTORE_FLAGS_AND_RETURN;
      }
      if (copy_from_user(&parms,
			 (struct pps_params *) arg,
			 sizeof(struct pps_params)) != 0) {
	error = -EFAULT;
	RESTORE_FLAGS_AND_RETURN;
      }
#ifdef DEBUG_NTP_PPS
      printk(KERN_INFO "PPS_IOC_SETPARAMS: "
	     "vers/mode(cap) = %#x/%#x(%#x)\n",
	     parms.api_version, parms.mode, pps->state.cap);
#endif /* DEBUG_NTP_PPS */
      if (parms.api_version != PPS_API_VERS_1) {
	error = -EINVAL;
	RESTORE_FLAGS_AND_RETURN;
      }
      if ((parms.mode & ~pps->state.cap) != 0 ) {
	error = -EINVAL;
	RESTORE_FLAGS_AND_RETURN;
      }
      if ((parms.mode &
	   (PPS_TSFMT_TSPEC|PPS_TSFMT_NTPFP)) == 0 ) {
	/* section 3.3 of RFC 2783 interpreted */
	parms.mode |= PPS_TSFMT_TSPEC;
      }
      may_bits = pps_allowed_mode(filp, filp->f_mode,
				  &must_bits);
      if ((parms.mode & must_bits) != must_bits ||
	  (parms.mode & ~may_bits) != 0) {
	error = -EPERM;
	RESTORE_FLAGS_AND_RETURN;
      }
      if (capable(CAP_SYS_TIME)) {
	/* allow setting offsets */
	pps->state.parm = parms;
      } else {
	pps_params_t	*ppspp = &pps->state.parm;

	ppspp->api_version = parms.api_version;
	ppspp->mode = parms.mode;
	/* not offsets! */
      }
#if 0
      if (parms.mode & (PPS_CAPTUREASSERT|PPS_CAPTURECLEAR)) {
	/* enable interrupts */
	info->IER |= UART_IER_MSI;
	info->flags |= ASYNC_LOW_LATENCY;
	if (info->flags & ASYNC_INITIALIZED) {
	  serial_out(info, UART_IER, info->IER);
#ifdef DEBUG_NTP_PPS
	  printk(KERN_INFO
		 "PPS_IOC_SETPARAMS: IER:%02x\n",
		 info->IER);
#endif /* DEBUG_NTP_PPS */
	}
      }
#endif
      RESTORE_FLAGS_AND_RETURN;
    }

    case PPS_IOC_GETPARMS:
    {
      const struct pps *pps = (struct pps*)filp->private_data;

      /*save_flags(flags); cli();*/
#ifdef DEBUG_NTP_PPS
      printk(KERN_INFO
	     "PPS_IOC_GETPARAMS: file/pps = %p/%p\n",
	     filp, pps);
#endif /* DEBUG_NTP_PPS */
      if (pps == NULL || pps->magic != PPSCLOCK_MAGIC) {
	error = -EOPNOTSUPP;
	RESTORE_FLAGS_AND_RETURN;
      }
      if (copy_to_user((pps_params_t *) arg,
		       &(pps->state.parm),
		       sizeof(struct pps_params)) != 0) {
	error = -EFAULT;
	RESTORE_FLAGS_AND_RETURN;
      }
    }

    case PPS_IOC_GETCAP:
    {
      const struct pps *pps = (struct pps*)filp->private_data;
      int may_bits, must_bits;

      /*save_flags(flags); cli();*/
#ifdef DEBUG_NTP_PPS
      printk(KERN_INFO
	     "PPS_IOC_GETCAP: file/pps = %p/%p\n", filp, pps);
#endif /* DEBUG_NTP_PPS */
      if (pps == NULL || pps->magic != PPSCLOCK_MAGIC) {
	error = -EOPNOTSUPP;
	RESTORE_FLAGS_AND_RETURN;
      }
      may_bits = pps_allowed_mode(filp, filp->f_mode,
				  &must_bits);
      if (copy_to_user((int *) arg, &may_bits,
		       sizeof(int)) != 0)
	error = -EFAULT;
      RESTORE_FLAGS_AND_RETURN;
    }

    case PPS_IOC_KC_BIND:
    {
      struct pps *pps = filp->private_data;
      struct pps_bind_args parms;

      /*save_flags(flags); cli();*/
      if ((filp->f_mode & FMODE_WRITE) == 0) {
	error = -EBADF;
	RESTORE_FLAGS_AND_RETURN;
      }
#ifdef DEBUG_NTP_PPS
      printk(KERN_INFO
	     "PPS_IOC_KC_BIND: file/pps = %p/%p\n", filp, pps);
      printk(KERN_INFO
	     "PPS_IOC_KC_BIND: current dev/mode = %p/0x%x\n",
	     pps_kc_hardpps_dev, pps_kc_hardpps_mode);
			
#endif /* DEBUG_NTP_PPS */
      if (pps == NULL || pps->magic != PPSCLOCK_MAGIC)
      {
	error = -EOPNOTSUPP;
	RESTORE_FLAGS_AND_RETURN;
      }
      if (copy_from_user(&parms,
			 (struct pps_bind_args *) arg,
			 sizeof(struct pps_bind_args)) != 0)
      {
	error = -EFAULT;
	RESTORE_FLAGS_AND_RETURN;
      }
      /* generic parameter validation */
      if (parms.tsformat != PPS_TSFMT_TSPEC ||
	  (parms.edge & ~PPS_CAPTUREBOTH) != 0 ||
#if 0
	  parms.consumer < PPS_KC_HARDPPS ||
	  parms.consumer > PPS_KC_HARDPPS_FLL ||
#endif
	  parms.consumer != PPS_KC_HARDPPS) {
	error = -EINVAL;
	RESTORE_FLAGS_AND_RETURN;
      }
      /* permission check */
      if (!capable(CAP_SYS_TIME)) {
	error = -EPERM;
	RESTORE_FLAGS_AND_RETURN;
      }
      /* detailed parameter check */
      if (parms.edge == 0) {
	if (pps_kc_hardpps_dev == NULL ||
	    pps_kc_hardpps_dev == (void *) filp) {
	  pps_kc_hardpps_mode = parms.edge;
	  pps_kc_hardpps_dev = NULL;
#ifdef DEBUG_NTP_PPS
	  printk(KERN_INFO "PPS_IOC_KC_BIND: "
		 "unbound kernel consumer\n");
#endif /* DEBUG_NTP_PPS */
	} else {	/* another consumer bound */
	  error = -EINVAL;
	  RESTORE_FLAGS_AND_RETURN;
	}
      } else {
	if (pps_kc_hardpps_dev == (void *) filp ||
	    pps_kc_hardpps_dev == NULL) {
	  pps_kc_hardpps_mode = parms.edge;
	  pps_kc_hardpps_dev = filp;
#ifdef DEBUG_NTP_PPS
	  printk(KERN_INFO "PPS_IOC_KC_BIND: "
		 "new kernel consumer: dev=%p, "
		 "edge=0x%x\n",
		 pps_kc_hardpps_dev,
		 pps_kc_hardpps_mode);
#endif /* DEBUG_NTP_PPS */
	} else {
	  error = -EINVAL;
	  RESTORE_FLAGS_AND_RETURN;
	}
      }
      RESTORE_FLAGS_AND_RETURN;
    }
#endif /* CONFIG_NTP_PPS_SPID */
  }
  return -1;
}
#endif /* __sparc__ */
