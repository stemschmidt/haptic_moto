/*
 * Copyright (c) 2024 Cirrus Logic, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <sys/_stdint.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/haptics.h>
#include <zephyr/drivers/haptics/drv2605.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#define LOG_LEVEL 4

LOG_MODULE_REGISTER(main);

volatile bool m_terminate = false;

static struct drv2605_rom_data rom_data = {
    .library = DRV2605_LIBRARY_TS2200_B,  // 3 V motor
    .brake_time = 0,
    .overdrive_time = 0,
    .sustain_neg_time = 0,
    .sustain_pos_time = 0,
    .trigger = DRV2605_MODE_INTERNAL_TRIGGER,
    .seq_regs[0] = 1,
    .seq_regs[1] = 10 | 0x80,
    .seq_regs[2] = 2,
    .seq_regs[3] = 10 | 0x80,
    .seq_regs[4] = 3,
    .seq_regs[5] = 10 | 0x80,
    .seq_regs[6] = 4,
};

static void key_press(struct input_event *evt, void *user_data) {
  // Handle INPUT_KEY_0:
  LOG_INF("key event %d", evt->code);
  if (evt->type == INPUT_EV_KEY && evt->code == INPUT_KEY_0 &&
      evt->value == 1) {
    m_terminate = true;
  }
}

INPUT_CALLBACK_DEFINE(NULL, key_press, NULL);

int main(void) {
  const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(haptic));
  union drv2605_config_data config_data = {};

  int ret;

  if (!dev) {
    LOG_ERR("DRV2605 device not found");
    return -ENODEV;
  } else if (!device_is_ready(dev)) {
    LOG_ERR("DRV2605 device %s is not ready", dev->name);
    return -EIO;
  }

  LOG_INF("Found DRV2605 device %s", dev->name);

  config_data.rom_data = &rom_data;

  ret = drv2605_haptic_config(dev, DRV2605_HAPTICS_SOURCE_ROM, &config_data);
  if (ret < 0) {
    LOG_ERR("Failed to configure ROM event: %d", ret);
    return ret;
  }

  while (m_terminate == false) {
    LOG_INF("Do stuff!");

    ret = haptics_start_output(dev);
    if (ret < 0) {
      LOG_ERR("Failed to start ROM event: %d", ret);
      return ret;
    }
    k_sleep(K_SECONDS(3));
  }

  LOG_INF("Input 0 pressed, bye bye");
  return 0;
}
