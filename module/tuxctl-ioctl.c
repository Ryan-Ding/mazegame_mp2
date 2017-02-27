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


#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

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
    switch (cmd) {
	case TUX_INIT:
		//tuxctl_send_back();
		return 0;

	case TUX_BUTTONS:
		if(arg == NULL)
		{
			return -EINVAL;
		}

	case TUX_SET_LED:
		//int led_counter = 1;
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
			led_buffer[3] = search_table[arg & 0x000000f0];
			if(arg & 0x02000000)
			{
				led_buffer[3]+=DP;
			}
		}
		if(arg & 0x00040000)
		{
			//led_counter++;
			led_buffer[4] = search_table[arg & 0x00000f00];
			if(arg & 0x04000000)
			{
				led_buffer[4]+=DP;
			}
		}
		if(arg & 0x00080000)
		{
			//led_counter++;
			led_buffer[5] = search_table[arg & 0x0000f000];
			if(arg & 0x08000000)
			{
				led_buffer[5]+=DP;
			}
		}

		tuxctl_ldisc_put(tty, led_buffer, 6);




	case TUX_LED_ACK:
	case TUX_LED_REQUEST:
	case TUX_READ_LED:
	default:
	    return -EINVAL;
    }
}

