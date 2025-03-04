/*
 * Copyright (c) 2024 Cirrus Logic, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdlib.h>
#include <sys/_stdint.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/haptics.h>
#include <zephyr/drivers/haptics/drv2605.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

#define LOG_LEVEL 4

LOG_MODULE_REGISTER(main);

volatile bool m_terminate = false;

K_MUTEX_DEFINE(m_effect_mutex);
static uint8_t m_effect = 0U;

static struct drv2605_rom_data rom_data = {
    .library = DRV2605_LIBRARY_TS2200_B,  // 3 V motor
    .brake_time = 0,
    .overdrive_time = 0,
    .sustain_neg_time = 0,
    .sustain_pos_time = 0,
    .trigger = DRV2605_MODE_INTERNAL_TRIGGER,
    .seq_regs[0] = 1,
    .seq_regs[1] = 52 | 0x80,
    //   .seq_regs[2] = 2,
    //    .seq_regs[3] = 107 | 0x80,
    //    .seq_regs[4] = 3,
    //    .seq_regs[5] = 107 | 0x80,
    //    .seq_regs[6] = 4,
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
    k_mutex_lock(&m_effect_mutex, K_FOREVER);
    if (m_effect != 0) {
      rom_data.seq_regs[1] = m_effect | 0x80;
      m_effect = 0U;
      ret =
          drv2605_haptic_config(dev, DRV2605_HAPTICS_SOURCE_ROM, &config_data);
      if (ret < 0) {
        LOG_ERR("Failed to configure ROM event: %d", ret);
      }
    }
    k_mutex_unlock(&m_effect_mutex);
    LOG_INF("Execute effect %d!", rom_data.seq_regs[1] & ~0x80);

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

static int cmd_drv2605_set_effect(const struct shell *sh, size_t argc,
                                  char **argv) {
  uint8_t effect = (uint8_t)atoi(argv[1]);

  if (effect > 0 && effect < 125) {
    k_mutex_lock(&m_effect_mutex, K_FOREVER);
    m_effect = effect;
    k_mutex_unlock(&m_effect_mutex);
  } else {
    shell_print(sh, "Effect %s not supported", argv[1]);
  }
  return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(drv2605_cmds,
                               SHELL_CMD_ARG(set_effect, NULL, "Set effect",
                                             cmd_drv2605_set_effect, 2, 0),
                               SHELL_SUBCMD_SET_END);

SHELL_CMD_ARG_REGISTER(drv2605, &drv2605_cmds, "drv2605 commands", NULL, 1, 1);
