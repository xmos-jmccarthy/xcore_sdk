// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef APP_CONF_CHECK_H_
#define APP_CONF_CHECK_H_

#if MIC_ARRAY_CONFIG_MIC_COUNT != 2
#error This application requires 2 mics
#endif

#endif /* APP_CONF_CHECK_H_ */
