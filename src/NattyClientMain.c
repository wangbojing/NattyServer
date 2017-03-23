/*
 *  Author : WangBoJing , email : 1989wangbojing@gmail.com
 * 
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Author. (C) 2016
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

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "NattyProtoClient.h"
#include "NattyProtocol.h"



void ntyUserRecvCb(int len) {
	//int i = 0;
	U8 *buffer = ntyGetRecvBuffer();

	//for (i = 0;i < len;i ++) {
	//ntydbg(" devid:%lld, len:%d, %s ", devid, len, buffer);
	//}
	ntydbg("\n");
}

void ntySendSuccess(int arg) {
	ntydbg("Success\n");
}

void ntySendFailed(int arg) {
	if (arg == STATUS_TIMEOUT)
		ntydbg("STATUS_TIMEOUT\n");
	else if (arg == STATUS_NOEXIST)
		ntydbg("STATUS_NOEXIST\n");
}

void ntyDisconnect(int arg) {
	ntydbg("ntyDisconnect\n");
}

void ntyReconnected(int arg) {
	ntydbg("ntyReconnected\n");
}

void ntyBindResult(int arg) {
	ntydbg(" ntyBindResult --> arg: %d\n", arg);
}

void ntyUnBindResult(int arg) {
	ntydbg(" ntyUnBindResult --> arg: %d\n", arg);
}

void ntyPacketRecv(DEVID fromId, DEVID gId, int arg) { //voice data recv success, arg: length of voice data
	ntydbg(" ntyUnBindResult --> arg: %d\n", arg);

	U8 *pBuffer = ntyGetRecvBigBuffer();	//
}

void ntyPacketSuccess(int arg) {
	ntydbg(" ntyUnBindResult --> arg: %d\n", arg);
}

void ntyLoginAckResult(U8 *json, int length) {
	ntydbg(" ntyLoginAckResult:%s\n", json);
}

void ntyHeartBeatAckResult(int status) {
	ntydbg(" ntyLoginAckResult:%d\n", status);
}

void ntyLogoutAckResult(int status) {
	ntydbg(" ntyLogoutAckResult:%d\n", status);
}

void ntyTimeAckResult(U8 *json, int length) {
	
}

void ntyICCIDAckResult(U8 *json, int length) {
	
}

void ntyCommonReqResult(DEVID fromId, U8 *json, int length) {
	ntydbg(" ntyCommonReqResult : %lld\n", fromId);
	ntydbg(" ntyCommonReqResult : %s\n", json);
}

void ntyCommonAckResult(U8 *json, int length) {
	
}

void ntyVoiceDataAckResult(int status) {
	//voice data send success
	ntydbg(" ntyVoiceDataAckResult: %d\n", status);
}

void ntyOfflineMsgAckResult(U8 *json, int length) {
	ntydbg(" ntyOfflineMsgAckResult:%s\n", json);
}

void ntyLocationPushResult(U8 *json, int length) {
	ntydbg(" ntyLocationPushResult:%s\n", json);
}

void ntyWeatherPushResult(U8 *json, int length) {
	ntydbg(" ntyWeatherPushResult:%s\n", json);
}

void ntyDataRoute(DEVID fromId, U8 *json, int length) {
	ntydbg(" ntyDataRoute:%s\n", json);
}

void ntyDataResult(U8 *json, int length) {
	ntydbg(" ntyDataResult:%d\n", length);
}

void ntyVoiceBroadCastResult(DEVID fromId, U8 *json, int length) {
	ntydbg(" ntyVoiceBroadCastResult:%s\n", json);
}

void ntyLocationBroadCastResult(DEVID fromId, U8 *json, int length) {
	ntydbg(" ntyLocationBroadCastResult:%s\n", json);
}

void ntyCommonBoradCastResult(DEVID fromId, U8 *json, int length) {
	ntydbg(" ntyCommonBoradCastResult:%s\n", json);
}


void ntyLocationPush(U8 *arg, int length) {
	ntydbg(" ntyPush --> length: %s\n", arg);
}



DEVID g_devid = 0x352315052834187;

int main() {
	DEVID AppId = 10794;
	DEVID aid = 0;
	
	int n = 0, length, i, result = 0;
	int ch;
	U8 tempBuf[CLIENT_BUFFER_SIZE] = {0};
	const char *url =  "GET http://apilocate.amap.com/position?accesstype=1&imei=352315052834187&macs=30:FC:68:B9:E6:E6,-48,TP-LINK_E6E6_JUGUI|50:BD:5F:00:5F:62,-79,TP-LINK_5F62|28:10:7B:F6:E1:BA,-80,joshen?D|C0:61:18:90:1D:64,-86,ttt222????|D4:EE:07:3E:C8:8A,-87,HiWiFi_3EC88A|&output=xml&key=81040f256992a218a8a20ffb7f13ba9f HTTP/1.1";
	
	//ntydbg(" Press DevId <1 or 2>: ");   	
	//n = scanf("%lld", &AppId);
	
	ntySetProxyCallback(ntyUserRecvCb);
	ntySetSendFailedCallback(ntySendFailed);
	ntySetSendSuccessCallback(ntySendSuccess);
	ntySetProxyDisconnect(ntyDisconnect);
	ntySetProxyReconnect(ntyReconnected);
	ntySetBindResult(ntyBindResult);
	ntySetUnBindResult(ntyUnBindResult);
	ntySetPacketRecv(ntyPacketRecv);
	ntySetPacketSuccess(ntyPacketSuccess);
	ntySetLoginAckResult(ntyLoginAckResult);
	ntySetLocationPushResult(ntyLocationPush);
	ntySetDataResult(ntyDataResult);

	
	//ntySetDevId(did);
	ntySetDevId(g_devid);
	
	ntyStartupClient(&result);
	sleep(5);
	
	//ntyBindClient(0xEDFF12342345613);
	//ntyUnBindClient(0xEDFF12342345613);
#if 0
	int count = 0;
	DEVID *list = ntyGetFriendsList(&count);
	for (i = 0;i < count;i ++) {
		ntydbg(" %d --> %lld\n", i+1, *(list+i));
	}
	aid = *list;
	ntyReleaseFriendsList(&list);
#endif
	//while(1);
#if 1
	while(1) {
		ntydbg("Proxy Please send msg:\n");
#if 0
		char *ptr = fgets(tempBuf, CLIENT_BUFFER_SIZE, stdin);
#else
		/*
		//{"results":[{"location":{"id":"WX4FBXXFKE4F","name":"±±??","country":"CN","path": "±±??,±±??,?D1ú","timezone":"Asia/Shanghai","timezone_offset":"+08:00"},"daily":[{"date":"2017-02-15","text_day":"?à??","code_day":"4","text_night":"ò?","code_night":"9","high":"8","low":"-1","precip":"","wind_direction":"??","wind_direction_degree":"180","wind_speed":"10","wind_scale":"2"},{"date":"2017-02-16","text_day":"?à??","code_day":"4","text_night":"?à??","code_night":"4","high":"7","low":"-4","precip":"","wind_direction":"?T3?D?·??ò","wind_direction_degree":"","wind_speed":"20","wind_scale":"4"},{"date":"2017-02-17","text_day":"??","code_day":"0","text_night":"??","code_night":"0","high":"5","low":"-4","precip":"","wind_direction":"??","wind_direction_degree":"180","wind_speed":"10","wind_scale":"2"}],"last_update":"2017-02-15T18:00:00+08:00"}]}
		char *weatherStr = "{\"results\":[{\"location\":{\"id\":\"WX4FBXXFKE4F\",\"name\":\"±±??\",\"country\":\"CN\",\"path\": \"±±??,±±??,?D1ú\",\"timezone\":\"Asia/Shanghai\",\"timezone_offset\":\"+08:00\"},\"daily\":[{\"date\":\"2017-02-15\",\"text_day\":\"?à??\",\"code_day\":\"4\",\"text_night\":\"ò?\",\"code_night\":\"9\",\"high\":\"8\",\"low\":\"-1\",\"precip\":\"\",\"wind_direction\":\"??\",\"wind_direction_degree\":\"180\",\"wind_speed\":\"10\",\"wind_scale\":\"2\"},{\"date\":\"2017-02-16\",\"text_day\":\"?à??\",\"code_day\":\"4\",\"text_night\":\"?à??\",\"code_night\":\"4\",\"high\":\"7\",\"low\":\"-4\",\"precip\":\"\",\"wind_direction\":\"?T3?D?·??ò\",\"wind_direction_degree\":\"\",\"wind_speed\":\"20\",\"wind_scale\":\"4\"},{\"date\":\"2017-02-17\",\"text_day\":\"??\",\"code_day\":\"0\",\"text_night\":\"??\",\"code_night\":\"0\",\"high\":\"5\",\"low\":\"-4\",\"precip\":\"\",\"wind_direction\":\"??\",\"wind_direction_degree\":\"180\",\"wind_speed\":\"10\",\"wind_scale\":\"2\"}],\"last_update\":\"2017-02-15T18:00:00+08:00\"}]}";
		U16 weather_len = (U16)strlen(weatherStr);
		tempBuf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
		tempBuf[NTY_PROTO_DEVTYPE_IDX] = NTY_PROTO_CLIENT_WATCH;
		tempBuf[NTY_PROTO_PROTOTYPE_IDX] = 0x08;
		tempBuf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_WEATHER_ASYNCREQ;
		memcpy(tempBuf+4, &g_devid, sizeof(DEVID));
		memcpy(tempBuf+12, &weather_len, 2);
		memcpy(tempBuf+14, weatherStr, weather_len);
		*/

		/*
		char *electricFenceStr = "{\"results\":[{\"location\":{\"id\":\"WX4FBXXFKE4F\",\"name\":\"±±??\",\"country\":\"CN\",\"path\": \"±±??,±±??,?D1ú\",\"timezone\":\"Asia/Shanghai\",\"timezone_offset\":\"+08:00\"},\"daily\":[{\"date\":\"2017-02-15\",\"text_day\":\"?à??\",\"code_day\":\"4\",\"text_night\":\"ò?\",\"code_night\":\"9\",\"high\":\"8\",\"low\":\"-1\",\"precip\":\"\",\"wind_direction\":\"??\",\"wind_direction_degree\":\"180\",\"wind_speed\":\"10\",\"wind_scale\":\"2\"},{\"date\":\"2017-02-16\",\"text_day\":\"?à??\",\"code_day\":\"4\",\"text_night\":\"?à??\",\"code_night\":\"4\",\"high\":\"7\",\"low\":\"-4\",\"precip\":\"\",\"wind_direction\":\"?T3?D?·??ò\",\"wind_direction_degree\":\"\",\"wind_speed\":\"20\",\"wind_scale\":\"4\"},{\"date\":\"2017-02-17\",\"text_day\":\"??\",\"code_day\":\"0\",\"text_night\":\"??\",\"code_night\":\"0\",\"high\":\"5\",\"low\":\"-4\",\"precip\":\"\",\"wind_direction\":\"??\",\"wind_direction_degree\":\"180\",\"wind_speed\":\"10\",\"wind_scale\":\"2\"}],\"last_update\":\"2017-02-15T18:00:00+08:00\"}]}";
		U16 electricFence_len = (U16)strlen(electricFenceStr);
		tempBuf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
		tempBuf[NTY_PROTO_DEVTYPE_IDX] = NTY_PROTO_CLIENT_WATCH;
		tempBuf[NTY_PROTO_PROTOTYPE_IDX] = 0x04;
		tempBuf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_COMMON_REQ;
		memcpy(tempBuf+4, &g_devid, sizeof(DEVID));
		memcpy(tempBuf+12, &electricFence_len, 2);
		memcpy(tempBuf+14, electricFenceStr, electricFence_len);
		*/

		//1. Config2ù×÷
		//char *json_str_config = "{\"IMEI\":\"355637052788650\",\"Action\":\"Get\",\"Category\":\"Config\"}";
		//char *json_str = "{\"IMEI\":\"355637052788650\",\"Category\":\"Schedule\",\"Action\":\"Add\",\"Schedule\":{\"Daily\":\"Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday\",\"Time\":\"18:00:00\",\"Details\":\"chifan\"}}";
		//U16 json_len = (U16)strlen(json_str);
		//tempBuf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
		//tempBuf[NTY_PROTO_DEVTYPE_IDX] = NTY_PROTO_CLIENT_WATCH;
		//tempBuf[NTY_PROTO_PROTOTYPE_IDX] = PROTO_ROUTE;
		//tempBuf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_COMMON_REQ;
		//tempBuf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_DATA_ROUTE;



		//char *json_str = "{\"IMEI\":\"355637052788650\",\"Category\":\"Efence\",\"Efence\":{\"Num\":\"3\",\"Points\":[\"113.2409402,23.1326885\",\"113.2409412,23.1326895\",\"113.2409408,23.1326890\"]}}";

		//char *json_str = "{\"IMEI\":\"355637052788650\",\"Category\":\"RunTime\",\"RunTime\":{\"AutoConnection\":\"1\",\"LossReport\":\"1\",\"LightPanel\":\"30\",\"WatchBell\":\"11,11\",\"TagetStep\":\"5000\"}}";

		//char *json_str = "{\"IMEI\":\"355637052788650\",\"Category\":\"Schedule\",\"Action\":\"Add\",\"Schedule\":{\"Daily\":\"Monday|Tuesday| Wednesday|Thursday|Friday|Saturday|Sunday\",\"Time\":\"18:00:00\",\"Details\":\"3?·1\"}}";
		
		char *json_str1 = "{\"IMEI\":\"355637052788650\",\"Category\":\"TimeTables\",\"TimeTables\":[{\"Daily\":\"Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday\",\"Morning\":{\"Status\":\"On\",\"StartTime\":\"08:00:00\",\"EndTime\":\"12:00:00\"},\"Afternoon\":{\"Status\":\"On\",\"StartTime\":\"13:00:00\",\"EndTime\":\"18:00:00\"}}]}";
		char *json_str = "{\"IMEI\":\"355637052788650\",\"Category\":\"LAB\",\"LAB\":{\"Bts\":\"460,01,40977,2205409,-65\",\"Nearbts\":[{\"Cell\":\"460,01,40977,2205409\",\"Signal\":\"-65\"},{\"Cell\":\"460,01,40977,2205409\",\"Signal\":\"-65\"},{\"Cell\":\"460,01,40977,2205409\",\"Signal\":\"-65\"}]}}";



#if 0
		U8 tempBuf2[4] = {0x01,0x01,0x01,0x01};
		memcpy(tempBuf+4, &g_devid, sizeof(DEVID));
		memcpy(tempBuf+12, tempBuf2, 4);
		memcpy(tempBuf+16, &g_devid, sizeof(DEVID));
		memcpy(tempBuf+24, &json_len, 2);
		memcpy(tempBuf+26, json_str, json_len);
		ntydbg(" json_str:%s\n", json_str);
		ntydbg(" tempBuf:%s\n", tempBuf+26);
		
#endif

		
#endif
#if 0
		if (ntyGetNetworkStatus() == -1) {
			ntydbg("Startup Client\n");
			ntyStartupClient();
			continue;
		}
		if (tempBuf[0] == 'c') {
			ntydbg("Shutdown Client\n");
			ntyShutdownClient();
			continue;
		}
#endif


		//memset(tempBuf, 0, RECV_BUFFER_SIZE);
		//memcpy(tempBuf, url, strlen(url));
		//int len = strlen(tempBuf+26);
		//ntydbg("tempBuf:%s, len:%d\n", tempBuf+26, len);
#if 0
		ntySendMassDataPacket(tempBuf, len-1);
#else
		//ntySendDataPacket(aid, tempBuf, len);
		//ntyCommonReqClient(g_devid, json_str, json_len);
		//ntyDataRouteClient(g_devid,json_str,json_len);

		//char *json_dataroute = "{\"IMEI\":\"352315052834187\",\"Action\":\"Get\",\"Category\":\"Config\"}";
		//ntydbg("Request DataRoute : %s\n", json_dataroute);
		//ntyDataRouteClient(g_devid,json_dataroute,strlen(json_dataroute));
		//sleep(20);
		//break;

		//char *json_iccid = "{\"IMEI\":\"355637052788650\",\"ICCID\":\"89860116425100650085\"}";
		//ntydbg("Request Weather : %s\n", json_iccid);
		//ntyICCIDReqClient(g_devid, json_iccid, strlen(json_iccid));
		//sleep(20);
		//break;
		//char *json_common = "{\"Action\":\"Update\",\"Category\":\"Schedule\",\"IMEI\":\"355637053995130\",\"Id\":\"41\",\"Schedule\":{\"Daily\":\"Monday|Wednesday|Thursday|Sunday\",\"Details\":\"给力\",\"Time\":\"17:30:0tion\":\",,255\"}}}";

		//char *json_common = "{\"Action\":\"Update\",\"Category\":\"Schedule\",\"IMEI\":\"355637053995130\",\"Id\":\"40\",\"Schedule\":{\"Daily\":\"Tuesday|Wednesday|Thursday|Saturday\",\"Details\":\"给力\",\"Time\":\"17:28:00\"}}";
		//char *json_common = "{\"Category\":\"RunTime\",\"IMEI\":\"355637053995130\",\"RunTime\":{\"AutoConnection\":\"0\"}}";
		//char *json_common = "{\"IMEI\":\"355637052788650\",\"Category\":\"TimeTables\",\"TimeTables\":[{\"Daily\":\"\",\"Morning\":{\"Status\":\"1\",\"StartTime\":\"08:00:00\",\"EndTime\":\"12:00:00\"},\"Afternoon\":{\"Status\":\"1\",\"StartTime\":\"13:00:00\",\"EndTime\":\"18:00:00\"}}]}";
		//char *json_common = "{\"IMEI\":\"355637052788650\",\"Category\":\"TimeTables\",\"TimeTables\":[{\"Daily\":\"Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday\",\"Morning\":{\"Status\":\"1\",\"StartTime\":\"08:00:00\",\"EndTime\":\"12:00:00\"},\"Afternoon\":{\"Status\":\"1\",\"StartTime\":\"13:00:00\",\"EndTime\":\"18:00:00\"}}]}";
		//char *json_common = "{\"IMEI\":\"352315052834187\",\"Category\":\"Turn\",\"Turn\":{\"Status\":\"0\",\"On\":{\"Time\":\"08:00:00\"},\"Off\":{\"Time\":\"18:00:00\"}}}";
		//char *json_common="{\"Schedule\":{\"Daily\":\"Wednesday|Thursday|Friday|Saturday\",\"Details\":\"%E4%BE%A7%E9%9D%A2\",\"Time\":\"21:14:14\"},\"Category\":\"Schedule\",\"IMEI\":\"352315052834187\",\"Action\":\"Update\",\"Id\":\"94\"}";
		//char *json_common = "{\"IMEI\":\"352315052834187\",\"Category\":\"Schedule\",\"Action\":\"Add\",\"Schedule\":{\"Daily\":\"Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday\",\"Time\":\"18:00:00\",\"Details\":\"3\"}}";

		char *json_common = "{\"Action\":\"Add\",\"Category\":\"Contacts\",\"Contacts\":{\"Image\":\"8\",\"Name\":\"%E9%BB%84%E6%B5%B7\",\"Tel\":\"13695287465\"},\"IMEI\":\"355637053995130\"}";
		//char *json_common = "{\"Contact\":{\"Tel\":\"15218293347\",\"Name\":\"%E5%A4%96%E5%85%AC\",\"Image\":\"4\"},\"Category\":\"Contacts\",\"Action\":\"Add\",\"IMEI\":\"352315052834187\"}}";
		//char *json_common = "{\"IMEI\":\"355637053995130\",\"Action\":\"Delete\",\"Category\":\"Efence\",\"Index\":\"1\"}";
		ntyCommonReqClient(g_devid,json_common,strlen(json_common));
		sleep(30);
		
		//char *json_location = "{\"IMEI\":\"135790246811220\",\"Category\":\"WIFI\",\"WIFI\":[{\"SSID\":\"TP-LINK_42ED\",\"MAC\":\"F4:83:CD:3A:42:ED\",\"V\":\"-37\"},{\"SSID\":\"EXT\",\"MAC\":\"04:95:E6:22:DA:48\",\"V\":\"-52\"}]}";
		//ntydbg("%s\n", json_location);
		//ntyLocationReqClient(g_devid, json_location, strlen(json_location));
		//sleep(20);

		//char *json_weather = "{\"IMEI\":\"355637052788650\",\"Category\":\"Weather\",\"Bts\":\"460,01,40977,2205409,-65\"}";
		//ntydbg("Request Weather : %s\n", json_weather);
		//ntyWeatherReqClient(g_devid, json_weather, strlen(json_weather));
		//sleep(20);
		
		break;
		//sleep(3000);
#endif
	}
#endif
	getchar();
}


