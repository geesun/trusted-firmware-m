/*
 * Copyright (c) 2017-2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "sst_utils.h"

#include <stdint.h>

#include "secure_fw/core/tfm_secure_api.h"
#include "tfm_sst_defs.h"

void sst_global_lock(void)
{
    /* FIXME: a system call to be added for acquiring lock */
    return;
}

void sst_global_unlock(void)
{
    /* FIXME: a system call to be added for releasing lock */
    return;
}

enum tfm_sst_err_t sst_utils_memory_bound_check(void *addr,
                                                uint32_t size,
                                                int32_t client_id,
                                                uint32_t access)
{
    enum tfm_sst_err_t err;

    /* FIXME:
     * The only check that may be required is whether the caller client
     * has permission to read/write to the memory area specified
     * by addr and size.
     */
    (void) client_id;
    err = tfm_core_memory_permission_check(addr, size, access);

    return err;
}

enum tfm_sst_err_t sst_utils_bound_check_and_copy(uint8_t *src,
                                                  uint8_t *dest,
                                                  uint32_t size,
                                                  int32_t client_id)
{
    enum tfm_sst_err_t bound_check;

    /* src is passed on from untrusted domain, verify boundary */
    bound_check = sst_utils_memory_bound_check(src, size, client_id,
                                               TFM_MEMORY_ACCESS_RO);
    if (bound_check == TFM_SST_ERR_SUCCESS) {
        sst_utils_memcpy(dest, src, size);
    }

    return bound_check;
}

enum tfm_sst_err_t sst_utils_check_contained_in(uint32_t superset_size,
                                                uint32_t subset_offset,
                                                uint32_t subset_size)
{
    /* Check that subset_offset is valid */
    if (subset_offset > superset_size) {
        return TFM_SST_ERR_OFFSET_INVALID;
    }

    /* Check that subset_offset + subset_size fits in superset_size.
     * The previous check passed, so we know that subset_offset <= superset_size
     * and so the right hand side of the inequality cannot underflow.
     */
    if (subset_size > (superset_size - subset_offset)) {
        return TFM_SST_ERR_INCORRECT_SIZE;
    }

    return TFM_SST_ERR_SUCCESS;
}

uint32_t sst_utils_validate_secure_caller(void)
{
    return tfm_core_validate_secure_caller();
}

enum tfm_sst_err_t sst_utils_validate_fid(uint32_t fid)
{
    if (fid == SST_INVALID_FID) {
        return TFM_SST_ERR_UID_NOT_FOUND;
    }

    return TFM_SST_ERR_SUCCESS;
}

/* FIXME: following functions are not optimized and will eventually to be
 *        replaced by system provided APIs.
 */
void sst_utils_memcpy(void *dest, const void *src, uint32_t size)
{
    uint32_t i;
    uint8_t *p_dst = (uint8_t *)dest;
    const uint8_t *p_src = (const uint8_t *)src;

    for (i = size; i > 0; i--) {
        *p_dst = *p_src;
        p_src++;
        p_dst++;
    }
}

void sst_utils_memset(void *dest, const uint8_t pattern, uint32_t size)
{
    uint32_t i;
    uint8_t *p_dst = (uint8_t *)dest;

    for (i = size; i > 0; i--) {
        *p_dst = pattern;
        p_dst++;
    }
}
