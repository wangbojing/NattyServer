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


#include "NattyNodeAgent.h"

#include <string.h>


extern void *ntyPacketCtor(void *_self, va_list *params);
extern void *ntyPacketDtor(void *_self);
extern void ntyPacketSetSuccessor(void *_self, void *_succ);
extern void *ntyPacketGetSuccessor(const void *_self);

const ntyNode ntyNodeTable[] = {
	{"Power", 5},
	{"Signal", 6},
	{"PhoneBook", 9},
	{"FamilyNumber", 12},
	{"Fallen", 6},
	{"GPS", 3},
	{"Wifi", 4},
	{"Lab", 3},
	{"Step", 4},
	{"HeartRate", 9},
	{"Location", 8},
	{"Status", 6},
};



int ntyNodeCompare(const U8 *taken) {
	int node = 0;

	for (node = DB_NODE_START;node <= DB_NODE_END;node ++) {
		if (!strcmp((const char*)taken, (const char*)ntyNodeTable[node].name)) {
			return node;
		}
	}
	return -1;
}

U8 ntyCompareGetOpera(const U8 *taken) {
	if (!strcmp(taken, "GET") || !strcmp(taken, "Get") || !strcmp(taken, "get")) {
		return 1;
	}
	return 0;
}

U8 ntyCompareSetOpera(const U8 *taken) {
	if (!strcmp(taken, "SET") || !strcmp(taken, "Set") || !strcmp(taken, "set")) {
		return 1;
	}
	return 0;
}

static void ntySetSuccessor(void *_filter, void *_succ) {
	NodeFilter **filter = _filter;
	if (_filter && (*filter) && (*filter)->setSuccessor) {
		(*filter)->setSuccessor(_filter, _succ);
	}
}

static void ntyHandleNodeRequest(const void *_filter, unsigned char **table, int count, C_DEVID id) {
	NodeFilter * const *filter = _filter;
	if (_filter && (*filter) && (*filter)->handleRequest) {
		(*filter)->handleRequest(_filter, table, count, id);
	}
}

static void ntyGetOperaHandleRequest(const void *_self, U8 **table, int count, C_DEVID id) {
	//const UdpClient *client = obj;
	if (ntyCompareGetOpera(table[0])) {
		U8 devIdBuf[9] = {0};
		int index = ntyNodeCompare(table[1]);
		if (index == -1) {
			ntylog(" Opera: %s %s, length:%ld is not Exist\n", table[0], table[1],strlen(table[1]));
			return ;
		}

		switch (index) {
			case DB_NODE_POWER: {
				break;
			}
			case DB_NODE_SIGNAL: {
				break;
			}
			case DB_NODE_PHONEBOOK:
				break;
			case DB_NODE_FAMILYNUMBER:
				break;
			case DB_NODE_FALLEN:
				break;
			case DB_NODE_LOCATION:
			case DB_NODE_GPS:
				break;
			case DB_NODE_WIFI:
				break;
			case DB_NODE_LAB:
				break;
			case DB_NODE_STEP: {
				break;
			}
			case DB_NODE_HEARTRATE: {
				break;
			}
			case DB_NODE_STATUS: {
				break;
			}
			default:
				break;
		}
	
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const NodeFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, table, count, id);
	} else {
		fprintf(stderr, "Can't deal with: %s\n", table[0]);
	}

}

static const NodeFilter ntyGetOperaFilter = {
	sizeof(NodePacket),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntyGetOperaHandleRequest,
};

static void ntySetOperaHandleRequest(const void *_self, U8 **table, int count, C_DEVID id) {
	if (ntyCompareSetOpera(table[0])) {
		int index = ntyNodeCompare(table[1]);
		if (index == -1) {
			ntylog(" Opera: %s %s, length:%ld is not Exist\n", table[0], table[1],strlen(table[1]));
			return ;
		}
	
		switch (index) {
			case DB_NODE_PHONEBOOK:
				break;
			case DB_NODE_FAMILYNUMBER: 
			case DB_NODE_FALLEN:
				break;
			case DB_NODE_STEP: {
				if(ntyIsAvailableNum(table[2])) {					
					int value = atoi(table[2]);
					ntyExecuteStepInsertHandle(id, value);
				}
				break;
			}
			case DB_NODE_HEARTRATE: {
				if(ntyIsAvailableNum(table[2])) {					
					int value = atoi(table[2]);
					ntyExecuteHeartRateInsertHandle(id, value);
				}
				break;
			}
			case DB_NODE_GPS:
			case DB_NODE_WIFI:
			case DB_NODE_LAB:
			case DB_NODE_LOCATION: {
				unsigned char **pTable = (unsigned char**)malloc(sizeof(unsigned char**));
				int Count = 0, i;

				ntylog(" table:%s\n", table[2]);
				ntySeparation(':', table[2], strlen(table[2]), &pTable, &Count);
				if (Count == 3) {
					U8 *lat = pTable[0];
					U8 *lng = pTable[1];
					U8 type = 2;
					if (!strcmp(pTable[2], "1")) {
						type = 1;
					} else if (!strcmp(pTable[2], "2")) {
						type = 2;
					} else if (!strcmp(pTable[2], "3")) {
						type = 3;
					} 
					ntyExecuteLocationInsertHandle(id, lng, lat, type, "");
				}

				ntyFreeTable(&pTable, Count);
				free(pTable);
				
				break;
			}
			default:
				break;
		}
		
	} else if (ntyPacketGetSuccessor(_self) != NULL) {
		const NodeFilter * const *succ = ntyPacketGetSuccessor(_self);
		(*succ)->handleRequest(succ, table, count, id);
	} else {
		fprintf(stderr, "Can't deal with: %s\n", table[0]);
	}

}

static const NodeFilter ntySetOperaFilter = {
	sizeof(NodePacket),
	ntyPacketCtor,
	ntyPacketDtor,
	ntyPacketSetSuccessor,
	ntyPacketGetSuccessor,
	ntySetOperaHandleRequest,
};


const void *pNtyGetOperaFilter = &ntyGetOperaFilter;
const void *pNtySetOperaFilter = &ntySetOperaFilter;


void* ntyNodeFilterInit(void) {
	void *pGetOperaFilter = New(pNtyGetOperaFilter);
	void *pSetOperaFilter = New(pNtySetOperaFilter);

	ntySetSuccessor(pGetOperaFilter, pSetOperaFilter);
	ntySetSuccessor(pSetOperaFilter, NULL);
	/*
	 * add your Filter
	 * for example:
	 * void *pFilter = New(NtyFilter);
	 * ntySetSuccessor(pLogoutFilter, pFilter);
	 */

	return pGetOperaFilter;
}

static void *ntyNodeFilter = NULL;

void* ntyNodeFilterInstance(void) {
	if (ntyNodeFilter == NULL) {
		void *pNodeFilter = ntyNodeFilterInit();
		if ((unsigned long)NULL != cmpxchg((void*)(&ntyNodeFilter), (unsigned long)NULL, (unsigned long)pNodeFilter, WORD_WIDTH)) {
			Delete(pNodeFilter);
		}
	}
	return ntyNodeFilter;
}




void ntyNodeAgentProcess(const U8 *buffer, int length, C_DEVID id) {
	void *filter = ntyNodeFilterInstance();
	U8 **pTable = (U8**)malloc(sizeof(U8**));
	int Count = 0;

	ntySeparation(NTY_SEPARATOR_COMMAS, buffer, length, &pTable, &Count);
	if (Count <= 2) { //Paser Set CMD
		goto NODE_EXIT;
	}

	ntyHandleNodeRequest(filter, pTable, Count, id);

NODE_EXIT:
	ntyFreeTable(&pTable, Count);
	free(pTable);

	return ;
}



