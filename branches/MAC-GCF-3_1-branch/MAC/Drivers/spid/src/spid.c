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

#ifdef MODULE
MODULE_AUTHOR("Tilman Muller");
MODULE_DESCRIPTION("Simple Parallel (Port) Interrupt Driver for MAC");
MODULE_LICENSE("GPL");
#endif

int spid_major = 0;

#ifdef  __alpha__
int spid_base = 0x3bc;   /* Port address Parallel Port */ 
#else
int spid_base = 0x378;   /* Port address Parallel Port */
#endif

unsigned int spid_interrupt = 0;
int spid_irq = -1;

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

    if (spid_irq < 0)
    {
      printk(KERN_INFO "spid: probe failed %i times, giving up\n", count);
    }
    else
    {
      printk(KERN_INFO "spid: Found and use irq %d\n", spid_irq);
      request_irq(spid_irq, spid_isr, SA_INTERRUPT, "spid interrupt service routine", NULL);
      outb(0x10, spid_base + 2); // enables reporting the interrupt
    }
  }
  return 0;
}

// This is the interrupt service routine, which is passed in the request_irq method 
// in spid_open, it will invoked if the value of pin 10 in the parallel port changed 
// from 0 to 1 (interrupt)

void spid_isr(int irq, void* dev_id, struct pt_regs* regs)
{
  printk(KERN_INFO "spid: Interrupt\n");
  int value;
  outb_p(0x00, spid_base);
  // in some case it seams this is needed to force a down edge of the pin 10
  value = inb(spid_base + 1); 
  if (MOD_IN_USE) spid_interrupt++;
}

// Closing a Device because device is treated like a normal file.
// Call to MOD_DEC_USE_COUNT, because kernel administrates the number of
// use of the device. Therefor count has to be 0 to release a Module.

int spid_release (struct inode* inode, struct file* filp)
{
  MOD_DEC_USE_COUNT;
  if (!MOD_IN_USE)
  {    
    if (spid_irq > 0)
    {
      printk(KERN_INFO "spid: Release irq %d\n", spid_irq);
      free_irq(spid_irq, NULL);
      spid_irq = -1;
      spid_interrupt = 0;
    }
    outb(0x00, spid_base + 2); // disable reporting the interrupt
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

// This method will be called on ioctl in userspace
// NOTE: Only FIONREAD will be serviced to find out there is an interrupt or not
int spid_ioctl(struct inode* inode, struct file* filp, unsigned int cmd, unsigned long arg)
{    
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
  }
  return -1;
}
#endif /* __sparc__ */
