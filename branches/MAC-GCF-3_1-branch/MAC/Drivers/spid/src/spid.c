mal file.
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
