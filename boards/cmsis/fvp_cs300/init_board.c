#include "RTE_Components.h"
#include CMSIS_device_header

#include "init_board.h"
#include "ethosu_driver.h"

#include <stdio.h>
#include <inttypes.h>
/* Ethos NPU driver instance. */
static struct ethosu_driver EthosDriver;

/*
  Ethos NPU interrupt handler.
*/
void arm_ethosu_npu_irq_handler(void)
{
    ethosu_irq_handler(&EthosDriver);
}

static void arm_ethosu_npu_irq_init(void)
{
    const IRQn_Type ethosu_irqnum = (IRQn_Type)ETHOS_U55_IRQn;

    /* Register the EthosU IRQ handler in our vector table.
     * Note, this handler comes from the EthosU driver */
    NVIC_SetVector(ethosu_irqnum, (uint32_t)arm_ethosu_npu_irq_handler);

    /* Enable the IRQ */
    NVIC_EnableIRQ(ethosu_irqnum);

}

/*
  Initializes the Ethos NPU driver.
*/
int32_t NpuInit(void)
{
    arm_ethosu_npu_irq_init();
    void *const ethos_base_addr = (void *) ETHOS_U55_APB_BASE_NS;

    /*  Initialize Ethos-U NPU driver. */
    if (ethosu_init(&EthosDriver,    /* Ethos-U device driver */
                    ethos_base_addr, /* Ethos-U base address  */
                    0,               /* Cache memory pointer  */
                    0,               /* Cache memory size     */
                    1,               /* Secure enable         */
                    1)               /* Privileged mode       */
    ) {
        /* Failed to initialize Arm Ethos-U driver */
        return 1;
    }

    printf("Ethos-U device initialised\n");

    /* Get Ethos-U version */
    struct ethosu_driver_version driver_version;
    struct ethosu_hw_info hw_info;

    ethosu_get_driver_version(&driver_version);
    ethosu_get_hw_info(&EthosDriver, &hw_info);

    printf("Ethos-U version info:\n");
    printf("\tArch:       v%" PRIu32 ".%" PRIu32 ".%" PRIu32 " \n",
         hw_info.version.arch_major_rev,
         hw_info.version.arch_minor_rev,
         hw_info.version.arch_patch_rev);
    printf("\tDriver:     v%d.%d.%d\n",
         driver_version.major,
         driver_version.minor,
         driver_version.patch);
    printf("\tMACs/cc:    %" PRIu32 "\n", (uint32_t)(1 << hw_info.cfg.macs_per_cc));
    printf("\tCmd stream: v%" PRIu32 "\n", hw_info.cfg.cmd_stream_version);


    NVIC_EnableIRQ(ETHOS_U55_IRQn);

    return 0;
}

void init_board(void)
{
    /* Initialize the board */
    NpuInit();

    /* Enable I-Cache */
    SCB_EnableICache();

    /* Enable D-Cache */
    SCB_EnableDCache();
}