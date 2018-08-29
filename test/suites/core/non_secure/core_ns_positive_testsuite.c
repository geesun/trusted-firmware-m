/*
 * Copyright (c) 2017-2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "core_ns_tests.h"
#include "tfm_api.h"
#include "tfm_veneers.h"
#include "test/suites/core/non_secure/core_test_api.h"
#include "test/test_services/tfm_core_test/core_test_defs.h"

#include <stdio.h>
#include <string.h>

/* Define test suite for core tests */
/* List of tests */

#define TOSTRING(x) #x
#define CORE_TEST_DESCRIPTION(number, fn, description) \
    {fn, "TFM_CORE_TEST_"TOSTRING(number),\
     description, {0} }

static void tfm_core_test_ns_thread(struct test_result_t *ret);
static void tfm_core_test_check_init(struct test_result_t *ret);
static void tfm_core_test_recursion(struct test_result_t *ret);
static void tfm_core_test_permissions(struct test_result_t *ret);
static void tfm_core_test_mpu_access(struct test_result_t *ret);
static void tfm_core_test_buffer_check(struct test_result_t *ret);
static void tfm_core_test_ss_to_ss(struct test_result_t *ret);
static void tfm_core_test_share_change(struct test_result_t *ret);
static void tfm_core_test_ss_to_ss_buffer(struct test_result_t *ret);
static void tfm_core_test_peripheral_access(struct test_result_t *ret);
static void tfm_core_test_get_caller_client_id(struct test_result_t *ret);
static void tfm_core_test_spm_request(struct test_result_t *ret);
static void tfm_core_test_iovec_sanitization(struct test_result_t *ret);
static void tfm_core_test_outvec_write(struct test_result_t *ret);

static struct test_t core_tests[] = {
CORE_TEST_DESCRIPTION(CORE_TEST_ID_NS_THREAD, tfm_core_test_ns_thread,
    "Test service request from NS thread mode"),
CORE_TEST_DESCRIPTION(CORE_TEST_ID_CHECK_INIT, tfm_core_test_check_init,
    "Test the success of service init"),
CORE_TEST_DESCRIPTION(CORE_TEST_ID_RECURSION, tfm_core_test_recursion,
    "Test direct recursion of secure services (DEPRECATED)"),
CORE_TEST_DESCRIPTION(CORE_TEST_ID_MEMORY_PERMISSIONS,
    tfm_core_test_permissions,
    "Test secure service memory access permissions"),
CORE_TEST_DESCRIPTION(CORE_TEST_ID_MPU_ACCESS, tfm_core_test_mpu_access,
    "Test secure service MPU accesses"),
CORE_TEST_DESCRIPTION(CORE_TEST_ID_BUFFER_CHECK, tfm_core_test_buffer_check,
    "Test secure service buffer accesses"),
CORE_TEST_DESCRIPTION(CORE_TEST_ID_SS_TO_SS, tfm_core_test_ss_to_ss,
    "Test secure service to service call"),
CORE_TEST_DESCRIPTION(CORE_TEST_ID_SHARE_REDIRECTION,
    tfm_core_test_share_change,
    "Test secure service share change request"),
CORE_TEST_DESCRIPTION(CORE_TEST_ID_SS_TO_SS_BUFFER,
    tfm_core_test_ss_to_ss_buffer,
    "Test secure service to service call with buffer handling"),
CORE_TEST_DESCRIPTION(CORE_TEST_ID_PERIPHERAL_ACCESS,
    tfm_core_test_peripheral_access,
    "Test service peripheral access"),
CORE_TEST_DESCRIPTION(CORE_TEST_ID_GET_CALLER_CLIENT_ID,
    tfm_core_test_get_caller_client_id,
    "Test get caller client ID function"),
CORE_TEST_DESCRIPTION(CORE_TEST_ID_SPM_REQUEST,
    tfm_core_test_spm_request,
    "Test SPM request function"),
CORE_TEST_DESCRIPTION(CORE_TEST_ID_IOVEC_SANITIZATION,
    tfm_core_test_iovec_sanitization,
    "Test service parameter sanitization"),
CORE_TEST_DESCRIPTION(CORE_TEST_ID_OUTVEC_WRITE,
    tfm_core_test_outvec_write,
    "Test outvec write"),
};

void register_testsuite_ns_core_positive(struct test_suite_t *p_test_suite)
{
    uint32_t list_size;

    list_size = (sizeof(core_tests) / sizeof(core_tests[0]));

    set_testsuite("Core non-secure positive tests (TFM_CORE_TEST_1XXX)",
                  core_tests, list_size, p_test_suite);
}

static void tfm_core_test_ns_thread(struct test_result_t *ret)
{
    int32_t err;
    int32_t test_case_id = CORE_TEST_ID_NS_THREAD;
    psa_invec in_vec[] = { {&test_case_id, sizeof(int32_t)} };

    err = tfm_spm_core_test_sfn_veneer(in_vec, 1, NULL, 0);

    if (err != TFM_SUCCESS) {
        TEST_FAIL("Secure function call from thread mode should be successful");
        return;
    }

    ret->val = TEST_PASSED;
}

static void tfm_core_test_peripheral_access(struct test_result_t *ret)
{
    int32_t err;
    int32_t test_case_id = CORE_TEST_ID_PERIPHERAL_ACCESS;
    psa_invec in_vec[] = { {&test_case_id, sizeof(int32_t)} };
    struct tfm_core_test_call_args_t args = {in_vec, 1, NULL, 0};

    err = tfm_core_test_call(tfm_spm_core_test_sfn_veneer, &args);

    if ((err != TFM_SUCCESS) && (err < TFM_PARTITION_SPECIFIC_ERROR_MIN)) {
        TEST_FAIL("TFM Core returned error.");
        return;
    }

    switch (err) {
    case CORE_TEST_ERRNO_SUCCESS:
        ret->val = TEST_PASSED;
        return;
    case CORE_TEST_ERRNO_PERIPHERAL_ACCESS_FAILED:
        TEST_FAIL("Service peripheral access failed.");
        return;
    default:
        TEST_FAIL("Unexpected return value received.");
        return;
    }
}
static void empty_iovecs(psa_invec invec[], psa_outvec outvec[])
{
    int i = 0;

    for (i = 0; i < PSA_MAX_IOVEC; ++i) {
        invec[i].len = 0;
        invec[i].base = NULL;
        outvec[i].len = 0;
        outvec[i].base = NULL;
    }
}

static void full_iovecs(psa_invec invec[], psa_outvec outvec[])
{
    int i = 0;

    for (i = 0; i < PSA_MAX_IOVEC; ++i) {
        invec[i].len = PSA_MAX_IOVEC*sizeof(psa_invec);
        invec[i].base = invec;
        outvec[i].len = PSA_MAX_IOVEC*sizeof(psa_outvec);
        outvec[i].base = outvec;
    }
}

static void tfm_core_test_iovec_sanitization(struct test_result_t *ret)
{
    int32_t err;
    psa_invec in_vec[PSA_MAX_IOVEC] = {
                                   {NULL, 0}, {NULL, 0}, {NULL, 0}, {NULL, 0} };
    psa_outvec out_vec[PSA_MAX_IOVEC] = {
                                   {NULL, 0}, {NULL, 0}, {NULL, 0}, {NULL, 0} };
    struct tfm_core_test_call_args_t args = {NULL, 0, NULL, 0};

    /* Check a few valid cases */

    /* Execute a call with valid iovecs (empty) */
    empty_iovecs(in_vec, out_vec);
    args.in_vec = NULL;
    args.in_len = 0;
    args.out_vec = NULL;
    args.out_len = 0;
    err = tfm_core_test_call(tfm_spm_core_test_2_slave_service_veneer, &args);
    if (err != TFM_SUCCESS) {
        TEST_FAIL("iovec sanitization failed on empty vectors.");
        return;
    }

    /* Execute a call with valid iovecs (full) */
    full_iovecs(in_vec, out_vec);
    args.in_vec = in_vec;
    args.in_len = 2;
    args.out_vec = out_vec;
    args.out_len = PSA_MAX_IOVEC - args.in_len;
    err = tfm_core_test_call(tfm_spm_core_test_2_slave_service_veneer, &args);
    if (err != TFM_SUCCESS) {
        TEST_FAIL("iovec sanitization failed on full vectors.");
        return;
    }

    /* Execute a call with valid iovecs (different number of vectors) */
    full_iovecs(in_vec, out_vec);
    args.in_vec = in_vec;
    args.in_len = 2;
    args.out_vec = out_vec;
    args.out_len = 1;
    err = tfm_core_test_call(tfm_spm_core_test_2_slave_service_veneer, &args);
    if (err != TFM_SUCCESS) {
        TEST_FAIL(
                 "iovec sanitization failed on valid, partially full vectors.");
        return;
    }

    /* Check some further valid cases to be sure that checks happen only iovecs
     * that specified valid by the parameters
     */

    /* Execute a call with base = 0 in single vector in outvec that is out of
     * range
     */
    full_iovecs(in_vec, out_vec);
    args.in_vec = in_vec;
    args.in_len = 2;
    args.out_vec = out_vec;
    args.out_len = 1;
    out_vec[1].base = NULL;
    err = tfm_core_test_call(tfm_spm_core_test_2_slave_service_veneer, &args);
    if (err != TFM_SUCCESS) {
        TEST_FAIL("content of an outvec out of range should not be checked");
        return;
    }

    /* Execute a call with len = 0 in single vector in invec that is out of
     * range
     */
    full_iovecs(in_vec, out_vec);
    args.in_vec = in_vec;
    args.in_len = 2;
    args.out_vec = out_vec;
    args.out_len = 1;
    in_vec[2].len = 0;
    err = tfm_core_test_call(tfm_spm_core_test_2_slave_service_veneer, &args);
    if (err != TFM_SUCCESS) {
        TEST_FAIL("content of an outvec out of range should not be checked");
        return;
    }

    /* Execute a call with len = 0 in single vector in invec */
    full_iovecs(in_vec, out_vec);
    args.in_vec = in_vec;
    args.in_len = 2;
    args.out_vec = out_vec;
    args.out_len = 2;
    in_vec[1].len = 0;
    in_vec[1].base = NULL;
    err = tfm_core_test_call(tfm_spm_core_test_2_slave_service_veneer, &args);
    if (err != TFM_SUCCESS) {
        TEST_FAIL("If the len of an invec is 0, the base should be ignored");
        return;
    }

    /* Execute a call with len = 0 in single vector in outvec */
    full_iovecs(in_vec, out_vec);
    args.in_vec = in_vec;
    args.in_len = 2;
    args.out_vec = out_vec;
    args.out_len = 2;
    out_vec[1].len = 0;
    out_vec[1].base = NULL;
    err = tfm_core_test_call(tfm_spm_core_test_2_slave_service_veneer, &args);
    if (err != TFM_SUCCESS) {
        TEST_FAIL("If the len of an outvec is 0, the base should be ignored");
        return;
    }

    ret->val = TEST_PASSED;
}

static void tfm_core_test_outvec_write(struct test_result_t *ret)
{
    int32_t err;
    int32_t test_case_id = CORE_TEST_ID_OUTVEC_WRITE;
    int i;
    uint8_t in_buf_0[] = {0, 1, 2, 3, 4};
    uint8_t in_buf_1[] = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89};
    uint8_t out_buf_0[sizeof(in_buf_0)];
    uint8_t out_buf_1[sizeof(in_buf_1)];

    psa_invec in_vec[PSA_MAX_IOVEC] = { {in_buf_0, sizeof(in_buf_0)},
                                        {in_buf_1, sizeof(in_buf_1)} };
    psa_outvec out_vec[PSA_MAX_IOVEC] = { {out_buf_0, sizeof(out_buf_0) },
                                        {out_buf_1, sizeof(out_buf_1)} };
    struct tfm_core_test_call_args_t args1 = {in_vec, 2, out_vec, 2};
    struct tfm_core_test_call_args_t args2 = {in_vec, 1, NULL, 0};

    err = tfm_core_test_call(tfm_spm_core_test_2_get_every_second_byte_veneer,
                                                                        &args1);

    if (err != TFM_SUCCESS) {
        TEST_FAIL("call to secure function should be successful");
        return;
    }

    if (out_vec[0].len != sizeof(in_buf_0)/2 ||
        out_vec[1].len != sizeof(in_buf_1)/2) {
        TEST_FAIL("Number of elements in outvec is not set properly");
        return;
    }
    for (i = 1; i < sizeof(in_buf_0); i += 2) {
        if (((uint8_t *)out_vec[0].base)[i/2] != in_buf_0[i]) {
            TEST_FAIL("result is not correct");
            return;
        }
    }
    for (i = 1; i < sizeof(in_buf_1); i += 2) {
        if (((uint8_t *)out_vec[1].base)[i/2] != in_buf_1[i]) {
            TEST_FAIL("result is not correct");
            return;
        }
    }

    /* do the same test on the secure side */
    in_vec[0].base = &test_case_id;
    in_vec[0].len = sizeof(int32_t);
    err = tfm_core_test_call(tfm_spm_core_test_sfn_veneer, &args2);

    if (err != TFM_SUCCESS) {
        TEST_FAIL("Failed to execute secure side test");
        return;
    }

    ret->val = TEST_PASSED;
}


/*
 * \brief Tests whether the initialisation of the service was successful.
 *
 */
static void tfm_core_test_check_init(struct test_result_t *ret)
{
    int32_t err;
    struct tfm_core_test_call_args_t args = {NULL, 0, NULL, 0};

    err = tfm_core_test_call(tfm_spm_core_test_sfn_init_success_veneer, &args);

    if ((err != TFM_SUCCESS) && (err < TFM_PARTITION_SPECIFIC_ERROR_MIN)) {
        TEST_FAIL("TFM Core returned error.");
        return;
    }

    if (err != CORE_TEST_ERRNO_SUCCESS) {
        TEST_FAIL("Failed to initialise test service.");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests what happens when a service calls itself directly
 */
static void tfm_core_test_recursion(struct test_result_t *ret)
{
    /* This test triggers a non-recoverable security violation. Don't run */
    TEST_LOG("This test is DEPRECATED and the test execution was SKIPPED\r\n");
    ret->val = TEST_PASSED;
}

static char *error_to_string(const char *desc, int32_t err)
{
    static char info[80];

    sprintf(info, "%s. Error code: %d, extra data: %d",
        desc,
        CORE_TEST_ERROR_GET_CODE(err),
        CORE_TEST_ERROR_GET_EXTRA(err));
    return info;
}

static void tfm_core_test_mpu_access(struct test_result_t *ret)
{
    int32_t err;
    int32_t test_case_id = CORE_TEST_ID_MPU_ACCESS;
    uint32_t data[4] = {0};
    psa_invec in_vec[] = { {&test_case_id, sizeof(int32_t)},
                          {data, sizeof(data)},
                          {(void *)((int32_t)tfm_core_test_mpu_access &
                                                                      (~(0x3))),
                           sizeof(uint32_t)} };
    psa_outvec outvec[] = { {data, sizeof(data)} };
    struct tfm_core_test_call_args_t args = {in_vec, 3, outvec, 1};

    err = tfm_core_test_call(tfm_spm_core_test_sfn_veneer, &args);

    if (err != TFM_SUCCESS && err < TFM_PARTITION_SPECIFIC_ERROR_MIN) {
        TEST_FAIL("TFM Core returned error.");
        return;
    }

    if (err != CORE_TEST_ERRNO_SUCCESS) {
        char *info = error_to_string(
            "Service memory accesses configured incorrectly.", err);
        TEST_FAIL(info);
        return;
    }

    ret->val = TEST_PASSED;
}

static void tfm_core_test_permissions(struct test_result_t *ret)
{
    int32_t err;
    int32_t test_case_id = CORE_TEST_ID_MEMORY_PERMISSIONS;
    uint32_t data[4] = {0};
    psa_invec in_vec[] = { {&test_case_id, sizeof(int32_t)},
                          {data, sizeof(data)},
                          {(void *)((int32_t)tfm_core_test_mpu_access &
                                                                      (~(0x3))),
                           sizeof(uint32_t)} };
    psa_outvec outvec[] = { {data, sizeof(data)} };
    struct tfm_core_test_call_args_t args = {in_vec, 3, outvec, 1};

    err = tfm_core_test_call(tfm_spm_core_test_sfn_veneer, &args);

    if (err != TFM_SUCCESS && err < TFM_PARTITION_SPECIFIC_ERROR_MIN) {
        TEST_FAIL("TFM Core returned error.");
        return;
    }

    if (err != CORE_TEST_ERRNO_SUCCESS) {
        char *info = error_to_string(
            "Service memory accesses configured incorrectly.", err);
        TEST_FAIL(info);
        return;
    }

    ret->val = TEST_PASSED;
}

static void tfm_core_test_buffer_check(struct test_result_t *ret)
{
    int32_t res, i;

    uint32_t inbuf[] = {1, 2, 3, 4, 0xAAAFFF, 0xFFFFFFFF};
    uint32_t outbuf[16] = {0};
    int32_t result;
    psa_invec in_vec[] = { {inbuf, sizeof(inbuf)} };
    psa_outvec outvec[] = { {outbuf, sizeof(outbuf)},
                           {&result, sizeof(int32_t)} };
    struct tfm_core_test_call_args_t args = {in_vec, 1, outvec, 2};

    res = tfm_core_test_call(tfm_spm_core_test_2_sfn_invert_veneer, &args);
    if ((res != TFM_SUCCESS) && (res < TFM_PARTITION_SPECIFIC_ERROR_MIN)) {
        TEST_FAIL("Call to secure service should be successful.");
        return;
    }
    if ((result == 0) && (res == TFM_SUCCESS)) {
        for (i = 0; i < sizeof(inbuf) >> 2; i++) {
            if (outbuf[i] != ~inbuf[i]) {
                TEST_FAIL("Secure function failed to modify buffer.");
                return;
            }
        }
        for (; i < sizeof(outbuf) >> 2; i++) {
            if (outbuf[i] != 0) {
                TEST_FAIL("Secure function buffer access overflow.");
                return;
            }
        }
    } else {
        TEST_FAIL("Secure service returned error.");
        return;
    }

    ret->val = TEST_PASSED;
}

static void tfm_core_test_ss_to_ss(struct test_result_t *ret)
{
    int32_t err;

    int32_t test_case_id = CORE_TEST_ID_SS_TO_SS;
    psa_invec in_vec[] = { {&test_case_id, sizeof(int32_t)} };
    struct tfm_core_test_call_args_t args = {in_vec, 1, NULL, 0};

    err = tfm_core_test_call(tfm_spm_core_test_sfn_veneer, &args);

    if (err != TFM_SUCCESS && err < TFM_PARTITION_SPECIFIC_ERROR_MIN) {
        TEST_FAIL("Call to secure service should be successful.");
        return;
    }

    if (err != CORE_TEST_ERRNO_SUCCESS) {
        TEST_FAIL("The internal service call failed.");
        return;
    }

    ret->val = TEST_PASSED;
}

static void tfm_core_test_share_change(struct test_result_t *ret)
{
    int32_t err;
    int32_t test_case_id = CORE_TEST_ID_SHARE_REDIRECTION;
    psa_invec in_vec[] = { {&test_case_id, sizeof(int32_t)} };
    struct tfm_core_test_call_args_t args = {in_vec, 1, NULL, 0};

    err = tfm_core_test_call(tfm_spm_core_test_sfn_veneer, &args);

    if ((err != TFM_SUCCESS) && (err < TFM_PARTITION_SPECIFIC_ERROR_MIN)) {
        TEST_FAIL("TFM Core returned error.");
        return;
    }

    if (err != CORE_TEST_ERRNO_SUCCESS) {
        TEST_FAIL("Failed to redirect share region in service.");
        return;
    }

    ret->val = TEST_PASSED;
}

static void tfm_core_test_ss_to_ss_buffer(struct test_result_t *ret)
{
    int32_t res, i;

    uint32_t inbuf[] = {1, 2, 3, 4, 0xAAAFFF, 0xFFFFFFFF};
    uint32_t outbuf[16] = {0};
    int32_t len = (int32_t)sizeof(inbuf) >> 2;
    int32_t test_case_id = CORE_TEST_ID_SS_TO_SS_BUFFER;
    psa_invec in_vec[] = { {&test_case_id, sizeof(int32_t)},
                          {inbuf, sizeof(inbuf)},
                          {&len, sizeof(int32_t)} };
    psa_outvec outvec[] = { {outbuf, sizeof(outbuf)} };
    struct tfm_core_test_call_args_t args = {in_vec, 3, outvec, 1};

    res = tfm_core_test_call(tfm_spm_core_test_sfn_veneer, &args);
    if ((res != TFM_SUCCESS) && (res < TFM_PARTITION_SPECIFIC_ERROR_MIN)) {
        TEST_FAIL("Call to secure service should be successful.");
        return;
    }
    switch (res) {
    case TFM_SUCCESS:
        for (i = 0; i < sizeof(inbuf) >> 2; i++) {
            if (outbuf[i] != ~inbuf[i]) {
                TEST_FAIL("Secure function failed to modify buffer.");
                return;
            }
        }
        for (; i < sizeof(outbuf) >> 2; i++) {
            if (outbuf[i] != 0) {
                TEST_FAIL("Secure function buffer access overflow.");
                return;
            }
        }
        ret->val = TEST_PASSED;
        return;
    case CORE_TEST_ERRNO_INVALID_BUFFER:
        TEST_FAIL("NS buffer rejected by TF-M core.");
        return;
    case CORE_TEST_ERRNO_SLAVE_SP_CALL_FAILURE:
        TEST_FAIL("Slave service call failed.");
        return;
    case CORE_TEST_ERRNO_SLAVE_SP_BUFFER_FAILURE:
        TEST_FAIL("Slave secure function failed to modify buffer.");
        return;
    default:
        TEST_FAIL("Secure service returned error.");
        return;
    }
}

static void tfm_core_test_get_caller_client_id(struct test_result_t *ret)
{
    int32_t err;
    int32_t test_case_id = CORE_TEST_ID_GET_CALLER_CLIENT_ID;
    psa_invec in_vec[] = { {&test_case_id, sizeof(int32_t)} };
    struct tfm_core_test_call_args_t args = {in_vec, 1, NULL, 0};

    err = tfm_core_test_call(tfm_spm_core_test_sfn_veneer, &args);

    if (err != TFM_SUCCESS && err < TFM_PARTITION_SPECIFIC_ERROR_MIN) {
        TEST_FAIL("Call to secure service should be successful.");
        return;
    }

    if (err != CORE_TEST_ERRNO_SUCCESS) {
        TEST_FAIL("The internal service call failed.");
        return;
    }

    ret->val = TEST_PASSED;
}

static void tfm_core_test_spm_request(struct test_result_t *ret)
{
    int32_t err;
    int32_t test_case_id = CORE_TEST_ID_SPM_REQUEST;
    psa_invec in_vec[] = { {&test_case_id, sizeof(int32_t)} };
    struct tfm_core_test_call_args_t args = {in_vec, 1, NULL, 0};

    err = tfm_core_test_call(tfm_spm_core_test_sfn_veneer, &args);

    if (err != TFM_SUCCESS && err < TFM_PARTITION_SPECIFIC_ERROR_MIN) {
        TEST_FAIL("Call to secure service should be successful.");
        return;
    }

    if (err != CORE_TEST_ERRNO_SUCCESS) {
        TEST_FAIL("The SPM request failed.");
        return;
    }

    ret->val = TEST_PASSED;
}
