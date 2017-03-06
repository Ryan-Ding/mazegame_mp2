/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define DP 0x10
#define GET_START_BIT 0x01
#define GET_A_BIT 0x02 
#define GET_B_BIT 0x04 
#define GET_C_BIT 0x08

#define GET_UP_BIT 0x01
#define GET_LEFT_BIT 0x02 
#define GET_DOWN_BIT 0x04 
#define GET_RIGHT_BIT 0x08

#define GET_LAST_FOUR 0xF	
 

char led_buffer[6] = {MTCP_LED_SET, 0x0f, 0, 0, 0, 0};

char search_table[10] = {
	0xe7,   //0
	0x06,	//1
	0xcb,	//2
	0x8f,	//3
	0x2e,	//4
	0xad,	//5
	0xed,	//6
	0x86,	//7
	0xef,	//8
	0xaf,	//9
};

int ack=1;


#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

static spinlock_t tuxctl_my_lock = SPIN_LOCK_UNLOCKED;
//static spinlock_t tuxctl_your_lock = SPIN_LOCK_UNLOCKED;
unsigned long flags;
//unsigned long flags1;

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;
    char init_buffer;

    char temp1;
    char temp2;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

    switch(a){
    	case MTCP_RESET:

		init_buffer = MTCP_BIOC_ON;
		tuxctl_ldisc_put(tty, &init_buffer, 1);

		init_buffer = MTCP_LED_USR;
		tuxctl_ldisc_put(tty, &init_buffer, 1);

		if(ack ==1)
		{
		tuxctl_ldisc_put(tty, led_buffer, 6);
		ack =0;
		}


		break;

    	case MTCP_BIOC_EVENT:

    	spin_lock_irqsave(&tuxctl_my_lock, flags);

    	button = 0x00000000;

    	//right left down up c b a start

    	temp1 = b & GET_LAST_FOUR;
	    temp2 = c & GET_LAST_FOUR;

    	button = temp1 ^ GET_LAST_FOUR;
    	button = button | (((temp2 ^ GET_LAST_FOUR) & GET_UP_BIT)<<4);
    	button = button | (((temp2 ^ GET_LAST_FOUR) & GET_DOWN_BIT)<<3);
    	button = button | (((temp2 ^ GET_LAST_FOUR) & GET_LEFT_BIT)<<5);
    	button = button | (((temp2 ^ GET_LAST_FOUR) & GET_RIGHT_BIT)<<4);
    	spin_unlock_irqrestore(&tuxctl_my_lock,flags);

		break;

		case MTCP_ACK:
		ack = 1;



    	default:
    	return;

    }

    /*printk("packet : %x %x %x\n", a, b, c); */
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
	char init_buffer;
	//int i;
    switch (cmd) {
	case TUX_INIT:
		//initialize the tux controllers

		init_buffer = MTCP_BIOC_ON;
		tuxctl_ldisc_put(tty, &init_buffer, 1);

		init_buffer = MTCP_LED_USR;
		tuxctl_ldisc_put(tty, &init_buffer, 1);

		return 0;

	case TUX_BUTTONS:
		if((unsigned long*)arg == NULL)
		{
			return -EINVAL;
		}
		//printk("%lu",button);
	spin_lock_irqsave(&tuxctl_my_lock,flags);
		//return copy_to_user(arg, &button, sizeof(unsigned long) );
		return copy_to_user((unsigned long*)arg, &button, sizeof(unsigned long));
	spin_unlock_irqrestore(&tuxctl_my_lock,flags);

		//might need modify the type of arg

	case TUX_SET_LED:
		//int led_counter = 1;
		/*for(i=0; i<6; i++)
		{
			led_buffer[i]=0x00;
		}*/
	//spin_lock_irqsave(&tuxctl_your_lock,flags1);
		if(arg & 0x00010000)
		{
			//led_counter++;
			led_buffer[2] = search_table[arg & 0x0000000f];
			if(arg & 0x01000000)
			{
				led_buffer[2]+=DP;
			}

		}
		if(arg & 0x00020000)
		{
			//led_counter++;
			led_buffer[3] = search_table[(arg & 0x000000f0) >> 4];
			if(arg & 0x02000000)
			{
				led_buffer[3]+=DP;
			}
		}
		if(arg & 0x00040000)
		{
			//led_counter++;
			led_buffer[4] = search_table[(arg & 0x00000f00) >> 8];
			if(arg & 0x04000000)
			{
				led_buffer[4]+=DP;
			}
		}
		if(arg & 0x00080000)
		{
			//led_counter++;
			led_buffer[5] = search_table[(arg & 0x0000f000) >> 12];
			if(arg & 0x08000000)
			{
				led_buffer[5]+=DP;
			}
		}
		//spin_unlock_irqrestore(&tuxctl_your_lock,flags1);


		if(ack==1)
		{
		tuxctl_ldisc_put(tty, led_buffer, 6);
		ack=0;
		}


		return 0;

	case TUX_LED_ACK:
	case TUX_LED_REQUEST:
	case TUX_READ_LED:
	default:
	    return -EINVAL;
    }
}

