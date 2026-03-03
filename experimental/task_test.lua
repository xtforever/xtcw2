print("Lua Task Test Started")

function my_event_handler(ev)
    print(string.format("[Lua] Task %d Event %d: %.1f%% - %s", 
          ev.job_id, ev.type, ev.progress, ev.message))
    
    if ev.type == TASK_EVENT_PROGRESS and ev.progress == 30.0 then
        print("[Lua] Requesting PAUSE")
        task_control(ev.job_id, TASK_CMD_STOP)
        -- Resume after 2 seconds (using a Lua-side timer if we had one, but we'll use C side for now)
    end
end

task_set_handler("my_event_handler")

local id = task_spawn("dummy_task", "some data")
print("[Lua] Spawned job ID: " .. id)
