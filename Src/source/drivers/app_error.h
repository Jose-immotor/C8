/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
 
/** @file
 *
 * @defgroup app_error Common application error handler
 * @{
 * @ingroup app_common
 *
 * @brief Common application error handler and macros for utilizing a common error handler.
 */

#ifndef APP_ERROR_H__
#define APP_ERROR_H__

#include <stdint.h>
#include <stdbool.h>
//#include "nrf_error.h"

#define APP_ERROR_BASE_NUM      (0x0)       ///< Global error base

#define APP_SUCCESS                           (APP_ERROR_BASE_NUM + 0)  ///< Successful command
#define APP_ERROR_INTERNAL                    (APP_ERROR_BASE_NUM + 1)  ///< Internal Error
#define APP_ERROR_NO_MEM                      (APP_ERROR_BASE_NUM + 2)  ///< No Memory for operation
#define APP_ERROR_NOT_FOUND                   (APP_ERROR_BASE_NUM + 3)  ///< Not found
#define APP_ERROR_INVALID_PARAM               (APP_ERROR_BASE_NUM + 4)  ///< Invalid Parameter
#define APP_ERROR_INVALID_STATE               (APP_ERROR_BASE_NUM + 5)  ///< Invalid state, operation disallowed in this state
#define APP_ERROR_BUSY                        (APP_ERROR_BASE_NUM + 6) ///< Busy

#define UNUSED_VARIABLE(X)  ((void)(X))

/**@brief Function for error handling, which is called when an error has occurred. 
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name. 
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name);

/**@brief Macro for calling error handler function. 
 *
 * @param[in] ERR_CODE Error code supplied to the error handler.
 */
#ifdef DEBUG
#define APP_ERROR_HANDLER(ERR_CODE)                         \
    do                                                      \
    {                                                       \
        app_error_handler((ERR_CODE), __LINE__, (uint8_t*) __FILE__);  \
    } while (0)
#else
#define APP_ERROR_HANDLER(ERR_CODE)                         \
    do                                                      \
    {                                                       \
        app_error_handler((ERR_CODE), 0, 0);  \
    } while (0)
#endif
/**@brief Macro for calling error handler function if supplied error code any other than NRF_SUCCESS. 
 *
 * @param[in] ERR_CODE Error code supplied to the error handler.
 */    
#define APP_ERROR_CHECK(ERR_CODE)                           \
    do                                                      \
    {                                                       \
        const uint32_t LOCAL_ERR_CODE = (ERR_CODE);         \
        if (LOCAL_ERR_CODE != APP_SUCCESS)                  \
        {                                                   \
            APP_ERROR_HANDLER(LOCAL_ERR_CODE);              \
        }                                                   \
    } while (0)    
    
/**@brief Macro for calling error handler function if supplied boolean value is false. 
 *
 * @param[in] BOOLEAN_VALUE Boolean value to be evaluated.
 */
#define APP_ERROR_CHECK_BOOL(BOOLEAN_VALUE)                   \
    do                                                        \
    {                                                         \
        const uint32_t LOCAL_BOOLEAN_VALUE = (BOOLEAN_VALUE); \
        if (!LOCAL_BOOLEAN_VALUE)                             \
        {                                                     \
            APP_ERROR_HANDLER(0);                             \
        }                                                     \
    } while (0)        

#endif // APP_ERROR_H__

/** @} */
