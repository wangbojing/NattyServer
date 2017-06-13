%% 	*
%% 	*  Author : WangBoJing , email : 1989wangbojing@gmail.com
%% 	* 
%% 	*  Copyright Statement:
%% 	*  --------------------
%% 	*  This software is protected by Copyright and the information contained
%% 	*  herein is confidential. The software may not be copied and the information
%% 	*  contained herein may not be used or disclosed except with the written
%% 	*  permission of Author. (C) 2016
%% 	* 
%% 	*
 
%% ****       *****
%%   ***        *
%%   ***        *                         *               *
%%   * **       *                         *               *
%%   * **       *                         *               *
%%   *  **      *                        **              **
%%   *  **      *                       ***             ***
%%   *   **     *       ******       ***********     ***********    *****    *****
%%   *   **     *     **     **          **              **           **      **
%%   *    **    *    **       **         **              **           **      *
%%   *    **    *    **       **         **              **            *      *
%%   *     **   *    **       **         **              **            **     *
%%   *     **   *            ***         **              **             *    *
%%   *      **  *       ***** **         **              **             **   *
%%   *      **  *     ***     **         **              **             **   *
%%   *       ** *    **       **         **              **              *  *
%%   *       ** *   **        **         **              **              ** *
%%   *        ***   **        **         **              **               * *
%%   *        ***   **        **         **     *        **     *         **
%%   *         **   **        **  *      **     *        **     *         **
%%   *         **    **     ****  *       **   *          **   *          *
%% *****        *     ******   ***         ****            ****           *
%%                                                                        *
%%                                                                       *
%%                                                                   *****
%%                                                                   ****
%%
%%
%%  
%%  


-author('wangbojing').

-record(natty_request, {
          prototype,
          msgtype,
          devid,
          length,
          data
         }).


-record(natty_response, {
          prototype,
          msgtype,
          status,
          acknum,
          length,
          data
         }).


-record(natty_login, {
			prototype,
			msgtype,
			fromId
		}).

-record(natty_heartbeat, {
			prototype,
			msgtype,
			fromId
		}).

-record(natty_logout, {
			prototype,
			msgtype,
			fromId
		}).

-record(natty_locationasync, {
			prototype,
			msgtype,
			fromId,
			length,
			data
		}).


-record(natty_weatherasync, {
			prototype,
			msgtype,
			fromId,
			length,
			data
		}).


-record(natty_route, {
			prototype,
			msgtype,
			fromId,
			toId,
			length,
			data
		}).

-record(natty_bind, {
			prototype,
			msgtype,
			fromId,
			devId,
			length,
			data
		}).

-record(natty_unbind, {
			prototype,
			msgtype,
			fromId,
			devId
		}).

-record(natty_packet, {
			prototype,
			msgtype,
			fromId,
			length,
			data
		}).

-record(natty_common, {
			prototype,
			msgtype,
			fromId,
			toId,
			length,
			data
		}).


-record(natty_dyndata, {
          none
         }
       ).

