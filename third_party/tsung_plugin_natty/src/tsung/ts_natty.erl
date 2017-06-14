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
%% 
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
    Buffer;

decode_buffer(Buffer, #natty_login{}) ->
    Buffer;

decode_buffer(Buffer, #natty_heartbeat{}) ->
    Buffer;

decode_buffer(Buffer, #natty_logout{}) ->
    Buffer;

decode_buffer(Buffer, #natty_locationasync{}) ->
    Buffer;

decode_buffer(Buffer, #natty_weatherasync{}) ->
    Buffer;
%
decode_buffer(Buffer, #natty_bind{}) ->
    Buffer;

decode_buffer(Buffer, #natty_unbind{}) ->
    Buffer;

decode_buffer(Buffer, #natty_packet{}) ->
    Buffer;

decode_buffer(Buffer, #natty_common{}) ->
    Buffer;

decode_buffer(Buffer, #natty_route{}) ->
    Buffer.

%%----------------------------------------------------------------------
%% Function: new_session/0
%% Purpose: initialize session information
%% Returns: record or []
%%----------------------------------------------------------------------
new_session() ->
    #natty_request{}.
%%----------------------------------------------------------------------
%% Function: get_message/2
%% Purpose: Build a message/request
%% Args:    #wangbojing
%% Returns: binary
%%----------------------------------------------------------------------
get_message(#natty_request{prototype=ProtoType, msgtype = MsgType, devid = DevId, length = Length, data = Data},
            #state_rcv{session = S})->
    
    MsgBin = list_to_binary(Data),
    PType = list_to_integer(ProtoType),
    MType = list_to_integer(MsgType),
    DeviceId = case is_list(DevId) of
              true -> list_to_integer(DevId);
              false -> DevId
          end,

    %ReqBody = <<"**##", Uid:32/integer, MsgBin/binary, "##**">>,
    %BodyLen = byte_size(ReqBody),
    %ReqBin = <<BodyLen:32/little, ReqBody/binary>>,

    ReqBody = <<"AA", PType:8/integer, MType:8/integer, DeviceId:64/little, Length:16/little, MsgBin/binary>>,
    BodyCrc = erlang:crc32(ReqBody),
    ReqBin = <<ReqBody/binary, BodyCrc:32/little>>,

    ?LOGF("natty_request encode result : ~p", [{PType, MType, DeviceId, Length, MsgBin}], ?DEB),
    {ReqBin, S};



%% ---------------------------------------------- login ---------------------------------------------- %%

get_message(#natty_login{prototype=ProtoType, msgtype = MsgType, fromId = FromId},
            #state_rcv{session = S})->

    PType = list_to_integer(ProtoType),
    MType = list_to_integer(MsgType),
    FId = case is_list(FromId) of
              true -> list_to_integer(FromId);
              false -> FromId
          end,

    ReqBody = <<"AA", PType:8/integer, MType:8/integer, FId:64/little>>,
    BodyCrc = erlang:crc32(ReqBody),
    ReqBin = <<ReqBody/binary, BodyCrc:32/little>>,

    ?LOGF("natty_login encode result : ~p", [{PType, MType, FId}], ?DEB),
    {ReqBin, S};

%% ---------------------------------------------- heartbeat ---------------------------------------------- %%

get_message(#natty_heartbeat{prototype=ProtoType, msgtype = MsgType, fromId = FromId},
            #state_rcv{session = S})->

    PType = list_to_integer(ProtoType),
    MType = list_to_integer(MsgType),
    FId = case is_list(FromId) of
              true -> list_to_integer(FromId);
              false -> FromId
          end,

    ReqBody = <<"AA", PType:8/integer, MType:8/integer, FId:64/little>>,
    BodyCrc = erlang:crc32(ReqBody),
    ReqBin = <<ReqBody/binary, BodyCrc:32/little>>,

    ?LOGF("natty_heartbeat encode result : ~p", [{PType, MType, FId}], ?DEB),
    {ReqBin, S};


%% ---------------------------------------------- logout ---------------------------------------------- %%

get_message(#natty_logout{prototype=ProtoType, msgtype = MsgType, fromId = FromId},
            #state_rcv{session = S})->

    PType = list_to_integer(ProtoType),
    MType = list_to_integer(MsgType),
    FId = case is_list(FromId) of
              true -> list_to_integer(FromId);
              false -> FromId
          end,

    ReqBody = <<"AA", PType:8/integer, MType:8/integer, FId:64/little>>,
    BodyCrc = erlang:crc32(ReqBody),
    ReqBin = <<ReqBody/binary, BodyCrc:32/little>>,

    ?LOGF("natty_logout encode result : ~p", [{PType, MType, FId}], ?DEB),
    {ReqBin, S};


%% ---------------------------------------------- locationasync ---------------------------------------------- %%

get_message(#natty_locationasync{prototype=ProtoType, msgtype = MsgType, fromId = FromId, length = Length, data = Data},
            #state_rcv{session = S})->

    PType = list_to_integer(ProtoType),
    MType = list_to_integer(MsgType),
    FId = case is_list(FromId) of
              true -> list_to_integer(FromId);
              false -> FromId
          end,

    MsgBin = list_to_binary(Data),



    ReqBody = <<"AA", PType:8/integer, MType:8/integer, FId:64/little, Length:16/little, MsgBin/binary>>,
    BodyCrc = erlang:crc32(ReqBody),
    ReqBin = <<ReqBody/binary, BodyCrc:32/little>>,

    ?LOGF("natty_locationasync encode result : ~p", [{PType, MType, FId, Length, MsgBin}], ?DEB),
    {ReqBin, S};


%% ---------------------------------------------- weatherasync ---------------------------------------------- %%

get_message(#natty_weatherasync{prototype=ProtoType, msgtype = MsgType, fromId = FromId, length = Length, data = Data},
            #state_rcv{session = S})->

    PType = list_to_integer(ProtoType),
    MType = list_to_integer(MsgType),
    FId = case is_list(FromId) of
              true -> list_to_integer(FromId);
              false -> FromId
          end,

    MsgBin = list_to_binary(Data),

    ReqBody = <<"AA", PType:8/integer, MType:8/integer, FId:64/little, Length:16/little, MsgBin/binary>>,
    BodyCrc = erlang:crc32(ReqBody),
    ReqBin = <<ReqBody/binary, BodyCrc:32/little>>,

    ?LOGF("natty_locationasync encode result : ~p", [{PType, MType, FId, Length, MsgBin}], ?DEB),
    {ReqBin, S};


%% ---------------------------------------------- route ---------------------------------------------- %%

get_message(#natty_route{prototype=ProtoType, msgtype = MsgType, fromId = FromId, toId = ToId, length = Length, data = Data},
            #state_rcv{session = S})->

    PType = list_to_integer(ProtoType),
    MType = list_to_integer(MsgType),
    FId = case is_list(FromId) of
              true -> list_to_integer(FromId);
              false -> FromId
          end,

    TId = case is_list(ToId) of
              true -> list_to_integer(ToId);
              false -> ToId
          end,

    MsgBin = list_to_binary(Data),

    ReqBody = <<"AA", PType:8/integer, MType:8/integer, FId:64/little, TId:64/little, Length:16/little, MsgBin/binary>>,
    BodyCrc = erlang:crc32(ReqBody),
    ReqBin = <<ReqBody/binary, BodyCrc:32/little>>,

    ?LOGF("natty_route encode result : ~p", [{PType, MType, FId, TId, Length, MsgBin}], ?DEB),
    {ReqBin, S};


%% ---------------------------------------------- bind ---------------------------------------------- %%

get_message(#natty_bind{prototype=ProtoType, msgtype = MsgType, fromId = FromId, devId = DevId, length = Length, data = Data},
            #state_rcv{session = S})->

    PType = list_to_integer(ProtoType),
    MType = list_to_integer(MsgType),
    FId = case is_list(FromId) of
              true -> list_to_integer(FromId);
              false -> FromId
          end,

    DeviceId = case is_list(DevId) of
              true -> list_to_integer(DevId);
              false -> DevId
          end,

    MsgBin = list_to_binary(Data),

    ReqBody = <<"AA", PType:8/integer, MType:8/integer, FId:64/little, DeviceId:64/little, Length:16/little, MsgBin/binary>>,
    BodyCrc = erlang:crc32(ReqBody),
    ReqBin = <<ReqBody/binary, BodyCrc:32/little>>,

    ?LOGF("natty_bind encode result : ~p", [{PType, MType, FId, DeviceId, Length, MsgBin}], ?DEB),
    {ReqBin, S};


%% ---------------------------------------------- unbind ---------------------------------------------- %%

get_message(#natty_unbind{prototype=ProtoType, msgtype = MsgType, fromId = FromId, devId = DevId},
            #state_rcv{session = S})->

    PType = list_to_integer(ProtoType),
    MType = list_to_integer(MsgType),
    FId = case is_list(FromId) of
              true -> list_to_integer(FromId);
              false -> FromId
          end,

    DeviceId = case is_list(DevId) of
              true -> list_to_integer(DevId);
              false -> DevId
          end,

    %MsgBin = list_to_binary(Data),

    ReqBody = <<"AA", PType:8/integer, MType:8/integer, FId:64/little, DeviceId:64/little>>,
    BodyCrc = erlang:crc32(ReqBody),
    ReqBin = <<ReqBody/binary, BodyCrc:32/little>>,

    ?LOGF("natty_bind encode result : ~p", [{PType, MType, FId, DeviceId}], ?DEB),
    {ReqBin, S};



%% ---------------------------------------------- packet ---------------------------------------------- %%

get_message(#natty_packet{prototype=ProtoType, msgtype = MsgType, fromId = FromId, length = Length, data = Data},
            #state_rcv{session = S})->

    PType = list_to_integer(ProtoType),
    MType = list_to_integer(MsgType),
    FId = case is_list(FromId) of
              true -> list_to_integer(FromId);
              false -> FromId
          end,

    MsgBin = list_to_binary(Data),

    ReqBody = <<"AA", PType:8/integer, MType:8/integer, FId:64/little, Length:16/little, MsgBin/binary>>,
    BodyCrc = erlang:crc32(ReqBody),
    ReqBin = <<ReqBody/binary, BodyCrc:32/little>>,

    ?LOGF("natty_bind encode result : ~p", [{PType, MType, FId, Length, MsgBin}], ?DEB),
    {ReqBin, S};



%% ---------------------------------------------- common ---------------------------------------------- %%

get_message(#natty_common{prototype=ProtoType, msgtype = MsgType, fromId = FromId, toId = ToId, length = Length, data = Data},
            #state_rcv{session = S})->

    PType = list_to_integer(ProtoType),
    MType = list_to_integer(MsgType),
    FId = case is_list(FromId) of
              true -> list_to_integer(FromId);
              false -> FromId
          end,

    Rsvd = 0,

    TId = case is_list(ToId) of
              true -> list_to_integer(ToId);
              false -> ToId
          end,

    MsgBin = list_to_binary(Data),

    ReqBody = <<"AA", PType:8/integer, MType:8/integer, FId:64/little,Rsvd:32/little, TId:64/little, Length:16/little, MsgBin/binary>>,
    BodyCrc = erlang:crc32(ReqBody),
    ReqBin = <<ReqBody/binary, BodyCrc:32/little>>,

    ?LOGF("natty_bind encode result : ~p", [{PType, MType, FId, TId, Length, MsgBin}], ?DEB),
    {ReqBin, S}.




%%----------------------------------------------------------------------
%% Function: parse/2
%% Purpose: Parse the response data
%% Returns: {NewState, Opts, Close}
%%----------------------------------------------------------------------
parse(closed, State) ->
    ?LOGF("natty_response got closed", [], ?DEB),
    {State#state_rcv{ack_done = true, datasize = 0}, [], true};

%parse(<<ReqBody/binary, BodyCrc:32/little>> = Data, State = #state_rcv{datasize = DataSize}) ->
%    <<"A", "W", PType:8/integer, MType:8/integer, Status:16/integer, AckNum:32/integer, Length:16/integer, MsgBin/binary>> = ReqBody,
%
%    ?LOGF("natty_response decode result : ~p", [{PType, MType, Status, AckNum, Length, MsgBin}], ?DEB),
%
%    NewCrc = erlang:crc32(ReqBody),
%    if 
%      NewCrc = BodyCrc ->
%        io:format("Parse Crc Success");
%      ture ->
%        io:format("Parse Crc Failed")
%    end, 
%
%    NewDataSize = Length + 16,
%    {State#state_rcv{ack_done = AckNum, acc = [], datasize = NewDataSize}, [], false};

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

subst(Req = #natty_login{prototype=ProtoType, msgtype = MsgType, fromId = FromId}, DynVars) ->
    NewProto = ts_search:subst(ProtoType, DynVars),
    NewMsgType = ts_search:subst(MsgType, DynVars),
    NewFromId = ts_search:subst(FromId, DynVars),

    ?LOGF("subst data result : ~p", [{NewProto, NewMsgType, NewFromId}], ?DEB),
    Req#natty_login{prototype=NewProto, msgtype = NewMsgType, fromId = NewFromId};


subst(Req = #natty_heartbeat{prototype=ProtoType, msgtype = MsgType, fromId = FromId}, DynVars) ->
    NewProto = ts_search:subst(ProtoType, DynVars),
    NewMsgType = ts_search:subst(MsgType, DynVars),
    NewFromId = ts_search:subst(FromId, DynVars),

    ?LOGF("subst data result : ~p", [{NewProto, NewMsgType, NewFromId}], ?DEB),
    Req#natty_heartbeat{prototype=NewProto, msgtype = NewMsgType, fromId = NewFromId};


subst(Req = #natty_logout{prototype=ProtoType, msgtype = MsgType, fromId = FromId}, DynVars) ->
    NewProto = ts_search:subst(ProtoType, DynVars),
    NewMsgType = ts_search:subst(MsgType, DynVars),
    NewFromId = ts_search:subst(FromId, DynVars),

    ?LOGF("subst data result : ~p", [{NewProto, NewMsgType, NewFromId}], ?DEB),
    Req#natty_logout{prototype=NewProto, msgtype = NewMsgType, fromId = NewFromId};


subst(Req = #natty_locationasync{prototype=ProtoType, msgtype = MsgType, fromId = FromId, length = Length, data = Data}, DynVars) ->
    NewProto = ts_search:subst(ProtoType, DynVars),
    NewMsgType = ts_search:subst(MsgType, DynVars),
    NewFromId = ts_search:subst(FromId, DynVars),

    NewLength = ts_search:subst(Length, DynVars),
    NewData = ts_search:subst(Data, DynVars),

    ?LOGF("subst data result : ~p", [{NewProto, NewMsgType, NewFromId, NewLength, NewData}], ?DEB),
    Req#natty_locationasync{prototype=NewProto, msgtype = NewMsgType, fromId = NewFromId, length = NewLength, data = NewData};


subst(Req = #natty_weatherasync{prototype=ProtoType, msgtype = MsgType, fromId = FromId, length = Length, data = Data}, DynVars) ->
    NewProto = ts_search:subst(ProtoType, DynVars),
    NewMsgType = ts_search:subst(MsgType, DynVars),
    NewFromId = ts_search:subst(FromId, DynVars),

    NewLength = ts_search:subst(Length, DynVars),
    NewData = ts_search:subst(Data, DynVars),

    ?LOGF("subst data result : ~p", [{NewProto, NewMsgType, NewFromId, NewLength, NewData}], ?DEB),
    Req#natty_weatherasync{prototype=NewProto, msgtype = NewMsgType, fromId = NewFromId, length = NewLength, data = NewData};



subst(Req = #natty_route{prototype=ProtoType, msgtype = MsgType, fromId = FromId, toId = ToId, length = Length, data = Data}, DynVars) ->
    NewProto = ts_search:subst(ProtoType, DynVars),
    NewMsgType = ts_search:subst(MsgType, DynVars),
    NewFromId = ts_search:subst(FromId, DynVars),

    NewToId = ts_search:subst(ToId, DynVars),
    NewLength = ts_search:subst(Length, DynVars),
    NewData = ts_search:subst(Data, DynVars),

    ?LOGF("subst data result : ~p", [{NewProto, NewMsgType, NewFromId, NewToId, NewLength, NewData}], ?DEB),
    Req#natty_route{prototype=NewProto, msgtype = NewMsgType, fromId = NewFromId, toId = NewToId, length = NewLength, data = NewData};


subst(Req = #natty_bind{prototype=ProtoType, msgtype = MsgType, fromId = FromId, devId = DevId, length = Length, data = Data}, DynVars) ->
    NewProto = ts_search:subst(ProtoType, DynVars),
    NewMsgType = ts_search:subst(MsgType, DynVars),
    NewFromId = ts_search:subst(FromId, DynVars),

    NewDevId = ts_search:subst(DevId, DynVars),
    NewLength = ts_search:subst(Length, DynVars),
    NewData = ts_search:subst(Data, DynVars),

    ?LOGF("subst data result : ~p", [{NewProto, NewMsgType, NewFromId, NewDevId, NewLength, NewData}], ?DEB),
    Req#natty_bind{prototype=NewProto, msgtype = NewMsgType, fromId = NewFromId, devId = NewDevId, length = NewLength, data = NewData};


subst(Req = #natty_unbind{prototype=ProtoType, msgtype = MsgType, fromId = FromId, devId = DevId}, DynVars) ->
    NewProto = ts_search:subst(ProtoType, DynVars),
    NewMsgType = ts_search:subst(MsgType, DynVars),
    NewFromId = ts_search:subst(FromId, DynVars),

    NewDevId = ts_search:subst(DevId, DynVars),

    ?LOGF("subst data result : ~p", [{NewProto, NewMsgType, NewFromId, NewDevId}], ?DEB),
    Req#natty_unbind{prototype=NewProto, msgtype = NewMsgType, fromId = NewFromId, devId = NewDevId};


subst(Req = #natty_packet{prototype=ProtoType, msgtype = MsgType, fromId = FromId, length = Length, data = Data}, DynVars) ->
    NewProto = ts_search:subst(ProtoType, DynVars),
    NewMsgType = ts_search:subst(MsgType, DynVars),
    NewFromId = ts_search:subst(FromId, DynVars),

    NewLength = ts_search:subst(Length, DynVars),
    NewData = ts_search:subst(Data, DynVars),

    ?LOGF("subst data result : ~p", [{NewProto, NewMsgType, NewFromId, NewLength, NewData}], ?DEB),
    Req#natty_packet{prototype=NewProto, msgtype = NewMsgType, fromId = NewFromId, length = NewLength, data = NewData};



subst(Req = #natty_common{prototype=ProtoType, msgtype = MsgType, fromId = FromId, toId = ToId, length = Length, data = Data}, DynVars) ->
    NewProto = ts_search:subst(ProtoType, DynVars),
    NewMsgType = ts_search:subst(MsgType, DynVars),
    NewFromId = ts_search:subst(FromId, DynVars),

    NewToId = ts_search:subst(ToId, DynVars),
    NewLength = ts_search:subst(Length, DynVars),
    NewData = ts_search:subst(Data, DynVars),

    ?LOGF("subst data result : ~p", [{NewProto, NewMsgType, NewFromId, NewToId, NewLength, NewData}], ?DEB),
    Req#natty_common{prototype=NewProto, msgtype = NewMsgType, fromId = NewFromId, toId = NewToId, length = NewLength, data = NewData};


subst(Req = #natty_request{prototype=ProtoType, msgtype = MsgType, devid = DevId, length = Length, data = Data}, DynVars) ->
    NewProto = ts_search:subst(ProtoType, DynVars),
    NewMsgType = ts_search:subst(MsgType, DynVars),
    NewDevId = ts_search:subst(DevId, DynVars),
    NewLength = ts_search:subst(Length, DynVars),
    NewData = ts_search:subst(Data, DynVars),

    % NewData = ts_search:subst(Data, DynVars),
    % NewUid = ts_search:subst(Uid, DynVars),
    ?LOGF("subst data result : ~p", [{NewProto, NewMsgType, NewDevId, NewLength, NewData}], ?DEB),
    Req#natty_request{prototype=NewProto, msgtype = NewMsgType, devid = NewDevId, length = NewLength, data = NewData}.


