local function delay<Args...>(sec, fn: (Args...) -> (), ...)
    local handle: Lou_Callback_Handle
    local accumulated = 0
    local args = table.pack(...)
    handle = lou:on_update(function(delta_seconds: number)
        accumulated += delta_seconds
        if accumulated >= sec then
            fn(table.unpack(args))
            handle:destroy()
        end
    end)
end
local e = lou:on_render(function()
    lou.renderer:set_draw_color(0xff, 0xff, 0xff)
    lou.renderer:fill_rect(Rect(0, 0, 100, 100))
end)
delay(5, function(...)
    print("unbinding")
    e:destroy()
    print("bye")
end)
print(typeof(e), e)
