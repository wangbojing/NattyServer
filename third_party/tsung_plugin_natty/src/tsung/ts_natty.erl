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

-module(ts_natty).
-author('wangbojing').

-behavior(ts_plugin).

-include("ts_profile.hrl").
-include("ts_natty.hrl").

-include("ts_macros.hrl").

-export([add_dynparams/4,
         get_message/2,
         session_defaults/0,
         subst/2,
         parse/2,
         parse_bidi/2,
         dump/2,
         parse_config/2,
         decode_buffer/2,
         new_session/0]).

%%----------------------------------------------------------------------
%% Function: session_default/0
%% Purpose: default parameters for session (ack_type and persistent)
%% Returns: {ok, true|false}
%%----------------------------------------------------------------------
session_defaults() ->
    {ok, true}.

%% @spec decode_buffer(Buffer::binary(),Session::record(raw)) ->  NewBuffer::binary()
%% @doc We need to decode buffer (remove chunks, decompress ...) for
%%      matching or dyn_variables
%% @end
decode_buffer(Buffer, #natty_request{}) ->
    Buffer.

%%----------------------------------------------------------------------
%% Function: new_session/0
%% Purpose: initialize session information
%% Returns: record or []
%%----------------------------------------------------------------------
new_session() ->
    #natty_request{}.
%%----------------------------------------------------------------------
%% Function: get_message/1
%% Purpose: Build a message/request
%% Args:    #jabber
%% Returns: binary
%%----------------------------------------------------------------------
get_message(#natty_request{uid = StrUid, data = Data}, #state_rcv{session = S})->
    MsgBin = list_to_binary(Data),
    Uid = case is_list(StrUid) of
              true -> list_to_integer(StrUid);
              false -> StrUid
          end,
    ReqBody = <<"**##", Uid:32/integer, MsgBin/binary, "##**">>,
    BodyLen = byte_size(ReqBody),
    ReqBin = <<BodyLen:32/little, ReqBody/binary>>,

    ?LOGF("natty_request encode result : ~p", [{"**##", Uid, Data, "##**"}], ?DEB),
    {ReqBin, S}.

%%----------------------------------------------------------------------
%% Function: parse/2
%% Purpose: Parse the response data
%% Returns: {NewState, Opts, Close}
%%----------------------------------------------------------------------
parse(closed, State) ->
    ?LOGF("natty_response got closed", [], ?DEB),
    {State#state_rcv{ack_done = true, datasize = 0}, [], true};
parse(<<Len:32/little, LeftBin:Len/binary>> = Data, State = #state_rcv{datasize = DataSize}) ->
    <<"**##", Uid:32/integer, Random:32/integer, "##**">> = LeftBin,

    ?LOGF("natty_response decode result : ~p", [{"**##", Uid, Random, "##**"}], ?DEB),

    AckResult =
    case Random of
        Num when Num > 0 ->
            true;
        0 ->
            false
    end,

    NewDataSize = DataSize + Len + 4,
    {State#state_rcv{ack_done = AckResult, acc = [], datasize = NewDataSize}, [], false};
parse(Data, State) ->
    ?LOGF("natty_response got unmatched data : ~p", [Data], ?DEB),
    {State, [], false}.

parse_bidi(Data, State) ->
    ts_plugin:parse_bidi(Data,State).

dump(A,B) ->
    ts_plugin:dump(A,B).

parse_config(Element, Conf) ->
    ts_config_natty:parse_config(Element, Conf).

%%----------------------------------------------------------------------
%% Function: add_dynparams/4
%% Purpose: add dynamic parameters to build the message
%%----------------------------------------------------------------------
add_dynparams(_,[], Param, _Host) ->
    Param;
add_dynparams(true, {DynVars, _Session}, OldReq, _Host) ->
    subst(OldReq, DynVars);
add_dynparams(_Subst, _DynData, Param, _Host) ->
    Param.

%%----------------------------------------------------------------------
%% Function: subst/1
%%----------------------------------------------------------------------
subst(Req = #natty_request{uid = Uid, data = Data}, DynVars) ->
    NewData = ts_search:subst(Data, DynVars),
    NewUid = ts_search:subst(Uid, DynVars),
    ?LOGF("subst data result : ~p", [{NewUid, NewData}], ?DEB),
    Req#natty_request{uid = NewUid, data = NewData}.
