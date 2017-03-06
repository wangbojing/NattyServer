/*
 *  Author : luopeng , email : hibroad@hotmail.com
 * 
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Author. (C) 2017
 * 
 *
 
****       *****
  ***        *
  ***        *                         *               *
  * **       *                         *               *
  * **       *                         *               *
  *  **      *                        **              **
  *  **      *                       ***             ***
  *   **     *       ******       ***********     ***********    *****    *****
  *   **     *     **     **          **              **           **      **
  *    **    *    **       **         **              **           **      *
  *    **    *    **       **         **              **            *      *
  *     **   *    **       **         **              **            **     *
  *     **   *            ***         **              **             *    *
  *      **  *       ***** **         **              **             **   *
  *      **  *     ***     **         **              **             **   *
  *       ** *    **       **         **              **              *  *
  *       ** *   **        **         **              **              ** *
  *        ***   **        **         **              **               * *
  *        ***   **        **         **     *        **     *         **
  *         **   **        **  *      **     *        **     *         **
  *         **    **     ****  *       **   *          **   *          *
*****        *     ******   ***         ****            ****           *
                                                                       *
                                                                      *
                                                                  *****
                                                                  ****


 *
 */



#ifndef __NATTY_USER_PROTOCOL_H__
#define __NATTY_USER_PROTOCOL_H__


#define NATTY_USER_PROTOCOL__IMEI			"IMEI"
#define NATTY_USER_PROTOCOL__CATEGORY		"Category"

#define NATTY_USER_PROTOCOL__WIFI			"WIFI"
#define NATTY_USER_PROTOCOL__SSID			"SSID"
#define NATTY_USER_PROTOCOL__MAC			"MAC"
#define NATTY_USER_PROTOCOL__V				"V"

#define NATTY_USER_PROTOCOL__LAB			"LAB"
#define NATTY_USER_PROTOCOL__BTS			"bts"
#define NATTY_USER_PROTOCOL__NEARBTS		"Nearbts"
#define NATTY_USER_PROTOCOL__CELL			"Cell"
#define NATTY_USER_PROTOCOL__SIGNAL			"Signal"

#define NATTY_USER_PROTOCOL__CONFIG			"Config"
#define NATTY_USER_PROTOCOL__POWER			"Power"
#define NATTY_USER_PROTOCOL__STEPS			"Steps"
#define NATTY_USER_PROTOCOL__PHONENUM		"PhoneNum"

#define NATTY_USER_PROTOCOL__RESULTS		"Results"
#define NATTY_USER_PROTOCOL__RESULT			"Result"
#define NATTY_USER_PROTOCOL__TYPE			"Type"
#define NATTY_USER_PROTOCOL__RADIUS			"Tadius"
#define NATTY_USER_PROTOCOL__LOCATION		"Location"

#define NATTY_USER_PROTOCOL__STATUS 		"Status"
#define NATTY_USER_PROTOCOL__INFO 			"Info"
#define NATTY_USER_PROTOCOL__INFOCODE 		"Infocode"
#define NATTY_USER_PROTOCOL__DESC 			"Desc"
#define NATTY_USER_PROTOCOL__COUNTRY 		"Country"
#define NATTY_USER_PROTOCOL__CITY 			"City"
#define NATTY_USER_PROTOCOL__CITYCODE 		"Citycode"
#define NATTY_USER_PROTOCOL__ADCODE 		"Adcode"
#define NATTY_USER_PROTOCOL__ROAD 			"Road"
#define NATTY_USER_PROTOCOL__STREET 		"Street"
#define NATTY_USER_PROTOCOL__POI 			"Poi"

#define NATTY_USER_PROTOCOL__EFENEC			"Efence"
#define NATTY_USER_PROTOCOL__NUM 			"Num"
#define NATTY_USER_PROTOCOL__POINTS			"Points"

#define NATTY_USER_PROTOCOL__RUNTIME 		"RunTime"
#define NATTY_USER_PROTOCOL__AUTOCONNECTION "AutoConnection"
#define NATTY_USER_PROTOCOL__LOSSREPORT 	"LossReport"
#define NATTY_USER_PROTOCOL__LIGHTPANEL 	"LightPanel"
#define NATTY_USER_PROTOCOL__WATCHBELL 		"WatchBell"
#define NATTY_USER_PROTOCOL__TAGETSTEP 		"TagetStep"

#define NATTY_USER_PROTOCOL__TURN 			"Turn"
#define NATTY_USER_PROTOCOL__ON 			"On"
#define NATTY_USER_PROTOCOL__OFF 			"Off"
#define NATTY_USER_PROTOCOL__TIME 			"Time"

#define NATTY_USER_PROTOCOL__ID 			"Id"
#define NATTY_USER_PROTOCOL__ACTION 		"Action"
#define NATTY_USER_PROTOCOL__SCHEDULE 		"Schedule"
#define NATTY_USER_PROTOCOL__DAILY 			"Daily"
#define NATTY_USER_PROTOCOL__DETAILS 		"Details"

#define NATTY_USER_PROTOCOL__TIMETABLES		"TimeTables"
#define NATTY_USER_PROTOCOL__MORNING 		"Morning"
#define NATTY_USER_PROTOCOL__AFTERNOON 		"Afternoon"
#define NATTY_USER_PROTOCOL__STARTTIME 		"StartTime"
#define NATTY_USER_PROTOCOL__ENDTIME 		"EndTime"



#include <stdio.h>


typedef struct _WIFIItem {
	const char *SSID;
	const char *MAC;
	const char *V;
} WIFIItem;

typedef struct _WIFIReq {
	const char *IMEI;
	const char *category;
	WIFIItem *pWIFI;
} WIFIReq;


typedef struct _Nearbts {
	const char *cell;
	const char *signal;
} Nearbts;

typedef struct _LABItem {
	const char *bts;
	Nearbts *pNearbts;
} LABItem;

typedef struct _LABReq {
	const char *IMEI;
	const char *category;
	LABItem lab;
} LABReq;



typedef struct _AMapResult {
	const char *type;
	const char *location;
	const char *radius;
	const char *desc;
	const char *country;
	const char *province;
	const char *city;
	const char *citycode;
	const char *adcode;
	const char *road;
	const char *street;
	const char *poi;
} AMapResult;

typedef struct _AMap {
	const char *status;
	const char *info;
	const char *infocode;
	AMapResult result;
} AMap;



typedef struct _WeatherLocation {
	const char *id;
	const char *name;
	const char *country;
	const char *path;
	const char *timezone;
	const char *timezone_offset;
} WeatherLocation;

typedef struct _WeatherDaily {
	const char *date;
	const char *text_day;
	const char *code_day;
	const char *code_night;
	const char *high;
	const char *low;
	const char *precip;
	const char *wind_direction;
	const char *wind_direction_degree;
	const char *wind_speed;
	const char *wind_scale;
} WeatherDaily;

typedef struct _WeatherResults {
	WeatherLocation location;
	WeatherDaily *pDaily;		
	const char *last_update;
} WeatherResults;

typedef struct _WeatherAck {
	size_t size;
	WeatherResults *pResults;
} WeatherAck;



typedef struct _CommonResult {
	const char *status;
} CommonResult;

typedef struct _CommonAck {
	CommonResult result;
} CommonAck;

typedef struct _CommonReq {
	const char *IMEI;
	const char *action;
	const char *category; //Config  Power  Signal
} CommonReq;


typedef struct _ConfigItem {
	const char *power;
	const char *signal;
	const char *steps;
	const char *phone_num;
	const char *location;
} ConfigItem;

typedef struct _ConfigResults {
	const char *IMEI;
	const char *category;
	ConfigItem config;
} ConfigResults;

typedef struct _ConfigAck {
	ConfigResults results;
} ConfigAck;


typedef struct _PowerResults {
	const char *IMEI;
	const char *category;
	const char *power;
} PowerResults;

typedef struct _PowerAck {
	PowerResults results;
} PowerAck;


typedef struct _SignalResults {
	const char *IMEI;
	const char *category;
	const char *signal;
} SignalResults;

typedef struct _SignalAck {
	SignalResults results;
} SignalAck;


typedef struct _LocationResults {
	const char *IMEI;
	const char *category;
	const char *type;
	const char *radius;
	const char *location;
} LocationResults;
typedef struct _LocationAck {
	LocationResults results;
} LocationAck;



typedef struct _EfencePoints {
	const char *point;
} EfencePoints;
typedef struct _EfenceItem {
	const char *num;
	size_t size;
	EfencePoints *pPoints;
} EfenceItem;
typedef struct _EfenceReq {
	const char *IMEI;
	const char *category;
	EfenceItem efence;
} EfenceReq, EfenceResult;
typedef struct _EfenceAck {
	EfenceResult result;
} EfenceAck;


typedef struct _RunTimeItem {
	const char *auto_connection;
	const char *loss_report;
	const char *light_panel;
	const char *watch_bell;
	const char *taget_step;
} RunTimeItem;
typedef struct _RunTimeReq {
	const char *IMEI;
	const char *category;
	RunTimeItem runtime;
} RunTimeReq, RunTimeResult;
typedef struct _RunTimeAck {
	RunTimeResult result;
} RunTimeAck;



typedef struct _TurnOn {
	const char *time;
} TurnOn;
typedef struct _TurnOff {
	const char *time;
} TurnOff;
typedef struct _TurnItem {
	TurnOn on;
	TurnOff off;
} TurnItem;
typedef struct _TurnReq {
	const char *IMEI;
	const char *category;
	TurnItem turn;
} TurnReq, TurnResult;
typedef struct _TurnAck {
	TurnResult result;
} TurnAck;



typedef struct _ScheduleItem {
	const char *id;
	const char *daily;
	const char *time;
	const char *details;
} ScheduleItem;

typedef struct _AddScheduleReq {
	const char *IMEI;
	const char *category;
	const char *action;
	ScheduleItem schedule;
} AddScheduleReq;

typedef struct _DelScheduleReq {
	const char *IMEI;
	const char *category;
	const char *action;
	const char *id;
} DelScheduleReq;

typedef struct _UpdateScheduleReq {
	const char *IMEI;
	const char *category;
	const char *action;
	const char *id;
	ScheduleItem schedule;
} UpdateScheduleReq;

typedef struct _ScheduleResults {
	const char *IMEI;
	const char *category;
	const char *num;
	size_t size;
	ScheduleItem *pSchedule;
} ScheduleResults;

typedef struct _ScheduleAck {
	ScheduleResults results;
} ScheduleAck;



typedef struct _Morning {
	const char *status;
	const char *startTime;
	const char *endTime;
} Morning;

typedef struct _Afternoon {
	const char *status;
	const char *startTime;
	const char *endTime;
} Afternoon;

typedef struct _TimeTablesItem {
	const char *daily;
	Morning morning;
	Afternoon afternoon;
} TimeTablesItem;

typedef struct _TimeTablesReq {
	const char *IMEI;
	const char *category;
	size_t size;
	TimeTablesItem *pTimeTables;
} TimeTablesReq, TimeTablesResults;

typedef struct _TimeTablesAck {
	TimeTablesResults results;
} TimeTablesAck;


#endif


