/**
 * @file gui_businessReuse_reactionObjPage.h
 *
 */

#ifndef GUI_BUSINESSREUSE_REACTIONOBJPAGE_H
#define GUI_BUSINESSREUSE_REACTIONOBJPAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "iot_lvgl.h"

#include "devDriver_manage.h"

/*********************
 *      DEFINES
 *********************/
#define	PAGEREACTION_REUSE_GLOBAL_NUM					6

#define PAGEREACTION_REUSE_BUSSINESS_RESERVE_NULL		0

#define PAGEREACTION_REUSE_BUSSINESS_RESERVE_BASE		0x10

#define PAGEREACTION_REUSE_BUSSINESS_TIMERSET_IST		(PAGEREACTION_REUSE_BUSSINESS_RESERVE_BASE + 0)
#define PAGEREACTION_REUSE_BUSSINESS_DELAYSET_IST		(PAGEREACTION_REUSE_BUSSINESS_RESERVE_BASE + 1)
#define PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_TEMP	(PAGEREACTION_REUSE_BUSSINESS_RESERVE_BASE + 2)
#define PAGEREACTION_REUSE_BUSSINESS_LINKAGESET_PROX	(PAGEREACTION_REUSE_BUSSINESS_RESERVE_BASE + 3)

#define PAGEREACTION_REUSE_BUSSINESS_RESERVE_MAX		0xA0

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lvGui_businessReuse_reactionObjPageElement_creat(lv_obj_t *obj_Parent, uint8_t pageObjIst, lv_coord_t cy, stt_devDataPonitTypedef *cfgDataCurrent);
void lvGui_businessReuse_reactionObjPageElement_funValConfig_get(uint8_t pageIst, stt_devDataPonitTypedef *dataPointReaction);

/**
 * Create a test screen with a lot objects and apply the given theme on them
 * @param th pointer to a theme
 */


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*GUI_BUSINESSREUSE_REACTIONOBJPAGE_H*/



