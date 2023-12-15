/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the PSoC64 Basic TF-M Application
*              for ModusToolbox.
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2023, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

#include "cybsp.h"
#include "cy_retarget_io.h"
#include "cy_wdt.h"

#include "FreeRTOS.h"
#include "task.h"

#include "tfm_multi_core_api.h"
#include "tfm_ns_interface.h"
#include "tfm_ns_mailbox.h"
#include "psa/protected_storage.h"
#include "psa/crypto.h"

/*****************************************************************************
* Macros
*****************************************************************************/
/* The identifier for the protected storage (PS) data */
#define PS_UID 1U

/* Task stack size in words(32 bits) */
#define DEMO_TASK_STACK_SIZE   1024U

/* FreeRTOS Task Name */
#define DEMO_TASK_NAME         ("demo_task")

/* FreeRTOS task Priority */
#define DEMO_TASK_PRIORITY     4U

/* Buffer size for protected Storage */
#define PS_BUFF_SIZE           20U

/* Buffer size for AEAD operation */
#define CRYPTO_BUFF_SIZE       50U

/* Size of nonce in bytes used for AEAD operation */
#define NONCE_SIZE             ((size_t) 12)

/* Size of AES key used for AEAD operation */
#define AES_KEY_SIZE           ((size_t) 128)

/*****************************************************************************
* Global Variables
*****************************************************************************/
static struct ns_mailbox_queue_t ns_mailbox_queue;


/*****************************************************************************
* Function Name: tfm_ns_multi_core_boot
******************************************************************************
* Summary:
*  Sync cm0p and cm4 CPUs and initialize ns mailbox.
*
*
* Parameters:
*  None
*
* Return:
*  None
*
*****************************************************************************/
static void tfm_ns_multi_core_boot(void)
{
    int32_t ret;

    if (tfm_ns_wait_for_s_cpu_ready()) {
        /* Error sync'ing with secure core */
        /* Avoid undefined behavior after multi-core sync-up failed */
        for (;;) {
        }
    }

    ret = tfm_ns_mailbox_init(&ns_mailbox_queue);
    if (ret != MAILBOX_SUCCESS) {
        /* Non-secure mailbox initialization failed. */
        /* Avoid undefined behavior after NS mailbox initialization failed */
        for (;;) {
        }
    }
}

/*****************************************************************************
* Function Name: tfm_demo_task
******************************************************************************
* Summary:
*  Main application thread. The thread performs two tasks using PSA APIs
*  1. Store data in Protected Storage (PS) and retrieve it.
*  2. Encrypt data with AEAD-CCM and decrypt it.
*
* Parameters:
*  arg[in]  : not used
*
* Return:
*  None
*
*****************************************************************************/
static void tfm_demo_task(void* arg)
{
    for(;;)
    {
        (void)arg;
        char set_data[] = "Hello World";
        char get_data[PS_BUFF_SIZE] = {0};
        size_t get_len = 0;
        psa_status_t result = 0;
        uint8_t input_data[] = {0xA, 0xB, 0xC, 0xD, 0xE, 0xF};
        uint8_t additional_data[] = {0x01, 0x02};
        uint8_t enc_data[CRYPTO_BUFF_SIZE] = {0};
        uint8_t dec_data[CRYPTO_BUFF_SIZE] = {0};
        uint8_t nonce[NONCE_SIZE] = {0};
        size_t enc_data_len = 0;
        size_t dec_data_len = 0;
        psa_key_id_t key_id = 0;

        /* The below code demonstrates TF-M Protected Storage (PS)
         * service. Data is stored in PS and retrieved using PSA APIs */

        printf("\r\n");
        printf("*** TF-M Protected Storage (PS) service ***\r\n");
        printf("\r\n");
        printf("Protected Storage data: %s\r\n", set_data);
        printf("Storing data in Protected Storage...\r\n");

        result = psa_ps_set(PS_UID, sizeof(set_data), set_data, PSA_STORAGE_FLAG_NONE);
        if(result != PSA_SUCCESS)
        {
            printf("Failed to store data in protected Storage");
            CY_ASSERT(0);
        }

        printf("Retrieving data from Protected Storage...\r\n");

        result = psa_ps_get(PS_UID, 0, sizeof(set_data), get_data, &get_len);
        if(result != PSA_SUCCESS)
        {
            printf("Failed to retrieve data from protected Storage");
            CY_ASSERT(0);
        }

        printf("Retrieved data: %s\r\n", get_data);
        /* End of PS code */

        /* The below code demonstrates TF-M Cryptography service using PSA APIs.
         * The below code encrypts data with AEAD scheme, decrypts it and
         * prints it on the console. */

        /* Attribute structure of key */
        psa_key_attributes_t key_attributes = PSA_KEY_ATTRIBUTES_INIT;

        /* Set key attributes */
        psa_set_key_usage_flags(&key_attributes, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
        psa_set_key_algorithm(&key_attributes, PSA_ALG_CCM);
        psa_set_key_lifetime(&key_attributes, PSA_KEY_LIFETIME_VOLATILE);
        psa_set_key_type(&key_attributes, PSA_KEY_TYPE_AES);
        psa_set_key_bits(&key_attributes, AES_KEY_SIZE);

        /* Generate key for AEAD CCM encryption/decryption */
        result = psa_generate_key(&key_attributes, &key_id);
        if(result != PSA_SUCCESS)
        {
            printf("Failed to generate PSA key");
            CY_ASSERT(0);
        }

        /* Generate a random number for nonce */
        result = psa_generate_random(nonce, sizeof(nonce));
        if(result != PSA_SUCCESS)
        {
            printf("Failed to generate random number");
            CY_ASSERT(0);
        }

        printf("\r\n");
        printf("*** TF-M Cryptography service ***\r\n");
        printf("\r\n");
        printf("Encryption data: ");
        for(int i = 0; i < sizeof(input_data); i++)
        {
            printf("%x ", input_data[i]);
        }
        printf("\r\n");
        
        printf("Encrypting data with AEAD CCM...\r\n");
        /* Encrypt data with AEAD single part cipher*/
        result = psa_aead_encrypt(key_id, PSA_ALG_CCM, nonce, sizeof(nonce), additional_data,
                                    sizeof(additional_data), input_data, sizeof(input_data), enc_data,
                                    sizeof(enc_data), &enc_data_len);
        if(result != PSA_SUCCESS)
        {
            printf("Failed to encrypt data with AEAD");
            CY_ASSERT(0);
        }

        printf("Decrypting data...\r\n");

        /* Decrypt data */
        result = psa_aead_decrypt(key_id, PSA_ALG_CCM, nonce, sizeof(nonce),
                                additional_data, sizeof(additional_data), enc_data, enc_data_len,
                                dec_data, sizeof(dec_data), &dec_data_len);
        if(result != PSA_SUCCESS)
        {
            printf("Failed to decrypt data");
            CY_ASSERT(0);
        }
        
        printf("Decrypted data: ");
        for(int i = 0; i < dec_data_len; i++)
        {
            printf("%x ", dec_data[i]);
        }
        printf("\r\n");
        
        /* End of Cryptography code */

        vTaskDelete(NULL);
    }
}

/*****************************************************************************
* Function Name: main
******************************************************************************
* Summary:
* This is the main function for CPU. It demonstrates basic services offered
* by TF-M.
*
* Parameters:
*  void
*
* Return:
*  int
*
*****************************************************************************/
int main(void)
{
    cy_rslt_t result;
    int retval;
    psa_status_t rslt;

    /* Unlock and disable WDT */
    Cy_WDT_Unlock();
    Cy_WDT_Disable();

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    if (CY_RSLT_SUCCESS != result)
    {
        printf("Failed to initialize the device and board peripherals");
        CY_ASSERT(0);
    }

    __enable_irq();

    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                 CY_RETARGET_IO_BAUDRATE);

    /* retarget-io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        printf("Failed to initialize retarget-io");
        CY_ASSERT(0);
    }

    /* Sync CM0p and CM4 cores. Initialize mailbox for SPE NSPE communication. */
    tfm_ns_multi_core_boot();

    /* Initialize TFM interface */
    tfm_ns_interface_init();

    /* Initialize crypto library to use PSA Crypto APIs */
    rslt = psa_crypto_init();
    if(rslt != PSA_SUCCESS)
    {
        printf("Failed to initialize crypto library");
        CY_ASSERT(0);
    }

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("****************** "
           "PSoC 64 MCU: Trusted Firmware-M Example Application"
           "****************** \r\n\n");

    /* Create a Task */
    retval = xTaskCreate(tfm_demo_task, DEMO_TASK_NAME, DEMO_TASK_STACK_SIZE, NULL, DEMO_TASK_PRIORITY, NULL);

    if(pdPASS == retval)
    {
        /* Start Scheduler */
        vTaskStartScheduler();
    }

    for (;;)
    {
    }
}
/* [] END OF FILE */
