# vim:set ft= ts=4 sw=4 et fdm=marker:
our $SkipReason;

BEGIN {
    if ($ENV{TEST_NGINX_CHECK_LEAK}) {
        $SkipReason = "unavailable for the hup tests";

    } else {
        $ENV{TEST_NGINX_USE_HUP} = 1;
        undef $ENV{TEST_NGINX_USE_STAP};
    }
}

use Test::Nginx::Socket::Lua 'no_plan';

#worker_connections(1014);
master_on();
workers(4);
#log_level('warn');

repeat_each(2);

#no_diff();
no_long_string();
run_tests();

__DATA__

=== TEST 1: get worker pids with multiple worker
--- config
    location /lua {
        content_by_lua_block {
            local pids, err = njt.worker.pids()
            if err ~= nil then
                return
            end
            local pid = njt.worker.pid()
            njt.say("worker pid: ", pid)
            local count = njt.worker.count()
            njt.say("worker count: ", count)
            njt.say("worker pids count: ", #pids)
            for i = 1, count do
                if pids[i] == pid then
                    njt.say("worker pid is correct.")
                    return
                end
            end
        }
    }
--- request
GET /lua
--- response_body_like
worker pid: \d+
worker count: 4
worker pids count: 4
worker pid is correct\.
--- no_error_log
[error]
