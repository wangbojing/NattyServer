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



-module(ts_config_natty).
-author('wangbojing').

-export([parse_config/2]).

-include("ts_profile.hrl").
-include("ts_config.hrl").
-include("ts_natty.hrl").

-include("xmerl.hrl").

%%----------------------------------------------------------------------
%% Func: parse_config/2
%% Args: Element, Config
%% Returns: List
%% Purpose: parse a request defined in the XML config file
%%----------------------------------------------------------------------
%% Parsing other elements
parse_config(Element = #xmlElement{name = dyn_variable}, Conf = #config{}) ->
    ts_config:parse(Element,Conf);



parse_config(Element = #xmlElement{name = natty, attributes = Attrs},
             Config=#config{curid = Id, session_tab = Tab,
                            sessions = [CurS | _], dynvar = DynVar,
                            subst    = SubstFlag, match = MatchRegExp}) ->

    %% support dynparams subst
    %% Uid = ts_config:getAttr(string, Attrs, uid, "0"),
    Ack  = ts_config:getAttr(atom, Attrs, ack, parse),

    %% Version = ts_config:getAttr(atom, Attrs, version, a),
    %% DevType = ts_config:getAttr(atom, Attrs, devtype, w),
    ProtoType = ts_config:getAttr(string, Attrs, prototype, "0"),
    MsgType = ts_config:getAttr(string, Attrs, msgtype, "0"),
    DevId = ts_config:getAttr(string, Attrs, devid, "0"),

    Data = ts_config:getText(Element#xmlElement.content),

    Length = erlang:length(Data),

   %% Req = #natty_request{uid = Uid, data = Data},
    Req = #natty_request{prototype=ProtoType, msgtype = MsgType, devid = DevId, length = Length, data = Data},

    ts_config:mark_prev_req(Id - 1, Tab, CurS),
    Msg=#ts_request{ack     = Ack,
                    subst   = SubstFlag,
                    match   = MatchRegExp,
                    param   = Req},
    ets:insert(Tab,{{CurS#session.id, Id}, Msg#ts_request{endpage = true,
                                                         dynvar_specs = DynVar}}),
    lists:foldl( fun(A,B)->ts_config:parse(A, B) end,
                 Config#config{dynvar = []},
                 Element#xmlElement.content);


%% --------------------------------- login ---------------------------------------------- %%

parse_config(Element = #xmlElement{name = login, attributes = Attrs},
             Config=#config{curid = Id, session_tab = Tab,
                            sessions = [CurS | _], dynvar = DynVar,
                            subst    = SubstFlag, match = MatchRegExp}) ->

    
    Ack  = ts_config:getAttr(atom, Attrs, ack, parse),

    ProtoType = ts_config:getAttr(string, Attrs, prototype, "0"),
    MsgType = ts_config:getAttr(string, Attrs, msgtype, "0"),
    FromId = ts_config:getAttr(string, Attrs, fromId, "0"),

    Req = #natty_login{prototype=ProtoType, msgtype = MsgType, fromId = FromId},

    ts_config:mark_prev_req(Id - 1, Tab, CurS),
    Msg=#ts_request{ack     = Ack,
                    subst   = SubstFlag,
                    match   = MatchRegExp,
                    param   = Req},


    ets:insert(Tab,{{CurS#session.id, Id}, Msg#ts_request{endpage = true,
                                                         dynvar_specs = DynVar}}),
    lists:foldl( fun(A,B)->ts_config:parse(A, B) end,
                 Config#config{dynvar = []},
                 Element#xmlElement.content);


%% --------------------------------- heartbeat ---------------------------------------------- %%

parse_config(Element = #xmlElement{name = heartbeat, attributes = Attrs},
             Config=#config{curid = Id, session_tab = Tab,
                            sessions = [CurS | _], dynvar = DynVar,
                            subst    = SubstFlag, match = MatchRegExp}) ->

    
    Ack  = ts_config:getAttr(atom, Attrs, ack, parse),

    ProtoType = ts_config:getAttr(string, Attrs, prototype, "0"),
    MsgType = ts_config:getAttr(string, Attrs, msgtype, "0"),
    FromId = ts_config:getAttr(string, Attrs, fromId, "0"),

    Req = #natty_heartbeat{prototype=ProtoType, msgtype = MsgType, fromId = FromId},

    ts_config:mark_prev_req(Id - 1, Tab, CurS),
    Msg=#ts_request{ack     = Ack,
                    subst   = SubstFlag,
                    match   = MatchRegExp,
                    param   = Req},

                    
    ets:insert(Tab,{{CurS#session.id, Id}, Msg#ts_request{endpage = true,
                                                         dynvar_specs = DynVar}}),
    lists:foldl( fun(A,B)->ts_config:parse(A, B) end,
                 Config#config{dynvar = []},
                 Element#xmlElement.content);

%% --------------------------------- logout ---------------------------------------------- %%

parse_config(Element = #xmlElement{name = logout, attributes = Attrs},
             Config=#config{curid = Id, session_tab = Tab,
                            sessions = [CurS | _], dynvar = DynVar,
                            subst    = SubstFlag, match = MatchRegExp}) ->

    
    Ack  = ts_config:getAttr(atom, Attrs, ack, parse),

    ProtoType = ts_config:getAttr(string, Attrs, prototype, "0"),
    MsgType = ts_config:getAttr(string, Attrs, msgtype, "0"),
    FromId = ts_config:getAttr(string, Attrs, fromId, "0"),

    Req = #natty_logout{prototype=ProtoType, msgtype = MsgType, fromId = FromId},

    ts_config:mark_prev_req(Id - 1, Tab, CurS),
    Msg=#ts_request{ack     = Ack,
                    subst   = SubstFlag,
                    match   = MatchRegExp,
                    param   = Req},

                    
    ets:insert(Tab,{{CurS#session.id, Id}, Msg#ts_request{endpage = true,
                                                         dynvar_specs = DynVar}}),
    lists:foldl( fun(A,B)->ts_config:parse(A, B) end,
                 Config#config{dynvar = []},
                 Element#xmlElement.content);


%% --------------------------------- locationasync ---------------------------------------------- %%

parse_config(Element = #xmlElement{name = locationasync, attributes = Attrs},
             Config=#config{curid = Id, session_tab = Tab,
                            sessions = [CurS | _], dynvar = DynVar,
                            subst    = SubstFlag, match = MatchRegExp}) ->

    
    Ack  = ts_config:getAttr(atom, Attrs, ack, parse),

    ProtoType = ts_config:getAttr(string, Attrs, prototype, "0"),
    MsgType = ts_config:getAttr(string, Attrs, msgtype, "0"),
    FromId = ts_config:getAttr(string, Attrs, fromId, "0"),

    Data = ts_config:getText(Element#xmlElement.content),
    Length = erlang:length(Data),

    Req = #natty_locationasync{prototype=ProtoType, msgtype = MsgType, fromId = FromId, length = Length, data = Data},

    ts_config:mark_prev_req(Id - 1, Tab, CurS),
    Msg=#ts_request{ack     = Ack,
                    subst   = SubstFlag,
                    match   = MatchRegExp,
                    param   = Req},

                    
    ets:insert(Tab,{{CurS#session.id, Id}, Msg#ts_request{endpage = true,
                                                         dynvar_specs = DynVar}}),
    lists:foldl( fun(A,B)->ts_config:parse(A, B) end,
                 Config#config{dynvar = []},
                 Element#xmlElement.content);

%% --------------------------------- weatherasync ---------------------------------------------- %%

parse_config(Element = #xmlElement{name = weatherasync, attributes = Attrs},
             Config=#config{curid = Id, session_tab = Tab,
                            sessions = [CurS | _], dynvar = DynVar,
                            subst    = SubstFlag, match = MatchRegExp}) ->

    
    Ack  = ts_config:getAttr(atom, Attrs, ack, parse),

    ProtoType = ts_config:getAttr(string, Attrs, prototype, "0"),
    MsgType = ts_config:getAttr(string, Attrs, msgtype, "0"),
    FromId = ts_config:getAttr(string, Attrs, fromId, "0"),

    Data = ts_config:getText(Element#xmlElement.content),
    Length = erlang:length(Data),

    Req = #natty_weatherasync{prototype=ProtoType, msgtype = MsgType, fromId = FromId, length = Length, data = Data},

    ts_config:mark_prev_req(Id - 1, Tab, CurS),
    Msg=#ts_request{ack     = Ack,
                    subst   = SubstFlag,
                    match   = MatchRegExp,
                    param   = Req},

                    
    ets:insert(Tab,{{CurS#session.id, Id}, Msg#ts_request{endpage = true,
                                                         dynvar_specs = DynVar}}),
    lists:foldl( fun(A,B)->ts_config:parse(A, B) end,
                 Config#config{dynvar = []},
                 Element#xmlElement.content);


%% --------------------------------- route ---------------------------------------------- %%

parse_config(Element = #xmlElement{name = route, attributes = Attrs},
             Config=#config{curid = Id, session_tab = Tab,
                            sessions = [CurS | _], dynvar = DynVar,
                            subst    = SubstFlag, match = MatchRegExp}) ->

    
    Ack  = ts_config:getAttr(atom, Attrs, ack, parse),

    ProtoType = ts_config:getAttr(string, Attrs, prototype, "0"),
    MsgType = ts_config:getAttr(string, Attrs, msgtype, "0"),
    FromId = ts_config:getAttr(string, Attrs, fromId, "0"),
    ToId = ts_config:getAttr(string, Attrs, toId, "0"),

    Data = ts_config:getText(Element#xmlElement.content),
    Length = erlang:length(Data),

    Req = #natty_route{prototype=ProtoType, msgtype = MsgType, fromId = FromId, toId = ToId, length = Length, data = Data},

    ts_config:mark_prev_req(Id - 1, Tab, CurS),
    Msg=#ts_request{ack     = Ack,
                    subst   = SubstFlag,
                    match   = MatchRegExp,
                    param   = Req},

                    
    ets:insert(Tab,{{CurS#session.id, Id}, Msg#ts_request{endpage = true,
                                                         dynvar_specs = DynVar}}),
    lists:foldl( fun(A,B)->ts_config:parse(A, B) end,
                 Config#config{dynvar = []},
                 Element#xmlElement.content);


%% --------------------------------- bind ---------------------------------------------- %%

parse_config(Element = #xmlElement{name = bind, attributes = Attrs},
             Config=#config{curid = Id, session_tab = Tab,
                            sessions = [CurS | _], dynvar = DynVar,
                            subst    = SubstFlag, match = MatchRegExp}) ->

    
    Ack  = ts_config:getAttr(atom, Attrs, ack, parse),

    ProtoType = ts_config:getAttr(string, Attrs, prototype, "0"),
    MsgType = ts_config:getAttr(string, Attrs, msgtype, "0"),
    FromId = ts_config:getAttr(string, Attrs, fromId, "0"),
    DevId = ts_config:getAttr(string, Attrs, devId, "0"),

    Data = ts_config:getText(Element#xmlElement.content),
    Length = erlang:length(Data),

    Req = #natty_bind{prototype=ProtoType, msgtype = MsgType, fromId = FromId, devId = DevId, length = Length, data = Data},

    ts_config:mark_prev_req(Id - 1, Tab, CurS),
    Msg=#ts_request{ack     = Ack,
                    subst   = SubstFlag,
                    match   = MatchRegExp,
                    param   = Req},

                    
    ets:insert(Tab,{{CurS#session.id, Id}, Msg#ts_request{endpage = true,
                                                         dynvar_specs = DynVar}}),
    lists:foldl( fun(A,B)->ts_config:parse(A, B) end,
                 Config#config{dynvar = []},
                 Element#xmlElement.content);

%% --------------------------------- unbind ---------------------------------------------- %%

parse_config(Element = #xmlElement{name = unbind, attributes = Attrs},
             Config=#config{curid = Id, session_tab = Tab,
                            sessions = [CurS | _], dynvar = DynVar,
                            subst    = SubstFlag, match = MatchRegExp}) ->

    
    Ack  = ts_config:getAttr(atom, Attrs, ack, parse),

    ProtoType = ts_config:getAttr(string, Attrs, prototype, "0"),
    MsgType = ts_config:getAttr(string, Attrs, msgtype, "0"),
    FromId = ts_config:getAttr(string, Attrs, fromId, "0"),
    DevId = ts_config:getAttr(string, Attrs, devId, "0"),


    Req = #natty_unbind{prototype=ProtoType, msgtype = MsgType, fromId = FromId, devId = DevId},

    ts_config:mark_prev_req(Id - 1, Tab, CurS),
    Msg=#ts_request{ack     = Ack,
                    subst   = SubstFlag,
                    match   = MatchRegExp,
                    param   = Req},

                    
    ets:insert(Tab,{{CurS#session.id, Id}, Msg#ts_request{endpage = true,
                                                         dynvar_specs = DynVar}}),
    lists:foldl( fun(A,B)->ts_config:parse(A, B) end,
                 Config#config{dynvar = []},
                 Element#xmlElement.content);


%% --------------------------------- packet ---------------------------------------------- %%

parse_config(Element = #xmlElement{name = packet, attributes = Attrs},
             Config=#config{curid = Id, session_tab = Tab,
                            sessions = [CurS | _], dynvar = DynVar,
                            subst    = SubstFlag, match = MatchRegExp}) ->

    
    Ack  = ts_config:getAttr(atom, Attrs, ack, parse),

    ProtoType = ts_config:getAttr(string, Attrs, prototype, "0"),
    MsgType = ts_config:getAttr(string, Attrs, msgtype, "0"),
    FromId = ts_config:getAttr(string, Attrs, fromId, "0"),

    Data = ts_config:getText(Element#xmlElement.content),
    Length = erlang:length(Data),

    Req = #natty_packet{prototype=ProtoType, msgtype = MsgType, fromId = FromId, length = Length, data = Data},

    ts_config:mark_prev_req(Id - 1, Tab, CurS),
    Msg=#ts_request{ack     = Ack,
                    subst   = SubstFlag,
                    match   = MatchRegExp,
                    param   = Req},

                    
    ets:insert(Tab,{{CurS#session.id, Id}, Msg#ts_request{endpage = true,
                                                         dynvar_specs = DynVar}}),
    lists:foldl( fun(A,B)->ts_config:parse(A, B) end,
                 Config#config{dynvar = []},
                 Element#xmlElement.content);


%% --------------------------------- common ---------------------------------------------- %%

parse_config(Element = #xmlElement{name = common, attributes = Attrs},
             Config=#config{curid = Id, session_tab = Tab,
                            sessions = [CurS | _], dynvar = DynVar,
                            subst    = SubstFlag, match = MatchRegExp}) ->

    
    Ack  = ts_config:getAttr(atom, Attrs, ack, parse),

    ProtoType = ts_config:getAttr(string, Attrs, prototype, "0"),
    MsgType = ts_config:getAttr(string, Attrs, msgtype, "0"),
    FromId = ts_config:getAttr(string, Attrs, fromId, "0"),
    ToId = ts_config:getAttr(string, Attrs, toId, "0"),

    Data = ts_config:getText(Element#xmlElement.content),
    Length = erlang:length(Data),

    Req = #natty_common{prototype=ProtoType, msgtype = MsgType, fromId = FromId, toId = ToId, length = Length, data = Data},

    ts_config:mark_prev_req(Id - 1, Tab, CurS),
    Msg=#ts_request{ack     = Ack,
                    subst   = SubstFlag,
                    match   = MatchRegExp,
                    param   = Req},

                    
    ets:insert(Tab,{{CurS#session.id, Id}, Msg#ts_request{endpage = true,
                                                         dynvar_specs = DynVar}}),
    lists:foldl( fun(A,B)->ts_config:parse(A, B) end,
                 Config#config{dynvar = []},
                 Element#xmlElement.content);






%% Parsing other elements
parse_config(Element = #xmlElement{}, Conf = #config{}) ->
    ts_config:parse(Element,Conf);
%% Parsing non #xmlElement elements
parse_config(_, Conf = #config{}) ->
    Conf.
