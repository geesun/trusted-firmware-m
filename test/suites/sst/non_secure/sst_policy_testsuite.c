/*
 * Copyright (c) 2017-2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "sst_ns_tests.h"

#include <string.h>

#include "ns_test_helpers.h"
#include "secure_fw/services/secure_storage/assets/sst_asset_defs.h"
#include "test/framework/helpers.h"
#include "tfm_sst_api.h"

/* The tests in this test suite cover access to an asset when the application
 * has:
 *  - REFERENCE/READ/WRITE permissions,
 *  - REFERENCE/READ permissions,
 *  - REFERENCE/WRITE permissions,
 *  - REFERENCE permission,
 *  - no permissions (NONE).
 *
 * In each case, every function in the SST API is tested for correct access
 * policy.
 */

/**
 * \note List of relations between thread name, app ID and permissions.
 *
 * Asset permissions: SST_ASSET_ID_X509_CERT_LARGE
 *
 *   THREAD NAME | APP_ID       | Permissions
 *   ------------|--------------------------------------
 *     Thread_A  | SST_APP_ID_0 | REFERENCE
 *     Thread_B  | SST_APP_ID_1 | REFERENCE, READ
 *     Thread_C  | SST_APP_ID_2 | REFERENCE, READ, WRITE
 *
 * Asset permissions: SST_ASSET_ID_SHA224_HASH
 *
 *   THREAD NAME | APP_ID       | Permissions
 *   ------------|--------------------------------------
 *     Thread_A  | SST_APP_ID_0 | NONE
 *     Thread_B  | SST_APP_ID_1 | REFERENCE, READ, WRITE
 *     Thread_C  | SST_APP_ID_2 | NONE
 *
 * Asset permissions: SST_ASSET_ID_SHA384_HASH
 *
 *   THREAD NAME | APP_ID       | Permissions
 *   ------------|--------------------------------
 *     Thread_A  | SST_APP_ID_0 | NONE
 *     Thread_B  | SST_APP_ID_1 | NONE
 *     Thread_C  | SST_APP_ID_2 | REFERENCE, WRITE
 */

/* Test suite defines */
/* Each thread has different write data so that the tests can verify the correct
 * thread's data is stored in the asset.
 */
#define WRITE_DATA_A "AAAA"
#define WRITE_DATA_B "BBBB"
#define WRITE_DATA_C "CCCC"

/* Original contents of the read buffer */
#define READ_DATA "XXXXXXXXXXX"

/* Contents of the read buffer after reading back write_data */
#define RESULT_DATA(write_data) ("XXX" write_data "\0XXX")

/* Sizes of the read and write buffers */
#define WRITE_BUF_SIZE (sizeof(WRITE_DATA_A))
#define READ_BUF_SIZE  (sizeof(READ_DATA))

/* Asset handle to be shared between threads. Used to test policy in the case
 * where an unauthorised thread has gained access to a valid handle.
 */
static uint32_t shared_handle;

/* Define test suite for SST policy tests */
/* List of tests */
static void tfm_sst_test_4001(struct test_result_t *ret);
static void tfm_sst_test_4002(struct test_result_t *ret);
static void tfm_sst_test_4003(struct test_result_t *ret);
static void tfm_sst_test_4004(struct test_result_t *ret);
static void tfm_sst_test_4005(struct test_result_t *ret);
static void tfm_sst_test_4006(struct test_result_t *ret);
static void tfm_sst_test_4007(struct test_result_t *ret);
static void tfm_sst_test_4008(struct test_result_t *ret);
static void tfm_sst_test_4009(struct test_result_t *ret);
static void tfm_sst_test_4010(struct test_result_t *ret);

static struct test_t policy_tests[] = {
    { &tfm_sst_test_4001, "TFM_SST_TEST_4001",
      "Check policy for X509_CERT_LARGE from Thread_C", {0} },
    { &tfm_sst_test_4002, "TFM_SST_TEST_4002",
      "Check policy for X509_CERT_LARGE from Thread_A", {0} },
    { &tfm_sst_test_4003, "TFM_SST_TEST_4003",
      "Check policy for X509_CERT_LARGE from Thread_B", {0} },
    { &tfm_sst_test_4004, "TFM_SST_TEST_4004",
      "Delete X509_CERT_LARGE from Thread_C", {0} },
    { &tfm_sst_test_4005, "TFM_SST_TEST_4005",
      "Check policy for SHA224_HASH from Thread_B", {0} },
    { &tfm_sst_test_4006, "TFM_SST_TEST_4006",
      "Check policy for SHA224_HASH from Thread_A",  {0} },
    { &tfm_sst_test_4007, "TFM_SST_TEST_4007",
      "Delete SHA224_HASH from Thread_B", {0} },
    { &tfm_sst_test_4008, "TFM_SST_TEST_4008",
      "Check policy for SHA384_HASH from Thread_C", {0} },
    { &tfm_sst_test_4009, "TFM_SST_TEST_4009",
      "Check policy for SHA384_HASH from Thread_A", {0} },
    { &tfm_sst_test_4010, "TFM_SST_TEST_4010",
      "Delete SHA384_HASH from Thread_C", {0} },
};

void register_testsuite_ns_sst_policy(struct test_suite_t *p_test_suite)
{
    uint32_t list_size = (sizeof(policy_tests) / sizeof(policy_tests[0]));

    set_testsuite("SST policy tests (TFM_SST_TEST_4XXX)",
                  policy_tests, list_size, p_test_suite);
}

/**
 * \brief Tests policy for SST_ASSET_ID_X509_CERT_LARGE with the following
 *        permissions:
 *          THREAD NAME | APP_ID       | Permissions
 *          ------------|--------------|-----------------------
 *          Thread_C    | SST_APP_ID_2 | REFERENCE, READ, WRITE
 */
TFM_SST_NS_TEST(4001, "Thread_C")
{
    const uint16_t asset_uuid = SST_ASSET_ID_X509_CERT_LARGE;
    struct tfm_sst_attribs_t attribs;
    struct tfm_sst_buf_t buf;
    enum tfm_sst_err_t err;
    uint32_t hdl;
    uint8_t write_data[WRITE_BUF_SIZE] = WRITE_DATA_C;
    uint8_t read_data[READ_BUF_SIZE] = READ_DATA;

    /* The create function requires WRITE permission */
    err = tfm_sst_create(asset_uuid);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for Thread_C");
        return;
    }

    /* The get handle function requires any permission other than NONE */
    err = tfm_sst_get_handle(asset_uuid, &hdl);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Get handle should not fail for Thread_C");
        return;
    }

    /* Sets the tfm_sst_buf_t structure */
    buf.data = write_data;
    buf.size = WRITE_BUF_SIZE;
    buf.offset = 0;

    /* The write function requires WRITE permission */
    err = tfm_sst_write(hdl, &buf);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Write should not fail for Thread_C");
        return;
    }

    /* Sets the tfm_sst_buf_t structure */
    buf.data = read_data + 3;
    buf.size = WRITE_BUF_SIZE;
    buf.offset = 0;

    /* The read function requires READ permission */
    err = tfm_sst_read(hdl, &buf);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Read should not fail for Thread_C");
        return;
    }

    /* Checks correct data was read-back */
    if (memcmp(read_data, RESULT_DATA(WRITE_DATA_C), READ_BUF_SIZE) != 0) {
        TEST_FAIL("Read buffer contains incorrect data");
        return;
    }

    /* The get attributes function requires any permission other than NONE */
    err = tfm_sst_get_attributes(hdl, &attribs);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Get attributes should not fail for Thread_C");
        return;
    }

    /* Checks attributes are correct */
    if (attribs.size_current != WRITE_BUF_SIZE) {
        TEST_FAIL("Current size of the asset is incorrect");
        return;
    }

    if (attribs.size_max != SST_ASSET_MAX_SIZE_X509_CERT_LARGE) {
        TEST_FAIL("Max size of the asset is incorrect");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests policy for SST_ASSET_ID_X509_CERT_LARGE with the following
 *        permissions:
 *          THREAD NAME | APP_ID       | Permissions
 *          ------------|--------------|------------
 *          Thread_A    | SST_APP_ID_0 | REFERENCE
 */
TFM_SST_NS_TEST(4002, "Thread_A")
{
    const uint16_t asset_uuid = SST_ASSET_ID_X509_CERT_LARGE;
    struct tfm_sst_attribs_t attribs;
    struct tfm_sst_buf_t buf;
    enum tfm_sst_err_t err;
    uint32_t hdl;
    uint8_t write_data[WRITE_BUF_SIZE] = WRITE_DATA_A;
    uint8_t read_data[READ_BUF_SIZE] = READ_DATA;

    /* Create should fail as Thread_A does not have WRITE permission */
    err = tfm_sst_create(asset_uuid);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Create should not succeed for Thread_A");
        return;
    }

    /* Get handle should succeed as Thread_A has at least one permission */
    err = tfm_sst_get_handle(asset_uuid, &hdl);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Get handle should not fail for Thread_A");
        return;
    }

    /* Sets the tfm_sst_buf_t structure */
    buf.data = write_data;
    buf.size = WRITE_BUF_SIZE;
    /* Increases offset so that current size will change if write succeeds */
    buf.offset = 1;

    /* Write should fail as Thread_A does not have WRITE permission */
    err = tfm_sst_write(hdl, &buf);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Write should not succeed for Thread_A");
        return;
    }

    /* Sets the tfm_sst_buf_t structure */
    buf.data = read_data + 3;
    buf.size = WRITE_BUF_SIZE;
    buf.offset = 0;

    /* Read should fail as Thread_A does not have READ permission */
    err = tfm_sst_read(hdl, &buf);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Read should not succeed for Thread_A");
        return;
    }

    /* Checks read_data has not been changed by the call to read */
    if (memcmp(read_data, READ_DATA, READ_BUF_SIZE) != 0) {
        TEST_FAIL("Read buffer should not have changed");
        return;
    }

    /* Get attributes should succeed as Thread_A has at least one permission */
    err = tfm_sst_get_attributes(hdl, &attribs);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Get attributes should not fail for Thread_A");
        return;
    }

    /* Checks attributes are correct */
    if (attribs.size_current != WRITE_BUF_SIZE) {
        TEST_FAIL("Current size of the asset is incorrect");
        return;
    }

    if (attribs.size_max != SST_ASSET_MAX_SIZE_X509_CERT_LARGE) {
        TEST_FAIL("Max size of the asset is incorrect");
        return;
    }

    /* Delete should fail as Thread_A does not have WRITE permission */
    err = tfm_sst_delete(hdl);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Delete should not succeed for Thread_A");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests policy for SST_ASSET_ID_X509_CERT_LARGE with the following
 *        permissions:
 *          THREAD NAME | APP_ID       | Permissions
 *          ------------|--------------|----------------
 *          Thread_B    | SST_APP_ID_1 | REFERENCE, READ
 */
TFM_SST_NS_TEST(4003, "Thread_B")
{
    const uint16_t asset_uuid = SST_ASSET_ID_X509_CERT_LARGE;
    struct tfm_sst_attribs_t attribs;
    struct tfm_sst_buf_t buf;
    enum tfm_sst_err_t err;
    uint32_t hdl;
    uint8_t write_data[WRITE_BUF_SIZE] = WRITE_DATA_B;
    uint8_t read_data[READ_BUF_SIZE] = READ_DATA;

    /* Create should fail as Thread_B does not have WRITE permission */
    err = tfm_sst_create(asset_uuid);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Create should not succeed for Thread_B");
        return;
    }

    /* Get handle should succeed as Thread_B has at least one permission */
    err = tfm_sst_get_handle(asset_uuid, &hdl);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Get handle should not fail for Thread_B");
        return;
    }

    /* Sets the tfm_sst_buf_t structure */
    buf.data = write_data;
    buf.size = WRITE_BUF_SIZE;
    /* Increases offset so that current size will change if write succeeds */
    buf.offset = 2;

    /* Write should fail as Thread_B does not have WRITE permission */
    err = tfm_sst_write(hdl, &buf);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Write should not succeed for Thread_B");
        return;
    }

    /* Sets the tfm_sst_buf_t structure */
    buf.data = read_data + 3;
    buf.size = WRITE_BUF_SIZE;
    buf.offset = 0;

    /* Read should succeed as Thread_B has READ permission */
    err = tfm_sst_read(hdl, &buf);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Read should not fail for Thread_B");
        return;
    }

    /* Checks correct data was read-back. It should still be Thread_C's data as
     * Thread_B does not have write permission.
     */
    if (memcmp(read_data, RESULT_DATA(WRITE_DATA_C), READ_BUF_SIZE) != 0) {
        TEST_FAIL("Read buffer contains incorrect data");
        return;
    }

    /* Get attributes should succeed as Thread_B has at least one permission */
    err = tfm_sst_get_attributes(hdl, &attribs);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Get attributes should not fail for Thread_B");
        return;
    }

    /* Checks attributes are correct */
    if (attribs.size_current != WRITE_BUF_SIZE) {
        TEST_FAIL("Current size of the asset is incorrect");
        return;
    }

    if (attribs.size_max != SST_ASSET_MAX_SIZE_X509_CERT_LARGE) {
        TEST_FAIL("Max size of the asset is incorrect");
        return;
    }

    /* Delete should fail as Thread_B does not have WRITE permission */
    err = tfm_sst_delete(hdl);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Delete should not succeed for Thread_B");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests delete policy for SST_ASSET_ID_X509_CERT_LARGE with the
 *        following permissions:
 *          THREAD NAME | APP_ID       | Permissions
 *          ------------|--------------|-----------------------
 *          Thread_C    | SST_APP_ID_2 | REFERENCE, READ, WRITE
 *
 * This test is performed last so that the asset still exists during the
 * preceeding test cases.
 */
TFM_SST_NS_TEST(4004, "Thread_C")
{
    const uint16_t asset_uuid = SST_ASSET_ID_X509_CERT_LARGE;
    enum tfm_sst_err_t err;
    uint32_t hdl;

    err = tfm_sst_get_handle(asset_uuid, &hdl);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Get handle should not fail for Thread_C");
        return;
    }

    /* The delete function requires WRITE permission */
    err = tfm_sst_delete(hdl);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Delete should not fail for Thread_C");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests policy for SST_ASSET_ID_SHA224_HASH with the following
 *        permissions:
 *          THREAD NAME | APP_ID       | Permissions
 *          ------------|--------------|-----------------------
 *          Thread_B    | SST_APP_ID_1 | REFERENCE, READ, WRITE
 */
TFM_SST_NS_TEST(4005, "Thread_B")
{
    const uint16_t asset_uuid = SST_ASSET_ID_SHA224_HASH;
    struct tfm_sst_attribs_t attribs;
    struct tfm_sst_buf_t buf;
    enum tfm_sst_err_t err;
    uint8_t write_data[WRITE_BUF_SIZE] = WRITE_DATA_B;
    uint8_t read_data[READ_BUF_SIZE] = READ_DATA;

    /* Create should succeed as Thread_B has WRITE permission */
    err = tfm_sst_create(asset_uuid);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for Thread_B");
        return;
    }

    /* Get handle should succeed as Thread_B has at least one permission. Writes
     * the handle into shared_handle so that the next test has access to a valid
     * handle that it does not have the right to get or use.
     */
    err = tfm_sst_get_handle(asset_uuid, &shared_handle);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Get handle should not fail for Thread_B");
        return;
    }

    /* Sets the tfm_sst_buf_t structure */
    buf.data = write_data;
    buf.size = WRITE_BUF_SIZE;
    buf.offset = 0;

    /* Write should succeed as Thread_B has WRITE permission */
    err = tfm_sst_write(shared_handle, &buf);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Write should not fail for Thread_B");
        return;
    }

    /* Sets the tfm_sst_buf_t structure */
    buf.data = read_data + 3;
    buf.size = WRITE_BUF_SIZE;
    buf.offset = 0;

    /* Read should succeed as Thread_B has READ permission */
    err = tfm_sst_read(shared_handle, &buf);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Read should not fail for Thread_B");
        return;
    }

    /* Checks correct data was read-back */
    if (memcmp(read_data, RESULT_DATA(WRITE_DATA_B), READ_BUF_SIZE) != 0) {
        TEST_FAIL("Read buffer contains incorrect data");
        return;
    }

    /* Get attributes should succeed as Thread_B has at least one permission */
    err = tfm_sst_get_attributes(shared_handle, &attribs);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Get attributes should not fail for Thread_B");
        return;
    }

    /* Checks attributes are correct */
    if (attribs.size_current != WRITE_BUF_SIZE) {
        TEST_FAIL("Current size of the asset is incorrect");
        return;
    }

    if (attribs.size_max != SST_ASSET_MAX_SIZE_SHA224_HASH) {
        TEST_FAIL("Max size of the asset is incorrect");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests policy for SST_ASSET_ID_SHA224_HASH with the following
 *        permissions:
 *          THREAD NAME | APP_ID       | Permissions
 *          ------------|--------------|------------
 *          Thread_A    | SST_APP_ID_0 | NONE
 */
TFM_SST_NS_TEST(4006, "Thread_A")
{
    const uint16_t asset_uuid = SST_ASSET_ID_SHA224_HASH;
    struct tfm_sst_attribs_t attribs = { 0 };
    struct tfm_sst_buf_t buf;
    enum tfm_sst_err_t err;
    uint32_t hdl = 0;
    uint8_t write_data[WRITE_BUF_SIZE] = WRITE_DATA_A;
    uint8_t read_data[READ_BUF_SIZE] = READ_DATA;

    /* Create should fail as Thread_A has no permissions */
    err = tfm_sst_create(asset_uuid);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Create should not succeed for Thread_A");
        return;
    }

    /* Get handle should fail as Thread_A has no permissions */
    err = tfm_sst_get_handle(asset_uuid, &hdl);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Get handle should not succeed for Thread_A");
        return;
    }

    /* Checks handle has not been changed by the call to get handle */
    if (hdl != 0) {
        TEST_FAIL("Handle should not have changed");
        return;
    }

    /* Sets the tfm_sst_buf_t structure */
    buf.data = write_data;
    buf.size = WRITE_BUF_SIZE;
    buf.offset = 0;

    /* The write function uses a valid handle, obtained by the previous test, to
     * check that Thread_A cannot perform the write without the proper access
     * permissions even if it has a valid handle. So the write should fail as
     * Thread_A has no permissions.
     */
    err = tfm_sst_write(shared_handle, &buf);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Write should not succeed for Thread_A");
        return;
    }

    /* Sets the tfm_sst_buf_t structure */
    buf.data = read_data + 3;
    buf.size = WRITE_BUF_SIZE;
    buf.offset = 0;

    /* Read should fail as Thread_A has no permissions */
    err = tfm_sst_read(shared_handle, &buf);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Read should not succeed for Thread_A");
        return;
    }

    /* Checks read_data has not been changed by the call to read */
    if (memcmp(read_data, READ_DATA, READ_BUF_SIZE) != 0) {
        TEST_FAIL("Read buffer should not have changed");
        return;
    }

    /* Get attributes should fail as Thread_A has no permissions */
    err = tfm_sst_get_attributes(shared_handle, &attribs);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Get attributes should not succeed for Thread_A");
        return;
    }

    /* Checks attributes have not been changed by the call to get attributes */
    if (attribs.size_current != 0) {
        TEST_FAIL("Current size of the asset should not have changed");
        return;
    }

    if (attribs.size_max != 0) {
        TEST_FAIL("Max size of the asset should not have changed");
        return;
    }

    /* Delete should fail as Thread_A has no permissions */
    err = tfm_sst_delete(shared_handle);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Delete should not succeed for Thread_A");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests delete policy for SST_ASSET_ID_SHA224_HASH with the following
 *        permissions:
 *          THREAD NAME | APP_ID       | Permissions
 *          ------------|--------------|-----------------------
 *          Thread_B    | SST_APP_ID_1 | REFERENCE, READ, WRITE
 *
 * This test is performed last so that the asset still exists during the
 * preceeding test cases.
 */
TFM_SST_NS_TEST(4007, "Thread_B")
{
    enum tfm_sst_err_t err;

    /* Delete should succeed as Thread_B has WRITE permission */
    err = tfm_sst_delete(shared_handle);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Delete should not fail for Thread_B");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests policy for SST_ASSET_ID_SHA384_HASH with the following
 *        permissions:
 *          THREAD NAME | APP_ID       | Permissions
 *          ------------|--------------|-----------------
 *          Thread_C    | SST_APP_ID_2 | REFERENCE, WRITE
 */
TFM_SST_NS_TEST(4008, "Thread_C")
{
    const uint16_t asset_uuid = SST_ASSET_ID_SHA384_HASH;
    struct tfm_sst_attribs_t attribs;
    struct tfm_sst_buf_t buf;
    enum tfm_sst_err_t err;
    uint8_t write_data[WRITE_BUF_SIZE] = WRITE_DATA_C;
    uint8_t read_data[READ_BUF_SIZE] = READ_DATA;

    /* Create should succeed as Thread_C has WRITE permission */
    err = tfm_sst_create(asset_uuid);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Create should not fail for Thread_C");
        return;
    }

    /* Get handle should succeed as Thread_C has at least one permission. Writes
     * the handle into shared_handle so that the next test has access to a valid
     * handle that it does not have the right to get or use.
     */
    err = tfm_sst_get_handle(asset_uuid, &shared_handle);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Get handle should not fail for Thread_C");
        return;
    }

    /* Sets the tfm_sst_buf_t structure */
    buf.data = write_data;
    buf.size = WRITE_BUF_SIZE;
    buf.offset = 0;

    /* Write should succeed as Thread_C has WRITE permission */
    err = tfm_sst_write(shared_handle, &buf);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Write should not fail for Thread_C");
        return;
    }

    /* Sets the tfm_sst_buf_t structure */
    buf.data = read_data + 3;
    buf.size = WRITE_BUF_SIZE;
    buf.offset = 0;

    /* Read should fail as Thread_C does not have READ permission */
    err = tfm_sst_read(shared_handle, &buf);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Read should not succeed for Thread_C");
        return;
    }

    /* Checks read_data has not been changed by the call to read */
    if (memcmp(read_data, READ_DATA, READ_BUF_SIZE) != 0) {
        TEST_FAIL("Read buffer should not have changed");
        return;
    }

    /* Get attributes should succeed as Thread_C has at least one permission */
    err = tfm_sst_get_attributes(shared_handle, &attribs);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Get attributes should not fail for Thread_C");
        return;
    }

    /* Checks attributes are correct */
    if (attribs.size_current != WRITE_BUF_SIZE) {
        TEST_FAIL("Current size of the asset is incorrect");
        return;
    }

    if (attribs.size_max != SST_ASSET_MAX_SIZE_SHA384_HASH) {
        TEST_FAIL("Max size of the asset is incorrect");
        return;
    }

    ret->val = TEST_PASSED;
}


/**
 * \brief Tests policy for SST_ASSET_ID_SHA384_HASH with the following
 *        permissions:
 *          THREAD NAME | APP_ID       | Permissions
 *          ------------|--------------|------------
 *          Thread_A    | SST_APP_ID_0 | NONE
 */
TFM_SST_NS_TEST(4009, "Thread_A")
{
    const uint16_t asset_uuid = SST_ASSET_ID_SHA384_HASH;
    struct tfm_sst_attribs_t attribs = { 0 };
    struct tfm_sst_buf_t buf;
    enum tfm_sst_err_t err;
    uint32_t hdl = 0;
    uint8_t write_data[WRITE_BUF_SIZE] = WRITE_DATA_A;
    uint8_t read_data[READ_BUF_SIZE] = READ_DATA;

    /* Create should fail as Thread_A has no permissions */
    err = tfm_sst_create(asset_uuid);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Create should not succeed for Thread_A");
        return;
    }

    /* Get handle should fail as Thread_A has no permissions */
    err = tfm_sst_get_handle(asset_uuid, &hdl);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Get handle should not succeed for Thread_A");
        return;
    }

    /* Checks handle has not been changed by the call to get handle */
    if (hdl != 0) {
        TEST_FAIL("Handle should not have changed");
        return;
    }

    /* Sets the tfm_sst_buf_t structure */
    buf.data = write_data;
    buf.size = WRITE_BUF_SIZE;
    buf.offset = 0;

    /* The write function uses a valid handle, obtained by the previous test, to
     * check that Thread_A cannot perform the write without the proper access
     * permissions even if it has a valid handle. So the write should fail as
     * Thread_A has no permissions.
     */
    err = tfm_sst_write(shared_handle, &buf);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Write should not succeed for Thread_A");
        return;
    }

    /* Sets the tfm_sst_buf_t structure */
    buf.data = read_data + 3;
    buf.size = WRITE_BUF_SIZE;
    buf.offset = 0;

    /* Read should fail as Thread_A has no permissions */
    err = tfm_sst_read(shared_handle, &buf);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Read should not succeed for Thread_A");
        return;
    }

    /* Checks read_data has not been changed by the call to read */
    if (memcmp(read_data, READ_DATA, READ_BUF_SIZE) != 0) {
        TEST_FAIL("Read buffer should not have changed");
        return;
    }

    /* Get attributes should fail as Thread_A has no permissions */
    err = tfm_sst_get_attributes(shared_handle, &attribs);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Get attributes should not succeed for Thread_A");
        return;
    }

    /* Checks attributes have not been changed by the call to get attributes */
    if (attribs.size_current != 0) {
        TEST_FAIL("Current size of the asset should not have changed");
        return;
    }

    if (attribs.size_max != 0) {
        TEST_FAIL("Max size of the asset should not have changed");
        return;
    }

    /* Delete should fail as Thread_A has no permissions */
    err = tfm_sst_delete(shared_handle);
    if (err != TFM_SST_ERR_ASSET_NOT_FOUND) {
        TEST_FAIL("Delete should not succeed for Thread_A");
        return;
    }

    ret->val = TEST_PASSED;
}

/**
 * \brief Tests delete policy for SST_ASSET_ID_SHA384_HASH with the following
 *        permissions:
 *          THREAD NAME | APP_ID       | Permissions
 *          ------------|--------------|-----------------
 *          Thread_C    | SST_APP_ID_2 | REFERENCE, WRITE
 *
 * This test is performed last so that the asset still exists during the
 * preceeding test cases.
 */
TFM_SST_NS_TEST(4010, "Thread_C")
{
    enum tfm_sst_err_t err;

    /* Delete should succeed as Thread_C has WRITE permission */
    err = tfm_sst_delete(shared_handle);
    if (err != TFM_SST_ERR_SUCCESS) {
        TEST_FAIL("Delete should not fail for Thread_C");
        return;
    }

    ret->val = TEST_PASSED;
}