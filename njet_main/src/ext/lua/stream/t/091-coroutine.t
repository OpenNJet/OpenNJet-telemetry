# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket::Lua::Stream;

repeat_each(2);

plan tests => repeat_each() * (blocks() * 3 + 27);

$ENV{TEST_NGINX_RESOLVER} ||= '8.8.8.8';

our $StapScript = <<'_EOC_';
global ids, cur

function gen_id(k) {
    if (ids[k]) return ids[k]
    ids[k] = ++cur
    return cur
}

F(njt_http_handler) {
    delete ids
    cur = 0
}

/*
F(njt_http_lua_run_thread) {
    id = gen_id($ctx->cur_co)
    printf("run thread %d\n", id)
}

probe process("/usr/local/openresty-debug/luajit/lib/libluajit-5.1.so.2").function("lua_resume") {
    id = gen_id($L)
    printf("lua resume %d\n", id)
}
*/

M(http-lua-user-coroutine-resume) {
    p = gen_id($arg2)
    c = gen_id($arg3)
    printf("resume %x in %x\n", c, p)
}

M(http-lua-thread-yield) {
    println("thread yield")
}

/*
F(njt_http_lua_coroutine_yield) {
    printf("yield %x\n", gen_id($L))
}
*/

M(http-lua-user-coroutine-yield) {
    p = gen_id($arg2)
    c = gen_id($arg3)
    printf("yield %x in %x\n", c, p)
}

F(njt_http_lua_atpanic) {
    printf("lua atpanic(%d):", gen_id($L))
    print_ubacktrace();
}

M(http-lua-user-coroutine-create) {
    p = gen_id($arg2)
    c = gen_id($arg3)
    printf("create %x in %x\n", c, p)
}

F(njt_http_lua_njt_exec) { println("exec") }

F(njt_http_lua_njt_exit) { println("exit") }
F(njt_http_lua_ffi_exit) { println("exit") }
_EOC_

no_shuffle();
no_long_string();
run_tests();

__DATA__

=== TEST 1: basic coroutine print
--- stream_server_config
    content_by_lua_block {
        local cc, cr, cy = coroutine.create, coroutine.resume, coroutine.yield

        function f()
            local cnt = 0
            for i = 1, 20 do
                njt.say("Hello, ", cnt)
                cy()
                cnt = cnt + 1
            end
        end

        local c = cc(f)
        for i=1,3 do
            cr(c)
            njt.say("***")
        end
    }

--- config
--- stap2 eval: $::StapScript
--- stream_response
Hello, 0
***
Hello, 1
***
Hello, 2
***
--- no_error_log
[error]



=== TEST 2: basic coroutine2
--- stream_server_config
    content_by_lua_block {
        function f(fid)
            local cnt = 0
            while true do
                njt.say("cc", fid, ": ", cnt)
                coroutine.yield()
                cnt = cnt + 1
            end
        end

        local ccs = {}
        for i=1,3 do
            ccs[#ccs+1] = coroutine.create(function() f(i) end)
        end

        for i=1,9 do
            local cc = table.remove(ccs, 1)
            coroutine.resume(cc)
            ccs[#ccs+1] = cc
        end
    }

--- config
--- stream_response
cc1: 0
cc2: 0
cc3: 0
cc1: 1
cc2: 1
cc3: 1
cc1: 2
cc2: 2
cc3: 2
--- no_error_log
[error]



=== TEST 3: basic coroutine and cosocket
--- stream_server_config
    resolver $TEST_NGINX_RESOLVER ipv6=off;
    content_by_lua_block {
        function worker(url)
            local sock = njt.socket.tcp()
            local ok, err = sock:connect(url, 80)
            coroutine.yield()
            if not ok then
                njt.say("failed to connect to: ", url, " error: ", err)
                return
            end
            coroutine.yield()
            njt.say("successfully connected to: ", url)
            sock:close()
        end

        local urls = {
            "agentzh.org",
            "openresty.com",
            "openresty.org"
        }

        local ccs = {}
        for i, url in ipairs(urls) do
            local cc = coroutine.create(function() worker(url) end)
            ccs[#ccs+1] = cc
        end

        while true do
            if #ccs == 0 then break end
            local cc = table.remove(ccs, 1)
            local ok = coroutine.resume(cc)
            if ok then
                ccs[#ccs+1] = cc
            end
        end

        njt.say("*** All Done ***")
    }

--- config
--- stream_response
successfully connected to: agentzh.org
successfully connected to: openresty.com
successfully connected to: openresty.org
*** All Done ***
--- no_error_log
[error]
--- timeout: 10



=== TEST 4: coroutine.wrap(generate prime numbers)
--- stream_server_config
    content_by_lua_block {
        -- generate all the numbers from 2 to n
        function gen (n)
          return coroutine.wrap(function ()
            for i=2,n do coroutine.yield(i) end
          end)
        end

        -- filter the numbers generated by g, removing multiples of p
        function filter (p, g)
          return coroutine.wrap(function ()
            while 1 do
              local n = g()
              if n == nil then return end
              if math.fmod(n, p) ~= 0 then coroutine.yield(n) end
            end
          end)
        end

        N = 10
        x = gen(N)		-- generate primes up to N
        while 1 do
          local n = x()		-- pick a number until done
          if n == nil then break end
          njt.say(n)		-- must be a prime number
          x = filter(n, x)	-- now remove its multiples
        end
    }

--- config
--- stream_response
2
3
5
7
--- no_error_log
[error]



=== TEST 5: coroutine.wrap(generate prime numbers,reset create and resume)
--- stream_server_config
    content_by_lua_block {
        coroutine.create = nil
        coroutine.resume = nil
        -- generate all the numbers from 2 to n
        function gen (n)
          return coroutine.wrap(function ()
            for i=2,n do coroutine.yield(i) end
          end)
        end

        -- filter the numbers generated by g, removing multiples of p
        function filter (p, g)
          return coroutine.wrap(function ()
            while 1 do
              local n = g()
              if n == nil then return end
              if math.fmod(n, p) ~= 0 then coroutine.yield(n) end
            end
          end)
        end

        N = 10 
        x = gen(N)		-- generate primes up to N
        while 1 do
          local n = x()		-- pick a number until done
          if n == nil then break end
          njt.say(n)		-- must be a prime number
          x = filter(n, x)	-- now remove its multiples
        end
    }

--- config
--- stream_response
2
3
5
7
--- no_error_log
[error]



=== TEST 6: coroutine.wrap(generate fib)
--- stream_server_config
    content_by_lua_block {
        function generatefib (n)
          return coroutine.wrap(function ()
            local a,b = 1, 1
            while a <= n do
              coroutine.yield(a)
              a, b = b, a+b
            end
          end)
        end

        -- In lua, because OP_TFORLOOP uses luaD_call to execute the iterator function,
        -- and luaD_call is a C function, so we can not yield in the iterator function.
        -- So the following case(using for loop) will be failed.
        -- Luajit is OK.
        if package.loaded["jit"] then
            for i in generatefib(1000) do njt.say(i) end
        else
            local gen = generatefib(1000)
            while true do
                local i = gen()
                if not i then break end
                njt.say(i)
            end
        end
    }

--- config
--- stream_response
1
1
2
3
5
8
13
21
34
55
89
144
233
377
610
987
--- no_error_log
[error]



=== TEST 7: coroutine wrap and cosocket
--- stream_server_config
    resolver $TEST_NGINX_RESOLVER ipv6=off;
    content_by_lua_block {
        function worker(url)
            local sock = njt.socket.tcp()
            local ok, err = sock:connect(url, 80)
            coroutine.yield()
            if not ok then
                njt.say("failed to connect to: ", url, " error: ", err)
                return
            end
            coroutine.yield()
            njt.say("successfully connected to: ", url)
            sock:close()
        end

        local urls = {
            "agentzh.org",
            "openresty.com",
            "openresty.org"
        }

        local cfs = {}
        for i, url in ipairs(urls) do
            local cf = coroutine.wrap(function() worker(url) end)
            cfs[#cfs+1] = cf
        end

        for i=1,3 do cfs[i]() end
        for i=1,3 do cfs[i]() end
        for i=1,3 do cfs[i]() end

        njt.say("*** All Done ***")
    }

--- config
--- stream_response
successfully connected to: agentzh.org
successfully connected to: openresty.com
successfully connected to: openresty.org
*** All Done ***
--- no_error_log
[error]
--- timeout: 10



=== TEST 8: coroutine status, running
--- stream_server_config
    content_by_lua_block {
        local cc, cr, cy = coroutine.create, coroutine.resume, coroutine.yield
        local st, rn = coroutine.status, coroutine.running

        function f(self)
            local cnt = 0
            if rn() ~= self then njt.say("error"); return end
            njt.say("running: ", st(self)) --running
            cy()
            local c = cc(function(father)
                njt.say("normal: ", st(father))
            end) -- normal
            cr(c, self)
        end

        local c = cc(f)
        njt.say("suspended: ", st(c)) -- suspended
        cr(c, c)
        njt.say("suspended: ", st(c)) -- suspended
        cr(c, c)
        njt.say("dead: ", st(c)) -- dead
    }

--- config
--- stream_response
suspended: suspended
running: running
suspended: suspended
normal: normal
dead: dead
--- no_error_log
[error]



=== TEST 9: entry coroutine yielded will be resumed immediately
--- stream_server_config
    content_by_lua_block {
        njt.say("[", {coroutine.yield()}, "]")
        njt.say("[", {coroutine.yield(1, "a")}, "]")
        njt.say("done")
    }

--- config
--- stream_response
[]
[]
done
--- no_error_log
[error]



=== TEST 10: thread traceback (multi-thread)
Note: only coroutine.wrap propagates errors to the parent coroutine
(and thus produces a traceback)
--- stream_server_config
    content_by_lua_block {
        local f = function(cr) coroutine.resume(cr) end
        -- emit a error
        local g = function() unknown.unknown = 1 end
        local l1 = coroutine.wrap(f)
        local l2 = coroutine.wrap(g)
        local l3 = coroutine.wrap(function() l1(l2) end)
        l3()
        njt.say("hello")
    }
--- stream_response
--- error_log eval
["stack traceback:", "coroutine 0:", "coroutine 1:", "coroutine 2:"]
--- no_error
[crit]



=== TEST 11: thread traceback (only the entry thread)
--- stream_server_config
    content_by_lua_block {
        -- emit a error
        unknown.unknown = 1
        njt.say("hello")
    }

--- config
--- error_log eval
["stack traceback:", "coroutine 0:"]



=== TEST 12: bug: resume dead coroutine with args
--- stream_server_config
    content_by_lua_block {
        function print(...)
            local args = {...}
            local is_first = true
            for i,v in ipairs(args) do
                if is_first then
                    is_first = false
                else
                    njt.print(" ")
                end
                njt.print(v)
            end
            njt.print("\n")
        end

        function foo (a)
            print("foo", a)
            return coroutine.yield(2*a)
        end

        co = coroutine.create(function (a,b)
                print("co-body", a, b)
                local r = foo(a+1)
                print("co-body", r)
                local r, s = coroutine.yield(a+b, a-b)
                print("co-body", r, s)
                return b, "end"
            end)

        print("main", coroutine.resume(co, 1, 10))
        print("main", coroutine.resume(co, "r"))
        print("main", coroutine.resume(co, "x", "y"))
        print("main", coroutine.resume(co, "x", "y"))
    }

--- config
--- stream_response
co-body 1 10
foo 2
main true 4
co-body r
main true 11 -9
co-body x y
main true 10 end
main false cannot resume dead coroutine
--- no_error_log
[error]



=== TEST 13: deeply nested coroutines
--- stream_server_config
    content_by_lua_block {
        local create = coroutine.create
        local resume = coroutine.resume
        local yield = coroutine.yield
        function f()
            njt.say("f begin")
            yield()
            local c2 = create(g)
            njt.say("1: resuming c2")
            resume(c2)
            njt.say("2: resuming c2")
            resume(c2)
            yield()
            njt.say("3: resuming c2")
            resume(c2)
            njt.say("f done")
        end

        function g()
            njt.say("g begin")
            yield()
            njt.say("g going")
            yield()
            njt.say("g done")
        end

        local c1 = create(f)
        njt.say("1: resuming c1")
        resume(c1)
        njt.say("2: resuming c1")
        resume(c1)
        njt.say("3: resuming c1")
        resume(c1)
        njt.say("main done")
    }

--- config
--- stream_response
1: resuming c1
f begin
2: resuming c1
1: resuming c2
g begin
2: resuming c2
g going
3: resuming c1
3: resuming c2
g done
f done
main done
--- no_error_log
[error]



=== TEST 14: using njt.exit in user coroutines
--- stream_server_config
    content_by_lua_block {
        local create = coroutine.create
        local resume = coroutine.resume
        local yield = coroutine.yield

        local code = 400

        function f()
            local c2 = create(g)
            yield()
            code = code + 1
            resume(c2)
            yield()
            resume(c2)
        end

        function g()
            code = code + 1
            yield()
            code = code + 1
            njt.exit(code)
        end

        local c1 = create(f)
        resume(c1)
        resume(c1)
        resume(c1)
        njt.say("done")
    }

--- config
--- stap eval: $::StapScript
--- stap_out
create 2 in 1
resume 2 in 1
create 3 in 2
yield 2 in 1
resume 2 in 1
resume 3 in 2
yield 3 in 2
yield 2 in 1
resume 2 in 1
resume 3 in 2
exit

--- stream_response
--- no_error_log
[error]



=== TEST 15: resume coroutines from within another one that is not its parent
--- stream_server_config
    content_by_lua_block {
        local print = njt.say

        local c1, c2

        function f()
            print("f 1")
            print(coroutine.resume(c2))
            print("f 2")
        end

        function g()
            print("g 1")
            -- print(coroutine.resume(c1))
            print("g 2")
        end

        c1 = coroutine.create(f)
        c2 = coroutine.create(g)

        coroutine.resume(c1)
    }

--- config
--- stream_response
f 1
g 1
g 2
true
f 2
--- no_error_log
[error]



=== TEST 16: infinite recursive calls of coroutine.resume
--- stream_server_config
    content_by_lua_block {
        local print = njt.say

        local c1, c2

        function f()
            print("f 1")
            print(coroutine.resume(c2))
            print("f 2")
        end

        function g()
            print("g 1")
            print(coroutine.resume(c1))
            print("g 2")
        end

        c1 = coroutine.create(f)
        c2 = coroutine.create(g)

        coroutine.resume(c1)
    }

--- config
--- stap2 eval: $::StapScript
--- stream_response
f 1
g 1
falsecannot resume normal coroutine
g 2
true
f 2
--- no_error_log
[error]



=== TEST 17: resume running (entry) coroutines
--- stream_server_config
    content_by_lua_block {
        njt.say(coroutine.status(coroutine.running()))
        njt.say(coroutine.resume(coroutine.running()))
    }

--- config
--- stream_response
running
falsecannot resume running coroutine
--- no_error_log
[error]



=== TEST 18: resume running (user) coroutines
--- stream_server_config
    content_by_lua_block {
        local co
        function f()
            njt.say("f: ", coroutine.status(co))
            njt.say("f: ", coroutine.resume(co))
        end
        co = coroutine.create(f)
        njt.say("chunk: ", coroutine.status(co))
        njt.say("chunk: ", coroutine.resume(co))
    }

--- config
--- stream_response
chunk: suspended
f: running
f: falsecannot resume running coroutine
chunk: true
--- no_error_log
[error]



=== TEST 19: user coroutine end with errors, and the parent coroutine gets the right status
--- stream_server_config
    content_by_lua_block {
        local co
        function f()
            error("bad")
        end
        co = coroutine.create(f)
        njt.say("child: resume: ", coroutine.resume(co))
        njt.say("child: status: ", coroutine.status(co))
        njt.say("parent: status: ", coroutine.status(coroutine.running()))
    }

--- config
--- stream_response eval
qr/^child: resume: falsecontent_by_lua\(nginx\.conf:\d+\):4: bad
child: status: dead
parent: status: running
$/s
--- no_error_log
[error]



=== TEST 20: entry coroutine is yielded by hand and still gets the right status
--- stream_server_config
    content_by_lua_block {
        local co = coroutine.running()
        njt.say("status: ", coroutine.status(co))
        coroutine.yield(co)
        njt.say("status: ", coroutine.status(co))
    }

--- config
--- stream_response
status: running
status: running
--- no_error_log
[error]



=== TEST 21: github issue #208: coroutine as iterator doesn't work
--- stream_server_config
    content_by_lua_block {
        local say = njt.say
        local wrap, yield = coroutine.wrap, coroutine.yield

        local function it(it_state)
          for i = 1, it_state.i do
            yield(it_state.path, tostring(i))
          end
          return nil
        end

        local function it_factory(path)
          local it_state = { i = 10, path = path }
          return wrap(it), it_state
        end

        --[[
        for path, value in it_factory("test") do
          say(path, value)
        end
        ]]

        do
          local f, s, var = it_factory("test")
          while true do
            local path, value = f(s, var)
            var = path
            if var == nil then break end
            say(path, value)
          end
        end
    }

--- config
--- more_headers
Cookie: abc=32
--- stap2 eval: $::StapScript
--- stream_response
test1
test2
test3
test4
test5
test6
test7
test8
test9
test10
--- no_error_log
[error]



=== TEST 22: init_by_lua + our own coroutines in content_by_lua
--- stream_config
    init_by_lua_block { return }
--- stream_server_config
    resolver $TEST_NGINX_RESOLVER ipv6=off;
    content_by_lua_block {
        function worker(url)
            local sock = njt.socket.tcp()
            local ok, err = sock:connect(url, 80)
            coroutine.yield()
            if not ok then
                njt.say("failed to connect to: ", url, " error: ", err)
                return
            end
            coroutine.yield()
            njt.say("successfully connected to: ", url)
            sock:close()
        end

        local urls = {
            "agentzh.org",
        }

        local ccs = {}
        for i, url in ipairs(urls) do
            local cc = coroutine.create(function() worker(url) end)
            ccs[#ccs+1] = cc
        end

        while true do
            if #ccs == 0 then break end
            local cc = table.remove(ccs, 1)
            local ok = coroutine.resume(cc)
            if ok then
                ccs[#ccs+1] = cc
            end
        end

        njt.say("*** All Done ***")
    }

--- config
--- stream_response
successfully connected to: agentzh.org
*** All Done ***
--- no_error_log
[error]
--- timeout: 10



=== TEST 23: init_by_lua_file + our own coroutines in content_by_lua
--- stream_config
    init_by_lua_file html/init.lua;

--- stream_server_config
    resolver $TEST_NGINX_RESOLVER ipv6=off;
    content_by_lua_block {
        function worker(url)
            local sock = njt.socket.tcp()
            local ok, err = sock:connect(url, 80)
            coroutine.yield()
            if not ok then
                njt.say("failed to connect to: ", url, " error: ", err)
                return
            end
            coroutine.yield()
            njt.say("successfully connected to: ", url)
            sock:close()
        end

        local urls = {
            "agentzh.org"
        }

        local ccs = {}
        for i, url in ipairs(urls) do
            local cc = coroutine.create(function() worker(url) end)
            ccs[#ccs+1] = cc
        end

        while true do
            if #ccs == 0 then break end
            local cc = table.remove(ccs, 1)
            local ok = coroutine.resume(cc)
            if ok then
                ccs[#ccs+1] = cc
            end
        end

        njt.say("*** All Done ***")
    }

--- config
--- user_files
>>> init.lua
return

--- stream_response
successfully connected to: agentzh.org
*** All Done ***
--- no_error_log
[error]
--- timeout: 10



=== TEST 24: mixing coroutine.* API between init_by_lua and other contexts (github #304) - init_by_lua
--- stream_config
    init_by_lua_block {
        co_wrap = coroutine.wrap
        co_yield = coroutine.yield
    }

--- stream_server_config
    content_by_lua_block {
        function generator()
            return co_wrap(function()
                co_yield("data")
            end)
        end

        local co = generator()
        local data = co()
        njt.say(data)
    }

--- config

--- stap2 eval: $::StapScript
--- stream_response
data
--- no_error_log
[error]



=== TEST 25: mixing coroutine.* API between init_by_lua and other contexts (github #304) - init_by_lua_file
--- stream_config
    init_by_lua_file html/init.lua;

--- stream_server_config
    content_by_lua_block {
        function generator()
            return co_wrap(function()
                co_yield("data")
            end)
        end

        local co = generator()
        local data = co()
        njt.say(data)
    }

--- config

--- user_files
>>> init.lua
co_wrap = coroutine.wrap
co_yield = coroutine.yield

--- stap2 eval: $::StapScript
--- stream_response
data
--- no_error_log
[error]



=== TEST 26: coroutine context collicisions
--- stream_server_config
    content_by_lua_block {
        local cc, cr, cy = coroutine.create, coroutine.resume, coroutine.yield

        function f()
            return 3
        end

        for i = 1, 10 do
            collectgarbage()
            local c = cc(f)
            if coroutine.status(c) == "dead" then
                njt.say("found a dead coroutine")
                return
            end
            cr(c)
        end
        njt.say("ok")
    }

--- config
--- stap2 eval: $::StapScript
--- stream_response
ok
--- no_error_log
[error]



=== TEST 27: require "coroutine"
--- stream_server_config
    content_by_lua_block {
        local coroutine = require "coroutine"
        local cc, cr, cy = coroutine.create, coroutine.resume, coroutine.yield

        function f()
            local cnt = 0
            for i = 1, 20 do
                njt.say("Hello, ", cnt)
                njt.sleep(0.001)
                cy()
                cnt = cnt + 1
            end
        end

        local c = cc(f)
        for i=1,3 do
            cr(c)
            njt.say("***")
        end
    }

--- config
--- stap2 eval: $::StapScript
--- stream_response
Hello, 0
***
Hello, 1
***
Hello, 2
***
--- no_error_log
[error]



=== TEST 28: coroutine.wrap propagates errors to parent coroutine
--- stream_server_config
    content_by_lua_block {
        local co = coroutine.wrap(function()
            print("in wrapped coroutine")
            error("something went wrong")
        end)

        co()

        njt.say("ok")
    }
--- stream_response
--- error_log eval
[
    qr/\[notice\] .*? in wrapped coroutine/,
    qr/\[error\] .*? lua entry thread aborted: runtime error: content_by_lua\(nginx\.conf:\d+\):\d+: something went wrong/,
    "stack traceback:",
    "coroutine 0:",
    "coroutine 1:"
]



=== TEST 29: coroutine.wrap propagates nested errors to parent coroutine
Note: in this case, both the error message and the traceback are constructed
from co1's stack level.
--- stream_server_config
    content_by_lua_block {
        local co1 = coroutine.wrap(function()
            error("something went wrong in co1")
        end)

        local co2 = coroutine.wrap(function()
            co1()
        end)

        co2()
    }
--- stream_response
--- error_log eval
[
    qr/\[error\] .*? lua entry thread aborted: runtime error: content_by_lua\(nginx\.conf:\d+\):\d+: something went wrong in co1/,
    "stack traceback:",
    "coroutine 0:",
    "coroutine 1:",
    "coroutine 2:"
]



=== TEST 30: coroutine.wrap propagates nested errors with stack level to parent coroutine
Note: in this case, the error message is constructed at the entry thread stack
level, and the traceback is constructed from co1's stack level.
--- stream_server_config
    content_by_lua_block {
        local co1 = coroutine.wrap(function()
            error("something went wrong in co1", 2)
        end)

        local co2 = coroutine.wrap(function()
            co1()
        end)

        co2()
    }
--- stream_response
--- error_log eval
[
    qr/\[error\] .*? lua entry thread aborted: runtime error: something went wrong in co1/,
    "stack traceback:",
    "coroutine 0:",
    "coroutine 1:",
    "coroutine 2:"
]



=== TEST 31: coroutine.wrap runtime errors do not log errors
--- stream_server_config
    content_by_lua_block {
        local co = coroutine.wrap(function()
            print("in wrapped coroutine")
            error("something went wrong")
        end)

        co()
    }
--- stream_response
--- no_error_log eval
[
    qr/\[error\] .*? lua coroutine: runtime error:/,
    "[alert]",
    "[crit]",
    "[emerg]",
    "[warn]",
]



=== TEST 32: coroutine.wrap does not return status boolean on yield
--- stream_server_config
    content_by_lua_block {
        local co = coroutine.wrap(function()
            coroutine.yield("ok", "err")
        end)

        local ret1, ret2 = co()

        njt.say(ret1, ", ", ret2)
    }
--- stream_response
ok, err
--- no_error_log
[error]



=== TEST 33: coroutine.wrap does not return status boolean on done
--- stream_server_config
    content_by_lua_block {
        local co = coroutine.wrap(function() end)

        local ret1 = co()

        njt.say(ret1)
    }
--- stream_response
nil
--- no_error_log
[emerg]
[alert]
[crit]
[error]
[warn]



=== TEST 34: coroutine.wrap does not return status boolean on error
--- SKIP: not supported
--- stream_server_config
    content_by_lua_block {
        local co = coroutine.wrap(function()
            error("something went wrong")
        end)

        local ret1, ret2 = pcall(co)

        njt.say(ret1, ", ", ret2)
    }
--- response_body
false, something went wrong
--- no_error_log
[error]



=== TEST 35: coroutine.wrap creates different function refs
--- stream_server_config
    content_by_lua_block {
        local f = function() end
        local co = coroutine.wrap(f)
        local co2 = coroutine.wrap(f)

        njt.say("co == co2: ", co == co2)
    }
--- stream_response
co == co2: false
--- no_error_log
[emerg]
[alert]
[crit]
[error]
[warn]



=== TEST 36: coroutine.wrap supports yielding and resuming
--- stream_server_config
    content_by_lua_block {
        local function f()
            local cnt = 0
            for i = 1, 20 do
                njt.say("co yield: ", cnt)
                coroutine.yield()
                cnt = cnt + 1
            end
        end

        local f = coroutine.wrap(f)
        for i = 1, 3 do
            njt.say("co resume")
            f()
        end
    }
--- stream_response
co resume
co yield: 0
co resume
co yield: 1
co resume
co yield: 2
--- no_error_log
[error]



=== TEST 37: coroutine.wrap return values
--- stream_server_config
    content_by_lua_block {
        local function f()
            local cnt = 0
            for i = 1, 20 do
                coroutine.yield(cnt, cnt + 1)
                cnt = cnt + 1
            end
        end

        local f = coroutine.wrap(f)
        for i = 1, 3 do
            njt.say("co resume")
            local ret1, ret2 = f()
            njt.say("co yield: ", ret1, ", ", ret2)
        end
    }
--- stream_response
co resume
co yield: 0, 1
co resume
co yield: 1, 2
co resume
co yield: 2, 3
--- no_error_log
[error]



=== TEST 38: coroutine.wrap arguments
--- stream_server_config
    content_by_lua_block {
        local function f(step)
            local cnt = 0
            for i = 1, 20 do
                njt.say("co yield: ", cnt)
                coroutine.yield()
                cnt = cnt + step
            end
        end

        local f = coroutine.wrap(f)
        for i = 1, 3 do
            njt.say("co resume")
            f(i)
        end
    }
--- stream_response
co resume
co yield: 0
co resume
co yield: 1
co resume
co yield: 2
--- no_error_log
[error]



=== TEST 39: coroutine.wrap in init_by_lua propagates errors (orig coroutine.wrap)
--- stream_config
    init_by_lua_block {
        local co = coroutine.wrap(function()
            print("in wrapped coroutine")
            error("something went wrong")
        end)

        local err = co()

        njt.log(njt.CRIT, "err: ", err)
    }
--- stream_server_config
    content_by_lua_block {
        njt.say("ok")
    }
--- must_die
--- grep_error_log eval: qr/init_by_lua error: .*? something went wrong/
--- grep_error_log_out
init_by_lua error: init_by_lua:7: init_by_lua:4: something went wrong



=== TEST 40: coroutine.resume runtime errors do not log errors
--- stream_server_config
    content_by_lua_block {
        local function f()
            error("something went wrong")
        end

        local ret1, ret2 = coroutine.resume(coroutine.create(f))
        njt.say(ret1)
        njt.say(ret2)
    }
--- stream_response_like
false
content_by_lua\(nginx.conf:\d+\):\d+: something went wrong
--- no_error_log eval
[
    qr/\[error\] .*? lua coroutine: runtime error:",
    "stack traceback:",
]