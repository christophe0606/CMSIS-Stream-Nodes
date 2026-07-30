/*
 * Copyright (c) 2026 Arm Limited or its affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef I2S_VSI_H
#define I2S_VSI_H

struct device;

/* The file path must remain valid until it is replaced by another call. */
int vsi_i2s_set_file_path(const struct device *dev, const char *file_path);

#endif /* I2S_VSI_H */
