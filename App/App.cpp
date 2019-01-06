/*
 * Copyright (C) 2011-2017 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


// App.cpp : Defines the entry point for the console application.

//This project is modified from https://github.com/01org/linux-sgx/tree/master/SampleCode/LocalAttestation 

#include <stdio.h>
#include <map>
#include "../Enclave1/Enclave1_u.h"
#include "../Enclave2/Enclave2_u.h"
#include "sgx_eid.h"
#include "sgx_urts.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#define UNUSED(val) (void)(val)
#define TCHAR   char
#define _T(str) str
#define scanf_s scanf
//======================================================================
// this Error handling code is taken and modified from:
// https://github.com/01org/linux-sgx/tree/master/SampleCode/Cxx11SGXDemo
typedef struct _sgx_errlist_t {
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
    {
        SGX_ERROR_UNEXPECTED,
        "SGX_ERROR_UNEXPECTED: Unexpected error occurred.",
        NULL
    },
    {
        SGX_ERROR_INVALID_PARAMETER,
        "SGX_ERROR_INVALID_PARAMETER: Invalid parameter.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_MEMORY,
        "SGX_ERROR_OUT_OF_MEMORY: Out of memory.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_LOST,
        "SGX_ERROR_ENCLAVE_LOST: Power transition occurred.",
        "Please refer to the sample \"PowerTransition\" for details."
    },
    {
        SGX_ERROR_INVALID_ENCLAVE,
        "Invalid enclave image.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ENCLAVE_ID,
        "SGX_ERROR_INVALID_ENCLAVE: Invalid enclave identification.",
        NULL
    },
    {
        SGX_ERROR_INVALID_SIGNATURE,
        "SGX_ERROR_INVALID_SIGNATURE: Invalid enclave signature.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_EPC,
        "SGX_ERROR_OUT_OF_EPC: Out of EPC memory.",
        NULL
    },
    {
        SGX_ERROR_NO_DEVICE,
        "SGX_ERROR_NO_DEVICE: Invalid SGX device.",
        "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."
    },
    {
        SGX_ERROR_MEMORY_MAP_CONFLICT,
        "SGX_ERROR_MEMORY_MAP_CONFLICT: Memory map conflicted.",
        NULL
    },
    {
        SGX_ERROR_INVALID_METADATA,
        "SGX_ERROR_INVALID_METADATA: Invalid enclave metadata.",
        NULL
    },
    {
        SGX_ERROR_DEVICE_BUSY,
        "SGX_ERROR_DEVICE_BUSY: SGX device was busy.",
        NULL
    },
    {
        SGX_ERROR_INVALID_VERSION,
        "SGX_ERROR_INVALID_VERSION: Enclave version was invalid.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ATTRIBUTE,
        "SGX_ERROR_INVALID_ATTRIBUTE: Enclave was not authorized.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_FILE_ACCESS,
        "SGX_ERROR_ENCLAVE_FILE_ACCESS: Can't open enclave file.",
        NULL
    },
    {
        SGX_ERROR_NDEBUG_ENCLAVE,
        "SGX_ERROR_NDEBUG_ENCLAVE: The enclave is signed as product enclave, and can not be created as debuggable enclave.",
        NULL
    },
};

//Check error conditions for loading enclave 
//modified to give more detailed information
void print_error_message(sgx_status_t ret)
{
    size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];
	size_t idx = 0;
    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
			printf("\nError (%d): %s\n", ret, sgx_errlist[idx].msg);
            if(NULL != sgx_errlist[idx].sug)
            {
				printf("Info: %s\n", sgx_errlist[idx].sug);
			}
            break;
        }
    }
    
    if (idx == ttl){
		printf("Error: Unexpected error occurred.\n");
	}
}
//Error handling code ends
//======================================================================


extern std::map<sgx_enclave_id_t, uint32_t>g_enclave_id_map;


sgx_enclave_id_t e1_enclave_id = 0;
sgx_enclave_id_t e2_enclave_id = 0;

#define ENCLAVE1_PATH "libenclave1.so"
#define ENCLAVE2_PATH "libenclave2.so"

void waitForKeyPress()
{
    char ch;
    int temp;
    printf("\n\nHit a key....\n");
    temp = scanf_s("%c", &ch);
}

sgx_status_t load_enclaves()
{
    uint32_t enclave_temp_no = 0;
    int launch_token_updated = 0;
    sgx_launch_token_t launch_token = {0};
    sgx_status_t ret;


    ret = sgx_create_enclave(ENCLAVE1_PATH, SGX_DEBUG_FLAG, &launch_token, &launch_token_updated, &e1_enclave_id, NULL);
    if (ret != SGX_SUCCESS) {
				print_error_message(ret);
                return ret;
    }

    enclave_temp_no++;
    g_enclave_id_map.insert(std::pair<sgx_enclave_id_t, uint32_t>(e1_enclave_id, enclave_temp_no));

    ret = sgx_create_enclave(ENCLAVE2_PATH, SGX_DEBUG_FLAG, &launch_token, &launch_token_updated, &e2_enclave_id, NULL);
    if (ret != SGX_SUCCESS) {
				print_error_message(ret);
                return ret;
    }

    enclave_temp_no++;
    g_enclave_id_map.insert(std::pair<sgx_enclave_id_t, uint32_t>(e2_enclave_id, enclave_temp_no));

    return SGX_SUCCESS;
}

/* OCall functions */
void ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate 
     * the input string to prevent buffer overflow. 
     */
    printf("%s", str);
}

void ocall_print_ans(double ans){
    printf("The avg is %f\n",ans);
}

double ocall_scan_d(){
    double tmp = 0;
    scanf("%lf",&tmp);
    return tmp;
}

int main(int argc, char* argv[])
{
    uint32_t ret_status;
    sgx_status_t status;
    
    sgx_status_t ret;

    int total_number = 0;
    int ii = 0;
    double tmp;

    (void)(argc);
    (void)(argv);
	
	ret = load_enclaves();
    if(ret != SGX_SUCCESS)
    {
        printf("\nLoad Enclave Failure");
    }

    printf("\nAvaliable Enclaves");
    printf("\nEnclave1 - EnclaveID %" PRIx64, e1_enclave_id);
    printf("\nEnclave2 - EnclaveID %" PRIx64, e2_enclave_id);
    
    status = Enclave1_test_create_session(e1_enclave_id, &ret_status, e1_enclave_id, e2_enclave_id);
	if (status!=SGX_SUCCESS)
	{
		printf("\nConnect to attestaion provider failed: Error code is %x", status);
		return -1;
	}
	else
	{
		if(ret_status==0)
		{
			printf("\nConnected to the local attestaion provider");
		}
		else
		{
			printf("\nSession establishment and key exchange failure: Error code is %x", ret_status);
			return -1;
		}
	}

	//Test message exchange between Enclave1(Source) and Enclave2(Destination)
	status = Enclave1_test_message_exchange(e1_enclave_id, &ret_status, e1_enclave_id, e2_enclave_id);
	if (status!=SGX_SUCCESS)
	{
		printf("\nSending ID failed: Error code is %x", status);
		return -1;
	}
	else
	{
		if(ret_status==0)
		{
			printf("\nSended the ID to the attestation provider.");
		}
		else
		{
			printf("\nId Checked Error: Error code is %x", ret_status);
			return -1;
		}
	}
   
	status = Enclave1_test_close_session(e1_enclave_id, &ret_status, e1_enclave_id, e2_enclave_id);
	if (status!=SGX_SUCCESS)
	{
		printf("\nClosing Session failed: Error code is %x", status);
		return -1;
	}
	else
	{
		if(ret_status==0)
		{
			printf("\nClosed Session to the attestation provider!");
		}
		else
		{
			printf("\nClose session failed: Error code is %x", ret_status);
			return -1;
		}
	}

    printf("\nChecked the ID of Enclave.")
    printf("\n需要输入几个工资？");
    scanf("%d",&total_number);
    
    Enclave1_ecall_init_array( e1_enclave_id);

    for(; ii < total_number; ++ ii) {
        printf("\n输入工资");
        //scanf("%lf",&tmp);
        Enclave1_ecall_insert_number( e1_enclave_id);
    }
    Enclave1_ecall_get_avg( e1_enclave_id);

    sgx_destroy_enclave(e1_enclave_id);
    sgx_destroy_enclave(e2_enclave_id);
	printf("\n");
    return 0;
}
