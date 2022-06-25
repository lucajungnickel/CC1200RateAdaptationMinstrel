// Standard header files

#include <stdio.h>

#include <stdlib.h>

#include <sys/mman.h>

#include <fcntl.h>

#include <errno.h>

#include <signal.h>

#include <prussdrv.h>

#include <pruss_intc_mapping.h>

#include <unistd.h>

#include <string.h>

#include <time.h>

#include <sys/time.h>



#include "SPIv1.h"



#include "SPIv1_bin.h"



/******************************************************************************

* Local Macro Declarations                                                    * 

******************************************************************************/

#define PRU_NUM 	0

#define OFFSET_SHAREDRAM 2048		//equivalent with 0x00002000



#define PRUSS0_SHARED_DATARAM    4

const char *STATUS_CC1200[8] = {

    "IDLE",

    "RX",

    "TX",

    "FSTXON",

    "CALIBRATE",

    "SETTLING",

    "RX FIFO ERROR",

    "TX FIFO ERROR",

};

/******************************************************************************

* Global variable Declarations                                                * 

******************************************************************************/

static unsigned int *sharedMem_int;

static unsigned int status_CC1200 = 0xFFFFFFFF; //Undefined



/** 

 * Function to call on SIGINT

 * Thus ctrl+c kills off the PRU in any case

 **/

void sigint_handler(int action){

  prussdrv_pru_disable(PRU_NUM);

  prussdrv_exit();

}



int cc1200_reg_read(int adr, int *val)

{



    int value;



    if(adr & EXT_ADR){

      sharedMem_int[OFFSET_SHAREDRAM + 0] = (0x2F | 0x80);

      sharedMem_int[OFFSET_SHAREDRAM + 1] = adr & 0xFF;

      sharedMem_int[OFFSET_SHAREDRAM + 2] = 0x00;

      sharedMem_int[OFFSET_SHAREDRAM + 3] = 3;

    }else{

      sharedMem_int[OFFSET_SHAREDRAM + 0] = (adr | 0x80) & 0xFF;

      sharedMem_int[OFFSET_SHAREDRAM + 1] = 0x00;

      sharedMem_int[OFFSET_SHAREDRAM + 3] = 2;

    }

    prussdrv_pru_send_event(ARM_PRU0_INTERRUPT);

    prussdrv_pru_wait_event(PRU_EVTOUT_0);

    prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

    

    status_CC1200 = sharedMem_int[OFFSET_SHAREDRAM+5];

    if(adr & EXT_ADR)

      value = sharedMem_int[OFFSET_SHAREDRAM+7] &0xFF;

    else

      value = sharedMem_int[OFFSET_SHAREDRAM+6] & 0xFF;



    if (val!=NULL) *val=value;

    return value;

}



int cc1200_reg_write(int adr, int val)

{

    if(adr & EXT_ADR){

      sharedMem_int[OFFSET_SHAREDRAM + 0] = (0x2F);

      sharedMem_int[OFFSET_SHAREDRAM + 1] = adr &0xFF;

      sharedMem_int[OFFSET_SHAREDRAM + 2] = val & 0xFF;

      sharedMem_int[OFFSET_SHAREDRAM + 3] = 3;

    }else{

      sharedMem_int[OFFSET_SHAREDRAM + 0] = adr & 0xFF;

      sharedMem_int[OFFSET_SHAREDRAM + 1] = val;

      sharedMem_int[OFFSET_SHAREDRAM + 3] = 2;

    }

    prussdrv_pru_send_event(ARM_PRU0_INTERRUPT);

   

    prussdrv_pru_wait_event(PRU_EVTOUT_0);

    prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);



    status_CC1200 = sharedMem_int[OFFSET_SHAREDRAM+5] & 0xFF;

    return val;

}



/**

 * Command Strobe to String

 * @param cmd the command strobe

**/

char* cc1200_print_cmd (int cmd) {

	

	switch (cmd) {

		case SRES   : return "SRES"   ; break;

		case SFSTXON: return "SFSTXON"; break;

		case SXOFF  : return "SXOFF"  ; break;

		case SCAL   : return "SCAL"   ; break;

		case SRX    : return "SRX"    ; break;

		case STX    : return "STX"    ; break;

		case SIDLE  : return "SIDLE"  ; break;

		case SAFC   : return "SAFC"   ; break;

		case SWOR   : return "SWOR"   ; break;

		case SPWD   : return "SPWD"   ; break;

		case SFRX   : return "SFRX"   ; break;

		case SFTX   : return "SFTX"   ; break;

		case SWORRST: return "SWORRST"; break;

		case SNOP   : return "SNOP"   ; break;

		default: return "ERROR: UNKNOWN STROBE"; break;

	}

}



/**

 * Command strobe to CC1200

 * @param cmd the value of command to write

 **/

int cc1200_cmd(int cmd)

{

    sharedMem_int[OFFSET_SHAREDRAM + 0] = cmd;

    sharedMem_int[OFFSET_SHAREDRAM + 3] = 1;



    prussdrv_pru_send_event(ARM_PRU0_INTERRUPT);

    

    prussdrv_pru_wait_event(PRU_EVTOUT_0);

    prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

    

    status_CC1200 = sharedMem_int[OFFSET_SHAREDRAM+5];

    return cmd;

}



/**

 * last known CC1200 status

 *

 **/

int get_status_cc1200()

{

    //return status_CC1200 & 0xF;

    return (status_CC1200 >> 4) & 0x7;

}



char* print_status (CC1200_STATES s) {



	switch (s) {

	    case 0: return "IDLE          "; break;

        case 1: return "RX            "; break;

        case 2: return "TX            "; break;

        case 3: return "FSTXON        "; break;

        case 4: return "CALLIBRATE    "; break;

        case 5: return "SETTLING      "; break;

        case 6: return "RX_FIFO_ERROR "; break;

        case 7: return "TX_FIFO_ERROR "; break;

        default : return "UNKNOWN     "; break;

	}

}



const char * get_status_cc1200_str()

{

    return STATUS_CC1200[(status_CC1200 >> 4) & 0x7];

}

/**

 * Initialize SPI-CC1200 Interface

 **/

int spi_init()

{

    int ret;

    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    prussdrv_init();

    signal(SIGINT, sigint_handler);

    ret = prussdrv_open(PRU_EVTOUT_0);

    if (ret){

	printf("\tERROR: prussdrv_open open failed\n");

	return (ret);

    }



    prussdrv_pruintc_init(&pruss_intc_initdata);

    prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, (void *) &sharedMem_int);





    //sharedMem_int[OFFSET_SHAREDRAM + 2] = freqArray[frequency][1];

 

    

    /* Executing PRU. */

    //prussdrv_exec_program (PRU_NUM, "ADCCollector_bin.h");

    prussdrv_pru_write_memory(PRUSS0_PRU0_IRAM, 0, PRUcode, sizeof(PRUcode));

    prussdrv_pru_enable(0);

    return 0;

}



/**

 * Shut down SPI-CC1200 Interface

 * Needs to be called before exiting program

 * Disable PRU

 **/

void spi_shutdown()

{

    /* Disable PRU*/

    prussdrv_pru_disable(PRU_NUM);

    prussdrv_exit();

}


